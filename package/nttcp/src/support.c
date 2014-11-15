/* This code was written and is copyrighted 1994,1995,1996 by
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

/* This file is a partial copy from files of other packages.
 * Full information about these other packes can be obtained
 * from the author:
 *       		bartel@informatik.tu-muenchen.de
 */
#include <string.h>
#if defined(FreeBSD)
#include <stdlib.h>
#else
#include <malloc.h>
#endif /*FreeBSD*/
#include "support.h"

/* Pad the string to size from right with character pad.
 * CAUTION: No check is done to ensure there is room for the
 *          additional characters.
 */
char *strrpad(char *s, char pad, int size) {
    int i= strlen(s);
    while (i<size) {
    	s[i]= pad;
	i++;
    }
    s[i]= '\0';
    return s;
}
int LenStrLeng(LenStr *ls) {
    return ls->Leng;
}
LenStr *LenStrApp(LenStr *ls, char ch) {
    if (ls->Leng == ls->MaxLeng) {
	ls->MaxLeng+= (ls->MaxLeng >> 3) + 1;
	ls->s= rNew(ls->s, ls->MaxLeng+1, char);
    }
    ls->s[ls->Leng++]= ch;
    ls->s[ls->Leng]= '\0';
    return ls;
}
LenStr *LenStrEnsureSpace(LenStr *ls, int n) {
    if (ls->MaxLeng-ls->Leng < n) {
    	char *ns= rNew(ls->s, n+1, char);
	if (ns == Nil(char))
	    return Nil(LenStr);
	else {
	    ls->s= ns;
	    return ls;
	}
    }
    else
	return ls;
}
LenStr *LenStrPadRight(LenStr *ls, char c, int n) {
    if (ls->Leng >= n)
    	return ls;
    ls= LenStrEnsureSpace(ls, n);
    strrpad(ls->s, c, n);
    ls->Leng= n;
    return ls;
}
LenStr *LenStrcpy(LenStr *dvs, char *s) {
    int l= strlen(s);
    if (l >= dvs->MaxLeng) {
	dvs->s= rNew(dvs->s, l+1, char);
	dvs->MaxLeng= l;
    }
    strcpy(dvs->s, s);
    dvs->Leng= l;
    return dvs;
}
char *LenStrString(LenStr *ls) {
    return ls->s;
}
LenStr *LenStrncat(LenStr *dls, char *s, int n) {
    int l= strlen(s);
    if (l < n)
    	n= l;
    if (n+dls->Leng >= dls->MaxLeng) {
	dls->MaxLeng= (dls->Leng + l); 
	dls->s= rNew(dls->s, dls->MaxLeng+1, char);
    }
    strncat(dls->s+dls->Leng, s, n);
    dls->Leng+= n;
    dls->s[dls->Leng]= '\0';
    return dls;
}
LenStr *LenStrMake(char *s) {
    LenStr *vs= NewLenStr;
    vs->MaxLeng= vs->Leng= strlen(s);
    vs->s= (char *)malloc(vs->Leng+1);
    strcpy(vs->s, s);
    return vs;
}
LenStr *LenStrcat(LenStr *dls, char *s) {
    int l= strlen(s);
    if (l+dls->Leng >= dls->MaxLeng) {
	dls->MaxLeng= (dls->Leng + l); 
	dls->s= rNew(dls->s, dls->MaxLeng+1, char);
    }
    strcat(dls->s+dls->Leng, s);
    dls->Leng+= l;
    return dls;
}
LenStr *LenStrCreate(int l) {
    LenStr *vs= New(LenStr);
    vs->Leng= 0;
    vs->MaxLeng= l;
    vs->s= (char *)malloc(l+1);
    vs->s[0]= '\0';
    return vs;
}
void LenStrDestroy(LenStr *ls) {
    if (ls->s)
	free(ls->s);
    free(ls);
    return;
}

