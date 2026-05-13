#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "user/user.h"

#define DEFAULT_PROMPT "EAFITos"
#define MAXPROMPT 64
#define MAXLINE 256
#define MAXARGS 32
#define HIST_SIZE 10

// Uptimez ticks desde boot.
#define TICKS_PER_SEC_APPROX 10

// -------------------- Utils --------------------
static char prompt_text[MAXPROMPT] = DEFAULT_PROMPT;
static char prompt_color[16] = "\033[1;36m";  
static char color_reset[] = "\033[0m";
static char color_error[] = "\033[1;31m";
static char color_info[] = "\033[1;34m";

static int streq(const char *a, const char *b) {
  return strcmp(a, b) == 0;
}

static void rstrip_newlines(char *s) {
  if (!s) return;
  int n = strlen(s);
  while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
    s[n-1] = 0;
    n--;
  }
}

static int is_blank(const char *s) {
  if (!s) return 1;
  while (*s) {
    if (*s != ' ' && *s != '\t' && *s != '\n' && *s != '\r') return 0;
    s++;
  }
  return 1;
}

static int safe_copy(char *dst, int dsz, const char *src) {
  if (!dst || dsz <= 0) return 0;
  if (!src) { dst[0] = 0; return 1; }
  int n = strlen(src);
  if (n >= dsz) return 0;
  memmove(dst, src, n);
  dst[n] = 0;
  return 1;
}

static int safe_cat(char *dst, int dsz, const char *src) {
  if (!dst || dsz <= 0) return 0;
  if (!src) return 1;
  int a = strlen(dst);
  int b = strlen(src);
  if (a + b >= dsz) return 0;
  memmove(dst + a, src, b);
  dst[a + b] = 0;
  return 1;
}

static void print_prompt(void) {
  printf("%s%s>%s ", prompt_color, prompt_text, color_reset);
}

static void print_error(const char *msg) {
  printf("%s%s%s\n", color_error, msg, color_reset);
}

static void print_info(const char *msg) {
  printf("%s%s%s\n", color_info, msg, color_reset);
}

static void reset_ui(void) {
  safe_copy(prompt_text, sizeof(prompt_text), DEFAULT_PROMPT);
  safe_copy(prompt_color, sizeof(prompt_color), "\033[1;36m");
}

static int set_theme_color(const char *name) {
  if (streq(name, "default") || streq(name, "cyan")) {
    return safe_copy(prompt_color, sizeof(prompt_color), "\033[1;36m");
  } else if (streq(name, "verde")) {
    return safe_copy(prompt_color, sizeof(prompt_color), "\033[1;32m");
  } else if (streq(name, "azul")) {
    return safe_copy(prompt_color, sizeof(prompt_color), "\033[1;34m");
  } else if (streq(name, "rojo")) {
    return safe_copy(prompt_color, sizeof(prompt_color), "\033[1;31m");
  } else if (streq(name, "amarillo")) {
    return safe_copy(prompt_color, sizeof(prompt_color), "\033[1;33m");
  } else if (streq(name, "magenta")) {
    return safe_copy(prompt_color, sizeof(prompt_color), "\033[1;35m");
  } else if (streq(name, "blanco")) {
    return safe_copy(prompt_color, sizeof(prompt_color), "\033[1;37m");
  }
  return 0;
}

// Parser simple
static int parse_line(char *line, char **argv, int max_args) {
  int argc = 0;
  char *p = line;

  while (*p && argc < max_args - 1) {
    while (*p == ' ' || *p == '\t') p++;
    if (*p == 0) break;

    argv[argc++] = p;

    while (*p && *p != ' ' && *p != '\t') p++;
    if (*p == 0) break;
    *p = 0;
    p++;
  }

  argv[argc] = 0;
  return argc;
}

// History
static char hist[HIST_SIZE][MAXLINE];
static int hist_count = 0;
static int hist_next = 0;

static void history_add(const char *line) {
  if (!line || line[0] == 0) return;

  int n = strlen(line);
  if (n >= MAXLINE) n = MAXLINE - 1;

  memmove(hist[hist_next], line, n);
  hist[hist_next][n] = 0;

  hist_next = (hist_next + 1) % HIST_SIZE;
  if (hist_count < HIST_SIZE) hist_count++;
}

