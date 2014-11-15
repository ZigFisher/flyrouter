#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/reboot.h>

#include <stdio.h>
#include <wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <string.h>
#include <dirent.h>
#include "bbutils.h"
#include "utils.h"
/*---------------------------------------------------------------------------*/
/*                                  BUSYBOX                                  */
/*---------------------------------------------------------------------------*/

// Die with an error message if we can't malloc() enough space and do an
// sprintf() into that space.
char* xasprintf(const char *format, ...)
{
	va_list p;
	int r;
	char *string_ptr;

#if 1
	// GNU extension
	va_start(p, format);
	r = vasprintf(&string_ptr, format, p);
	va_end(p);
#else
	// Bloat for systems that haven't got the GNU extension.
	va_start(p, format);
	r = vsnprintf(NULL, 0, format, p);
	va_end(p);
	string_ptr = xmalloc(r+1);
	va_start(p, format);
	r = vsnprintf(string_ptr, r+1, format, p);
	va_end(p);
#endif

	if (r < 0)
		perror("sprintf");
	return string_ptr;
}

/* Find out if the last character of a string matches the one given.
 * Don't underrun the buffer if the string length is 0.
 */
char* last_char_is(const char *s, int c)
{
	if (s && *s) {
		size_t sz = strlen(s) - 1;
		s += sz;
		if ( (unsigned char)*s == c)
			return (char*)s;
	}
	return NULL;
}

char* concat_path_file(const char *path, const char *filename)
{
	char *lc;

	if (!path)
		path = "";
	lc = last_char_is(path, '/');
	while (*filename == '/')
		filename++;
	return xasprintf("%s%s%s", path, (lc==NULL ? "/" : ""), filename);
}

char* concat_subpath_file(const char *path, const char *f)
{
	if (f && DOT_OR_DOTDOT(f))
		return NULL;
	return concat_path_file(path, f);
}

/*
 * Walk down all the directories under the specified
 * location, and do something (something specified
 * by the fileAction and dirAction function pointers).
 *
 * Unfortunately, while nftw(3) could replace this and reduce
 * code size a bit, nftw() wasn't supported before GNU libc 2.1,
 * and so isn't sufficiently portable to take over since glibc2.1
 * is so stinking huge.
 */

static int true_action(const char *fileName,
		struct stat *statbuf,
		void* userData,
		int depth)
{
	return TRUE;
}

/* fileAction return value of 0 on any file in directory will make
 * recursive_action() return 0, but it doesn't stop directory traversal
 * (fileAction/dirAction will be called on each file).
 *
 * If !ACTION_RECURSE, dirAction is called on the directory and its
 * return value is returned from recursive_action(). No recursion.
 *
 * If ACTION_RECURSE, recursive_action() is called on each directory.
 * If any one of these calls returns 0, current recursive_action() returns 0.
 *
 * If ACTION_DEPTHFIRST, dirAction is called after recurse.
 * If it returns 0, the warning is printed and recursive_action() returns 0.
 *
 * If !ACTION_DEPTHFIRST, dirAction is called before we recurse.
 * Return value of 0 (FALSE) or 2 (SKIP) prevents recursion
 * into that directory, instead recursive_action() returns 0 (if FALSE)
 * or 1 (if SKIP)
 *
 * followLinks=0/1 differs mainly in handling of links to dirs.
 * 0: lstat(statbuf). Calls fileAction on link name even if points to dir.
 * 1: stat(statbuf). Calls dirAction and optionally recurse on link to dir.
 */

int recursive_action(const char *fileName,
		unsigned flags,
		int (*fileAction)(const char *fileName, struct stat *statbuf, void* userData, int depth),
		int (*dirAction)(const char *fileName, struct stat *statbuf, void* userData, int depth),
		void* userData,
		unsigned depth)
{
	struct stat statbuf;
	int status;
	DIR *dir;
	struct dirent *next;

	if (!fileAction) fileAction = true_action;
	if (!dirAction) dirAction = true_action;

	status = ACTION_FOLLOWLINKS; /* hijack a variable for bitmask... */
	if (!depth)
		status = ACTION_FOLLOWLINKS | ACTION_FOLLOWLINKS_L0;
	status = ((flags & status) ? stat : lstat)(fileName, &statbuf);
	if (status < 0) {
#ifdef DEBUG_RECURS_ACTION
		dbg("status=%d flags=%x", status, flags);
#endif
		goto done_nak_warn;
	}

	/* If S_ISLNK(m), then we know that !S_ISDIR(m).
	 * Then we can skip checking first part: if it is true, then
	 * (!dir) is also true! */
	if ( /* (!(flags & ACTION_FOLLOWLINKS) && S_ISLNK(statbuf.st_mode)) || */
	 !S_ISDIR(statbuf.st_mode)
	) {
		return fileAction(fileName, &statbuf, userData, depth);
	}

	/* It's a directory (or a link to one, and followLinks is set) */

	if (!(flags & ACTION_RECURSE)) {
		return dirAction(fileName, &statbuf, userData, depth);
	}

	if (!(flags & ACTION_DEPTHFIRST)) {
		status = dirAction(fileName, &statbuf, userData, depth);
		if (!status)
			goto done_nak_warn;
		if (status == SKIP)
			return TRUE;
	}

	dir = opendir(fileName);
	if (!dir) {
		/* findutils-4.1.20 reports this */
		/* (i.e. it doesn't silently return with exit code 1) */
		/* To trigger: "find -exec rm -rf {} \;" */
		goto done_nak_warn;
	}
	status = 1;
	while ((next = readdir(dir)) != NULL) {
		char *nextFile;

		nextFile = concat_subpath_file(fileName, next->d_name);
		if (nextFile == NULL)
			continue;
		/* process every file (NB: ACTION_RECURSE is set in flags) */
		if (!recursive_action(nextFile, flags, fileAction, dirAction,
						userData, depth + 1))
			status = FALSE;
//		s = recursive_action(nextFile, flags, fileAction, dirAction,
//						userData, depth + 1);
		free(nextFile);
//#define RECURSE_RESULT_ABORT 3
//		if (s == RECURSE_RESULT_ABORT) {
//			closedir(dir);
//			return s;
//		}
//		if (s == FALSE)
//			status = FALSE;
	}
	closedir(dir);

	if (flags & ACTION_DEPTHFIRST) {
		if (!dirAction(fileName, &statbuf, userData, depth))
			goto done_nak_warn;
	}

	return status;

 done_nak_warn:
	if (!(flags & ACTION_QUIET))
		perror(fileName);
	return FALSE;
}

/*---------------------------------------------------------------------------*/
/*                           END OF BUSYBOX                                  */
/*---------------------------------------------------------------------------*/

