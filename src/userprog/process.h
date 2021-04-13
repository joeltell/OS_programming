#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "userprog/plist.h"

void process_init (void);
void process_print_list (void);
void process_exit (int status);
tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_cleanup (void);
void process_activate (void);
//main stack
void* setup_main_stack(const char* command_line, void* stack_top);
int calc_total(int line_size, int argc);
int calc_line(size_t size);
int count_args(const char* buf, const char* delimeters);
bool exists_in(char c, const char* d);
bool do_free(key_t k UNUSED, process_value_t v, int aux UNUSED);
void print_plist(key_t key, process_value_t value,int aux);
bool flist_free(key_t k UNUSED, value_t v, int aux UNUSED);
void check_child(key_t k UNUSED, process_value_t v, int aux);
bool erase(key_t k UNUSED, process_value_t v, int aux UNUSED);
void erase_plist(void);
//bool check_parent(process_value_t child);

struct main_args
{
  /* Hint: When try to interpret C-declarations, read from right to
   * left! It is often easier to get the correct interpretation,
   * altough it does not always work. */

  /* Variable "ret" that stores address (*ret) to a function taking no
   * parameters (void) and returning nothing. */
  void (*ret)(void);

  /* Just a normal integer. */
  int argc;

  /* Variable "argv" that stores address to an address storing char.
   * That is: argv is a pointer to char*
   */
  char** argv;
};

/* This is unacceptable solutions. */
#define INFINITE_WAIT() for ( ; ; ) thread_yield()
#define BUSY_WAIT(n)       \
    do {                   \
      int i = n;           \
      while ( i --> 0 )    \
        thread_yield();    \
    } while ( 0 )

#endif /* userprog/process.h */
