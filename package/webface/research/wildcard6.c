	

/* wildcard.c - test if filenames match a UNIX shell pattern
 * Copyright (C) 1995-99 Andrew Pipkin (minitrue@pagesz.net)
 * MiniTrue is free software released with no warranty. See COPYING for details
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "wildcard.h"
#include "minitrue.h"

char *WildCard_parse(WildCard *wc_ptr, char *src, int case_insense);
static char *init_set(unsigned char *set, char *src, int case_insense);

static char *match_str(WildCard *wc_ptr, char *fname_ptr);
static char *find_str(WildCard *wc_ptr, char *fname_ptr);
static char *match_str_ci(WildCard *wc_ptr, char *fname_ptr);
static char *find_str_ci(WildCard *wc_ptr, char *fname_ptr);
static char *match_set(WildCard *wc_ptr, char *fname_ptr);
static char *find_set(WildCard *wc_ptr, char *fname_ptr);

enum { PREV_STAR = 1, CASE_INSENSE = 2, SET = 4, WC_END = 8, SET_LEN = 32 };

/* Set of characters represented by ? - all non-nul characters */
static unsigned char Qmark_set[SET_LEN] =
{   0xfe,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff
};

/* Convert the pattern into bytecodes, return the first bytecode. If the
 * pattern has no wildcard characters, return NULL unless recursing through
 *  subdirectories is desired */
WildCard *WildCard_init(char *pattern, int case_insense, int recurse)
{
    WildCard *wc_start = NULL, *wc_ptr;
    int crnt_i = 0, end_i = 0;
    int star_i = -1; /* star_i is index of last data preceded by star */

 /* If filename NUL & recursion desired, match all filenames */
    if(!*pattern && recurse)
        pattern = "*";

    do
    {/* Allocate blocks in chunks of 16, if end of chunk reached allocate
      *   a new chunk */
        if(crnt_i == end_i)
            wc_start  = x_realloc(wc_start, sizeof(WildCard) * (end_i += 16));
        wc_ptr            = &wc_start[crnt_i];
        wc_ptr->tag       = 0;
        wc_ptr->backtrack = star_i;

     /* If data is preceded by a star, can scan forward for it instead of
      * only examining the current position */
        if(*pattern == '*')
        {   while(*pattern == '*') /* ignore multiple stars */
                ++pattern;
            wc_ptr->tag |= PREV_STAR;
        }
        pattern = WildCard_parse(wc_ptr, pattern, case_insense);

     /* If preceded by star, update location of last star */
        if(wc_ptr->tag & PREV_STAR)
            star_i = crnt_i;

        ++crnt_i;

    }while(*pattern || wc_ptr->tag & SET);
    wc_ptr->tag |= WC_END;

 /* If pattern consists of only a single fixed string, return unless recursing
  *   through subdirectories */
    if(crnt_i == 1 && !(wc_ptr->tag & (SET | PREV_STAR)) && !recurse)
    {   free(wc_ptr);
        return NULL;
    }
 /* Realloc to chop off unused values before returning */
    return x_realloc(wc_start, sizeof(WildCard) * (wc_ptr + 1 - wc_start));
}

/* Convert the pattern to the appropriate bytecode */
char *WildCard_parse(WildCard *wc_ptr, char *src, int case_insense)
{
    static char * (* Funct_arr[])(WildCard *, char *) =
    { match_str, find_str, match_str_ci, find_str_ci, match_set, find_set };

 /* Sets are enclosed in [], with ? equivalent to a set with all values */
    if((*src == '[' && strchr(src, ']')) || *src == '?')
    {   wc_ptr->tag |= SET;
        wc_ptr->len  = 1;
     /* Question represents set containing all values except \0 */
        if(*src == '?')
        {   wc_ptr->data = Qmark_set;
            ++src;
        }
     /* If set in [], allocate memory for the set, then parse it */
        else
        {   wc_ptr->data = x_malloc(SET_LEN);
            src          = init_set(wc_ptr->data, src, case_insense);
        }
    }
 /* Fixed strings consist of characters with no special meaning */
    else
    {   wc_ptr->data = src;
        while(*src != '*' && *src != '?' && *src != '\0'
              && (*src != '[' || !strchr(src, ']')))
            ++src;
        wc_ptr->len = src - (char *)wc_ptr->data;
     /* If string at end of wildcard, include NUL character in string */
        if(!*src)
            ++wc_ptr->len;
        if(case_insense)
            wc_ptr->tag |= CASE_INSENSE;
    }
 /* set function pointer */
    wc_ptr->func = Funct_arr[wc_ptr->tag & 7];
    return src;
}

