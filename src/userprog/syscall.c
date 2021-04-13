#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "filesys/directory.h"
#include "flist.h"
#include "devices/timer.h"



static void syscall_handler (struct intr_frame *);
//int sys_read_func(int fd, void* buf,int size);
int sys_read(int fd, char *buf, int len);
int sys_open_func(const char* file);
void sys_close_func(int fd);
int sys_create_func(const char* file,int size);
int sys_write_func(int fd,void* buf,int size);
bool sys_remove_func(const char *file);
unsigned sys_tell_func(int fd);
void sys_seek_func(int fd, unsigned position);
int sys_filesize_func(int fd);
void sys_exit_func(int status);
bool verify_fix_length(void* start, int length);
bool verify_variable_length(char* start);
void sys_verify_sc(int32_t *esp);
bool valid_ptr(void *ptr);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}



/* This array defined the number of arguments each syscall expects.
   For example, if you want to find out the number of arguments for
   the read system call you shall write:

   int sys_read_arg_count = argc[ SYS_READ ];

   All system calls have a name such as SYS_READ defined as an enum
   type, see `lib/syscall-nr.h'. Use them instead of numbers.
 */
const int argc[] = {
  /* basic calls */
  0, 1, 1, 1, 2, 1, 1, 1, 3, 3, 2, 1, 1,
  /* not implemented */
  2, 1,    1, 1, 2, 1, 1,
  /* extended */
  0
};
bool valid_ptr(void *ptr)
{
	return(ptr != NULL && ptr < PHYS_BASE); /* Returns true if ptr is in user-space */
}

void sys_verify_sc(int32_t *esp)
{
	if(!verify_variable_length((char*)esp))
		sys_exit_func(-1);
	else if(esp[0] >= SYS_NUMBER_OF_CALLS || esp[0] <= 0)
		sys_exit_func(-1);

	for(int i = 0; i < argc[esp[0]]; ++i)
	{
			if(!verify_variable_length((char*)(esp + (i + 1))))
				sys_exit_func(-1);
	}
}


bool verify_variable_length(char* start)
{
	if(!valid_ptr(start))
  {
		sys_exit_func(-1);
  }

	char *cur = start;
	while(pagedir_get_page(thread_current()->pagedir, cur) != NULL)
	{
		int addr_to_read = PGSIZE - ((int)cur % PGSIZE);
		for(int i = 0; i < addr_to_read; ++i)
		{
			if(*(cur + i) == '\0')
				return true;
		}
		cur = (cur + addr_to_read); /* GOTO next page */
	}
	return false;
}

bool verify_fix_length(void* start, int length)
{
  if(!valid_ptr(start))
  {
    sys_exit_func(-1);
  }
  bool verify = true;

  for (void* in_page = pg_round_down(start); (void*)(unsigned long)in_page<(length+(void*)(unsigned long)start) ;
   in_page = (void*)(unsigned long)in_page + PGSIZE)
  {
    if (pagedir_get_page(thread_current()->pagedir,in_page) == NULL){
      return false;
    }
  }
  return verify;
}

void sys_exit_func(int status)
{
  process_exit (status);
  thread_exit();
}
int sys_filesize_func(int fd)
{
  if(map_find(&thread_current()->file_list,fd) != NULL)
  {
    return file_length(map_find(&thread_current()->file_list,fd));
  }
  else
  {
    return -1;
  }

}

void sys_seek_func(int fd, unsigned position)
{

  if(sys_filesize_func(fd) < (int)position ){
    //if user tries to seek past file
    //return last safe position in file
    file_seek(map_find(&thread_current()->file_list,fd),sys_filesize_func(fd));
  }
  else{
    file_seek(map_find(&thread_current()->file_list,fd),position);
  }
  return;
}

unsigned sys_tell_func(int fd)
{
  if(map_find(&thread_current()->file_list,fd) != NULL)
  {
    return file_tell(map_find(&thread_current()->file_list,fd));
  }
  else
  {
    return -1;
  }

}

bool sys_remove_func(const char *file)
{
  if(!verify_variable_length((char*)file))
  {
    sys_exit_func(-1);
  }
  return filesys_remove(file);
}

int sys_write_func(int fd,void* buf,int size)
{

  if(buf==NULL || (void*)(unsigned long)buf > (void*)(unsigned long)PHYS_BASE)
  {
    sys_exit_func(-1);
  }
  if(!verify_fix_length( buf, size))
  {
    sys_exit_func(-1);
  }
  if(fd == 1){
    putbuf((char*)buf,size);
    return size;//sizeof(&buf);
  }
  else if(fd == 0){
    return -1;
  }
  else
  {
    struct file* f= map_find(&thread_current()->file_list,fd);
    if( f == NULL)
    {
      return -1;
    }
    else
    {
        return file_write (f, buf, size);
      }
    }

return -1;
}

