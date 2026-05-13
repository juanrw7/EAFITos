# Reto 3 — Comandos EAFITos: Lazy Allocation (xv6-riscv)

- **Autor:** Juan Manuel Ramirez  
- **Modalidad:** Individual  
- **Entorno objetivo:** `xv6-riscv` (MIT, branch `riscv`)


---

## Resumen

Este proyecto corresponde a la tercera entrega de la shell educativa **EAFITos** sobre **xv6-riscv**.

El objetivo principal fue integrar a la shell comandos y pruebas relacionadas con **gestión de memoria**, para estudiar de forma práctica:

- page faults
- lazy allocation
- crecimiento virtual con `sbrk()`
- conteo de faults por proceso
- simulación sencilla de `mmap`

La solución final conserva el enfoque pedagógico de xv6 y añade tanto:

1. **programas de prueba independientes** para verificar cada funcionalidad, y  
2. **comandos integrados en EAFITos** para ejecutar esas pruebas desde la shell.

---

## Objetivo

Agregar a **EAFITos** comandos de gestión de memoria que permitan aplicar y consolidar conceptos fundamentales de Sistemas Operativos, en coherencia con la filosofía educativa de xv6.

---

## Temas cubiertos

- detección de **load page fault** y **store page fault**
- diferencia entre **reservar memoria virtual** y **materializar memoria física**
- implementación de **lazy allocation**
- conteo de page faults por proceso
- simulación de una región tipo `mmap`

---

## Funcionalidades implementadas

### 1. Observación de page faults
En `kernel/trap.c` se detectan:

- `scause == 13` → **load page fault**
- `scause == 15` → **store page fault**

En ambos casos se imprime información relevante del fault:

- PID
- `scause`
- `stval`
- `sepc`

Además, se creó un programa de prueba que provoca fallos de lectura y escritura sobre direcciones no mapeadas.

---

### 2. Transformación de `sbrk()` hacia lazy allocation
Se modificó `sys_sbrk()` para que, al crecer memoria:

- aumente el tamaño virtual del proceso (`sz`)
- no asigne páginas físicas inmediatamente

Esto permite que la memoria se reserve primero a nivel virtual, y se materialice solo cuando sea accedida.

---

### 3. Lazy allocation real
Cuando ocurre un page fault por acceso a una dirección válida dentro del tamaño del proceso:

- se asigna una nueva página con `kalloc()`
- se inicializa en cero
- se mapea con `mappages()`
- el proceso continúa sin crash

Con esto, la memoria se asigna **bajo demanda**.

---

### 4. Conteo de page faults por proceso
Se agregó un contador de faults por proceso en `struct proc`.

Además, se implementaron syscalls auxiliares para:

- consultar el contador actual
- reiniciarlo

Esto permite medir el comportamiento de distintos patrones de acceso.

---

### 5. Simulación simple de `mmap`
Se implementó una syscall `mapzero(size)` que:

- reserva una región virtual sin mapear
- no asigna memoria física en ese momento

Luego, si el proceso hace fault dentro de esa región:

- se asigna una página
- se inicializa con el patrón `'A'`
- se mapea dinámicamente

Esto simula de forma sencilla el comportamiento base de `mmap`.

---

## Archivos añadidos / modificados

### Kernel
- **Modificado:** `kernel/trap.c`
- **Modificado:** `kernel/sysproc.c`
- **Modificado:** `kernel/proc.h`
- **Modificado:** `kernel/proc.c`
- **Modificado:** `kernel/syscall.h`
- **Modificado:** `kernel/syscall.c`
- **Modificado:** `kernel/defs.h`
- **Modificado:** `kernel/vm.c`

### User
- **Modificado:** `user/user.h`
- **Modificado:** `user/usys.pl`
- **Modificado:** `user/eafitos.c`

### Nuevos programas de prueba
- **Añadido:** `user/tpf.c`
- **Añadido:** `user/tsbrkpf.c`
- **Añadido:** `user/tsbrklazy.c`
- **Añadido:** `user/tlazy.c`
- **Añadido:** `user/tmmap_sim.c`

### Build
- **Modificado:** `Makefile`

---

## Compilar y ejecutar

### En el host (WSL / Linux)

```bash
cd xv6-riscv
make clean
make qemu
```

### Dentro de xv6

Puedes entrar a la shell principal:

```text
$ eafitos
```

---