/* Test if value ch is in a set */
int in_shell_set(unsigned char *set, int ch)
{
    return set[ch >> 3] & (1 << (ch & 7));
}

/* Add value ch to a set */
void insert_set(unsigned char *set, int ch)
{
    set[ch / 8] |= 1 << (ch % 8);
}

/* Remove a value ch from a set */
void remove_set(unsigned char *set, int ch)
{
    set[ch / 8] &= 0xff - (1 << (ch % 8));
}

/* Add the upper and lower case versions of ch to a set */
void insert_set_ci(unsigned char *set, int ch)
{
    insert_set(set, tolower(ch));
    insert_set(set, toupper(ch));
}

/* Remove the upper and lower case versions of ch from a set */
void remove_set_ci(unsigned char *set, int ch)
{
    remove_set(set, tolower(ch));
    insert_set(set, toupper(ch));
}

/* parse a character set, it is assumed that it is known that set
 *  has closing ] */
char *init_set(unsigned char *set, char *src, int case_insense)
{
    static void (* Funct_arr[])(unsigned char *, int) =
    { insert_set, remove_set, insert_set_ci, remove_set_ci };
    void (* add_set_fn)(unsigned char *, int);
    int funct_i = case_insense ? 2 : 0;

 /* If set starts with '!', all values not in set will be matched */
    if(*++src == '!')
    { /* set all values in set, then use function for removing values */
        memcpy(set, Qmark_set, SET_LEN);
        funct_i |= 1;
        ++src;
    }
    else
        memset(set, 0, SET_LEN); /* clear set if not negated */

    add_set_fn = Funct_arr[funct_i];
    while(*src != ']')
    {   int range_i;
        if(!*src)
            return NULL;
        if(src[1] == '-' && src[2] != ']')
        {   for(range_i = *src; range_i <= src[2]; ++range_i)
                add_set_fn(set, range_i);
            src += 3;
        }
        else
            add_set_fn(set, *src++);
    }
    return src + 1;
}

/* Test if the basename of path_ptr matches the pattern beginning at wc_start*/
int WildCard_match(WildCard *wc_start, char *path_ptr)
{
    WildCard *wc_ptr = wc_start;
    int path_len;
    char *ext_ptr, *fname_ptr = parse_path(path_ptr, &ext_ptr, &path_len);

 /* Match is sucessful if last item is star and last item reached */
    for( ; ; )
    {   char *loc = wc_ptr->func(wc_ptr, fname_ptr);
     /* If item not matched, backtrack to last item preceded by a star */
        if(!loc)
        {   if(wc_ptr->backtrack == -1) /* quit if nowhere to backtrack */
                return FALSE;
            wc_ptr    = &wc_start[wc_ptr->backtrack];
            fname_ptr = wc_ptr->loc + 1;
        }
        else
        {/* if item successfully matched and end reached, return success */
            if(wc_ptr->tag & WC_END)
                break;
         /* Otherwise move to next item */
            wc_ptr->loc = loc;
            fname_ptr   = loc + wc_ptr->len;
            ++wc_ptr;
        }
    }
    return TRUE;
}

/* test if current position matches string */
char *match_str(WildCard *wc_ptr, char *fname_ptr)
{
    int len = wc_ptr->len;
    return memcmp(wc_ptr->data, fname_ptr, len) ? NULL : fname_ptr;
}

