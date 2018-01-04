#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lib.h"

int main(int argc, char *argv[]){
  unsigned char *val;

  val = malloc (1024);
  if ( val )
    printf("Calling malloc succeeded.\n");
  else {
    printf("malloc() returned NULL.\n");
    exit(1);
  }

  fill(val,1024,0);
  printf("Successfully used the space.\n");

  if ( check(val,1024,0) )
    printf("Some values didn't match in region %p.\n",val);

  printf("Allocated 1024 bytes.  Freeing val+512.\n");
  free(val+512);                /* free from the middle */
  printf("We survived the free call.\n");

  exit(0);
}