## Comandos nuevos integrados en EAFITos

Además de los comandos del proyecto anterior, EAFITos ahora incluye los siguientes comandos de memoria:

- **`pf <modo>`**  
  Ejecuta pruebas de page fault con `tpf`.

- **`sbrkpf`**  
  Demuestra acceso a memoria reservada con `sbrk` bajo lazy allocation.

- **`sbrklazy`**  
  Ejecuta una prueba progresiva de lazy allocation real con varias páginas.

- **`lazy`**  
  Compara acceso secuencial vs disperso e imprime la cantidad de page faults.

- **`mmapsim`**  
  Simula un comportamiento tipo `mmap` usando `mapzero(size)`.

---

## Programas de prueba y propósito

### `tpf`
Prueba page faults manuales en direcciones no mapeadas.

Modos soportados:

- `readlow`
- `writelow`
- `readhigh`
- `writehigh`

Ejemplos:

```text
EAFITos> pf readlow
EAFITos> pf writelow
```

Comportamiento esperado:
- lectura inválida → `scause=13`
- escritura inválida → `scause=15`

---

### `tsbrkpf`
Reserva memoria con `sbrk()` y muestra cómo una página virtual es materializada cuando se accede por primera vez.

Ejemplo esperado:

```text
EAFITos> sbrkpf
sbrk reservo una pagina virtual en 0x...
intentando escribir en la memoria reservada...
lazy alloc: pid=... scause=15 stval=0x...
escritura completada: la pagina fue materializada por lazy allocation
valor leido: A
```

---

### `tsbrklazy`
Reserva varias páginas con `sbrk()` y accede progresivamente a ellas, mostrando que cada nueva página se asigna bajo demanda.

Ejemplo esperado:

```text
EAFITos> sbrklazy
reserva virtual de 3 paginas desde 0x...
tocando pagina 0...
lazy alloc: ...
pagina 0 ahora contiene A
...
acceso progresivo completado sin crash
```

---

### `tlazy`
Mide el número de page faults según el patrón de acceso.

Resultado esperado:
- acceso secuencial → menos faults
- acceso disperso → más faults

Ejemplo esperado:

```text
EAFITos> lazy
faults secuencial: 2
faults disperso: 8
```

---

### `tmmap_sim`
Usa `mapzero(size)` para reservar una región virtual y demostrar asignación bajo demanda con inicialización en `'A'`.

Ejemplo esperado:

```text
EAFITos> mmapsim
mapzero devolvio 0x70000000
leyendo pagina 0...
mapzero alloc: ...
valor p[0] = A
...
escribiendo en pagina 1...
nuevo valor p[PGSIZE] = Z
```

---

## Checklist de pruebas finales

Dentro de xv6:

```text
$ eafitos
EAFITos> ayuda
EAFITos> pf readlow
EAFITos> pf writelow
EAFITos> sbrkpf
EAFITos> sbrklazy
EAFITos> lazy
EAFITos> mmapsim
```

---

## Estructura de la solución

La implementación final quedó organizada de manera modular:

- lógica del kernel en archivos de `kernel/`
- wrappers y comandos del usuario en `user/`
- comandos accesibles desde **EAFITos**
- programas de prueba separados para facilitar validación y depuración

Este enfoque permite:
- mantener el código más limpio
- aislar pruebas por funcionalidad
- integrar todo dentro de la shell sin duplicar lógica

---

## Consideraciones de diseño

- Los page faults inválidos siguen terminando el proceso.
- Los page faults válidos dentro de regiones lazy son atendidos por el kernel.
- La simulación de `mmap` usa una región reservada especial y páginas inicializadas con `'A'`.
- El conteo de faults por proceso permite comparar patrones de acceso.
- Se optó por integrar comandos de memoria a EAFITos como **wrappers** sobre programas de prueba ya implementados, para mejorar modularidad y facilitar depuración.

---

## Estado actual

El proyecto:

- compila correctamente con `make clean && make qemu`
- ejecuta los comandos de memoria desde **EAFITos**
- demuestra correctamente:
  - page faults
  - lazy allocation
  - conteo de faults
  - simulación de `mmap`

---

## Aclaración

Puede que la branch del repositorio diga reto1 pero el estado actual del repositorio cumple con el proyecto 3.


# Reto 2 — Shell **EAFITos** Gestión de memoria y syscalls (xv6-riscv)
Reto Pasado
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