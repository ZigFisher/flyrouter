	

/*
 * match.c
 * action matching and marker search routines
 *
 * Copyright (c) 2006 Kalev Soikonen. All rights reserved.
 * Distributed under the terms of BSD license. See the file COPYING.
 */

/*
 * NOTE: wildcards match non-empty strings. Greediness varies.
 * NOTE: when we hardcode ctypes to LATIN1, pcre/regex are still
 * matching in default "C" locale, i.e. plain ASCII ctypes.
 * TODO: maybe support flags, such as /RE/i case-insensitive.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WITH_REGEX
#include <sys/types.h>
#include <regex.h>
#endif

#ifdef WITH_PCRE
#include <pcre.h>
#endif

#include "cancan.h"
#include "list.h"
#include "term.h"
#include "parse.h"
#include "ctypes.h"
#include "match.h"

#if defined(WITH_REGEX)
static void convert_pattern(char *buf, char *pat);
static void get_match_params(char *str, pstring param[], regmatch_t *match);
#endif

#if defined(WITH_PCRE)
static void convert_pattern(char *buf, char *pat);
static void get_match_params(char *str, pstring param[], int *ovector, int n);
#endif

#if !defined(WITH_REGEX) && !defined(WITH_PCRE)
static void *compile_pattern(char *buf, char *pat);
static int match_simple(void *prog, char *str, pstring param[]);
#endif

/* line caching (to reduce search overhead) */
static void lmcache_update(lmatchnode *lm, char *str, int len);

static unsigned int lmcache_valid = 0;	/* age of oldest valid entry */
static unsigned int lmcache_cycle = 0;	/* age is relative to this */
#define LMC_PURGE	HASH_BUCKETS	/* entries kept (not exact) */
#define LMC_REFRESH	(LMC_PURGE/2)	/* hits: no update unless aged */

static void run_action(char *commands, pstring param[]);

/* marker matching */
static void pm_study(char *str, int len);
static void pm_adjust(char *str, int len, int spos);
static int pm_search(char *str, int len, int spos, marknode **match);

static unsigned char pm_delta[LINELEN];
static unsigned char pm_alpha[256];


#if defined(WITH_REGEX) || defined(WITH_PCRE)
/* #define rx_metachar(c) strchr("\\*+?{^.$|()[", (c)) */
#define rx_metachar(c) !isword(c)

/*
 * convert ""-pattern into ERE.
 */
static void convert_pattern(char *buf, char *pat)
{
	char *d = buf, *s = pat + 1;
	if (*s == '^')
		*d++ = *s++;
	for (; *s && *s != '"'; s++)
		if (*s == WILD_CHAR) {
			d += str_copy(d, "(.+)");
		} else {
			if (*s == '\\' && !*++s)
				break;
			if (rx_metachar(*s))
				*d++ = '\\';
			*d++ = *s;
		}
	*d = '\0';
}

/*
 * assign action pattern (which is passed along with delimiters).
 * return 1 for success, or print error msg and return 0.
 */
int set_action_pattern(actionnode *np, char *str)
{
	char buf[4 * LINELEN];
	void *preg = NULL;

	if (str) {				/* compile a new regex */
		if (*str != '"') {
			/* copy the pattern, but remove delimiters */
			int pl = str_copy(buf, str + 1);
			buf[pl - 1] = '\0';
		} else
			convert_pattern(buf, str);

#ifdef WITH_REGEX
		do {
			regex_t r;
			int err = regcomp(&r, buf, REG_EXTENDED);
			if (err) {
				regerror(err, &r, buf, sizeof buf);
				regfree(&r);
				printf("#regular expression: %s\n", buf);
				return 0;
			}
			preg = mem_dup(&r, sizeof r);
		} while (0);
#endif
#ifdef WITH_PCRE
		do {
			static const unsigned char *ctables = NULL;
			const char *errmsg; int erroff;
			if (!ctables)
				ctables = pcre_maketables();
			preg = pcre_compile(buf, PCRE_DOTALL,
						&errmsg, &erroff, ctables);
			if (!preg) {
				printf("#regular expression: %s\n", errmsg);
				return 0;
			}
		} while (0);
#endif
	}
	if (np->preg) {				/* free the old one */
#ifdef WITH_REGEX
		regfree((regex_t *)np->preg);
#endif
		free(np->preg);
	}
	np->preg = preg;
	str_assign(&np->pattern, str);
	return 1;
}

#ifdef WITH_REGEX
static void get_match_params(char *str, pstring param[], regmatch_t *match)
{
	pstring *pp = param;
	regmatch_t *m = match;
	int i;
	for (i = 10; i; m++, pp++, i--) {
		int l = 0;
		char *s = NULL;
		if (m->rm_so >= 0) {
			l = m->rm_eo - m->rm_so;
			s = str + m->rm_so;
		}
		pp->len = l;
		pp->str = s;
	}
}
#endif