StrVec *StrVecFromArgv(int argc, char *argv[]) {
    StrVec *sa= New(StrVec);
    int i;

    if (sa == Nil(StrVec))
    	return Nil(StrVec);
    if ((sa->String=nNew(argc, char *)) == Nil(char*)) {
    	free(sa);
    	return Nil(StrVec);
    }
    sa->Leng= 0;
    sa->MaxLeng= argc;
    for (i= 0; i<argc; i++) {
    	StrVec *nsa= StrVecAppend(sa, argv[i]);
	if (nsa == Nil(StrVec)) {
	    StrVecDestroy(sa);
	    return Nil(StrVec);
	}
	else
	    sa= nsa;
    }
    return sa;
}
StrVec *StrVecExpand(StrVec *sv, int n) {
    if (n > sv->MaxLeng) {
	char **ss;
	if (sv->String==Nil(char *))
	    ss= nNew(n, char*);
	else
	    ss= rNew(sv->String, n, char*);
	if (ss == Nil(char *))
	    return Nil(StrVec);
	sv->MaxLeng= n;
	sv->String= ss;
    }
    return sv;
}
StrVec *StrVecAppend(StrVec *sa, char *s) {
    char *ns;
    if (sa->Leng >= sa->MaxLeng) {
    	StrVec *nsa;
	int nl= sa->MaxLeng;
	nl+= (nl>>3) + 4;
    	if ((nsa=StrVecExpand(sa, nl)) == Nil(StrVec))
	    return Nil(StrVec);
    }
    if ((ns=strdup(s)) == Nil(char))
    	return Nil(StrVec);
    sa->String[sa->Leng++]= ns;
    return sa;
}
char *StrVecToString(StrVec *sa, char Sep) {
    int  i, StrLeng= 0;
    char *str, *dstr;

    for (i=0; i< sa->Leng; i++) {
        char *cp= sa->String[i];
	int l= 0;
	while (*cp) {
	    l++;
	    if (*cp == Sep) {
	    	l++;
		while (*cp && *cp==Sep)
		    cp++;
	    }
	    else
		cp++;
	}
	StrLeng+= l+1;
    }
    dstr= str= nNew(StrLeng+1, char);
    if (dstr == Nil(char))
    	return Nil(char);
    for (i=0; i< sa->Leng; i++) {
        char *cp= sa->String[i];
	if (i>0)
	    *dstr++= Sep;
	while (*cp) {
	    if (*cp == Sep)
	    	*dstr++= '\\';
	    *dstr++= *cp++;
	}
    }
    *dstr='\0';
    return str;
}
StrVec *StrVecJoin(StrVec *sv, StrVec *sv1) {
    int i;
    sv= StrVecExpand(sv, sv->Leng+sv1->Leng);
    
    if (sv == Nil(StrVec))
    	return Nil(StrVec);

    for (i=0; i<sv1->Leng; i++) {
    	char *sd= strdup(sv1->String[i]);
	if (sd != Nil(char))
	    sv->String[sv->Leng++]= sd;
	else {
	    while (--i>=0)
	    	free(sv->String[--sv->Leng]);
	    return Nil(StrVec);
	}
    }
    return sv;
}
StrVec *StrVecCreate() {
    StrVec *sa= New(StrVec);

    if (sa == Nil(StrVec))
	return Nil(StrVec);
    sa->Leng= 0;
    sa->MaxLeng= 0;
    sa->String= Nil(char*);
    return sa;
}
StrVec *StrVecFromString(char *s, char Sep) {
    char *scp, *rcp, *ss;
    StrVec *sa;

    if ((ss=strdup(s)) == Nil(char))
    	return Nil(StrVec);
    if ((sa=StrVecCreate()) == Nil(StrVec)) {
    	free(ss);
    	return Nil(StrVec);
    }
    rcp= ss;
    while (*rcp) {
    	StrVec *nsa;
	char *start;
        while (*rcp == Sep)
	    rcp++;
	scp= start= rcp;
	while (*rcp) {
	    if (*rcp == '\\') {
	        *scp++= *++rcp;
		if (*rcp) rcp++;
		continue;
	    }
	    if (*rcp == Sep) {
		if (*rcp) rcp++;
	    	*scp= '\0';
		break;
	    }
	    *scp++= *rcp++;
	}
	if ((nsa=StrVecAppend(sa, start)) == Nil(StrVec)) {
	    free(ss);
	    StrVecDestroy(sa);
	    return Nil(StrVec);
	}
	else
	    sa= nsa;
    }
    free(ss);
    return sa;
}
void StrVecDestroy(StrVec *sa) {
    int i;
    for (i=0; i<sa->Leng; i++)
    	free(sa->String[i]);
    if (sa->Leng > 0)
	free(sa->String);
    free(sa);
    return;
}
