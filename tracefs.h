 #ifndef TRACEFS_H
 #define TRACEFS_H

 #include <limits.h>

 typedef struct tracefs_ctx {
     char root[PATH_MAX];
     char current_tracer[PATH_MAX];
     char tracing_on[PATH_MAX];
     char set_ftrace_filter[PATH_MAX];
     char trace[PATH_MAX];
     char available_tracers[PATH_MAX];
 } tracefs_ctx_t;

 /**
  * tracefs_init - initialize tracefs context.
  * @ctx: output context pointer, must not be NULL.
  *
  * Detect the tracefs root directory and initialize all control file paths.
  *
  * Return: 0 on success, -1 on failure (errno is set).
  */
 int tracefs_init(tracefs_ctx_t *ctx);

 #endif