static void history_print(void) {
  int start = (hist_next - hist_count + HIST_SIZE) % HIST_SIZE;
  for (int i = 0; i < hist_count; i++) {
    int idx = (start + i) % HIST_SIZE;
    printf("%d  %s\n", i + 1, hist[idx]);
  }
}

// getcwd “user-space” para xv6 
static int getcwd_xv6(char *out, int outsz) {
  if (!out || outsz <= 0) return 0;

  char anc[128];
  if (!safe_copy(anc, sizeof(anc), ".")) return 0;

  char comps[32][DIRSIZ + 1];
  int depth = 0;

  while (1) {
    int fd_cur = open(anc, O_RDONLY);
    if (fd_cur < 0) break;

    struct stat st_cur;
    if (fstat(fd_cur, &st_cur) < 0) {
      close(fd_cur);
      break;
    }
    close(fd_cur);

    char parent[128];
    if (streq(anc, ".")) {
      if (!safe_copy(parent, sizeof(parent), "..")) break;
    } else {
      if (!safe_copy(parent, sizeof(parent), anc)) break;
      if (!safe_cat(parent, sizeof(parent), "/..")) break;
    }

    int fd_par = open(parent, O_RDONLY);
    if (fd_par < 0) break;

    struct stat st_par;
    if (fstat(fd_par, &st_par) < 0) {
      close(fd_par);
      break;
    }

    if (st_cur.ino == st_par.ino && st_cur.dev == st_par.dev) {
      close(fd_par);
      break;
    }

    struct dirent de;
    char found[DIRSIZ + 1];
    found[0] = 0;

    while (read(fd_par, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0) continue;

      char name[DIRSIZ + 1];
      memmove(name, de.name, DIRSIZ);
      name[DIRSIZ] = 0;

      if (streq(name, ".") || streq(name, "..")) continue;

      if ((uint)de.inum == st_cur.ino) {
        safe_copy(found, sizeof(found), name);
        break;
      }
    }

    close(fd_par);

    if (found[0] == 0) break;
    if (depth >= (int)(sizeof(comps) / sizeof(comps[0]))) break;

    safe_copy(comps[depth], sizeof(comps[depth]), found);
    depth++;

    if (!safe_copy(anc, sizeof(anc), parent)) break;
  }

 
  safe_copy(out, outsz, "/");
  for (int i = depth - 1; i >= 0; i--) {
    if (!streq(out, "/")) {
      if (!safe_cat(out, outsz, "/")) return 0;
    }
    if (!safe_cat(out, outsz, comps[i])) return 0;
  }

  return 1;
}

// Comandos
typedef int (*cmd_fn)(int argc, char **argv);

static int run_program(char *prog, char **argv) {
  int pid = fork();

  if (pid < 0) {
    printf("No se pudo crear el proceso para %s\n", prog);
    return 0;
  }

  if (pid == 0) {
    exec(prog, argv);
    printf("No se pudo ejecutar %s\n", prog);
    exit(1);
  }

  wait(0);
  return 0;
}

