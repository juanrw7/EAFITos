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

# Reto 1 — Shell **EAFITos** (xv6-riscv)
Reto pasado

## Resumen

**EAFITos** es una shell educativa tipo **REPL** (Read–Eval–Print Loop) que es integrada como **programa de usuario** en xv6.

> Aclaracion importante: **no modifique el kernel** de xv6 para reducir riesgos, mantener estabilidad y asegurar reproducibilidad al evaluar.

---

## Archivos añadidos / modificados

-**Añadido:** `user/eafitos.c`  
-**Modificado:** `Makefile` (se agregó `$U/_eafitos` a `UPROGS`)

---

## Compilar y ejecutar

### En el host (WSL/Linux)

```bash
cd xv6-riscv
make clean
make qemu
```

### Dentro de xv6

```text
$ eafitos
EAFITos> ayuda
```

---

## Salir

- Salir de EAFITos:
  - `EAFITos> salir`

---

## Cómo funciona

1. **Read:** lee una línea (`gets`)
2. **Parse:** tokeniza por espacios/tabs → `argc/argv`
3. **Dispatch:** `dispatch()` busca `argv[0]` en la tabla `COMMANDS[]`
4. **Exec:** ejecuta la función del comando y vuelve al prompt

---

## Comandos implementados (10)

### Básicos (6)

- **`listar:`** Lista el contenido del directorio actual.

- **`leer <archivo>:`** Imprime el contenido de un archivo.

- **`tiempo:`** Muestra **uptime** (ticks) y un formato **hh:mm:ss aproximado**.

- **`calc <n1> <op> <n2>`**
  - Operadores: `+  -  *  /  %`
  - Nota: en xv6 esta versión usa enteros.

- **`ayuda [comando]:`** Lista comandos o muestra el uso de un comando específico.

- **`salir:`** Termina EAFITos y vuelve al shell de xv6.

### Sistema (4)

- **`historial:`** Muestra los últimos **10** comandos (buffer circular).

- **`limpiar:`** Limpia pantalla (ANSI + fallback con saltos de línea).

- **`usuario`:** xv6 no maneja “usuarios” como lo hace Linux: se muestra informacion del proceso (**PID** y **heap end**).

- **`directorio`** xv6 estándar no tiene `getcwd()`: se reconstruye el path en userland usando `.` y `..` .  Si falla, imprime `/?`.

---

## Comandos diferentes a Linux

- **`tiempo`**
  - xv6 no expone hora/fecha real en userland.
  - Se implementa usando `uptime()` que se mide en ticks.
- **`usuario`**
  - No existe base de datos de usuarios como Linux.
  - Se imprime PID y el estado de memoria (heap).
- **`directorio`**
  - No existe `getcwd()` en xv6 estándar.
  - Se evita modificar kernel; se implementa reconstrucción “best-effort”.

**Motivo:** mantener xv6 intacto evita romper compatibilidad, reduce riesgo de bugs y facilita la evaluación (reproducible con `make qemu`).

---

## Checklist de prueba

```text
EAFITos> ayuda
EAFITos> listar
EAFITos> leer README
EAFITos> calc 10 % 3
EAFITos> tiempo
EAFITos> usuario
EAFITos> historial
EAFITos> limpiar
EAFITos> directorio
EAFITos> salir
```
---


## Proyecto sin xv6

Este repositorio fue entregado con xv6 en conjunto para una mayor facilidad al momento de compilarlo y usar el programa. Si se prefiere tener unicamente el documento .c y el makefile del xv6 que lo compila se tiene esta opcion.

**Enlace al repositorio:** https://github.com/juanrw7/eafitos_noXV6


## Proyecto hecho para linux

Antes de realizar este proyecto adaptado para xv6 se realizo en linux. De este proyecto hecho para linux se toma la gran mayoria de funciones y funcionalidad del proyecto.

**Enlace al repositorio:** https://github.com/juanrw7/EAFITos_linux