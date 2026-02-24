# Reto 1 — Shell **EAFITos** (xv6-riscv)

- **Autor:** Juan Manuel Ramirez
- **Modalidad:** Individual  
- **Entorno objetivo:** `xv6-riscv` (MIT, branch `riscv`)  

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