static void help_usage(const char *cmd) {
  if (streq(cmd, "listar")) {
    printf("Uso: listar\n  Lista el contenido del directorio actual.\n");
  } else if (streq(cmd, "leer")) {
    printf("Uso: leer <archivo>\n  Muestra el contenido de un archivo.\n");
  } else if (streq(cmd, "tiempo")) {
    printf("Uso: tiempo\n  Muestra uptime (ticks) y hh:mm:ss aprox.\n");
  } else if (streq(cmd, "calc")) {
    printf("Uso: calc <num1> <op> <num2>\n  Operadores: +  -  *  /  %%\n");
    printf("  Nota: en xv6 esta versión usa solo enteros.\n");
  } else if (streq(cmd, "ayuda")) {
    printf("Uso: ayuda [comando]\n  Lista comandos o muestra ayuda específica.\n");
  } else if (streq(cmd, "salir")) {
    printf("Uso: salir\n  Termina la shell.\n");
  } else if (streq(cmd, "historial")) {
    printf("Uso: historial\n  Muestra los últimos 10 comandos.\n");
  } else if (streq(cmd, "limpiar")) {
    printf("Uso: limpiar\n  Limpia la pantalla (ANSI / fallback).\n");
  } else if (streq(cmd, "usuario")) {
    printf("Uso: usuario\n  Muestra info del proceso (PID + heap end).\n");
  } else if (streq(cmd, "directorio")) {
    printf("Uso: directorio\n  Muestra el directorio actual (reconstruido desde . y ..).\n");
    } else if (streq(cmd, "pf")) {
    printf("Uso: pf <readlow|writelow|readhigh|writehigh>\n");
    printf("  Ejecuta tpf para observar page faults.\n");
  } else if (streq(cmd, "sbrkpf")) {
    printf("Uso: sbrkpf\n");
    printf("  Ejecuta tsbrkpf para mostrar el fallo tras sbrk lazy inicial.\n");
  } else if (streq(cmd, "sbrklazy")) {
    printf("Uso: sbrklazy\n");
    printf("  Ejecuta tsbrklazy para mostrar lazy allocation real.\n");
  } else if (streq(cmd, "lazy")) {
    printf("Uso: lazy\n");
    printf("  Ejecuta tlazy para comparar faults secuenciales y dispersos.\n");
  } else if (streq(cmd, "mmapsim")) {
    printf("Uso: mmapsim\n");
    printf("  Ejecuta tmmap_sim para simular mmap bajo demanda.\n");
    } else if (streq(cmd, "prompt")) {
    printf("Uso: prompt <texto>\n  Cambia el texto del prompt.\n");
  } else if (streq(cmd, "tema")) {
    printf("Uso: tema <default|verde|azul|rojo|amarillo|magenta|cyan|blanco>\n");
    printf("  Cambia el color del prompt.\n");
  } else if (streq(cmd, "resetui")) {
    printf("Uso: resetui\n  Restaura prompt y color por defecto.\n");
  } else {
    printf("Comando desconocido. Usa: ayuda\n");
  }
}

static int cmd_ayuda(int argc, char **argv);
static int cmd_salir(int argc, char **argv);
static int cmd_pf(int argc, char **argv);
static int cmd_sbrkpf(int argc, char **argv);
static int cmd_sbrklazy(int argc, char **argv);
static int cmd_lazy(int argc, char **argv);
static int cmd_mmapsim(int argc, char **argv);
static int cmd_prompt(int argc, char **argv);
static int cmd_tema(int argc, char **argv);
static int cmd_resetui(int argc, char **argv);

static int cmd_listar(int argc, char **argv) {
  (void)argc; (void)argv;

  int fd = open(".", O_RDONLY);
  if (fd < 0) {
    printf("listar: no se pudo abrir '.'\n");
    return 0;
  }

  struct dirent de;
  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0) continue;

    char name[DIRSIZ + 1];
    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = 0;

    if (streq(name, ".") || streq(name, "..")) continue;
    printf("%s\n", name);
  }

  close(fd);
  return 0;
}

static int cmd_leer(int argc, char **argv) {
  if (argc != 2) {
    help_usage("leer");
    return 0;
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    printf("leer: no se pudo abrir '%s'\n", argv[1]);
    return 0;
  }

  char buf[512];
  int n;
  while ((n = read(fd, buf, sizeof(buf))) > 0) {
    write(1, buf, n);
  }
  close(fd);
  return 0;
}

static int cmd_tiempo(int argc, char **argv) {
  (void)argc; (void)argv;

  int t = uptime();  // ticks desde boot
  int sec = t / TICKS_PER_SEC_APPROX;

  int h = sec / 3600;
  int m = (sec % 3600) / 60;
  int s = sec % 60;

  printf("uptime: %d ticks (~%d s) %d:%d:%d (aprox)\n", t, sec, h, m, s);
  return 0;
}

static int parse_int(const char *s, int *out) {
  if (!s || !out) return 0;
  int sign = 1;
  if (*s == '-') { sign = -1; s++; }
  if (*s == 0) return 0;

  int v = 0;
  for (; *s; s++) {
    if (*s < '0' || *s > '9') return 0;
    v = v * 10 + (*s - '0');
  }
  *out = sign * v;
  return 1;
}

