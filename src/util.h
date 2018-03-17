#ifndef MYCRAFT_DEMO_UTIL_H
#define MYCRAFT_DEMO_UTIL_H

char *file_read(const char *filepath);
char *buf_alloc(size_t len);
int buf_free(char **buf);

#endif //MYCRAFT_DEMO_UTIL_H
