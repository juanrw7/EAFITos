#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE 4096

int
main(void)
{
  uint64 addr;
  char *p;

  addr = mapzero(3 * PGSIZE);
  if(addr == (uint64)-1){
    printf("tmmap_sim: mapzero fallo\n");
    exit(1);
  }

  p = (char*)addr;

  printf("mapzero devolvio %p\n", p);

  printf("leyendo pagina 0...\n");
  printf("valor p[0] = %c\n", p[0]);

  printf("leyendo pagina 1...\n");
  printf("valor p[PGSIZE] = %c\n", p[PGSIZE]);

  printf("leyendo pagina 2...\n");
  printf("valor p[2*PGSIZE] = %c\n", p[2 * PGSIZE]);

  printf("escribiendo en pagina 1...\n");
  p[PGSIZE] = 'Z';
  printf("nuevo valor p[PGSIZE] = %c\n", p[PGSIZE]);

  exit(0);
}