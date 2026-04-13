#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int r = hello();
  printf("hello returned %d\n", r);
  exit(0);
}