static int cmd_calc(int argc, char **argv) {
  if (argc != 4) {
    help_usage("calc");
    return 0;
  }

  int a, b;
  if (!parse_int(argv[1], &a) || !parse_int(argv[3], &b)) {
    printf("calc: operandos inválidos (xv6: solo enteros)\n");
    help_usage("calc");
    return 0;
  }

  if (strlen(argv[2]) != 1) {
    printf("calc: operador inválido\n");
    help_usage("calc");
    return 0;
  }

  char op = argv[2][0];
  int res = 0;

  if (op == '+') res = a + b;
  else if (op == '-') res = a - b;
  else if (op == '*') res = a * b;
  else if (op == '/') {
    if (b == 0) { printf("calc: división por cero.\n"); return 0; }
    res = a / b;
  } else if (op == '%') {
    if (b == 0) { printf("calc: división por cero.\n"); return 0; }
    res = a % b;
  } else {
    printf("calc: operador no soportado: '%c'\n", op);
    help_usage("calc");
    return 0;
  }

  printf("%d\n", res);
  return 0;
}

static int cmd_historial(int argc, char **argv) {
  (void)argc; (void)argv;
  history_print();
  return 0;
}

static int cmd_limpiar(int argc, char **argv) {
  (void)argc; (void)argv;
  // ANSI (si terminal interpreta). Fallback: varias líneas.
  printf("\033[2J\033[H");
  for (int i = 0; i < 30; i++) printf("\n");
  return 0;
}

static int cmd_usuario(int argc, char **argv) {
  (void)argc; (void)argv;

  int pid = getpid();
  void *brk = sbrk(0);  
  printf("Proceso (xv6)\n");
  printf("PID: %d\n", pid);
  printf("Heap end: %p\n", brk);

  return 0;
}

static int cmd_directorio(int argc, char **argv) {
  (void)argc; (void)argv;

  char path[128];
  if (getcwd_xv6(path, sizeof(path))) {
    printf("%s\n", path);
  } else {
    printf("/?\n");
  }
  return 0;
}

static int cmd_pf(int argc, char **argv) {
  if (argc != 2) {
    help_usage("pf");
    return 0;
  }

  char *child_argv[3];
  child_argv[0] = "tpf";
  child_argv[1] = argv[1];
  child_argv[2] = 0;

  return run_program("tpf", child_argv);
}

static int cmd_sbrkpf(int argc, char **argv) {
  (void)argc; (void)argv;

  char *child_argv[2];
  child_argv[0] = "tsbrkpf";
  child_argv[1] = 0;

  return run_program("tsbrkpf", child_argv);
}

static int cmd_sbrklazy(int argc, char **argv) {
  (void)argc; (void)argv;

  char *child_argv[2];
  child_argv[0] = "tsbrklazy";
  child_argv[1] = 0;

  return run_program("tsbrklazy", child_argv);
}

static int cmd_lazy(int argc, char **argv) {
  (void)argc; (void)argv;

  char *child_argv[2];
  child_argv[0] = "tlazy";
  child_argv[1] = 0;

  return run_program("tlazy", child_argv);
}

static int cmd_mmapsim(int argc, char **argv) {
  (void)argc; (void)argv;

  char *child_argv[2];
  child_argv[0] = "tmmap_sim";
  child_argv[1] = 0;

  return run_program("tmmap_sim", child_argv);
}

static int cmd_ayuda(int argc, char **argv) {
  if (argc == 1) {
    printf("Comandos disponibles:\n");
    printf("  listar                 - Lista el directorio actual\n");
    printf("  leer <archivo>         - Muestra contenido de un archivo\n");
    printf("  tiempo                 - Uptime (ticks) + hh:mm:ss aprox\n");
    printf("  calc <n1> <op> <n2>    - Calculadora (enteros)\n");
    printf("  ayuda [comando]        - Ayuda general o por comando\n");
    printf("  historial              - Últimos 10 comandos\n");
    printf("  limpiar                - Limpia la pantalla\n");
    printf("  usuario                - Info del proceso (PID + heap)\n");
    printf("  directorio             - Directorio actual (reconstruido)\n");
    printf("  pf <modo>              - Ejecuta tpf para observar page faults\n");
    printf("  sbrkpf                 - Demuestra acceso a memoria reservada con sbrk bajo lazy allocation\n");
    printf("  sbrklazy               - Prueba lazy allocation real con sbrk\n");
    printf("  lazy                   - Compara faults secuencial vs disperso\n");
    printf("  mmapsim                - Simula mmap bajo demanda con mapzero\n");
    printf("  prompt <texto>         - Cambia el texto del prompt\n");
    printf("  tema <color>           - Cambia el color del prompt\n");
    printf("  resetui                - Restaura interfaz por defecto\n");
    printf("  salir                  - Termina la shell\n");
    return 0;
  }
  if (argc == 2) {
    help_usage(argv[1]);
    return 0;
  }
  help_usage("ayuda");
  return 0;
}

