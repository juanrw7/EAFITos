#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  char *p;

  p = sbrk(4096);
  if(p == (char*)-1){
    printf("tsbrkpf: sbrk fallo\n");
    exit(1);
  }

  printf("sbrk reservo una pagina virtual en %p\n", p);
  printf("intentando escribir en la memoria reservada...\n");

  p[0] = 'A';

  printf("escritura completada: la pagina fue materializada por lazy allocation\n");
  printf("valor leido: %c\n", p[0]);

  exit(0);
}