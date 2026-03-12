 #include "tracer_ctrl.h"

 #include <errno.h>
 #include <fcntl.h>
 #include <stdio.h>
 #include <string.h>
 #include <unistd.h>

 #include "file_util.h"
 #include "output.h"

 #define LT_SMALL_BUF 256
 #define LT_MEDIUM_BUF 1024
 #define LT_LARGE_BUF 8192

 static int is_permission_error(int err) {
     return err == EACCES || err == EPERM;
 }

 int lt_set_tracer(tracefs_ctx_t *ctx, const char *tracer) {
     char buf[LT_MEDIUM_BUF];
     int ret;

     if (!ctx) {
         errno = EINVAL;
         return -1;
     }

     if (!tracer) {
         print_err("missing argument");
         return -1;
     }

     if (strcmp(tracer, "function") != 0) {
         print_err("tracer '%s' is not supported, only 'function' is allowed",
                   tracer);
         return -1;
     }

     ret = read_text_file(ctx->available_tracers, buf, sizeof(buf));
     if (ret < 0) {
         if (is_permission_error(errno)) {
             print_err("permission denied, please run as root");
         } else {
             print_err("failed to read available_tracers");
         }
         return -1;
     }

     if (strstr(buf, "function") == NULL) {
         print_err("tracer 'function' is not available on this system");
         return -1;
     }

     ret = write_text_file(ctx->current_tracer, "function");
     if (ret < 0) {
         if (is_permission_error(errno)) {
             print_err("permission denied, please run as root");
         } else {
             print_err("failed to set current_tracer");
         }
         return -1;
     }

     print_ok("tracer set to function");
     return 0;
 }

 int lt_set_filter(tracefs_ctx_t *ctx, const char *filter) {
     int ret;

     if (!ctx) {
         errno = EINVAL;
         return -1;
     }

     if (!filter || filter[0] == '\0') {
         print_err("missing argument");
         return -1;
     }

     ret = write_text_file(ctx->set_ftrace_filter, filter);
     if (ret < 0) {
         if (is_permission_error(errno)) {
             print_err("permission denied, please run as root");
         } else {
             print_err("invalid filter function or pattern");
         }
         return -1;
     }

     print_ok("filter set to %s", filter);
     return 0;
 }

 int lt_clear_filter(tracefs_ctx_t *ctx) {
     int ret;

     if (!ctx) {
         errno = EINVAL;
         return -1;
     }

     ret = write_empty_file(ctx->set_ftrace_filter);
     if (ret < 0) {
         if (is_permission_error(errno)) {
             print_err("permission denied, please run as root");
         } else {
             print_err("failed to clear filter");
         }
         return -1;
     }

     print_ok("filter cleared");
     return 0;
 }

 int lt_start(tracefs_ctx_t *ctx) {
     char cur[LT_SMALL_BUF];
     int ret;

     if (!ctx) {
         errno = EINVAL;
         return -1;
     }

     /* Strict mode: require current_tracer == function */
     ret = read_text_file(ctx->current_tracer, cur, sizeof(cur));
     if (ret < 0) {
         if (is_permission_error(errno)) {
             print_err("permission denied, please run as root");
         } else {
             print_err("failed to read current_tracer");
         }
         return -1;
     }

     if (strcmp(cur, "function") != 0) {
         print_err("current_tracer is '%s', please run 'litetrace tracer function' first",
                   cur[0] ? cur : "<empty>");
         return -1;
     }

     ret = write_text_file(ctx->tracing_on, "1");
     if (ret < 0) {
         if (is_permission_error(errno)) {
             print_err("permission denied, please run as root");
         } else {
             print_err("failed to start tracing");
         }
         return -1;
     }

     print_ok("tracing started");
     return 0;
 }

 int lt_stop(tracefs_ctx_t *ctx) {
     int ret;

     if (!ctx) {
         errno = EINVAL;
         return -1;
     }

     ret = write_text_file(ctx->tracing_on, "0");
     if (ret < 0) {
         if (is_permission_error(errno)) {
             print_err("permission denied, please run as root");
         } else {
             print_err("failed to stop tracing");
         }
         return -1;
     }

     print_ok("tracing stopped");
     return 0;
 }

 int lt_status(tracefs_ctx_t *ctx) {
     char cur[LT_SMALL_BUF];
     char on[LT_SMALL_BUF];
     char filter[LT_MEDIUM_BUF];
     int ret;

     if (!ctx) {
         errno = EINVAL;
         return -1;
     }

     cur[0] = '\0';
     on[0] = '\0';
     filter[0] = '\0';

     ret = read_text_file(ctx->current_tracer, cur, sizeof(cur));
     if (ret < 0) {
         if (is_permission_error(errno)) {
             print_err("permission denied, please run as root");
         } else {
             print_err("failed to read current_tracer");
         }
         return -1;
     }

     ret = read_text_file(ctx->tracing_on, on, sizeof(on));
     if (ret < 0) {
         if (is_permission_error(errno)) {
             print_err("permission denied, please run as root");
         } else {
             print_err("failed to read tracing_on");
         }
         return -1;
     }

     ret = read_text_file(ctx->set_ftrace_filter, filter, sizeof(filter));
     if (ret < 0) {
         if (is_permission_error(errno)) {
             print_err("permission denied, please run as root");
         } else {
             /* If filter cannot be read, still treat as error. */
             print_err("failed to read set_ftrace_filter");
         }
         return -1;
     }

     if (filter[0] == '\0') {
         printf("current_tracer     : %s\n", cur);
         printf("tracing_on         : %s\n", on);
         printf("set_ftrace_filter  : <empty>\n");
     } else {
         printf("current_tracer     : %s\n", cur);
         printf("tracing_on         : %s\n", on);
         printf("set_ftrace_filter  : %s\n", filter);
     }

     return 0;
 }

 int lt_dump(tracefs_ctx_t *ctx, const char *outfile) {
     int fd_in = -1;
     int fd_out = -1;
     char buf[LT_LARGE_BUF];
     ssize_t n;

     if (!ctx) {
         errno = EINVAL;
         return -1;
     }

     fd_in = open(ctx->trace, O_RDONLY);
     if (fd_in < 0) {
         if (is_permission_error(errno)) {
             print_err("permission denied, please run as root");
         } else {
             print_err("failed to open trace");
         }
         return -1;
     }

     if (outfile == NULL) {
         /* Stream to stdout. */
         for (;;) {
             ssize_t w;
             ssize_t written = 0;

             n = read(fd_in, buf, sizeof(buf));
             if (n < 0) {
                 if (errno == EINTR) {
                     continue;
                 }
                 print_err("failed to read trace");
                 close(fd_in);
                 return -1;
             }
             if (n == 0) {
                 break;
             }

             while (written < n) {
                 w = write(STDOUT_FILENO, buf + written, (size_t)(n - written));
                 if (w < 0) {
                     if (errno == EINTR) {
                         continue;
                     }
                     print_err("failed to write to stdout");
                     close(fd_in);
                     return -1;
                 }
                 written += w;
             }
         }

         close(fd_in);
         return 0;
     }

     fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
     if (fd_out < 0) {
         print_err("failed to open output file '%s'", outfile);
         close(fd_in);
         return -1;
     }

     for (;;) {
         ssize_t w;
         ssize_t written = 0;

         n = read(fd_in, buf, sizeof(buf));
         if (n < 0) {
             if (errno == EINTR) {
                 continue;
             }
             print_err("failed to read trace");
             close(fd_in);
             close(fd_out);
             return -1;
         }
         if (n == 0) {
             break;
         }

         while (written < n) {
             w = write(fd_out, buf + written, (size_t)(n - written));
             if (w < 0) {
                 if (errno == EINTR) {
                     continue;
                 }
                 print_err("failed to write to output file");
                 close(fd_in);
                 close(fd_out);
                 return -1;
             }
             written += w;
         }
     }

     if (close(fd_in) < 0) {
         print_err("failed to close trace");
         close(fd_out);
         return -1;
     }
     if (close(fd_out) < 0) {
         print_err("failed to close output file");
         return -1;
     }

     print_ok("trace dumped to %s", outfile);
     return 0;
 }

