/*
 *	fakevms.c
 *
 * Test-driver routines for VAX/VMS filename parsing.
 * Written by T.E.Dickey for vile (august 1994).
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/fakevms.c,v 1.2 1994/11/29 04:02:03 pgf Exp $
 */
#include <sys/types.h>
#include <sys/stat.h>

#define FAKEVMS_INTERN
#include "estruct.h"

#define	dotname(s)	(!strcmp(s,".") || !strcmp(s,".."))

#if OPT_VMS_PATH && SYS_UNIX

static	void	abspath P((char *));

static
void	abspath(path)
	char *path;
{
	struct stat sb;
	char save[NFILEN];
	char head[NFILEN];
	char leaf[NFILEN];
	char *s;

	getcwd(save, NFILEN);
	*leaf = EOS;
	if (*path != '/')
		pathcat(head, save, path);
	else
		strcpy(head, path);
	if (stat(head, &sb) < 0) {
		strcpy(leaf, s = pathleaf(head));
		*s = EOS;
		if (stat(head, &sb) < 0) {
			return;	/* give up */
		}
	}
	if ((sb.st_mode & S_IFMT) != S_IFDIR) {
		strcpy(leaf, s = pathleaf(head));
		*s = EOS;
	}

	chdir(head);
	getcwd(path, NFILEN);
	if (*leaf)
		pathcat(path, path, leaf);
	chdir(save);
}

/* this is used from 'lengthen_path()', and doesn't have to actually find a file */
char *fakevms_filename(path)
	char *path;
{
	char	save[NFILEN];
	char	temp[NFILEN];
	char	*s;

	if (is_vms_pathname(path, TRUE)) {
		TRACE(("..dir:  %s\n", path))
		(void) vms2unix_path(temp, path);
		if ((getcwd(save, NFILEN) != 0) && (chdir(temp) == 0)
		 && (getcwd(temp, NFILEN) != 0) && (chdir(save) == 0)) {
			(void)unix2vms_path(path, strcat(temp, "/"));
			TRACE(("....>> '%s'\n", path))
		 	return path;
		}
	} else if (is_vms_pathname(path, FALSE)) {
		TRACE(("..file: %s\n", path))
		vms2unix_path(temp, path);
		abspath(temp);
		mkupper(unix2vms_path(path, temp));
		if ((s = strstr(path, ".;")) != 0)
			*s = EOS;
		else if ((s = strchr(path, ';')) != 0)
			*s = EOS;
		TRACE(("....>> '%s'\n", path))
		return path;
	}
	return 0;
}

int fakevms_chdir(buf)
	char *buf;
{
	char	temp[NFILEN];

	TRACE(("chdir '%s'\n", buf))
	if (is_vms_pathname(buf, -1)) {
		buf = vms2unix_path(temp, buf);
	} else {
		buf = mklower(strcpy(temp, buf));
	}
	TRACE((" ->    (%s)\n", temp))
	return chdir(buf);
}

char *fakevms_getcwd(buf, size)
	char *buf;
	size_t size;
{
	char	temp[NFILEN];

	buf = getcwd(buf, size);
	if (buf != 0) {
		unix2vms_path(temp, strcat(buf, "/"));
		mkupper(strcpy(buf, temp));
	}
	TRACE(("getcwd => '%s'\n", buf))
	return buf;
}

FILE *fakevms_fopen(path, mode)
	char *path;
	char *mode;
{
	char	temp[NFILEN];

	TRACE(("fopen(%s, %s)\n", path, mode))
	if (is_vms_pathname(path, 0)) {
		path = vms2unix_path(temp, path);
		TRACE((" ->    (%s)\n", temp))
	}
	return fopen(path, mode);
}

static FILE *opendir_pipe;
static char *opendir_path;

DIR *fakevms_opendir(path)
	const char *path;
{
	struct stat sb;
	char	*s;
	char	temp[NFILEN];
	char	command[NSTRING];

	TRACE(("opendir(%s)\n", path))
	if (is_vms_pathname((char *)path, TRUE)) {
		abspath(vms2unix_path(temp, path));
	} else if (is_vms_pathname((char *)path, FALSE)) {
		(void)strcpy(temp, path);
		if ((s = strrchr(temp, ';')) != 0)
			*s = EOS;
		abspath(vms2unix_path(temp, temp));
	} else {
		mklower(strcpy(temp, path));
	}

	if (temp[strlen(temp)-1] == '/')
		temp[strlen(temp)-1] = EOS;
	FreeIfNeeded(opendir_path);
	opendir_path = strmalloc(temp);

	TRACE((" ->    (%s)\n", temp))

	/* Question-marks are safer than dollar signs when the shell is
	 * interpreting them.
	 */
	for (s = temp; *s != EOS; s++) {
		if (*s == '$')
			*s = '?';
	}

	/* Use a pipe to get the list, since we're relying on the VMS
	 * directory-scanner to expand wildcards.
	 */
	if (stat(temp, &sb) < 0
	 || (sb.st_mode & S_IFMT) != S_IFDIR) {
		(void)sprintf(command, "/bin/ls -1ad %s 2>/dev/null", temp);
	} else {
		(void)sprintf(command, "/bin/ls -1a %s", temp);
	}
	TRACE((" ->    %% %s\n", command))
	if ((opendir_pipe = popen(command, "r")) != 0)
		return (DIR *)1;
	return 0;
}

DIRENT *fakevms_readdir(dp)
	DIR *dp;
{
	static	struct {
		long		d_ino;
		off_t		d_off;
		unsigned short	d_reclen;
		char		d_name[NFILEN];
	} fake_de;
	char	temp[NFILEN];

loop:
	if (fgets(temp, sizeof(temp), opendir_pipe) != 0) {
		char *leaf = pathleaf(temp);
		struct stat sb;

		leaf[strlen(leaf)-1] = EOS;	/* trim newline */
		if (dotname(leaf))
			goto loop;

		if (*temp != '/')
			pathcat(temp, opendir_path, temp);

		if (lstat(temp, &sb) < 0)
			goto loop;
		if ((sb.st_mode & S_IFMT) == S_IFLNK)
			goto loop;	/* symbolic links confuse me */

		TRACE(("readdir '%s'\n", temp))
		abspath(temp);
		if ((sb.st_mode & S_IFMT) == S_IFDIR)
		 	(void)strcat(temp, "/");
		unix2vms_path(temp, temp);
		if (is_vms_pathname(temp, TRUE))
			strcat(strcpy(temp, vms_path2dir(temp)), ";");
		strcat(mkupper(temp), "1");	/* version */

		fake_de.d_ino = sb.st_ino;
		strcpy(fake_de.d_name, temp);
		fake_de.d_reclen = strlen(temp);

		TRACE(("     => '%s'\n", temp))
		return (DIRENT *)&fake_de;
	}
	return 0;
}

int	fakevms_closedir(dp)
	DIR	*dp;
{
	if (opendir_pipe != 0) {
		pclose(opendir_pipe);
		opendir_pipe = 0;
		return 0;
	}
	return -1;
}

int	fakevms_stat(path, sb)
	char *path;
	struct stat *sb;
{
	char	temp[NFILEN];

	TRACE(("stat(%s, %p)\n", path, sb))
	if (is_vms_pathname(path, -1)) {
		path = vms2unix_path(temp, path);
		TRACE((" ->    (%s)\n", temp))
	}
	return stat(path, sb);
}
#endif	/* OPT_VMS_PATH && SYS_UNIX */
