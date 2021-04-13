#include <debug.h>
#include <stdio.h>
#include <string.h>

#include "userprog/gdt.h"      /* SEL_* constants */
#include "userprog/process.h"
#include "userprog/load.h"
#include "userprog/pagedir.h"  /* pagedir_activate etc. */
#include "userprog/tss.h"      /* tss_update */
#include "filesys/file.h"
#include "threads/flags.h"     /* FLAG_* constants */
#include "threads/thread.h"
#include "threads/vaddr.h"     /* PHYS_BASE */
#include "threads/interrupt.h" /* if_ */
#include "threads/init.h"      /* power_off() */

/* Headers not yet used that you may need for various reasons. */
#include "threads/synch.h"
#include "threads/malloc.h"
#include "lib/kernel/list.h"

#include "userprog/flist.h"
#include "userprog/plist.h"
#include "devices/timer.h"
//global processlista
struct process_map process_list;


/* HACK defines code you must remove and implement in a proper way */
#define HACK




/* This function is called at boot time (threads/init.c) to initialize
 * the process subsystem. */
void process_init(void)
{
  //initialize global process list
  process_map_init(&process_list);


}

/* This function is currently never called. As thread_exit does not
 * have an exit status parameter, this could be used to handle that
 * instead. Note however that all cleanup after a process must be done
 * in process_cleanup, and that process_cleanup are already called
 * from thread_exit - do not call cleanup twice! */
 void erase_plist()
 {
   int n=2;
   process_map_remove_if(&process_list,erase,n);
  // debug("# list size: %d\n",process_list_size(&process_list));

 }
void process_exit(int status )
{

  struct pdata* ptmp;
  ptmp = process_map_find( &process_list,thread_current()->tid );

  if(ptmp != NULL)
  {
      ptmp-> exit_status = status;

  }

}

bool do_free(key_t k, process_value_t v, int aux UNUSED)
{
  bool ret=false;
  if(process_map_find(&process_list,k) != NULL ){
      if(v->parent_alive == false && v->alive == false)
      {
      ret= true;
      //printf(" \nkommer vi hit \n");
      }
  }
  return ret;
}


/* Print a list of all running processes. The list shall include all
 * relevant debug information in a clean, readable format. */
 void print_plist(key_t key, process_value_t value,int aux UNUSED)
 {
   int pad=0;
   pad=(strlen(value->name)+8)-4;
   char* first = "name";
   char* sec = "pid";
    debug("%-s %*s parent_pid  exit  parent_alive  alive\n",first,pad,sec);
    debug("%-s %8d %6d %8d %8d %9d\n",
    value->name,key, value->parent_pid, value->exit_status, (int)value->parent_alive, (int)value->alive);
    debug("\n");
 }
void process_print_list()
{
  int n=2;
  process_map_for_each(&process_list, print_plist,n);

}

#define STACK_DEBUG(...) printf(__VA_ARGS__)



struct parameters_to_start_process
{
  char* command_line;
  int control;
  struct semaphore sema;
  int parent_pid;
};

static void
start_process(struct parameters_to_start_process* parameters) NO_RETURN;


/* Starts a new proccess by creating a new thread to run it. The
   process is loaded from the file specified in the COMMAND_LINE and
   started with the arguments on the COMMAND_LINE. The new thread may
   be scheduled (and may even exit) before process_execute() returns.
   Returns the new process's thread id, or TID_ERROR if the thread
   cannot be created. */

int
process_execute (const char *command_line)
{
  char debug_name[64];
  int command_line_size = strlen(command_line) + 1;
  tid_t thread_id = -1;
  int  process_id = -1;

  /* LOCAL variable will cease existence when function return! */
  struct parameters_to_start_process arguments;



  debug("%s#%d: process_execute(\"%s\") ENTERED\n",
        thread_current()->name,
        thread_current()->tid,
        command_line);

  /* COPY command line out of parent process memory */
  arguments.command_line = malloc(command_line_size);
  strlcpy(arguments.command_line, command_line, command_line_size);


  strlcpy_first_word (debug_name, command_line, 64);

arguments.parent_pid = thread_current()->tid;
  /* SCHEDULES function `start_process' to run (LATER) */
  /* initialize semaphore for the start_process thread*/
  sema_init(&arguments.sema,0);
  thread_id = thread_create (debug_name, PRI_DEFAULT,
                             (thread_func*)start_process, &arguments);


 process_id = thread_id;


if(thread_id != -1){
  sema_down(&arguments.sema);
  if(arguments.control == -1)
  {
    process_id = -1;
  }

}


  /* AVOID bad stuff by turning off. YOU will fix this! */

  // power_off();


  /* WHICH thread may still be using this right now? */
  free(arguments.command_line);

  debug("%s#%d: process_execute(\"%s\") RETURNS %d\n",
        thread_current()->name,
        thread_current()->tid,
        command_line, process_id);

  /* MUST be -1 if `load' in `start_process' return false */
  return process_id;
}

