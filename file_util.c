 #include "file_util.h"

 #include <errno.h>
 #include <fcntl.h>
 #include <string.h>
 #include <unistd.h>

 static void trim_trailing_newline(char *s) {
     size_t len;

     if (s == NULL) {
         return;
     }

     len = strlen(s);
     while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
         s[len - 1] = '\0';
         --len;
     }
 }

 int read_text_file(const char *path, char *buf, size_t size) {
     int fd;
     ssize_t n;
     size_t total = 0;

     if (!path || !buf || size == 0) {
         errno = EINVAL;
         return -1;
     }

     fd = open(path, O_RDONLY);
     if (fd < 0) {
         return -1;
     }

     while (total + 1 < size) {
         n = read(fd, buf + total, size - 1 - total);
         if (n < 0) {
             if (errno == EINTR) {
                 continue;
             }
             close(fd);
             return -1;
         }
         if (n == 0) {
             break;
         }
         total += (size_t)n;
     }

     buf[total] = '\0';
     close(fd);

     trim_trailing_newline(buf);

     return (int)total;
 }

 int write_text_file(const char *path, const char *text) {
     int fd;
     size_t len;
     ssize_t n;
     size_t written = 0;

     if (!path || !text) {
         errno = EINVAL;
         return -1;
     }

     fd = open(path, O_WRONLY | O_TRUNC);
     if (fd < 0) {
         return -1;
     }

     len = strlen(text);
     while (written < len) {
         n = write(fd, text + written, len - written);
         if (n < 0) {
             if (errno == EINTR) {
                 continue;
             }
             close(fd);
             return -1;
         }
         written += (size_t)n;
     }

     if (close(fd) < 0) {
         return -1;
     }

     return 0;
 }

 int write_empty_file(const char *path) {
     int fd;

     if (!path) {
         errno = EINVAL;
         return -1;
     }

     fd = open(path, O_WRONLY | O_TRUNC);
     if (fd < 0) {
         return -1;
     }

     if (close(fd) < 0) {
         return -1;
     }

     return 0;
 }

