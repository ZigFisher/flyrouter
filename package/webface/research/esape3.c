/* -*- linux-c -*- */
/*
    This file is part of cacpd

    Copyright (C) 2005 Cyclades Corp.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/* Author: Oliver Kurth <oliver.kurth@cyclades.com> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <ctype.h>

static
int _hexval(int c){
	return ((c >= '0' && c <= '9') ? (c - '0') : (tolower(c) - 'a' + 10));
}

#define _hex_digit(c) (((c) < 10) ? ('0' + (c)) : ('a' + (c) - 10))

char *unescape_b(const char *esc_str, char *str)
{
	char *q;
	const char *p;

	p = esc_str;
	q = str;
	while(*p){
		if(*p == '+'){
			*q++ = ' ';
			p++;
		}else if(*p == '%'){
			int hh, hl;
			p++;
			if(*p && isxdigit(*p) && *(p+1) && isxdigit(*(p+1))){
				hh = _hexval(*p++);
				hl = _hexval(*p++);
				*q++ = (hh << 4) | hl;
			}else
				*q++ = '%';
		}else{
			*q++ = *p++;
		}
	}
	*q = 0;
	return str;
}

char *unescape(const char *esc_str)
{
	char *str;

	str = malloc(strlen(esc_str)+1);
	return unescape_b(esc_str, str);
}

char *escape(const char *str)
{
	char *esc_str;
	char esc_tmp[strlen(str)*3+1]; /* worst case */
	char *q;
	const unsigned char *p;

	p = (unsigned char *)str;
	q = esc_tmp;

	while(*p){
		if(*p == ' '){
			*q++ = '+'; p++;
		}else if((*p != '.') && (*p > ' ') && (*p != '+') && (*p != '%') && (*p < 128)){
			*q++ = *p++;
		}else{
			*q++ = '%';
			*q++ = _hex_digit((*p >> 4) & 0x0f);
			*q++ = _hex_digit(*p & 0x0f);
			p++;
		}
	}
	*q = 0;

	esc_str = strdup(esc_tmp);

	return  esc_str;
}
