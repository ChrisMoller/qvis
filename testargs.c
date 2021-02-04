#include <stdio.h>
#include <stdlib.h>

int
main (int ac, char *av[])
{
  fprintf (stderr, "ac = %d\n", ac);
  int i;
  for (i = 0; i < ac; i++)
    fprintf (stderr, "av[%d], \"%s\"\n", i, av[i]);
}
