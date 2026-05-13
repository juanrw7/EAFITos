#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;
  struct proc *p = myproc();

  argint(0, &n);
  argint(1, &t);
  (void)t;   // se mantiene por compatibilidad con tu ABI actual
  addr = p->sz;

  if(n < 0){
    if(growproc(n) < 0)
      return -1;
  } else {
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    p->sz += n;
  }

  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//modificado
uint64
sys_shmem(void)
{
  int pid;
  uint64 va;
  struct proc *p;
  struct proc *other;
  char *mem;
  uint64 pa;

  argint(0, &pid);
  argaddr(1, &va);

  p = myproc();
  other = findproc(pid);
  if(other == 0)
    return -1;

  mem = kalloc();
  if(mem == 0)
    return -1;

  memset(mem, 0, PGSIZE);
  pa = (uint64)mem;

  if(mappages(p->pagetable, va, PGSIZE, pa, PTE_R | PTE_W | PTE_U) != 0){
    kfree(mem);
    return -1;
  }

  if(mappages(other->pagetable, va, PGSIZE, pa, PTE_R | PTE_W | PTE_U) != 0){
    uvmunmap(p->pagetable, va, 1, 0);
    kfree(mem);
    return -1;
  }

  return va;
}

uint64
sys_hello(void)
{
  return 42;
}

uint64
sys_trace(void)
{
  int mask;
  argint(0, &mask);
  myproc()->trace_mask = mask;
  return 0;
}

uint64
sys_dumpvm(void)
{
  vmprint(myproc()->pagetable);
  return 0;
}

uint64
sys_map_ro(void)
{
  uint64 va;
  char *mem;
  char msg[] = "mensaje solo lectura";

  argaddr(0, &va);
  va = PGROUNDDOWN(va);

  if(ismapped(myproc()->pagetable, va))
    return -1;

  mem = kalloc();
  if(mem == 0)
    return -1;

  memset(mem, 0, PGSIZE);
  safestrcpy(mem, msg, sizeof(msg));

  if(mappages(myproc()->pagetable, va, PGSIZE, (uint64)mem, PTE_R | PTE_U) != 0){
    kfree(mem);
    return -1;
  }

  return 0;
}

uint64
sys_getpfaults(void)
{
  return myproc()->page_faults;
}

uint64
sys_resetpfaults(void)
{
  myproc()->page_faults = 0;
  return 0;
}

uint64
sys_mapzero(void)
{
  int size;
  struct proc *p = myproc();
  uint64 start = 0x70000000;

  argint(0, &size);

  if(size <= 0)
    return -1;

  size = PGROUNDUP(size);

  if(p->has_region)
    return -1;

  if(start + size < start)
    return -1;

  if(start + size >= TRAPFRAME)
    return -1;

  p->region.start = start;
  p->region.size = size;
  p->has_region = 1;

  return start;
}