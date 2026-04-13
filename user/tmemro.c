#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  char *va = (char*)0x50000000;

  if(map_ro(va) < 0){
    printf("tmemro: map_ro fallo\n");
    exit(1);
  }

  printf("contenido lectura OK: %s\n", va);

  printf("ahora intentare escribir en memoria solo lectura...\n");
  va[0] = 'X';

  printf("ERROR: no debio llegar aqui\n");
  exit(1);
}