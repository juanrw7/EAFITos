#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void
usage(void)
{
  printf("uso: tpf readlow | writelow | readhigh | writehigh\n");
}

static int
read_byte(volatile char *p)
{
  return *p;
}

int
main(int argc, char *argv[])
{
  volatile char *p;

  if(argc != 2){
    usage();
    exit(1);
  }

  if(strcmp(argv[1], "readlow") == 0){
    p = (volatile char*)0x4000;
    printf("intentando leer desde 0x4000...\n");
    printf("valor=%d\n", read_byte(p));
  } else if(strcmp(argv[1], "writelow") == 0){
    p = (volatile char*)0x4000;
    printf("intentando escribir en 0x4000...\n");
    *p = 'A';
  } else if(strcmp(argv[1], "readhigh") == 0){
    p = (volatile char*)0x40000000;
    printf("intentando leer desde 0x40000000...\n");
    printf("valor=%d\n", read_byte(p));
  } else if(strcmp(argv[1], "writehigh") == 0){
    p = (volatile char*)0x40000000;
    printf("intentando escribir en 0x40000000...\n");
    *p = 'B';
  } else {
    usage();
    exit(1);
  }

  printf("ERROR: no debio llegar aqui\n");
  exit(0);
}