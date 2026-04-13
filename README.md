# Reto 2 — Shell **EAFITos** Gestión de memoria y syscalls (xv6-riscv)

- **Autor:** Juan Manuel Ramirez
- **Modalidad:** Individual  
- **Entorno objetivo:** `xv6-riscv` (MIT, branch `riscv`)  

### Objetivo

Agregar a xv6 nuevas syscalls y programas de prueba para estudiar:

- camino completo de una syscall
- trazado selectivo
- tablas de páginas
- permisos de memoria
- fallos controlados
- robustez frente a punteros inválidos

### Syscalls implementadas

- `hello()`
- `trace(int mask)`
- `dumpvm()`
- `map_ro(void *va)`
- `shmem(int pid, void *addr)` *(taller previo conservado en el repositorio)*

### Archivos añadidos / modificados para Proyecto 2

#### Kernel
- **Modificado:** `kernel/syscall.h`
- **Modificado:** `kernel/syscall.c`
- **Modificado:** `kernel/sysproc.c`
- **Modificado:** `kernel/trap.c`
- **Modificado:** `kernel/vm.c`
- **Modificado:** `kernel/proc.h`
- **Modificado:** `kernel/proc.c`
- **Modificado:** `kernel/defs.h`

#### User
- **Modificado:** `user/user.h`
- **Modificado:** `user/usys.pl`

#### Nuevos programas de prueba
- **Añadido:** `user/thello.c`
- **Añadido:** `user/ttrace.c`
- **Añadido:** `user/tdumpvm.c`
- **Añadido:** `user/tmemro.c`
- **Añadido:** `user/tuargs.c`

### Programas de prueba del Proyecto 2

#### `thello`
Prueba `hello()`.

**Ejemplo probado:**
```text
$ thello
hello returned 42
```

#### `ttrace`
Activa `trace(mask)` y luego ejecuta otro programa.

**Ejemplo probado:**
```text
$ ttrace 4194304 thello
trace: pid=5 name=thello syscall=22 (hello) a0=0x1 a1=0x3fe0 a2=0x9 ret=0x2a
hello returned 42
```

#### `tdumpvm`
Imprime la tabla de páginas del proceso actual, luego reserva memoria con `sbrk()` y la vuelve a imprimir.

**Resultado esperado:**
- salida jerárquica
- PTEs válidas
- PPNs
- permisos como `R`, `W`, `X`, `U`

#### `tmemro`
Mapea memoria solo lectura, la lee correctamente y luego intenta escribir.

**Resultado esperado:**
- lectura exitosa
- al escribir: `store page fault`
- el shell recupera el control

**Ejemplo probado:**
```text
$ tmemro
contenido lectura OK: mensaje solo lectura
ahora intentare escribir en memoria solo lectura...
store page fault: pid=7 scause=0xf stval=0x50000000 sepc=0x38
```

#### `tuargs`
Prueba casos válidos e inválidos de argumentos en syscalls.

Incluye:
- `hello`
- `write` válido
- `write` con puntero inválido
- `open` de archivo inexistente
- `read` con buffer inválido
- `read` válido

**Resultado esperado:**
- punteros inválidos retornan `-1`
- el kernel no hace `panic`

**Ejemplo probado:**
```text
$ tuargs
== prueba 1: hello ==
hello returned 42
== prueba 2: write con puntero valido a archivo ==
write valido retorno 18
== prueba 3: write con puntero invalido a archivo ==
write invalido retorno -1
== prueba 4: open de archivo inexistente ==
open inexistente retorno -1
== prueba 5: read con buffer invalido desde archivo ==
read buffer invalido retorno -1
== prueba 6: read con buffer valido desde archivo ==
read valido retorno 31, buf='# Reto 1 — Shell **EAFITos**'
```

### Checklist de prueba del Proyecto 2

Dentro de xv6:

```text
$ thello
$ ttrace 4194304 thello
$ tdumpvm
$ tmemro
$ tuargs
```

### Notas de implementación

- Se modificó el kernel de xv6 para registrar nuevas syscalls.
- Se añadio soporte de trazado por máscara en `struct proc`.
- Se implementó `vmprint()` en `kernel/vm.c`.
- Se añadió manejo explícito de `store page fault` para la prueba de memoria solo lectura.
- Se conservaron los cambios del taller de memoria compartida realizados previamente en clase.

### Build system

La compilación queda integrada en el `Makefile` mediante `UPROGS`, incluyendo:

- `eafitos`
- `thello`
- `ttrace`
- `tdumpvm`
- `tmemro`
- `tuargs`
- `smhtest`

### Estado actual

A la fecha, el repositorio compila con:

```bash
make clean
make qemu
```

las pruebas funcionales principales del Proyecto 2 ya corren correctamente.