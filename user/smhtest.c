#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int pid;
  int *shared;
  uint64 addr = 0x60000000;

  pid = fork();
  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }

  if(pid == 0){
    pause(20);
    shared = (int*)addr;
    printf("child: read %d\n", *shared);
    *shared = 42;
    printf("child: wrote 42\n");
    exit(0);
  } else {
    if(shmem(pid, (void*)addr) < 0){
      printf("shmem failed\n");
      kill(pid);
      wait(0);
      exit(1);
    }

    shared = (int*)addr;
    *shared = 17;
    printf("parent: wrote 17\n");

    wait(0);

    printf("parent: read %d\n", *shared);
    exit(0);
  }
}