/* include this _after_ the standard definitions, e.g., <stdlib.h> */

/*
 * $Header: /usr/build/VCS/pgf-vile/RCS/fakevms.h,v 1.1 1994/10/03 13:23:51 pgf Exp $
 */

#ifndef FAKE_VMS_H
#define FAKE_VMS_H

#ifndef FAKEVMS_INTERN

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

int fakevms_chdir P((char *buf));

char *fakevms_filename P((char *path));

FILE *fakevms_fopen P((char *buf, char *mode));

char *fakevms_getcwd P((char *buf, size_t size));

DIR *fakevms_opendir P((const char *path));

struct dirent *fakevms_readdir P((DIR *dp));

int fakevms_closedir P((DIR *dp));

int fakevms_stat P((char *path, struct stat *sb));

#endif /*FAKE_VMS_H*/
