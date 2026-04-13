#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(void)
{
  int r, fd;
  char buf[32];
  char msg[] = "hola desde tuargs\n";

  // Direcciones que en la práctica deberían estar fuera del espacio útil
  // del proceso y no deben ser rescatadas por la prueba.
  char *bad1 = (char*)0x40000000;
  char *bad2 = (char*)0x40001000;

  printf("== prueba 1: hello ==\n");
  r = hello();
  printf("hello returned %d\n", r);

  printf("== prueba 2: write con puntero valido a archivo ==\n");
  fd = open("tuargs.txt", O_CREATE | O_RDWR);
  if(fd < 0){
    printf("open tuargs.txt fallo\n");
    exit(1);
  }
  r = write(fd, msg, sizeof(msg) - 1);
  printf("write valido retorno %d\n", r);
  close(fd);

  printf("== prueba 3: write con puntero invalido a archivo ==\n");
  fd = open("tuargs.txt", O_WRONLY);
  if(fd < 0){
    printf("open tuargs.txt fallo\n");
    exit(1);
  }
  r = write(fd, bad1, 5);
  printf("write invalido retorno %d\n", r);
  close(fd);

  printf("== prueba 4: open de archivo inexistente ==\n");
  r = open("no_existe.txt", O_RDONLY);
  printf("open inexistente retorno %d\n", r);

  printf("== prueba 5: read con buffer invalido desde archivo ==\n");
  fd = open("README.md", O_RDONLY);
  if(fd < 0){
    printf("open README.md fallo\n");
    exit(1);
  }
  r = read(fd, bad2, 5);
  printf("read buffer invalido retorno %d\n", r);
  close(fd);

  printf("== prueba 6: read con buffer valido desde archivo ==\n");
  fd = open("README.md", O_RDONLY);
  if(fd < 0){
    printf("open README.md fallo\n");
    exit(1);
  }
  r = read(fd, buf, sizeof(buf) - 1);
  if(r >= 0){
    buf[r] = 0;
    printf("read valido retorno %d, buf='%s'\n", r, buf);
  } else {
    printf("read valido retorno %d\n", r);
  }
  close(fd);

  exit(0);
}