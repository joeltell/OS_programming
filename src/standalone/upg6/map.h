/* do not forget the guard against multiple includes */
#ifndef MAP_H
#define MAP_H
#include "../../lib/kernel/list.h"

typedef char* value_t;
typedef int key_t;

struct association
{
key_t   key;   /* nyckeln */
value_t value; /* värdet associerat med nyckeln */
/* list-element för att kunna sätta in i listan */
struct list_elem elem;
};

struct map
{
/* listan med alla lagrade associationer */
struct list content;
/* räknas upp varje gång en ny nyckel behövs */
int next_key;
};

void map_init(struct map* m);
key_t map_insert(struct map* m, value_t v);
value_t map_find(struct map* m, key_t k);
value_t map_remove(struct map* m, key_t k);
void map_for_each(struct map* m,
void (*exec)(key_t k, value_t v, int aux),
int aux);
void map_remove_if(struct map* m,
bool (*cond)(key_t k, value_t v, int aux),
int aux);

#endif
