/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef dirent_h_DEFINED
#define dirent_h_DEFINED

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <sys/types.h>

struct dirent {
    size_t _reserved1;
    char d_name[13];
    unsigned long _reserved2;
};

struct DIR;
typedef struct DIR DIR;

DIR *opendir(const char *path);
int closedir(DIR *dirp);
struct dirent *readdir(DIR *dirp);
void rewinddir(DIR *dirp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* dirent_h_DEFINED */