#ifdef WITH_PCRE
static void get_match_params(char *str, pstring param[], int *ovector, int n)
{
	pstring *pp = param;
	int *m = ovector;
	int i;
	for (i = 10; i; m += 2, pp++, i--) {
		int l = 0;
		char *s = NULL;
		if (--n >= 0 && m[0] >= 0) {
			l = m[1] - m[0];
			s = str + m[0];
		}
		pp->len = l;
		pp->str = s;
	}
}
#endif
#endif	/* defined(WITH_REGEX) || defined(WITH_PCRE) */


#if !defined(WITH_REGEX) && !defined(WITH_PCRE)
/*
 * compiled "program" has the following structure:
 * first char is anchor flag (boolean), then any number of
 * < prm, string... > tuples follow. prm is a wildcard flag,
 * and string the usual zero terminated. program ends with
 * < 0, "" > i.e. two zero chars.
 */
static void *compile_pattern(char *buf, char *pat)
{
	char *d = buf, *s = pat + 1;
	s += *d++ = (*s == '^');		/* store anchor flag */
	s += *d++ = (*s == WILD_CHAR);		/* store wildcard flag */
	for (; *s && *s != '"'; s++)
		if (*s == WILD_CHAR) {
			*d++ = '\0';		/* zero term. */
			*d++ = 1;		/* wildcard */
		} else {
			if (*s == '\\' && !*++s)
				break;
			*d++ = *s;
		}
	*d++ = '\0';				/* zero term. */
	*d++ = 0; *d++ = '\0';			/* the end */
	return mem_dup(buf, d - buf);
}

/*
 * assign action pattern (which is passed along with delimiters).
 * return 1 for success, or print error msg and return 0.
 */
int set_action_pattern(actionnode *np, char *str)
{
	char buf[4 * LINELEN];
	void *preg = NULL;

	if (str) {
		if (*str != '"') {
			printf("#only \"\"-patterns supported in this cancan\n");
			return 0;
		} else
			preg = compile_pattern(buf, str);
	}
	if (np->preg)
		free(np->preg);
	np->preg = preg;
	str_assign(&np->pattern, str);
	return 1;
}

/*
 * pattern match with wildcards. return negative if match fails.
 *
 * wildcards are handled in a non-greedy manner i.e. they match
 * shortest substring, except at pattern end.
 */
static int match_simple(void *prog, char *str, pstring param[])
{
	register char *p = prog, *s = str;
	int prm = 0;
	if (*p++ && !*p) {			/* anchored, no wildcard */
		for (p++; *p; s++, p++)
			if (*s != *p)
				return -1;
		p++;
	}
	for (;;) {
		char *s0 = s;
		if (*p++) {			/* a wildcard? */
			if (!*s)
				return -1;
			s++;
			prm++;
		} else if (!*p)			/* program end--stop */
			break;
		if (*p) {
			s = strstr(s, p);
			if (!s)
				return -1;
		} else if (!*(p + 1))		/* wildcard at the end */
			while (*s) s++;
		if (prm < 10) {			/* set parameter */
			param[prm].len = s - s0;
			param[prm].str = s0;
		}
		while (*p) p++, s++;		/* skip the subpattern */
		p++;
	}
	while (++prm < 10) {			/* clear unused params */
		param[prm].len = 0;
		param[prm].str = NULL;
	}
	return 0;
}
#endif	/* !defined(WITH_REGEX) && !defined(WITH_PCRE) */


/*
 * a pattern has been changed or new action added, so we invalidate
 * all cached negative entries (deleting an action, or toggling it
 * on/off, has no effect in our caching scheme).
 */
void reset_matcher(void)
{
	lmcache_valid = 0;
}

/*
 * keep a cache of lines that no action could match
 * NOTE: we fiddle with the hash structure directly here.
 * Every refresh is coupled with a purge cycle, in order
 * to enforce an upper limit on the size of the cache.
 * TODO: maybe flush the cache when no actions remain?
 * The results of marker search could be cached too...
 */
static void lmcache_update(lmatchnode *lm, char *str, int len)
{
	lmatchnode **np, *p;
	unsigned int cycle;

	if (!lm)
		lm = insert_linehash(str, len);

	lm->cycle = cycle = (++lmcache_valid, ++lmcache_cycle);

	/* purge aged cache entries (one bucket at a time) */
	np = (lmatchnode **)h__lines + cycle % HASH_BUCKETS;
	while ((p = *np)) {
		if (p->commands || (cycle - p->cycle) < LMC_PURGE) {
			np = &p->chain;
		} else {
			*np = p->chain;
			free_node(p);
		}
	}
}

/*
 * search for actions to trigger on a received line.
 * return true if an action is run.
 * TODO: lmatches should pass params, too. (param 0)
 */
