 #include "tracefs.h"

 #include <errno.h>
 #include <stddef.h>
 #include <stdio.h>
 #include <string.h>
 #include <unistd.h>

 static int build_path(char *dst, size_t dst_size, const char *root,
                       const char *name) {
     int n;

     if (!dst || !root || !name) {
         errno = EINVAL;
         return -1;
     }

     n = snprintf(dst, dst_size, "%s/%s", root, name);
     if (n < 0 || (size_t)n >= dst_size) {
         errno = ENAMETOOLONG;
         return -1;
     }

     return 0;
 }

 int tracefs_init(tracefs_ctx_t *ctx) {
     const char *candidates[] = {
         "/sys/kernel/tracing",
         "/sys/kernel/debug/tracing",
     };
     const size_t nr_candidates = sizeof(candidates) / sizeof(candidates[0]);
     const char *root = NULL;
     size_t i;
     int n;

     if (ctx == NULL) {
         errno = EINVAL;
         return -1;
     }

     memset(ctx, 0, sizeof(*ctx));

     for (i = 0; i < nr_candidates; ++i) {
         if (access(candidates[i], F_OK) == 0) {
             root = candidates[i];
             break;
         }
     }

     if (root == NULL) {
         errno = ENOENT;
         return -1;
     }

     n = snprintf(ctx->root, sizeof(ctx->root), "%s", root);
     if (n < 0 || (size_t)n >= sizeof(ctx->root)) {
         errno = ENAMETOOLONG;
         return -1;
     }

     if (build_path(ctx->current_tracer, sizeof(ctx->current_tracer), root,
                    "current_tracer") != 0) {
         return -1;
     }
     if (build_path(ctx->tracing_on, sizeof(ctx->tracing_on), root,
                    "tracing_on") != 0) {
         return -1;
     }
     if (build_path(ctx->set_ftrace_filter, sizeof(ctx->set_ftrace_filter),
                    root, "set_ftrace_filter") != 0) {
         return -1;
     }
     if (build_path(ctx->trace, sizeof(ctx->trace), root, "trace") != 0) {
         return -1;
     }
     if (build_path(ctx->available_tracers, sizeof(ctx->available_tracers),
                    root, "available_tracers") != 0) {
         return -1;
     }

     return 0;
 }

