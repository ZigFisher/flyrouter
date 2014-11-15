/* This code was written and is copyrighted 1996 by
 *
 *       Elmar Bartel
 *       Institut fuer Informatik
 *       Technische Universitaet Muenchen
 *       bartel@informatik.tu-muenchen.de
 *
 * Permission to use, copy, modify and distribute this software
 * and its documentation for any purpose, except making money, is
 * herby granted, provided that the above copyright notice and
 * this permission appears in all places, where this code is
 * referenced or used literally.
 */

/* This file is a partial copy of some other packages that contains
 * only those parts, that are used in nttcp
 * Full information about these pieces of software can be obtained
 * from the author:
 *       		bartel@informatik.tu-muenchen.de
 */

#ifndef NEW_INCLUDED
extern void *LastMallocResult;
#define	Record(x)	x
#define	New(type)	(type *)(Record(malloc(sizeof(type))))
#define	nNew(n,type)	(type *)(Record(malloc((n)*sizeof(type))))
#define nZero(p,n,type) memset(p, 0, (n)*sizeof(type))
#define	rNew(p,n,type)	(type *)(Record(realloc(p, (n)*sizeof(type))))
#define	Nil(type)	(type *)0

#if !defined(ERROR_LOG2)
#define	ERROR_LOG2(s, a, b)	fprintf(stderr, s, a, b)
#endif /*ERROR_LOG2*/

#define	Enlarge(p, t, Max, Failure) {	\
    t* NewP;				\
    int NewMax= (Max);			\
    NewMax+= ((NewMax)>>3) + 4;		\
    NewP= rNew(p, NewMax, t);		\
    if (NewP == Nil(t)) {		\
    	ERROR_LOG2("%s: no memory for %d " \
	#t "'s\n", myname, NewMax);	\
	Failure;			\
    }					\
    else {				\
	Max= NewMax;			\
	p= NewP;			\
    }					\
}
#endif /*NEW_INCLUDED*/


/* This code was written and is copyrighted by
 *
 *       Elmar Bartel
 *       Institut fuer Informatik
 *       Technische Universitaet Muenchen
 *       bartel@informatik.tu-muenchen.de
 *
 * Permission to use, copy, modify and distribute this software
 * and its documentation for any purpose except making money is
 * herby granted, provided that the above copyright notice and
 * this permission appear in all copies and supporting
 * documentation.
 *
 */
#ifndef LSTR_INCLUDED
#define LSTR_INCLUDED

#ifndef Nil
#define	Nil(type)	 (type *)0
#define New(type)	 (type *)malloc(sizeof(type))
#define nNew(n, type)	 (type *)malloc((n)*sizeof(type))
#define rNew(p, n, type) (type *)realloc(p,(n)*sizeof(type))
#endif /*Nil*/

typedef struct {
	int Leng;
	int MaxLeng;
	char *s;
} LenStr;
#define NilLenStr		(LenStr *)0
#define NewLenStr		(LenStr *)malloc(sizeof(LenStr))

void	LenStrDestroy		(LenStr *ls);
LenStr	*LenStrCreate		(int l);
LenStr	*LenStrcat		(LenStr *dls, char *s);
LenStr	*LenStrMake		(char *s);
LenStr	*LenStrncat		(LenStr *dls, char *s, int n);
char	*LenStrString		(LenStr *ls);
LenStr	*LenStrcpy		(LenStr *dvs, char *s);
LenStr	*LenStrPadRight		(LenStr *ls, char pad, int n);
LenStr	*LenStrApp		(LenStr *ls, char ch);
int	LenStrLeng		(LenStr *ls);

#endif /*LSTR_INCLUDED*/

#ifndef STRVEC_INCLUDED
#define STRVEC_INCLUDED

typedef struct {
	int Leng;
	int MaxLeng;
	char **String;
} StrVec;

StrVec	*StrVecCreate		();
void	StrVecDestroy		(StrVec *sa);
StrVec	*StrVecFromString	(char *s, char Sep);
StrVec	*StrVecJoin		(StrVec *sv, StrVec *sv1);
char	*StrVecToString		(StrVec *sv, char Sep);
StrVec	*StrVecAppend		(StrVec *sa, char *s);
StrVec	*StrVecFromArgv		(int argc, char *argv[]);

#endif /*STRVEC_INCLUDED*/

#if defined(ultrix)
#define strdup(x)		strcpy((char *)malloc(strlen(x)+1), x)
#endif /*ultrix*/
