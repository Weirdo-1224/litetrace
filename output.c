 #include "output.h"

 #include <stdarg.h>
 #include <stdio.h>

 void print_ok(const char *fmt, ...) {
     va_list ap;

     fputs("[OK] ", stdout);
     va_start(ap, fmt);
     vfprintf(stdout, fmt, ap);
     va_end(ap);
     fputc('\n', stdout);
 }

 void print_err(const char *fmt, ...) {
     va_list ap;

     fputs("[ERR] ", stderr);
     va_start(ap, fmt);
     vfprintf(stderr, fmt, ap);
     va_end(ap);
     fputc('\n', stderr);
 }

 void print_info(const char *fmt, ...) {
     va_list ap;

     va_start(ap, fmt);
     vfprintf(stdout, fmt, ap);
     va_end(ap);
     fputc('\n', stdout);
 }

 void print_usage(const char *prog) {
     const char *name = prog ? prog : "litetrace";

     fprintf(stdout,
             "Usage:\n"
             "  %s tracer function\n"
             "  %s filter <func|pattern>\n"
             "  %s filter-clear\n"
             "  %s start\n"
             "  %s stop\n"
             "  %s status\n"
             "  %s dump [-o file]\n",
             name, name, name, name, name, name, name);
 }

