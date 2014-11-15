/* IMPACT Public Release (www.crhc.uiuc.edu/IMPACT)            Version 2.00  */
/* IMPACT Trimaran Release (www.trimaran.org)                  Feb. 22, 1999 */
/*****************************************************************************\
 * LICENSE AGREEMENT NOTICE
 *
 * IT IS A BREACH OF THIS LICENSE AGREEMENT TO REMOVE THIS NOTICE FROM
 * THE FILE OR SOFTWARE, OR ANY MODIFIED VERSIONS OF THIS FILE OR
 * SOFTWARE OR DERIVATIVE WORKS.
 *
 * ------------------------------
 *
 * Copyright Notices/Identification of Licensor(s) of
 * Original Software in the File
 *
 * Copyright 1990-1999 The Board of Trustees of the University of Illinois
 * For commercial license rights, contact: Research and Technology
 * Management Office, University of Illinois at Urbana-Champaign;
 * FAX: 217-244-3716, or email: rtmo@uiuc.edu
 *
 * All rights reserved by the foregoing, respectively.
 *
 * ------------------------------
 *
 * Copyright Notices/Identification of Subsequent Licensor(s)/Contributors
 * of Derivative Works
 *
 * Copyright  <Year> <Owner>
 * <Optional:  For commercial license rights, contact:_____________________>
 *
 *
 * All rights reserved by the foregoing, respectively.
 *
 * ------------------------------
 *
 * The code contained in this file, including both binary and source
 * [if released by the owner(s)] (hereafter, Software) is subject to
 * copyright by the respective Licensor(s) and ownership remains with
 * such Licensor(s).  The Licensor(s) of the original Software remain
 * free to license their respective proprietary Software for other
 * purposes that are independent and separate from this file, without
 * obligation to any party.
 *
 * Licensor(s) grant(s) you (hereafter, Licensee) a license to use the
 * Software for academic, research and internal business purposes only,
 * without a fee.  "Internal business purposes" means that Licensee may
 * install, use and execute the Software for the purpose of designing and
 * evaluating products.  Licensee may submit proposals for research
 * support, and receive funding from private and Government sponsors for
 * continued development, support and maintenance of the Software for the
 * purposes permitted herein.
 *
 * Licensee may also disclose results obtained by executing the Software,
 * as well as algorithms embodied therein.  Licensee may redistribute the
 * Software to third parties provided that the copyright notices and this
 * License Agreement Notice statement are reproduced on all copies and
 * that no charge is associated with such copies.  No patent or other
 * intellectual property license is granted or implied by this Agreement,
 * and this Agreement does not license any acts except those expressly
 * recited.
 *
 * Licensee may modify the Software to make derivative works (as defined
 * in Section 101 of Title 17, U.S. Code) (hereafter, Derivative Works),
 * as necessary for its own academic, research and internal business
 * purposes.  Title to copyrights and other proprietary rights in
 * Derivative Works created by Licensee shall be owned by Licensee
 * subject, however, to the underlying ownership interest(s) of the
 * Licensor(s) in the copyrights and other proprietary rights in the
 * original Software.  All the same rights and licenses granted herein
 * and all other terms and conditions contained in this Agreement
 * pertaining to the Software shall continue to apply to any parts of the
 * Software included in Derivative Works.  Licensee's Derivative Work
 * should clearly notify users that it is a modified version and not the
 * original Software distributed by the Licensor(s).
 *
 * If Licensee wants to make its Derivative Works available to other
 * parties, such distribution will be governed by the terms and
 * conditions of this License Agreement.  Licensee shall not modify this
 * License Agreement, except that Licensee shall clearly identify the
 * contribution of its Derivative Work to this file by adding an
 * additional copyright notice to the other copyright notices listed
 * above, to be added below the line "Copyright Notices/Identification of
 * Subsequent Licensor(s)/Contributors of Derivative Works."  A party who
 * is not an owner of such Derivative Work within the meaning of
 * U.S. Copyright Law (i.e., the original author, or the employer of the
 * author if "work of hire") shall not modify this License Agreement or
 * add such party's name to the copyright notices above.
 *
 * Each party who contributes Software or makes a Derivative Work to this
 * file (hereafter, Contributed Code) represents to each Licensor and to
 * other Licensees for its own Contributed Code that:
 *
 * (a) Such Contributed Code does not violate (or cause the Software to
 * violate) the laws of the United States, including the export control
 * laws of the United States, or the laws of any other jurisdiction.
 *
 * (b) The contributing party has all legal right and authority to make
 * such Contributed Code available and to grant the rights and licenses
 * contained in this License Agreement without violation or conflict with
 * any law.
 *
 * (c) To the best of the contributing party's knowledge and belief,
 * the Contributed Code does not infringe upon any proprietary rights or
 * intellectual property rights of any third party.
 *
 * LICENSOR(S) MAKE(S) NO REPRESENTATIONS ABOUT THE SUITABILITY OF THE
 * SOFTWARE OR DERIVATIVE WORKS FOR ANY PURPOSE.  IT IS PROVIDED "AS IS"
 * WITHOUT EXPRESS OR IMPLIED WARRANTY, INCLUDING BUT NOT LIMITED TO THE
 * MERCHANTABILITY, USE OR FITNESS FOR ANY PARTICULAR PURPOSE AND ANY
 * WARRANTY AGAINST INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * LICENSOR(S) SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE USERS
 * OF THE SOFTWARE OR DERIVATIVE WORKS.
 *
 * Any Licensee wishing to make commercial use of the Software or
 * Derivative Works should contact each and every Licensor to negotiate
 * an appropriate license for such commercial use, and written permission
 * of all Licensors will be required for such a commercial license.
 * Commercial use includes (1) integration of all or part of the source
 * code into a product for sale by or on behalf of Licensee to third
 * parties, or (2) distribution of the Software or Derivative Works to
 * third parties that need it to utilize a commercial product sold or
 * licensed by or on behalf of Licensee.
 *
 * By using or copying this Contributed Code, Licensee agrees to abide by
 * the copyright law and all other applicable laws of the U.S., and the
 * terms of this License Agreement.  Any individual Licensor shall have
 * the right to terminate this license immediately by written notice upon
 * Licensee's breach of, or non-compliance with, any of its terms.
 * Licensee may be held legally responsible for any copyright
 * infringement that is caused or encouraged by Licensee's failure to
 * abide by the terms of this License Agreement.
\*****************************************************************************/