int sys_create_func(const char* file,int size)
{
if(!verify_variable_length((char*)file))
{
  sys_exit_func(-1);
}
return filesys_create(file,size);
  if(filesys_create(file,size))
  {
    return size;
  }
  else
  {
  return -1;
  }
}

void sys_close_func(int fd)
{

  filesys_close(map_find(&thread_current()->file_list,fd));

  map_remove(&thread_current()->file_list,fd);
}

int sys_open_func(const char* file)
{
  if(file==NULL || (void*)(unsigned long)file > (void*)(unsigned long)PHYS_BASE)
  {
    sys_exit_func(-1);
  }
  if (!verify_variable_length((char*)file))
  {
    sys_exit_func(-1);
  }
  struct file* f =filesys_open(file);
  if(f == NULL)
  {
    return -1;
  }
  else
  {
    int ret = map_insert(&thread_current()->file_list,f);
    if( ret == -1)
    {
    filesys_close(f);
    }
    return ret;

  }
}



int sys_read(int fd, char *buf, int len)
{
	int n_char = 0;
  if ((void*)(int)buf > (void*)(int)PHYS_BASE){
    sys_exit_func(-1);
  }
	if(fd == STDIN_FILENO)
	{
		while(n_char != len)
		{
			uint8_t tmp = input_getc();

			if(tmp == '\r')
				tmp = '\n';
			else if(tmp < 32) 	/* To handle special buttons when asking for input. */
				continue;
			else if(tmp == 127) /* Handle backspace for deletion of characters in buffer (Note: only in terminal!) */
			{
				if(n_char == 0)
					continue;

				buf[--n_char] = 0;
				putbuf("\b \b", 3);
				continue;
			}

			buf[n_char] = tmp;
			putbuf(&buf[n_char++], 1);
		}
		return len;
	}
	else if(fd > 1)
	{
		struct file *fp = map_find(&thread_current()->file_list, fd);
		if(fp == NULL) /* Is the file open? */
			return -1;

		if(!verify_fix_length(buf, len))
			sys_exit_func(-1);

		return file_read(fp, buf, len);
	}
	else
		return -1;
}

static void
syscall_handler (struct intr_frame *f)
{

  int32_t* esp = (int32_t*)f->esp;
  /*if esp points to last allowed addres when incremented with four
    address becomes the address it would be after a sys_call*/
if (esp == NULL || (void*)(int)esp+4 >= (void*)(int)PHYS_BASE)
{
  sys_exit_func(-1);
}


if(pagedir_get_page(thread_current()->pagedir, esp) == NULL)
{
  sys_exit_func(-1);
}


if(!verify_fix_length((char*)esp, (sizeof(uint32_t))))
{
  sys_exit_func(-1);
}

sys_verify_sc(esp);

if(!verify_fix_length((void*)esp, (sizeof(uint32_t)) * (argc[esp[0]] + 1)))
{
 sys_exit_func(-1);
}

if(!valid_ptr(esp))
{
  sys_exit_func(-1);
}





  switch ( *esp /* retrive syscall number */ )
  {
    case SYS_HALT:
    power_off();
    break;

    case SYS_EXIT:
    if(!verify_variable_length((char*)esp))
    {
          sys_exit_func(-1);
    }

    sys_exit_func(esp[1]);
    break;

    case SYS_CREATE:
    f->eax = sys_create_func((char*)esp[1],esp[2]);
    break;

    case SYS_OPEN:
    f->eax = sys_open_func((char*)esp[1]);
    break;

    case SYS_CLOSE:
    sys_close_func(esp[1]);
    f->eax=esp[1];
    break;

    case SYS_READ:
    f->eax = sys_read(esp[1],(char*)esp[2],esp[3]);
    break;

    case SYS_WRITE:

    f->eax = sys_write_func(esp[1],(void*)esp[2],esp[3]);
    break;

    case SYS_REMOVE:
    f->eax = sys_remove_func((char*)esp[1]);
    break;

    case SYS_TELL:
    f->eax = sys_tell_func(esp[1]);
    break;

    case SYS_SEEK:
    sys_seek_func(esp[1],esp[2]);
    f->eax = esp[1];
    break;

    case SYS_FILESIZE:
    f->eax = sys_filesize_func(esp[1]);
    break;

    case SYS_EXEC:
    if(!verify_variable_length((char*)esp[1]))
    {
      sys_exit_func(-1);
    }
    f->eax = process_execute((char*)esp[1]);
    break;

    case SYS_SLEEP:
    timer_sleep((int)esp[1]);
    break;

    case SYS_PLIST:
    process_print_list();
    break;

    case SYS_WAIT:
    f->eax = process_wait(esp[1]);
    break;


    default:
    {
      printf ("Executed an unknown system call!\n");

      printf ("Stack top + 0: %d\n", esp[0]);
      printf ("Stack top + 1: %d\n", esp[1]);

      thread_exit ();
    }
  }
}