/* A thread function that loads a user process and starts it
   running. */
static void
start_process (struct parameters_to_start_process* parameters)
{
  /* The last argument passed to thread_create is received here... */
  struct intr_frame if_;
  bool success;
  parameters->control = -1;

  char file_name[64];
  strlcpy_first_word (file_name, parameters->command_line, 64);

  debug("%s#%d: start_process(\"%s\") ENTERED\n",
        thread_current()->name,
        thread_current()->tid,
        parameters->command_line);

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;

  success = load (file_name, &if_.eip, &if_.esp);

  debug("%s#%d: start_process(...): load returned %d\n",
        thread_current()->name,
        thread_current()->tid,
        success);

  if (success)
  {
    /* We managed to load the new program to a process, and have
       allocated memory for a process stack. The stack top is in
       if_.esp, now we must prepare and place the arguments to main on
       the stack. */

    /* A temporary solution is to modify the stack pointer to
       "pretend" the arguments are present on the stack. A normal
       C-function expects the stack to contain, in order, the return
       address, the first argument, the second argument etc. */

    //HACK if_.esp -= 12; /* Unacceptable solution. */
    if_.esp = setup_main_stack(parameters->command_line, if_.esp);
  //  parameters->control =   thread_current()->tid;
    /* The stack and stack pointer should be setup correct just before
       the process start, so this is the place to dump stack content
       for debug purposes. Disable the dump when it works. */


    //dump_stack ( PHYS_BASE + 15, PHYS_BASE - if_.esp + 16 );
    struct pdata* process_data = (struct pdata*) malloc(sizeof(struct pdata));
    if(process_data != NULL)
    {
      process_data->parent_pid = parameters->parent_pid;
      process_data->exit_status = -100;
    if(parameters->parent_pid == 1)
    { //kernel tråd
      process_data->parent_alive = true;
    }
    else
    {
      process_data->parent_alive = true;
    }

    process_data->alive = true;
    process_data->pid = thread_current()->tid;

    sema_init(&process_data->process_sema,0);
    strlcpy ( process_data->name,thread_current()->name, sizeof(thread_current()->name));
    parameters->control = 1;
    process_data->success = success;
    process_map_insert(&process_list,process_data);
  }
  //printf("\nstart_process alive: %d\n",process_data->alive);
  }

  debug("%s#%d: start_process(\"%s\") DONE\n",
        thread_current()->name,
        thread_current()->tid,
        parameters->command_line);


  /* If load fail, quit. Load may fail for several reasons.
     Some simple examples:
     - File doeas not exist
     - File do not contain a valid program
     - Not enough memory
  */


  sema_up(&parameters->sema);
  if ( ! success )
  {
    thread_exit ();
  }

  /* Start the user process by simulating a return from an interrupt,
     implemented by intr_exit (in threads/intr-stubs.S). Because
     intr_exit takes all of its arguments on the stack in the form of
     a `struct intr_frame', we just point the stack pointer (%esp) to
     our stack frame and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();

}

/* Wait for process `child_id' to die and then return its exit
   status. If it was terminated by the kernel (i.e. killed due to an
   exception), return -1. If `child_id' is invalid or if it was not a
   child of the calling process, or if process_wait() has already been
   successfully called for the given `child_id', return -1
   immediately, without waiting.

   This function will be implemented last, after a communication
   mechanism between parent and child is established. */
// bool
// check_parent(process_value_t child)
// {
//   return(child->pid == thread_current()->tid);
// }
int
process_wait (int child_id)
{
  /* Yes! You need to do something good here ! */

  /* initial status set to -1 */
  int status = -1;
  struct thread *cur = thread_current ();

  //find child data from child_id
  //still in parent thread for the child
  debug("%s#%d: process_wait(%d) ENTERED\n",
        cur->name, cur->tid, child_id);

  struct pdata *child_cur = process_map_find_wait(&process_list,child_id,cur->tid);

    if(child_cur != NULL && child_cur->alive)
    {
      /* if currently thread number equals to the child's
         parent id, wait for the child to die */
      if(cur->tid == child_cur->parent_pid)
      {
        sema_down(&child_cur->process_sema);
        status = child_cur->exit_status;
      }
      child_cur->alive = false;
    }
  /*status should now be an actual exit status,
    or -1 if the child does not exist or if the child already dead*/

  debug("%s#%d: process_wait(%d) RETURNS %d\n",
        cur->name, cur->tid, child_id, status);

  return status;
}

/* Free the current process's resources. This function is called
   automatically from thread_exit() to make sure cleanup of any
   process resources is always done. That is correct behaviour. But
   know that thread_exit() is called at many places inside the kernel,
   mostly in case of some unrecoverable error in a thread.

   In such case it may happen that some data is not yet available, or
   initialized. You must make sure that nay data needed IS available
   or initialized to something sane, or else that any such situation
   is detected.
*/
void check_child(key_t k UNUSED, process_value_t v, int aux)
{

  if(v->parent_pid == aux)
  {
      v->parent_alive = false;
    }

}

bool erase(key_t k UNUSED, process_value_t v, int aux UNUSED)
{
  if (v->alive == false && v->parent_alive == false){
    return true;//ta bort ur listan både p_alive och parent_alive döda
  }
  else if(process_list_size(&process_list) == 1 && v->parent_alive == true && v->alive == false)
  {
    return true;
  }

return false;
}

bool flist_free(key_t k UNUSED, value_t v, int aux UNUSED)
{
  file_close(v);
  return true; /*  and remove from collection */
}
void
process_cleanup (void)
{

  struct thread  *cur = thread_current ();
  uint32_t       *pd  = cur->pagedir;
  int status =  -1;//-> exit_status;
  struct pdata* p = process_map_find(&process_list,thread_current()->tid);
  debug("%s#%d: process_cleanup() ENTERED\n", cur->name, cur->tid);


  if(p  == NULL){
  //  printf("\nthis is NULL?\n");
  }
  else {
      status = p->exit_status;
      printf("%s: exit(%d)\n", thread_name(), status);
      /*exit status set, thus semaup*/

      /* update alive and parent_alive values*/
      process_map_for_each(&process_list,check_child,p->pid);

      if(p != NULL)
      {
        /* if parent and currently dead erase */
        sema_up(&p->process_sema);
        erase_plist();

      }

  }



  /* Later tests DEPEND on this output to work correct. You will have
   * to find the actual exit status in your process list. It is
   * important to do this printf BEFORE you tell the parent process
   * that you exit.  (Since the parent may be the main() function,
   * that may sometimes poweroff as soon as process_wait() returns,
   * possibly before the printf is completed.)
   */

   map_remove_if(&thread_current()->file_list, flist_free, 0);


  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  if (pd != NULL)
    {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }
  debug("%s#%d: process_cleanup() DONE with status %d\n",
        cur->name, cur->tid, status);

}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}