/*****************************************************************************\
 *	File:	line.c
 *	Author:	Pohua Paul Chang, Wen-mei Hwu
\*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <library/line.h>

static void Punt(char *mesg)
{
    fprintf(stderr, "%s\n", mesg);
    exit(-1);
}

#define WHITE_SPACE(ch) ((ch==' ')||(ch=='\t')||(ch==''))

/*
 *	Read one line from file.
 *	Skip empty lines.
 *	Compress all white spaces between words into single spaces.
 *	Terminate the line with \0
 *	Punt if the buffer is not large enough.
 *	Else, return 0 upon EOF.
 *	Otherwise, return 1;
 */
int ReadLine(FILE *F, char buf[], int len)
{
    int white_space;
    int ch, i;
    i = 0;
    white_space = 0;
    while ((ch=getc(F)) != EOF) {
	if ((i==0)&&(ch=='\n'))		/* skip over empty line */
	    continue;
 	if (ch=='\n')			/* exit loop upon EOLN */
	    break;
	if (WHITE_SPACE(ch)) {		/* do nothing for white spaces */
	    white_space = 1;
	} else {
	    if ((i!=0) && white_space) {
		buf[i++] = ' ';
		if (i>=len)
		    Punt("ReadLine: buffer overflows");
	    }
	    buf[i++] = ch;
	    if (i>=len)
		Punt("ReadLine: buffer overflows");
	    white_space = 0;
	}
    }
    buf[i] = '\0';
    if (i==0) return 0;		/* EOF */
    return 1;
}
/*
 *	Read one word from file.
 *	Skip over white spaces.
 *	Punt if the buffer is not large enough.
 *	Else, return 0 upon EOF.
 *	Otherwise, return 1.
 */
