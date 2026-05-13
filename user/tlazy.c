#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE 4096
#define NPAGES 8

static void
touch_sequential(char *base)
{
  int i;

  // Recorre secuencialmente solo 2 páginas completas.
  // Debe provocar 2 faults.
  for(i = 0; i < 2 * PGSIZE; i++){
    base[i] = 'S';
  }
}

static void
touch_sparse(char *base)
{
  int order[NPAGES] = {7, 1, 5, 0, 6, 2, 4, 3};
  int i;

  // Toca una sola vez 8 páginas distintas, en orden disperso.
  // Debe provocar 8 faults.
  for(i = 0; i < NPAGES; i++){
    int p = order[i];
    base[p * PGSIZE] = 'a' + p;
  }
}

int
main(void)
{
  char *p1, *p2;
  int before, after;

  p1 = sbrk(NPAGES * PGSIZE);
  if(p1 == (char*)-1){
    printf("tlazy: primer sbrk fallo\n");
    exit(1);
  }

  // Ignorar faults previos del arranque o printf.
  resetpfaults();
  before = getpfaults();
  touch_sequential(p1);
  after = getpfaults();
  printf("faults secuencial: %d\n", after - before);

  p2 = sbrk(NPAGES * PGSIZE);
  if(p2 == (char*)-1){
    printf("tlazy: segundo sbrk fallo\n");
    exit(1);
  }

  resetpfaults();
  before = getpfaults();
  touch_sparse(p2);
  after = getpfaults();
  printf("faults disperso: %d\n", after - before);

  exit(0);
}