static int cmd_salir(int argc, char **argv) {
  (void)argc; (void)argv;
  return 1;
}

//Tabla + Dispatch
struct CommandEntry {
  const char *name;
  cmd_fn fn;
};

static int cmd_prompt(int argc, char **argv) {
  char buf[MAXPROMPT];
  int i;

  if (argc < 2) {
    help_usage("prompt");
    return 0;
  }

  buf[0] = 0;
  for (i = 1; i < argc; i++) {
    if (i > 1) {
      if (!safe_cat(buf, sizeof(buf), " ")) {
        print_error("prompt: texto demasiado largo");
        return 0;
      }
    }
    if (!safe_cat(buf, sizeof(buf), argv[i])) {
      print_error("prompt: texto demasiado largo");
      return 0;
    }
  }

  if (!safe_copy(prompt_text, sizeof(prompt_text), buf)) {
    print_error("prompt: no se pudo cambiar");
    return 0;
  }

  print_info("Prompt actualizado.");
  return 0;
}

static int cmd_tema(int argc, char **argv) {
  if (argc != 2) {
    help_usage("tema");
    return 0;
  }

  if (!set_theme_color(argv[1])) {
    print_error("tema: opcion no valida");
    help_usage("tema");
    return 0;
  }

  print_info("Tema actualizado.");
  return 0;
}

static int cmd_resetui(int argc, char **argv) {
  (void)argc; (void)argv;
  reset_ui();
  print_info("Interfaz restaurada.");
  return 0;
}

static struct CommandEntry COMMANDS[] = {
  {"listar", cmd_listar},
  {"leer", cmd_leer},
  {"tiempo", cmd_tiempo},
  {"calc", cmd_calc},
  {"ayuda", cmd_ayuda},
  {"historial", cmd_historial},
  {"limpiar", cmd_limpiar},
  {"usuario", cmd_usuario},
  {"directorio", cmd_directorio},
  {"pf", cmd_pf},
  {"sbrkpf", cmd_sbrkpf},
  {"sbrklazy", cmd_sbrklazy},
  {"lazy", cmd_lazy},
  {"mmapsim", cmd_mmapsim},
  {"prompt", cmd_prompt},
  {"tema", cmd_tema},
  {"resetui", cmd_resetui},
  {"salir", cmd_salir},
};

static int dispatch(int argc, char **argv) {
  if (argc == 0 || argv[0] == 0) return 0;

  for (int i = 0; i < (int)(sizeof(COMMANDS) / sizeof(COMMANDS[0])); i++) {
    if (streq(argv[0], COMMANDS[i].name)) {
      return COMMANDS[i].fn(argc, argv);
    }
  }

  printf("%sComando no reconocido:%s %s\n", color_error, color_reset, argv[0]);
  printf("%sEscribe 'ayuda' para ver comandos.%s\n", color_info, color_reset);
  return 0;
}

static void print_banner(void) {
  printf("\n");
  printf("\n");
  printf("%s", prompt_color); 

  printf("███████╗ █████╗ ███████╗██╗████████╗ ██████╗ ███████╗\n");
  printf("██╔════╝██╔══██╗██╔════╝██║╚══██╔══╝██╔═══██╗██╔════╝\n");
  printf("█████╗  ███████║█████╗  ██║   ██║   ██║   ██║███████╗\n");
  printf("██╔══╝  ██╔══██║██╔══╝  ██║   ██║   ██║   ██║╚════██║\n");
  printf("███████╗██║  ██║██║     ██║   ██║   ╚██████╔╝███████║\n");
  printf("╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝   ╚═╝    ╚═════╝ ╚══════╝\n");

  printf("%s\n", color_reset);
}

//Main (REPL)
int main(void) {
  char line[MAXLINE];
  char *argv[MAXARGS];

  print_banner();

  while (1) {
    print_prompt();

    if (gets(line, sizeof(line)) == 0) {
      printf("\n");
      break;
    }

    rstrip_newlines(line);
    if (is_blank(line)) continue;

    history_add(line);

    int argc = parse_line(line, argv, MAXARGS);
    int should_exit = dispatch(argc, argv);
    if (should_exit) break;
  }

  exit(0);
}