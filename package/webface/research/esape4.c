/* ----------------------------------------------------------------------- *
 *
 *  escape.c - escaping routines
 *
 *   Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* escape a string.  caller must free returned value. returns NULL on error */
char *escape(const char *s)
{
	static const char hex[] = {'0', '1', '2', '3', '4',
	                           '5', '6', '7', '8', '9',
	                           'a', 'b', 'c', 'd', 'e', 'f'};
	char *buf, *bufp;
	const char *p = s;

	buf = malloc((strlen(s) * 3) + 1);
	if (!buf)
		return NULL;
	bufp = buf;

	while (*p) {
		if (isalpha(*p)) {
			*bufp++ = *p++;
		} else {
			*bufp++ = '%';
			*bufp++ = hex[*p >> 4];
			*bufp++ = hex[*p++ & 0x0F];
		}
	}
	*bufp = '\0';
	return buf;
}

/* fills c with the hex representation of s[2].  returns 0 on success */
static char _unescape(char *c, const char *s)
{
	if ('0' <= *s && *s <= '9')
		*c = (*s - '0') << 4;
	else if ('a' <= *s && *s <= 'f')
		*c = (*s - 'a' + 10) << 4;
	else
		return -1;
	s++;
	if ('0' <= *s && *s <= '9')
		*c |= *s - '0';
	else if ('a' <= *s && *s <= 'f')
		*c |= (*s - 'a' + 10);
	else
		return -1;
	return 0;
}

/* unescape a string.  caller must free result.  returns NULL on error */
char *unescape(const char *s)
{
	char *buf, *bufp;
	const char *p = s;

	buf = malloc(strlen(s) + 1);
	if (!buf)
		return NULL;
	bufp = buf;

	while (*p) {
		if (*p == '%') {
			if (_unescape(bufp++, ++p)) {
				free(buf);
				return NULL;
			}
			p += 2;
		} else {
			*bufp++ = *p++;
		}
	}
	return buf;
}