int ReadWord(FILE *F, char buf[], int len)
{
    int ch, i;
    /*
     *	Skip over white spaces first.
     */
    while ((ch=getc(F)) != EOF) {	/* stop when EOF has been reached */
	if (ch=='\n')			/* skip over EOLN */
	    continue;
	if (! WHITE_SPACE(ch)) {	/* read until a non-white-space */
	     ungetc(ch, F);
	     break;
	}
    }
    if (ch==EOF) return 0;
    i = 0;
    while ((ch=getc(F)) != EOF) {
 	if ((ch=='\n') || WHITE_SPACE(ch))
	    break;
	buf[i++] = ch;
	if (i>=len)
	    Punt("ReadWord: buffer overflows");
    }
    buf[i] = '\0';
    if (i==0) return 0;		/* EOF */
    return 1;
}
/*
 *	Find the starting point of each word in the line.
 *	Returns -1 if the buffer is not large enough.
 *	Otherwise, returns the number of words found.
 */
int ParseLine(char *line, char *buf[], int len)
{
    int i, n;
    i = n = 0;
    while (line[i]!='\0') {
	/*
	 *	Skip over white-spaces.
	 */
	while (WHITE_SPACE(line[i]))
	    i++;
	if (line[i]=='\0')
	    break;
	buf[n++] = (line+i);
	if (n>=len) Punt("ParseLine: buffer overflows");
	/*
	 *	Go to the next white space.
	 */
	while ((line[i]!='\0') && ! WHITE_SPACE(line[i]))
	    i++;
    }
    buf[n] = 0;
    return n;
}
/*
 *	Find the starting point of each word in the line.
 *	Returns -1 if the buffer is not large enough.
 *	Otherwise, returns the number of words found.
 *	Make each word a separate string.
 */
int ParseLine2(char *line, char *buf[], int len)
{
    int i, n;
    i = n = 0;
    while (line[i]!='\0') {
	/*
	 *	Skip over white-spaces.
	 */
	while (WHITE_SPACE(line[i]))
	    i++;
	if (line[i]=='\0')
	    break;
	buf[n++] = (line+i);
	if (n>=len) Punt("ParseLine: buffer overflows");
	/*
	 *	Go to the next white space.
	 */
	while ((line[i]!='\0') && ! WHITE_SPACE(line[i]))
	    i++;
	if (line[i]=='\0')
	    break;
	line[i] = '\0';
	i++;
    }
    buf[n] = 0;
    return n;
}
/*
 *	Return 1 if the first part of the word
 *	totally match the prefix.
 */
int MatchPrefix(char *line, char *prefix)
{
    int len, i;
    len = strlen(prefix);
    if (strlen(line)<len) return 0;
    for (i=0; i<len; i++)
	if (line[i] != prefix[i])
	    return 0;
    return 1;
}
/*
 *      Return 1 if pattern matches name;
 *      otherwise return 0.
 *      Wildcard character is allowed in pattern.
 *      ? matches any one character.
 *      * matches from 1 to N characters.
 */
int WildcardMatch(char *pattern, char *name)
{
    if ((name[0] == '\0') && (pattern[0] == '\0'))
	return 1;
    if (pattern[0] == '?') {
	return WildcardMatch(pattern+1, name+1);
    } else
    if (pattern[0] == '*') {
	int i;
	i = 1;
 	if ((name[1] == '\0') && (pattern[1] == '\0'))
	    return 1;
    	while (name[i] != '\0') {
	    /*
	     * find the next possible starting point in name
	     * that matches the rest of the pattern.
  	     */
	    while ((name[i] != '\0') && (name[i] != pattern[1])
		   && (pattern[1] != '*') && (pattern[1] != '?'))
	    	i += 1;
	    if (WildcardMatch(pattern+1, name+i))
	    	return 1;
	    i += 1;
   	}
	return 0;
    } else
    if (pattern[0] == name[0]) {
	return WildcardMatch(pattern+1, name+1);
    } else
	return 0;
}
/*
 *	Remove the last field from the source string.
 *	Return 1 if successful, Return 0 if not successful
 *	for any reason.
 *	If the seperator character does not appear in the
 *	source string, then the result is an empty string,
 *	and 1 is returned.
 */
