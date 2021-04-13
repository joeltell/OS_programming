#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
//#include "../../lib/kernel/list.h"

#include "map.h"

#define UNUSED __attribute__((unused))

/* The code assumes that key_t is `int' and value_t is `char*' */

/* function passed as parameter to map_remove_if in order to free the
 * memory for all inseted values, and return true to remove them from
 * the map */
bool do_free(key_t k UNUSED, value_t v, int aux UNUSED)
{
  free(v);     /*! free memory */
  return true; /*  and remove from collection */
}

/* function to display all values in the map that are less than the
 * aux argument */
void print_less(key_t k UNUSED, value_t v, int aux)
{
  /* atoi converst from sequence of character to integer, it will fail
   * when the characters are letters, check the manpage to see how */
   //printf("\n atoi i preint_less %d %s \n",atoi(v),v);
  if ( atoi(v) < aux)
  {
    printf("%s ", v);
  }
}

int main()
{
  struct map container;
  char input_buffer[10];
  char* obj;
  int id;
  int i;
  struct association j;
  map_init(&container);

  if(list_empty(&(container.content)))
  {
    printf("tom lista \n");
  }
  printf("sizen på listan: %ld \n", list_size(&(container.content)));

  map_insert(&container, "hello");
  map_insert(&container, "penismyra");
  map_insert(&container, "tjosalabim");
  printf("sizen på listan efter en insättning: %ld \n", list_size(&(container.content)));
  printf("list size before remove: %ld \n", list_size (&container.content));

  // printf("Will now display all values less than N. Choose N: \n");
  // int p = 1;
  // map_for_each(&container, print_less, p);
  printf("\n \n ----------------------------------------------------------- \n \n");
  printf("Will now display all values less than N. Choose N: ");
  scanf("%d", &i);
  map_for_each(&container, print_less, i);
  printf("\n \n ----------------------------------------------------------- \n \n");
  map_remove(&container,1);
  printf("list size after remove: %ld \n", list_size (&container.content));
  //printf("value: %s",container.content);
//  list_pop_front(&(container.content));
  //printf("sizeof: %ld ",sizeof(struct association));
  printf("findens return 0: %s \n",map_find(&container,0));
  printf("findens return 1: %s \n",map_find(&container,1));
  printf("findens return 2: %s \n",map_find(&container,2));
  map_remove(&container,0);
  map_remove(&container,2);
  printf("atoi hello  %d:  \n", atoi("hello"));
}
