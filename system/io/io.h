#ifndef SYSTEM_IO_IO_H_
#define SYSTEM_IO_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

extern const char *getStartPath(void);
extern void setStartPath(const char *executablePath);
extern int fileExists(const char *filename);
extern int fileModified(const char *filename, time_t *fileLastModifiedTime);
extern const char* getFilePath(const char *filename);
extern char *ioReadFileToBuffer(const char *file, unsigned int *count);
extern char* strtok_reentrant(char *str, const char *delim, char **nextp);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif /* SYSTEM_IO_IO_H_ */
