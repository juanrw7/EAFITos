#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE 4096

int
main(void)
{
  char *p;
  int i;

  p = sbrk(3 * PGSIZE);
  if(p == (char*)-1){
    printf("tsbrklazy: sbrk fallo\n");
    exit(1);
  }

  printf("reserva virtual de 3 paginas desde %p\n", p);

  for(i = 0; i < 3; i++){
    printf("tocando pagina %d en %p\n", i, p + i * PGSIZE);
    p[i * PGSIZE] = 'A' + i;
    printf("pagina %d ahora contiene %c\n", i, p[i * PGSIZE]);
  }

  printf("acceso progresivo completado sin crash\n");
  exit(0);
}