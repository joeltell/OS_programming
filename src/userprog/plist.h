

/* Place functions to handle a running process here (process list).

   plist.h : Your function declarations and documentation.
   plist.c : Your implementation.

   The following is strongly recommended:

   - A function that given process inforamtion (up to you to create)
     inserts this in a list of running processes and return an integer
     that can be used to find the information later on.

   - A function that given an integer (obtained from above function)
     FIND the process information in th2e list. Should return some
     failure code if no process matching the integer is in the list.
     Or, optionally, several functions to access any information of a
     particular process that you currently need.

   - A function that given an integer REMOVE the process information
     from the list. Should only remove the information when no process
     or thread need it anymore, but must guarantee it is always
     removed EVENTUALLY.

   - A function that print the entire content of the list in a nice,
     clean, readable format.

 */
 /* do not forget the guard against multiple includes */
 #ifndef _PLIST_H_
 #define _PLIST_H_

 #include "../lib/kernel/list.h"
 #include "../threads/synch.h"


 typedef struct pdata* process_value_t;
 typedef int key_t;

struct pdata
{
  bool success; //gick det att skapa barnprocessen?
  int exit_status;
  int parent_pid;
  int pid;
  bool alive;
  bool parent_alive;
  char name[16];
  struct semaphore process_sema;
};

 struct process_association
 {
 key_t   key;   /* nyckeln */
 process_value_t value; /* värdet associerat med nyckeln */
 /* list-element för att kunna sätta in i listan */
 struct list_elem elem;
 };

 struct process_map
 {
 /* listan med alla lagrade process_associationer */
 struct list content;
 };

 void process_map_init(struct process_map* m);
 key_t process_map_insert(struct process_map* m, process_value_t v);
 process_value_t process_map_find(struct process_map* m, key_t k);
 process_value_t process_map_remove(struct process_map* m, key_t k);
 void process_map_for_each(struct process_map* m,
 void (*exec)(key_t k, process_value_t v, int aux),
 int aux);
 void process_map_remove_if(struct process_map* m,
 bool (*cond)(key_t k, process_value_t v, int aux),
 int aux);
size_t process_list_size(struct process_map* m);
process_value_t process_map_find_wait(struct process_map* m, key_t k, int parent);
 #endif
