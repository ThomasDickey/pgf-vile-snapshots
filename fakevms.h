/* include this _after_ the standard definitions, e.g., <stdlib.h> */

/*
 * $Header: /usr/build/VCS/pgf-vile/RCS/fakevms.h,v 1.4 1996/02/26 04:24:35 pgf Exp $
 */

#ifndef FAKE_VMS_H
#define FAKE_VMS_H

#ifndef FAKEVMS_INTERN

#undef  access
#define access  fakevms_access

#undef  chdir
#define chdir   fakevms_chdir

#undef  getcwd
#define getcwd  fakevms_getcwd

#undef  fopen 
#define fopen   fakevms_fopen

#undef  opendir
#define opendir fakevms_opendir

#undef  readdir
#define readdir fakevms_readdir

#undef  closedir
#define closedir fakevms_closedir

#endif

#include <sys/stat.h>

#define __select MySelect
#include "dirstuff.h"
#undef __select

int fakevms_access (const char *buf, int mode);

int fakevms_chdir (char *buf);

char *fakevms_filename (char *path);

FILE *fakevms_fopen (const char *buf, char *mode);

char *fakevms_getcwd (char *buf, size_t size);

DIR *fakevms_opendir (const char *path);

struct dirent *fakevms_readdir (DIR *dp);

int fakevms_closedir (DIR *dp);

int fakevms_stat (char *path, struct stat *sb);

#endif /*FAKE_VMS_H*/