int RemovePostfix(char src[], char seperator, char buf[], int buf_len)
{
    register int src_len, i, j;
    src_len = strlen(src);
    buf[0] = 0;				/* default result is an empty string */
    if (src_len==0) return 1;		/* nothing needs to be done */
    for (i=src_len-1; i>=0; i--) 	/* find the seperator */
	if (src[i]==seperator)
	    break;
    if (i<0) return 1;			/* the entire src is deleted */
    if (buf_len<=i) return 0;		/* buffer is not long enough */
    for (j=0; j<i; j++)
	buf[j] = src[j];
    buf[j] = '\0';
    return 1;
}
/*
 *	Remove the first field from the source string.
 *	Return 1 if successful, Return 0 if not successful
 *	for any reason.
 *	If the seperator character does not appear in the
 *	source string, then the result is an empty string,
 *	and 1 is returned.
 */
int RemovePrefix(char src[], char seperator, char buf[], int buf_len)
{
    register src_len, i, j, k;
    buf[0] = '\0';
    src_len = strlen(src);
    for (i=0; i<src_len; i++)		/* find the first seperator */
	if (src[i]==seperator)
	    break;
    j = src_len - (i+1);		/* number of char needs to be copied */
    if (j>=buf_len) return 0;
    for (k=0; k<=j; k++)
	buf[k] = src[i+k+1];
    buf[k] = '\0';
    return 1;
}
/*
 *	Extract the first field from the source string.
 *	Return 1 if successful, Return 0 if not successful
 *	for any reason.
 *	If the seperator character does not appear in the
 *	source string, then the result is the src string.
 */
int GetPrefix(char src[], char seperator, char buf[], int buf_len)
{
    register char *ch;
    register int i;
    i = 0;
    for (ch=src; (*ch!=seperator)&&(*ch!='\0'); ch++) {
	if (i>=buf_len) {
	    buf[buf_len-1] = '\0';
	    return 0;
	}
	buf[i] = *ch;
	i += 1;
    }
    buf[i] = '\0';
    return 1;
}


#ifdef DEBUG_LINE
main() {
	char line[512];
	char *ptr[100];
	int i, j;

	RemovePostfix("f:9:ccode", ':', line, 512);
	printf("%s\n", line);

	RemovePostfix("abc:def", ':', line, 512);
	printf("%s\n", line);
	RemovePostfix(":abc:", ':', line, 512);
	printf("%s\n", line);
	RemovePostfix("abc:def:ghi", ':', line, 512);
	printf("%s\n", line);
	RemovePostfix("abc.def.ghi", ':', line, 512);
	printf("%s\n", line);

	RemovePrefix("abc:def", ':', line, 512);
	printf("%s\n", line);
	RemovePrefix(":abc:", ':', line, 512);
	printf("%s\n", line);
	RemovePrefix("abc:def:ghi", ':', line, 512);
	printf("%s\n", line);
	RemovePrefix("abc.def.ghi", ':', line, 512);
	printf("%s\n", line);

/****
#define M(x, y) WildcardMatch(x, y)
 	printf("%d\n", M("abc", "abc"));
 	printf("%d\n", M("abcd", "abc"));
 	printf("%d\n", M("abc", "abcd"));
 	printf("%d\n", M("ab?d", "abcd"));
 	printf("%d\n", M("a*d", "abcd"));
 	printf("%d\n", M("a*d", "abce"));
 	printf("%d\n", M("a*de", "abcdfdde"));
 	printf("%d\n", M("ab?", "abc"));
 	printf("%d\n", M("ab*", "abc"));
 	printf("%d\n", M("a?*", "abc"));
 	printf("%d\n", M("a**", "abc"));
 	printf("%d\n", M("*", "abc"));
 	printf("%d\n", M("a*d", "ad"));
 	printf("%d\n", M("???", "abc"));
 	printf("%d\n", M("***", "abc"));
 	printf("%d\n", M("*?*", "abc"));
 	printf("%d\n", M("**?", "abc"));
****/
/***
	while (ReadWord(stdin, line, 512) != 0) {
	    printf("> %s\n", line);
	}
 ***/
/****
	while (ReadLine(stdin, line, 512) != 0) {
	    printf("> %s\n", line);
	    i = ParseLine(line, ptr, 100);
	    for (j=0; j<i; j++) {
		printf("# %s\n", ptr[j]);
	    }
	}
****/
}
#endif
