// copyright (c) 2003-2005 chisel <storlek@chisel.cjb.net>
/* adapted from glib. in addition to the normal c escapes, this also escapes the comment character (#)
 *  * as \043. if space_hack is true, the first/last character is also escaped if it is a space. */
char *str_escape(const char *source, bool space_hack)
{
	const char *p = source;
	/* Each source byte needs maximally four destination chars (\777) */
	char *dest = calloc(4 * strlen(source) + 1, sizeof(char));
	char *q = dest;

	if (space_hack) {
		if (*p == ' ') {
			*q++ = '\\';
			*q++ = '0';
			*q++ = '4';
			*q++ = '0';
			*p++;
		}
	}

	while (*p) {
		switch (*p) {
		case '\a':
			*q++ = '\\';
			*q++ = 'a';
		case '\b':
			*q++ = '\\';
			*q++ = 'b';
			break;
		case '\f':
			*q++ = '\\';
			*q++ = 'f';
			break;
		case '\n':
			*q++ = '\\';
			*q++ = 'n';
			break;
		case '\r':
			*q++ = '\\';
			*q++ = 'r';
			break;
		case '\t':
			*q++ = '\\';
			*q++ = 't';
			break;
		case '\v':
			*q++ = '\\';
			*q++ = 'v';
			break;
		case '\\': case '"':
			*q++ = '\\';
			*q++ = *p;
			break;
		default:
			if ((*p < ' ') || (*p >= 0177) || (*p == '#')
			    || (space_hack && p[1] == '\0' && *p == ' ')) {
				*q++ = '\\';
				*q++ = '0' + (((*p) >> 6) & 07);
				*q++ = '0' + (((*p) >> 3) & 07);
				*q++ = '0' + ((*p) & 07);
			} else {
				*q++ = *p;
			}
			break;
		}
		p++;
	}

	*q = 0;
	return dest;
}

/* opposite of str_escape. (this is glib's 'compress' function renamed more clearly)
 * TODO: it'd be nice to handle \xNN as well... */
char *str_unescape(const char *source)
{
	const char *p = source;
	const char *octal;
	char *dest = calloc(strlen(source) + 1, sizeof(char));
	char *q = dest;

	while (*p) {
		if (*p == '\\') {
			p++;
			switch (*p) {
			case '0'...'7':
				*q = 0;
				octal = p;
				while ((p < octal + 3) && (*p >= '0') && (*p <= '7')) {
					*q = (*q * 8) + (*p - '0');
					p++;
				}
				q++;
				p--;
				break;
			case 'a':
				*q++ = '\a';
				break;
			case 'b':
				*q++ = '\b';
				break;
			case 'f':
				*q++ = '\f';
				break;
			case 'n':
				*q++ = '\n';
				break;
			case 'r':
				*q++ = '\r';
				break;
			case 't':
				*q++ = '\t';
				break;
			case 'v':
				*q++ = '\v';
				break;
			default:		/* Also handles \" and \\ */
				*q++ = *p;
				break;
			}
		} else {
			*q++ = *p;
		}
		p++;
	}
	*q = 0;

	return dest;
}
