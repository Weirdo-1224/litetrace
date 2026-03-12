 #ifndef TRACER_CTRL_H
 #define TRACER_CTRL_H

 #include "tracefs.h"

 int lt_set_tracer(tracefs_ctx_t *ctx, const char *tracer);
 int lt_set_filter(tracefs_ctx_t *ctx, const char *filter);
 int lt_clear_filter(tracefs_ctx_t *ctx);
 int lt_start(tracefs_ctx_t *ctx);
 int lt_stop(tracefs_ctx_t *ctx);
 int lt_status(tracefs_ctx_t *ctx);
 int lt_dump(tracefs_ctx_t *ctx, const char *outfile);

 #endif

