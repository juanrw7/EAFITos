#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  char *p;

  printf("tdumpvm pid=%d\n", getpid());
  printf("primera llamada a dumpvm\n");
  dumpvm();

  p = sbrk(4096);
  if(p == (char*)-1){
    printf("tdumpvm: sbrk fallo\n");
    exit(1);
  }

  p[0] = 'A';
  p[4095] = 'Z';

  printf("segunda llamada a dumpvm\n");
  dumpvm();

  exit(0);
}