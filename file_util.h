 #ifndef FILE_UTIL_H
 #define FILE_UTIL_H

 #include <stddef.h>

 /**
  * read_text_file - read text content from file into buffer.
  * @path: file path.
  * @buf: destination buffer.
  * @size: buffer size.
  *
  * The buffer is always NUL-terminated on success. Trailing newlines may be
  * trimmed.
  *
  * Return: number of bytes read (excluding the terminating NUL) on success,
  *         -1 on failure (errno is set).
  */
 int read_text_file(const char *path, char *buf, size_t size);

 /**
  * write_text_file - write a string to file.
  * @path: file path.
  * @text: text to write (NUL-terminated).
  *
  * Return: 0 on success, -1 on failure (errno is set).
  */
 int write_text_file(const char *path, const char *text);

 /**
  * write_empty_file - truncate file to zero length.
  * @path: file path.
  *
  * Useful for clearing filter or trace buffers.
  *
  * Return: 0 on success, -1 on failure (errno is set).
  */
 int write_empty_file(const char *path);

 #endif

