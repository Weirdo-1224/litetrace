 #include <errno.h>
 #include <stdio.h>
 #include <string.h>

 #include "output.h"
 #include "tracefs.h"
 #include "tracer_ctrl.h"

 int main(int argc, char *argv[]) {
     tracefs_ctx_t ctx;
     int ret;

     if (argc < 2) {
         print_usage(argv[0]);
         return 2;
     }

     if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
         print_usage(argv[0]);
         return 0;
     }

     if (tracefs_init(&ctx) != 0) {
         if (errno == ENOENT) {
             print_err("tracefs not found, ensure ftrace is enabled in kernel");
             return 3;
         }
         print_err("failed to initialize tracefs");
         return 1;
     }

     if (strcmp(argv[1], "tracer") == 0) {
         if (argc < 3) {
             print_err("missing argument");
             print_usage(argv[0]);
             return 2;
         }
         ret = lt_set_tracer(&ctx, argv[2]);
         return ret == 0 ? 0 : 1;
     } else if (strcmp(argv[1], "filter") == 0) {
         if (argc < 3) {
             print_err("missing argument");
             print_usage(argv[0]);
             return 2;
         }
         ret = lt_set_filter(&ctx, argv[2]);
         return ret == 0 ? 0 : 1;
     } else if (strcmp(argv[1], "filter-clear") == 0) {
         ret = lt_clear_filter(&ctx);
         return ret == 0 ? 0 : 1;
     } else if (strcmp(argv[1], "start") == 0) {
         ret = lt_start(&ctx);
         return ret == 0 ? 0 : 1;
     } else if (strcmp(argv[1], "stop") == 0) {
         ret = lt_stop(&ctx);
         return ret == 0 ? 0 : 1;
     } else if (strcmp(argv[1], "status") == 0) {
         ret = lt_status(&ctx);
         return ret == 0 ? 0 : 1;
     } else if (strcmp(argv[1], "dump") == 0) {
         const char *outfile = NULL;
         int i = 2;

         while (i < argc) {
             if (strcmp(argv[i], "-o") == 0) {
                 if (i + 1 >= argc) {
                     print_err("missing argument");
                     print_usage(argv[0]);
                     return 2;
                 }
                 outfile = argv[i + 1];
                 i += 2;
             } else {
                 print_err("unknown option '%s' for dump", argv[i]);
                 print_usage(argv[0]);
                 return 2;
             }
         }

         ret = lt_dump(&ctx, outfile);
         return ret == 0 ? 0 : 1;
     } else {
         print_err("unknown command");
         print_usage(argv[0]);
         return 1;
     }
 }

