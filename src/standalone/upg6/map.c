#include "map.h"
#include <stdlib.h>
#include <stdio.h>


void map_init(struct map* m)
{
  list_init(&m->content);
  m->next_key=0;
}

key_t map_insert(struct map* m, value_t v)
 {
  struct association* tuple =  (struct association*)  malloc(sizeof(struct association));
 tuple->key = m->next_key;
 ++m->next_key;
 tuple->value = v;
 tuple->elem.next = &(m->content.tail);
 list_insert(tuple->elem.next,&tuple->elem);
 return tuple->key;
}

value_t map_find(struct map* m, key_t k)
{
  struct list_elem *e;
  for (e = list_begin (&m->content); e != list_end (&m->content);
       e = list_next (e))
       {
        struct association *f = list_entry (e, struct association, elem);
         if(f->key == k){
           return f->value;
         }
       }
  return NULL;
}
value_t map_remove(struct map* m, key_t k)
{
  struct list_elem *e;
  for (e = list_begin (&m->content); e != list_end (&m->content);
       e = list_next (e))
       {
        struct association *f = list_entry (e, struct association, elem);
         if(f->key == k){
           f->elem.prev->next = f->elem.next;
           f->elem.next->prev = f->elem.prev;
           f->elem.next = NULL;
           f->elem.prev = NULL;
           char *tmp = f->value;
           free(f);
           return tmp;
         }
       }
       return NULL;
}
void map_for_each(struct map* m,
void (*exec)(key_t k, value_t v, int aux),
int aux)
{
  struct list_elem *e;
  for (e = list_begin (&m->content); e != list_end (&m->content);
       e = list_next (e))
        {
          struct association *f = list_entry (e, struct association, elem);
          exec(f->key,f->value,aux);
        }
}

void map_remove_if(struct map* m,
bool (*cond)(key_t k, value_t v, int aux),
int aux)
{
  while (!list_empty (&(m->content)))
    {
      struct association *f = list_entry (list_pop_front(&(m->content)), struct association, elem);
           if(cond(f->key,f->value,aux))
          {
            free(f);
          }
      }
}