int search_action(char *line, int len)
{
	pstring param[10];
	actionnode *p;
	lmatchnode *lm = lookup_linehash(line, len);
	if (lm && lm->commands) {
		run_action(lm->commands, NULL);
		return 1;
	}
	if (lm && (lmcache_cycle - lm->cycle) < lmcache_valid) {
#ifdef LMC_REFRESH
		if ((lmcache_cycle - lm->cycle) < LMC_REFRESH)
			return 0;
		lmcache_update(lm, line, len);
#endif
		return 0;
	}

	if (!actions)
		return 0;

	for (p = actions; p; p = p->next) {
#ifdef WITH_REGEX
		regmatch_t match[10];
		if (regexec((regex_t *)p->preg, line, 10, match, 0))
			continue;
		get_match_params(line, param, match);
#endif
#ifdef WITH_PCRE
		int ovector[30];
		int n = pcre_exec((pcre *)p->preg, NULL,
					line, len, 0, 0, ovector, 30);
		if (n < 0)
			continue;
		get_match_params(line, param, ovector, (n ? n : 10));
#endif
#if !defined(WITH_REGEX) && !defined(WITH_PCRE)
		if (match_simple(p->preg, line, param) < 0)
			continue;
#endif
		if (!p->active)
			return 0;
		param->len = len;
		param->str = line;
		run_action(p->commands, param);
		return 1;
	}

	lmcache_update(lm, line, len);
	return 0;
}

/*
 * execute an action.
 */
static void run_action(char *commands, pstring *param)
{
	clear_prompt();
	parse_line(commands, param, CMD_ACTION);
}


void run_hook(char *name)
{
	aliasnode *np = lookup_alias(name);
	if (np)
		parse_line(name, NULL, CMD_ACTION);
}

void set_hook(char *name, char *cmd)
{
	aliasnode *np = lookup_alias(name);
	if (cmd && np) {
		str_assign(&np->subst, cmd);
	} else if (cmd) {
		add_aliasnode(name, cmd);
	} else if (np) {
		delete_aliasnode(np);
	}
}


/*
 * NOTE: in anticipation of trying several patterns against the text,
 * some preprocessing is done on the latter. (hashing, sort of).
 * alpha[c] is an index (mod 256) to first char c in text, or anything
 * at all if no occurrences are found. (256 - delta[pos]) is the number
 * of characters to skip forward when match at pos fails.
 * NOTE: could optimize with word boundary test in pm_study/pm_adjust.
 */
static void pm_study(char *str, int len)
{
	register char *s = str;
	register int i = len;
	while (--i >= 0) {
		register int c = s[i] & 0xff;
		pm_delta[i] = i - pm_alpha[c];
		pm_alpha[c] = i;
	}
}

/*
 * to start a search from a further position, we "unstudy" the range
 * from old spos through new spos (len). This modifies the alpha[].
 */
static void pm_adjust(char *str, int len, int spos)
{
	register char *s = str;
	register int i = spos;
	for (; i < len; i++) {
		register int c = s[i] & 0xff;
		pm_alpha[c] -= pm_delta[i];
	}
}

#define WBOUND(str, len, a, b) \
		!((a && isword((str)[-1])) || (b && isword((str)[len])))

/*
 * search for marks in a string str of length len, starting at spos.
 * return the position of match (or -1 for none), and fill in *match.
 * NOTE: the word boundary tests aren't quite right. We just check that
 * surrounding chars (including str[len]) are !isword.
 */
static int pm_search(char *str, int len, int spos, marknode **match)
{
	marknode *np;
	int mpos = -1;

	for (np = markers; np; np = np->next) {
		int i, scan = len - np->plen;
		if (spos > scan)
			continue;

		i = pm_alpha[np->pattern[0] & 0xff];
		i = spos + ((i - spos) & 0xff);

		if ((unsigned)scan >= (unsigned)mpos) {
			/* don't scan past previous match position */
			scan = mpos;
			/* only longer pattern is tried at prev. mpos */
			if (np->plen <= (*match)->plen) scan--;
		}

		for (; i <= scan; i += 0x100) {
			register char *s = str + i, *p = np->pattern;
			if (*s != *p)
				continue;
			if (WBOUND(s, np->plen, (i > 0), 1)) {
				do {
					p++, s++;
				} while (*p && *s == *p);
				if (!*p) {		/* a match! */
					*match = np;
					mpos = i;
					break;
				}
			}
			i -= pm_delta[i] & 0xff;
		}
	}
	return mpos;
}

/*
 * print a line with proper attributes
 * len must be < LINELEN.
 */
void mark_and_print(char *str, int len)
{
	if (markers) {
		int spos, mpos, again;
		marknode *match;
		pm_study(str, len);
		for (spos = 0;; spos = again) {
			mpos = pm_search(str, len, spos, &match);
			if (mpos - spos < 0)		/* no match */
				break;

			wrap_out(str + spos, mpos - spos, NOATTRCODE);
			wrap_out(str + mpos, match->plen, match->attrcode);

			again = mpos + match->plen;
			if (again >= len)
				return;
			pm_adjust(str, again, spos);
		}
		str += spos;
		len -= spos;
	}
	wrap_out(str, len, NOATTRCODE);
}

