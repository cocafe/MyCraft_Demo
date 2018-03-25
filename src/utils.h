#ifndef MYCRAFT_DEMO_UTIL_H
#define MYCRAFT_DEMO_UTIL_H

#define FILEPATH_MAX_LEN                (1024)

#define SUFFIX_PNG                      ".png"
#define SUFFIX_BMP                      ".bmp"
#define SUFFIX_TGA                      ".tga"
#define SUFFIX_DDS                      ".dds"
#define SUFFIX_OBJ                      ".obj"
#define SUFFIX_GLSL                     ".glsl"

int memzero(void *ptr, size_t size);
char *file_read(const char *filepath);
char *buf_alloc(size_t len);
int buf_free(char **buf);

#endif //MYCRAFT_DEMO_UTIL_H
