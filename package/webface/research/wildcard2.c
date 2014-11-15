/*
 * Copyright (c) 1997, 98, 2000, 01  
 *    Motoyuki Kasahara
 *    ndtpd-3.1.5
 */

/*
 * Do wildcard pattern matching.
 * In the pattern, the following characters have special meaning.
 * 
 *   `*'    matches any sequence of zero or more characters.
 *   '\x'   a character following a backslash is taken literally.
 *          (e.g. '\*' means an asterisk itself.)
 *
 * If `pattern' matches to `string', 1 is returned.  Otherwise 0 is
 * returned.
 */
int
match_wildcard(pattern, string)
    const char *pattern;
    const char *string;
{
    const char *pattern_p = pattern;
    const char *string_p = string;

    while (*pattern_p != '\0') {
	if (*pattern_p == '*') {
	    pattern_p++;
	    if (*pattern_p == '\0')
		return 1;
	    while (*string_p != '\0') {
		if (*string_p == *pattern_p
		    && match_wildcard(pattern_p, string_p))
		    return 1;
		string_p++;
	    }
	    return 0;
	} else {
	    if (*pattern_p == '\\' && *(pattern_p + 1) != '\0')
		pattern_p++;
	    if (*pattern_p != *string_p)
		return 0;
	}
	pattern_p++;
	string_p++;
    }

    return (*string_p == '\0');
}