void* setup_main_stack(const char* command_line, void* stack_top)
{
  /* Variable "esp" stores an address, and at the memory loaction
   * pointed out by that address a "struct main_args" is found.
   * That is: "esp" is a pointer to "struct main_args" */
  struct main_args* esp;
  int argc;
  int total_size;
  int line_size;


  /* "cmd_line_on_stack" and "ptr_save" are variables that each store
   * one address, and at that address (the first) char (of a possible
   * sequence) can be found. */
  char* cmd_line_on_stack;
  char* ptr_save;
  int i = 0;



  /* calculate the bytes needed to store the command_line */
  line_size = strlen(command_line)+1; //+1 för att strlen inte tar med avsultande tecken men man behöver ändå ha utrymme för det
  //STACK_DEBUG("# line_size = %d\n", line_size);
  /* round up to make it even divisible by 4 */
  line_size = calc_line(line_size);
  //STACK_DEBUG("# line_size (aligned) = %d\n", line_size);

  /* calculate how many words the command_line contain */
  argc = count_args(command_line, " ");
  //STACK_DEBUG("# argc = %d\n", argc);

  /* calculate the size needed on our simulated stack */
  total_size = calc_total(line_size,argc);
  //STACK_DEBUG("# total_size = %d\n", total_size);

  //const int S = sizeof(void*);
  /* calculate where the final stack top will be located */
  esp = stack_top-(total_size);


  /* setup return address and argument count */
   esp->ret = NULL;//(void*)esp+4;
   esp->argc = argc;
  // /* calculate where in the memory the argv array starts */
   esp->argv = (char**)(esp+1);


  // /* calculate where in the memory the words is stored */
  cmd_line_on_stack = stack_top - line_size;


  /* copy the command_line to where it should be in the stack */

  strlcpy(cmd_line_on_stack, command_line, line_size);

  /* build argv array and insert null-characters after each word */

   char* token;
   for (token = strtok_r (cmd_line_on_stack, " ", &ptr_save); token != NULL; token = strtok_r (NULL, " ", &ptr_save))
   {
     esp->argv[i] = token+'\0';
     ++i;
   }

  return esp; /* the new stack top */
}

int calc_total(int line_size, int argc)
{

  return (line_size + 4*argc + 4*4);
}

int calc_line(size_t size)
{
  int tmp = size;
  int rest = tmp % 4;
  if(rest == 0)
  {
    return tmp;
  }
  else
  {
  return tmp+(4-rest);
  }

}

int count_args(const char* buf, const char* delimeters)
{
  int i = 0;
  bool prev_was_delim;
  bool cur_is_delim = true;
  int argc = 0;

  while (buf[i] != '\0')
  {
    prev_was_delim = cur_is_delim;
    cur_is_delim = exists_in(buf[i], delimeters);
    argc += (prev_was_delim && !cur_is_delim);
    ++i;
  }
  return argc;
}

bool exists_in(char c, const char* d)
{
  int i = 0;
  while (d[i] != '\0' && d[i] != c)
    ++i;
  return (d[i] == c);
}
