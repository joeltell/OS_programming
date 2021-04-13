#include <stdio.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
  

  // create("file.1", 50000);
  // create("file.1", 50000);
  int i;
  for (i = 0; i < argc; i++)
    printf ("%s ", argv[i]);
  printf ("\n");

  return EXIT_SUCCESS;
}
