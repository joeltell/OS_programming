#include <stddef.h>
#include "plist.h"
#include <stdlib.h>
#include <stdio.h>
#include "../threads/malloc.h"

struct lock plist_lock;


void process_map_init(struct process_map* m)
{
  list_init(&m->content);
  lock_init(&plist_lock);
}

key_t process_map_insert(struct process_map* m, process_value_t v)
 {
lock_acquire(&plist_lock);
  struct process_association* tuple =  (struct process_association*)  malloc(sizeof(struct process_association));
  if( tuple == NULL)
  {
    lock_release(&plist_lock);
    return -1;
  }

  tuple->value = v;
  tuple->key = v->pid;
 tuple->elem.next = &(m->content.tail);
 list_insert(tuple->elem.next,&tuple->elem);
  lock_release(&plist_lock);
 return tuple->key;

}

process_value_t process_map_find_wait(struct process_map* m, key_t k,int parent)
{
  lock_acquire(&plist_lock);
  struct list_elem *e;
  for (e = list_begin (&m->content); e != list_end (&m->content);
       e = list_next (e))
       {
        struct process_association *f = list_entry (e, struct process_association, elem);
         if(f->key == k){
           if(f->value->parent_pid == parent)
           {
             lock_release(&plist_lock);
             return f->value;
           }

         }
       }
       lock_release(&plist_lock);
  return NULL;
}

process_value_t process_map_find(struct process_map* m, key_t k)
{
  lock_acquire(&plist_lock);
  struct list_elem *e;
  for (e = list_begin (&m->content); e != list_end (&m->content);
       e = list_next (e))
       {
        struct process_association *f = list_entry (e, struct process_association, elem);
         if(f->key == k){
           lock_release(&plist_lock);
           return f->value;
         }
       }
       lock_release(&plist_lock);
  return NULL;
}
process_value_t process_map_remove(struct process_map* m, key_t k)
{

  struct list_elem *e;
  for (e = list_begin (&m->content); e != list_end (&m->content);
       e = list_next (e))
       {
        struct process_association *f = list_entry (e, struct process_association, elem);
         if(f->key == k){
           f->elem.prev->next = f->elem.next;
           f->elem.next->prev = f->elem.prev;
           f->elem.next = NULL;
           f->elem.prev = NULL;

           free(f->value);
                free(f);

           return NULL;
         }
       }

       return NULL;

}
void process_map_for_each(struct process_map* m,
void (*exec)(key_t k, process_value_t v, int aux),
int aux)
{
  struct list_elem *e;
  for (e = list_begin (&m->content); e != list_end (&m->content);
       e = list_next (e))
        {
          struct process_association *f = list_entry (e, struct process_association, elem);
            if (f!=NULL){
              exec(f->key,f->value,aux);
            }
        }
}

void process_map_remove_if(struct process_map* m,
bool (*cond)(key_t k, process_value_t v, int aux),
int aux)
{
  lock_acquire(&plist_lock);
  struct list_elem *e;
  for (e = list_begin (&m->content); e != list_end (&m->content);
  e = list_next (e))
       {
        struct process_association *f = list_entry (e, struct process_association, elem);
           if(cond(f->key,f->value,aux))//remove ONLY if child and parent is dead
          {
              e=list_remove(&f->elem);
              free(f->value);
              free(f);
              e=list_prev(e);
          }
      }
       lock_release(&plist_lock);
}
size_t process_list_size(struct process_map* m)
{
  return list_size(&m->content);
}