/* scan forward for string for strings preceded by '*' */
char *find_str(WildCard *wc_ptr, char *fname_ptr)
{
    while((fname_ptr = strchr(fname_ptr, *(char *)wc_ptr->data)) != NULL)
    {   if(!memcmp(wc_ptr->data, fname_ptr, wc_ptr->len))
            break;
        ++fname_ptr;
    }
    return fname_ptr;
}

/* match fixed strings case insensitively */
char *match_str_ci(WildCard *wc_ptr, char *fname_ptr)
{
    int str_i;
    for(str_i = 0; str_i < wc_ptr->len; ++str_i)
    {   if(tolower(fname_ptr[str_i]) != tolower(((char *)wc_ptr->data)[str_i]))
            return NULL;
    }
    return fname_ptr;
}

/* scan forward for case-insensitive string preceded by '*' */
char *find_str_ci(WildCard *wc_ptr, char *fname_ptr)
{
    int first_ch = tolower(*(char *)wc_ptr->data);
    do
    {   if(first_ch == tolower(*fname_ptr) && match_str_ci(wc_ptr, fname_ptr))
           return fname_ptr;
    }while(*fname_ptr++);
    return NULL;
}

/* test if current position matches set */
char *match_set(WildCard *wc_ptr, char *fname_ptr)
{
    return in_shell_set(wc_ptr->data, *fname_ptr) ? fname_ptr : NULL;
}

/* scan forward for set for sets preceded by '*' */
char *find_set(WildCard *wc_ptr, char *fname_ptr)
{
    for( ; !in_shell_set(wc_ptr->data, *fname_ptr) ; ++fname_ptr)
    {   if(!*fname_ptr)
            return NULL;
    }
    return fname_ptr;
}

/* Return true if string has wildcard characters (*, ?, [,])
 * false otherwise */
int WildCard_Is(const char *str)
{
    while(*str != '\0' && *str != '*' && *str != '?' && *str != '[')
        ++str;
    return (*str != '\0');
}

/* Free the memory used by the wildcard and all associated sets */
void WildCard_kill(WildCard *wc_ptr)
{
    int wc_i = 0;
 /* Go through blocks and free sets if not aliased to question mark set */
    do {
        if(wc_ptr[wc_i].tag & SET && wc_ptr[wc_i].data != Qmark_set)
            free(wc_ptr[wc_i].data);
    } while(!(wc_ptr[wc_i++].tag & WC_END));
    free(wc_ptr);
}

#ifdef WILDCARD_TEST
void WildCard_dump(WildCard *wc_start, char *fname)
{
    WildCard *wc_ptr = wc_start;
    char *fname_ptr = fname;
    for( wc_ptr = wc_start; !(wc_ptr->tag & WC_END) ; ++wc_ptr)
    {   if(fname_ptr != wc_ptr->loc)
            printf(" %.*s  ", (int)(wc_ptr->loc - fname_ptr), fname_ptr);
        printf("%.*s ", wc_ptr->len, wc_ptr->loc);

        fname_ptr = wc_ptr->loc + wc_ptr->len;
    }
    printf("%s\n", fname_ptr);
}

# include <dirent.h>
int main(int argc, char *argv[])
{
    int arg_i = (argc <= 2) ? 1 : 2;
    for(; arg_i < argc; ++arg_i)
    {   DIR *dir  = opendir((argc <= 2) ? "." : argv[1]);
        struct dirent *dir_ent;
        WildCard *wild_card = WildCard_init(argv[arg_i], TRUE, FALSE);
        if(wild_card)
        {   while ((dir_ent = readdir(dir)) != NULL)
            {   if(WildCard_match(wild_card, dir_ent->d_name))
                    puts(dir_ent->d_name);
            }
        }
        closedir(dir);
    }
    return 0;
}
#endif /*  WILDCARD_TEST */
