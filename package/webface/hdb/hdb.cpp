/************************************************************************** 
 *
 * Copyright (c) 2005,2007    Vladislav Moskovets (webface-dev(at)vlad.org.ua)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *************************************************************************** */
extern "C"
{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/file.h>
#include <sys/types.h>
}


#define DEBUG
#define READ_BUF_SIZE 2048
#define MAX_LEVEL INT_MAX

#define BIT(x) (1UL << (x))
#define SET_BIT(target,bit)        ((target) |= (bit))
#define CLEAR_BIT(target,bit)      ((target) &= ~(bit))
#define TEST_BIT(target,bit)       ((target) & (bit))

#define PRINT_QUOT				BIT(0)
#define PRINT_DQUOT				BIT(1)
#define PRINT_LOCAL				BIT(2)
#define PRINT_EXPORT			BIT(3)
#define PRINT_COUNT				BIT(4)
#define PRINT_NAME				BIT(5)
#define PRINT_CHAINNAME			BIT(6)
#define PRINT_DATA				BIT(7)
#define DO_NOT_ESCAPE			BIT(8)
#define PRINT_GCP				BIT(9)
#define PRINT_GP				BIT(10)
#define PRINT_GC				BIT(11)
#define PRINT_GPARENTC			BIT(12)
#define PRINT_GN				BIT(13)
#define PRINT_GPARENTN			BIT(14)
#define PRINT_GD				BIT(15)
#define DO_NOT_PRINT_NEWLINE	BIT(16)
#define SET_SCP					BIT(17)

//#define match_wildcard ndtpd_match_wildcard
#define match_wildcard rsync_wildmatch

int debug_level = 1;

#ifdef DEBUG
void debug(int level,char *format, ...){
	if ( level > debug_level ) 
		return;
	for (int i = 0; i < level; i++)
		fprintf(stderr, " ");
	fprintf(stderr, "DEBUG%d: ", level);
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}
#else
inline void _debug(int level, char* format, ...){}
#define debug if(0)_debug
#endif

void warn(char *format, ...) {
	fprintf(stderr, "Warning: ");
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

void error(char *format, ...) {
	fprintf(stderr, "Error: ");
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

//                    ^
//    +---------------------------------+
//    |             parent              |
//  <-| prev                       next |->
//    |             fchild              |
//    +---------------------------------+
//                    v
//
//  RULES:
// 1. absolute root: next=NULL, prev=NULL, parent=NULL
// 2. first node: prev=NULL
// 3. not first node: parent=NULL
// 4. last: next=NULL
//
//  EXAMPLE TREE
//
//  *ROOT*
//    |           |-t1-t2-t3-t4-t5
//    n1-n2-n3-n4-n5             |
//      |   |                    a1-a2-a3-a4-a5
//      |   z1-z2-z3-z4-z5
//      |            |
//      y1-y2-y3     q1-q2-q3-q4-q5
//      |
//      r1-r2-r3-r4
//      
// 1. q4->get_parent(): returns z4
// 2. q4->get_first(): returns q1
// 3. q4->get_last(): returns q5
// 4. q5->get_parent()->get_first(): returns z1
// 5. q5->get_parent()->get_last(): returns z5
// 6. q4->get_first()->get_prev(): returns NULL
// 7. q4->get_absolute_root(): returns ROOT

char *str_escape(const char *source);
char *str_unescape(const char *source);
int match_wildcard(const char *pattern, const char *string);
int is_wildcarded(const char* str);


class hdb;
hdb *app = NULL;

char *level_delimiter=" ";
char *node_delimiter="_";

class node {
	node *parent;
	node *fchild;
	node *next;
	node *prev;

	char *name;
	char *data;
	char *pos_str;

	void init_values() {
		parent=NULL; 
		fchild=NULL;
		next=NULL;
		prev=NULL;
		name=strdup("");
		data=strdup("");
		pos_str=strdup("");
	}
public:
	node() { 
		init_values();
	}

	node(const char* name, const char* data) { 
		init_values();
		set_namedata(name, data);
	}
	node(node* r) {
		init_values();
		parent=r;
	}

	node(node* r, const char* name, const char* data) {
		init_values();
		parent=r;
		set_namedata(name, data);
	}

	node(const char* pair) {
		init_values();
		set_pair(pair);
	}

	~node() {
		free_childs();
		assert( data != NULL);
		assert( name != NULL);
		free(data);
		free(name);
	}

	void free_childs() {
		debug(3, "node['%s']::free_childs() \n", get_name());
		node *n;
		while ( (n = fchild) ) {
			//n->free_childs();
			n->remove();
			delete n;
		}
	}

	const char *get_name() { return name; }
	const char *get_data() { return data; }

	void set_namedata(const char* name, const char* data) {
		set_name(name);
		set_data(data);
	}

	const char *set_name(const char *str) { debug(6, "node['%s']::set_name('%s')\n", get_name(), str); assert(str); free(name); return name = strdup(str); }
	const char *set_data(const char *str) { 
			debug(6, "node['%s']::set_data('%s')\n", get_name(), str); 
			assert(str); 
			char buf[512];
			int value, str_value;
			if ( str[0] == '+' && str[1] == '=' ) {
				value=atoi(data);
				str_value=atoi(&str[2]);
				snprintf(buf, sizeof(buf), "%d", value+str_value);
				free(data); 
				return data = strdup(buf);
			} else if ( str[0] == '-' && str[1] == '=' ) {
				value=atoi(data);
				str_value=atoi(&str[2]);
				snprintf(buf, sizeof(buf), "%d", value-str_value);
				free(data); 
				return data = strdup(buf);
			} else
				return data = strdup(str); 
	}
	void set_parent(node* newparent) { assert(newparent); parent = newparent; }

	node *get_next() { return next; }
	node *get_prev() { return prev; }
	node *get_parent() { 
		if ( get_prev() ) 
			return get_first()->get_parent();
		else 
			return parent;
	}

	inline bool is_absolute_root() {
		return get_parent()?true:false;
	}

	node *get_absolute_root() {
		node *n = get_parent();
		while ( ! n )
			n = get_parent();
		return n;
	}

	inline node *get_fchild() { return fchild; }

	int get_levelpos() {
		int i = 0;
		node* n = get_first();

		if ( n == NULL)
			return -1;

		debug(7, "node['u']::get_levelpos(): get_first(): '%s'\n", n->get_name()); 
		while ( n != this ) {
			assert( n != NULL);
			i++;
			n = n->get_next();
		}
		debug(7, "node['u']::get_levelpos('%d')\n", i); 
		return i;
	}
	
	bool set_pair(const char* str) {
		debug(5, "node['%s']::set_pair('%s')\n", get_name(), str);
		int len = strlen(str);
		if (!len) {
			fprintf(stderr, "Error: can't parse: '%s'", str);
			return false;
		}
		char s[len+10];

		strcpy(s, str);
		if (s[len-1]=='\n')
			s[len-1]='\0';

		char *eq=strchr(s, '=');
		if ( eq == s ) {
			set_data( eq+1 );
		} else if ( eq ) { 
			eq[0]='\0';
			set_name( s );
			set_data( eq+1 );
		} else {
			set_name( s );
		}

		return true;
	}

	node *get_first() {
		debug(7, "node['%s']::get_first(): \n", get_name());
		node *n = prev;
		if ( !n ) {
			debug(7, "node['%s']\n", get_name());
			return this;
		}
		
		// look for first: on first *prev should be NULL
		while ( true ) {
			if ( n->get_prev() )
				n = n->get_prev();
			else {
				debug(5, "node['%s']\n", get_name());
				return n;
			}
		}
		
	}

	node *get_last() {
		debug(5, "node['%s']::get_last():  ", get_name());
		assert(this);
		if ( next == NULL ) {
			debug(5, "node['%s']\n", get_name());
			return this;
		}
		assert( next != NULL );
		debug(5, "node['%s']\n", get_name());
		return next->get_last();
	}
	
	int get_level(int curr_level=0) {
		if ( ! get_parent() )
			return curr_level;
		return get_parent()->get_level(curr_level+1);
	}

	node *find_name(const char* str, int maxlevel=MAX_LEVEL) {
		debug(3, "node['%s']::find_name('%s', %d)\n", get_name(), str, maxlevel);
		node *found=NULL;
		if ( ! strcmp(str, name) )
			return this;

		if ( maxlevel <= 0 )
			return NULL;

		node *n=fchild;
		while ( n ) {
			found=n->find_name(str, maxlevel-1);
			if ( found )
				return found;
			n=n->get_next();
		}
		return found;
	}
	int get_childcount() {
		int i = 0;
		node *n=fchild;
		while ( n ) {
			n=n->get_next();
			i++;
		}
		return i;
	}

	node *get_lastchild() {
		debug(4, "node['%s']::get_lastchild()\n", get_name());
		assert (fchild != NULL);
		return fchild->get_last();
	}

	node *insert_after(node* n) {
		assert(n);
		debug(3, "node['%s']::insert_after('%s')\n", get_name(), n->get_name());
		node* t = next;
		next=n;
		next->prev=this;
		next->next=t;
		if (t)
			t->prev=next;
		return n;
	}

	node *insert_before(node* n) {
		assert(n);
		debug(3, "node['%s']::insert_before('%s')\n", get_name(), n->get_name());
		node* t=prev;
		prev=n;
		prev->prev=t;
		prev->next=this;
		if (t) {
			t->next=prev;
		}
		if (parent) { 
			prev->parent=parent;
			parent->fchild=prev;
			parent=NULL;
		}
		return n;
	}
	
	node *add_child(node* child) {
		assert(child);
		debug(3, "node['%s']::add_child('%s')\n", get_name(), child->get_name());
		assert (child);
		if ( fchild == NULL ) {
			fchild = child;
			child->parent=this;
		} else {
			get_lastchild()->insert_after(child);
		}
		return child;
	}

	// buggy
	node* swap_with_next() {
		if (! get_next() )
			return NULL;

		debug(3, "node['%s']::swap_with_next(): next['%s']\n", get_name(), get_next()->get_name());
		if ( get_parent() ) {
			get_next()->parent = get_parent();
			get_parent()->fchild = get_next();
			parent = NULL;
		}

		if ( get_prev() ) {
			get_prev()->next = get_next();
			debug(4, "node['%s']::swap_with_next(): setting get_prev()[%s]->next = get_next()['%s']\n", get_name(), get_prev()->get_name(), get_next()?get_next()->get_name():"NULL");
		}


		node* n = get_next();
		debug(4, "node['%s']::swap_with_next(): n[%s]\n", get_name(), n?n->get_name():"NULL");
		node* t = n->get_next();
		debug(4, "node['%s']::swap_with_next(): t[%s]\n", get_name(), t?t->get_name():"NULL");
		debug(4, "node['%s']::swap_with_next(): setting n[%s]->prev to [%s]\n", get_name(), n->get_name(), get_prev()?get_prev()->get_name():"NULL");

		n->prev = get_prev();
		n->next = this;

		prev = n;
		next = t;
		if ( next )
			next->prev = this;
		return this;
	}

	node* move_left() {
		debug(2, "node['%s']::move_left()\n", get_name());
		node *p = get_prev();
		if ( p ) {
			remove();
			return p->insert_before( this );
		} else
			return NULL;
	}

	node* move_right() {
		debug(2, "node['%s']::move_right()\n", get_name());
		node *n = get_next();
		if ( n ) {
			remove();
			return n->insert_after( this );
		} else 
			return NULL;
	}

	int compare_name(node* n) {
		assert( n != NULL );
		if (! n )
			return 0;
		debug(5, "node['%s']::compare_name(['%s'])\n", get_name(), n->get_name());

		// try to compare integers
		int i1, i2;
		if ( (i1 = atoi(get_name())) && (i2 = atoi(n->get_name())) )
			return i1-i2;
		else
			return strcmp(get_name(), n->get_name());
	}

	// very simple sort
	void sort() {
		debug(3, "node['%s']::sort()\n", get_name());
		node *n=fchild;
		while ( n ) {
			if ( n->get_next() && ( n->compare_name(n->get_next()) > 0 ) ) {
				n->move_right();
				n = fchild;
				continue;
			}
			n=n->get_next();
		}

		n=fchild;
		while ( n ) {
			if ( n->get_fchild() )
				n->sort();
			n = n->get_next();
		}
		return;
	}

	int renumber_child() {
		debug(3, "node['%s']::renumber_child()\n", get_name());
		char str[32];
		int i = 0;
		node *n=fchild;
		while ( n ) {
			snprintf(str, sizeof(str), "%d", i);
			n->set_name(str);
			n = n->get_next();
			i++;
		}

		return i;
	}

	node *remove() {
		debug(3, "node['%s'-%d]::remove()\n", get_name(), get_levelpos());
		if ( (parent != NULL) && (prev != NULL) )
			debug(1, "parent: %x, prev: %x\n", parent, prev);

		if ( get_prev() ) {
			debug(5, "node['%s']::remove(): updating node['%s'].next = node['%s']\n", get_name(), get_prev()->get_name(), get_next()? get_next()->get_name():"NULL");
			get_prev()->next=get_next();
		}
		
		// handle parent updation
		if ( parent ) {
			debug(5, "node['%s']::remove(): updating node['%s'].fchild = node['%s']\n", get_name(), parent->get_name(), get_next()? get_next()->get_name():"NULL");
			parent->fchild = get_next();
			if (get_next()) {
				debug(5, "node['%s']::remove(): updating node['%s'].parent = node['%s']\n", get_name(), get_next()->get_name(), parent->get_name());
				get_next()->parent=parent;
			}
		}
			
		if ( get_next() ) {
			debug(5, "node['%s']::remove(): updating node['%s'].prev = node['%s']\n", get_name(), get_next()->get_name(), get_prev()? get_prev()->get_name():"NULL");
			get_next()->prev=get_prev();
		}

		parent=NULL;
		next=NULL;
		prev=NULL;
		return this;
	}
			
		
	node *new_child(){
		return add_child(new node(this));
	}	

	node *new_child(const char* str){
		debug(3, "node['%s']::new_child('%s')\n", get_name(), str);
		return add_child(new node(str));
	}	


	node *new_sibling(const char* str){
		debug(3, "node['%s']::new_sibling('%s')\n", get_name(), str);
		return get_last()->insert_after(new node(str));
	}	

	node *new_sibling(){
		return get_last()->insert_after(new node(this));
	}	

	void dump(int level=0) {
		for (int i=0; i < level; i++)
			fprintf(stderr, level_delimiter);
		fprintf(stderr, "node['%s']: data=%s, parent='%s'\n", get_name(), get_data(), get_parent()?get_parent()->get_name():"NULL");

		node *c=fchild;
		while ( c ) {
			c->dump(level+1);
			c=c->get_next();
		}
	}

	int serialize_to_file(FILE *file, int flags=0) {
		debug(3, "node['%s']::serialize_to_file(flags=%d)\n", get_name(), flags);
		return serialize(file, 0, flags);
	}

	int serialize(FILE *stream=stdout, int level=0, int flags=0) {
		debug(3, "node['%s']::serialize(%d, %d, %d)\n", get_name(), stream, level, flags);

		int i;
		for ( i=0; i < level; i++ )
			fprintf(stream, level_delimiter);

		char *esc_name = TEST_BIT(flags, DO_NOT_ESCAPE) ? strdup(name) : str_escape(name);
		char *esc_data = TEST_BIT(flags, DO_NOT_ESCAPE) ? strdup(data) : str_escape(data);

		// print 'name=data' or just 'name'
		if ( strlen(esc_data) )
			fprintf(stream, "%s=%s\n", esc_name, esc_data);
		else
			fprintf(stream, "%s\n", esc_name);

		free(esc_data);
		free(esc_name);

		node *c = fchild;
		while ( c ) {
			c->serialize(stream, level+1, flags);
			c = c->get_next();
		}

		return true;
	}

	node *unserialize(const char* str, int level=0) {
		node *n = this;
		int i;
		int my_level = get_level();
		char *unescaped_str=str_unescape(str);

		debug(3, "  node['%s']::unserialize(): my_level=%d, level=%d, read_buf='%s'\n", get_name(), my_level, level, unescaped_str);
		if ( level == my_level ) {
			n = new_sibling(unescaped_str);
		} else if ( level > my_level ) {
			n = new_child(unescaped_str);

		} else if ( level < my_level ) {
			n = this;
			for ( i = 0; i < (my_level - level); i++) {
				n = n->get_parent();
				assert( n != NULL );
			}
			n = n->new_sibling(unescaped_str);
		}
		return n;
	}

	int unserialize_from_file(FILE *file=stdin) {

		uint readed_level = 0;
		char read_buf[READ_BUF_SIZE];
		char *pair_begin;
		node *curr_node = this;

		fgets(read_buf, sizeof(read_buf), file);
		curr_node->set_pair(read_buf);

		while ( fgets(read_buf, sizeof(read_buf), file) ) {

			if ( read_buf[strlen(read_buf)-1] == '\n' )
				read_buf[strlen(read_buf)-1]='\0';

			pair_begin = NULL;
			debug(5, "-------------\n");
			for ( readed_level = 0; readed_level < sizeof(read_buf); readed_level++ )
				if ( read_buf[readed_level] != level_delimiter[0] ) {
					pair_begin = &read_buf[readed_level];
					break;
				}
			curr_node = curr_node->unserialize(pair_begin, readed_level);
		}
		return true;
	}

	char *get_fullchain() {
		char str_buf1[512];
		char str_buf2[512];
		memset((void*)str_buf1, 0, sizeof(str_buf1));
		memset((void*)str_buf2, 0, sizeof(str_buf2));

		node *n = this;

		while (n) {
			// dot not include ROOT node
			if ( ! n->get_parent() )
				break;
			strncpy(str_buf2, str_buf1, sizeof(str_buf2));
			strncpy(str_buf1, n->get_name(), sizeof(str_buf1));
			if ( n != this)
				strncat(str_buf1, node_delimiter, sizeof(str_buf1));
			strncat(str_buf1, str_buf2, sizeof(str_buf1));
			n = n->get_parent();
		}
		return strdup(str_buf1);
	}

	void print_fullchain(int maxlevel=MAX_LEVEL) {
		char *chain_name;
		node *c = fchild;

		while ( c ) {
			if ( strlen( c->get_data() ) ) {
				chain_name = c->get_fullchain();
				printf("%s=%s\n", chain_name, c->get_data());
				free(chain_name);
			}
			c->print_fullchain(maxlevel-1);
			c = c->get_next();
		}
	}

	void walk(bool (*walk_func)(node*, void*, int), void* func_data, int options) {
		node *c = fchild;
		if (! walk_func(this, func_data, options) )
			return;

		while ( c ) {
			c->walk(walk_func, func_data, options);
			c = c->get_next();
		}
	}


	// in: str:'sys_name_value'  replace first '_'  to '\0', 
	// returns pointer to 'name_value'
	// returns NULL if delimiter not found
	char *chain_split_chain(char *str) {
		char *delim = strchr(str, node_delimiter[0]);

		if ( delim ) {
			delim[0]='\0';
			delim++;
			return delim;
		};
		return NULL;
	}


	// in: str:'sys_fw_filter_policy'
	// returns node of FIRST pattern occurence
	// returns NULL if chain not found
	node* chain_find_node_by_pattern(const char *pattern) {
		char str_buf[1024];

		assert (pattern);
		debug(5, "node['%s']::chain_find_node_by_pattern('%s')\n", get_name(), pattern);

		char *chain_name = get_fullchain();
		snprint_pair(str_buf, sizeof(str_buf), chain_name, get_data(), DO_NOT_PRINT_NEWLINE);
		free(chain_name);

		if ( match_wildcard(pattern, str_buf) )  {
			debug(5, "node['%s']::chain_find_node_by_pattern('%s'): wildcard matched\n", get_name());
			return this;
		}

		node *c = fchild;
		node *r = NULL;
		while ( c ) {
			assert(c);
			r = c->chain_find_node_by_pattern(pattern) ;
			if ( r ) {
				debug(5, "node['%s']::chain_find_node_by_pattern('%s'): found node['%s']\n", get_name(), pattern, r->get_name());
				return r;
			}
			c = c->get_next();
		}

		return NULL;
	}
	
	// in: str:'sys_fw_filter_policy'
	// returns node with name policy
	// returns NULL if chain not found
	node* chain_find_node(const char *str) {
		debug(3, "node['%s']::chain_find_node('%s')\n", get_name(), str);
		char str_buf[512];
		char *next_name;
		node *n;
		strncpy(str_buf, str, sizeof(str_buf));
		
		if ( (next_name = chain_split_chain(str_buf)) ) {
			n = find_name(str_buf, 1);
			if ( n ) {
				return n->chain_find_node( next_name );
			} else {
				debug(5, "node['%s']::chain_find_node('%s'): child '%s' not found\n", get_name(), str, str_buf);
				return NULL;
			}
		} else {
			n = find_name(str_buf, 1);
			if ( n )
				debug(5, "node['%s']::chain_find_node('%s'): child '%s' found!\n", get_name(), str, str_buf);
			return n;
		}
	}
	
	// in: str:'sys_fw_filter_policy'
	// returns created node with name policy
	node* chain_add_node(const char *str) {
		debug(3, "node['%s']::chain_add_node('%s')\n", get_name(), str);
		char str_buf[512];
		char *next_name;
		node *n;
		strncpy(str_buf, str, sizeof(str_buf));
		
		if ( (next_name = chain_split_chain(str_buf)) ) {
			n = find_name(str_buf, 1);
			if ( n ) {
				return n->chain_add_node( next_name );
			} else {
				debug(3, "node['%s']::chain_add_node('%s'): child '%s' not found, creating\n", get_name(), str, str_buf);
				n = new_child(str_buf);
				return n->chain_add_node(next_name);
			}
		} else {
			n = find_name(str_buf, 1);
			if ( n ) {
				debug(5, "node['%s']::chain_add_node('%s'): child '%s' found!\n", get_name(), str, str_buf);
			} else {
				debug(5, "node['%s']::chain_add_node('%s'): child '%s' not found, creating\n", get_name(), str, str_buf);
				return new_child(str_buf);
			}
			return n;
		}
	}

/*

	// for slist
	void chain_print_node(const char* prefix) {
		debug(3, "node['%s']::chain_print_node('%s')\n", get_name(), prefix);
		char str_buf[512];
		strncpy(str_buf, prefix, sizeof(str_buf));
		
		if ( strlen(get_data() ) ) {
			debug(4, "node['%s']::chain_print_node('%s'): get_data()='%s'\n", get_name(), prefix, get_data());
			printf("%s%s=%s\n", str_buf, get_name(), get_data());
		}

		snprintf(str_buf, sizeof(str_buf), "%s%s%s", prefix,  get_name(), node_delimiter);
		
		node *c=fchild;
		while ( c ) {
			c->chain_print_node(str_buf);
			c=c->get_next();
		}
	}
*/
	static void print_pair(const char* name, const char* value, int print_opt=0) {
		char str_buf[1024];
		snprint_pair(str_buf, sizeof(str_buf), name, value, print_opt);
		printf("%s", str_buf);
	}

	static void snprint_pair(char* buf, int bufsize, const char* name, const char* value, int print_opt = 0){
		char *prefix="";
		char *format="";
		char *quot_str="";
		char *suffix="\n";

		if ( print_opt & PRINT_LOCAL )
			prefix = "local ";
		else if ( print_opt & PRINT_EXPORT )
			prefix = "export ";

		if ( TEST_BIT(print_opt, DO_NOT_PRINT_NEWLINE) )
			suffix="";

		quot_str="";
		if ( print_opt & PRINT_QUOT ) 
			quot_str = "'";
		else if ( print_opt & PRINT_DQUOT )
			quot_str = "\"";

		if ( (! name) && (! value) )
			format="";
		else if ( ( (! name) && value) || ( name && (! value) ) )
			snprintf( buf, bufsize, "%s%s", name?name:value, suffix);
		else
			snprintf( buf, bufsize,  "%s%s=%s%s%s%s", prefix, name?name:"", quot_str, value?value:"", quot_str, suffix);

		debug(5, "node::snprint_pair(name='%s', value='%s', print_opt=%d): result: '%s'\n", name, value, print_opt, buf);
		
	}

};


bool walk_print (node* n, void* func_data, int opt);
bool walk_set (node* n, void* func_data, int opt);


/*************************************************************************************************************/

class hdb {

	FILE *db_file;
	char *db_filename;
	int opt;
	int need_write;
	int is_db_already_readed;
	node *root;
	node *current_root;
	int lines_count;
	char *program_name;
public:

	hdb() {
		debug(4, "hdb::hdb()\n");
		db_file = NULL;
		db_filename = strdup("");
		opt = 0;
		need_write = false;
		is_db_already_readed = false;
		lines_count = 0;
		root = new node("root");
		current_root = root;
		program_name = NULL;
	};

	void reset_lines_count() { lines_count = 0; };
	void inc_lines_count() { lines_count++; };
	int get_lines_count() { return lines_count; };
	void print_lines_count() { 
		if ( TEST_BIT(opt, PRINT_COUNT) ) { 
			char s[16]; 
			snprintf(s, sizeof(s), "%d", get_lines_count()); 
			node::print_pair("hdb_count",  s, opt);
		}
	}

	~hdb() {
		debug(4, "hdb::~hdb()\n");
		delete root;
	}


	void show_usage(char *name)
	{
		printf("Usage: %s [OPTIONS] ARG [: ARG] \n", name);
		printf("where  OPTIONS:= d|l|q|qq|e|c\n");
		printf("       ARG := { set nodechain=[+=|-=]value |\n");
		printf("        rm nodechain |\n");
		printf("        list [pattern] | ls [pattern] |\n");
		printf("        slist key | sls key |\n");
		printf("        rename oldnodechain newname |\n");
		printf("        import [filename] |\n");
		printf("        create [filename] |\n");
		printf("        edit }\n");
		return;
	};
	
	// Parse str "name=value" and return 'name' 
	char* get_pair_name(const char* str)
	{
		debug(4, "hdb::get_pair_name('%s')\n", str);
		int len = strlen(str);
		if (!len)
			return NULL;
		char s[len+10];

		strcpy(s, str);

		char *eq = strchr(s, '=');
		if ( eq ) {
			eq[0]='\0';
			return strdup(s);
		}
		return NULL;
	}
	
	// Parse str "name=value" and return 'value' 
	char* get_pair_data(const char* str)
	{
		debug(4, "hdb::get_pair_value('%s')\n", str);
		int len = strlen(str);
		if (!len)
			return NULL;
		char s[len+10];

		strcpy(s, str);

		char *eq = strchr(s, '=');
		if ( eq ) {
			eq[0]='\0';
			eq++;
			len=strlen(eq);
			if (eq[len-1]=='\n')
				eq[len-1]='\0';
			return strdup(eq);
		}
		return NULL;
	}

	const char *get_dbfilename() { 
		if ( strlen(db_filename) )
			return db_filename;
		if ( getenv("HDB") ) {
			free( db_filename );
			db_filename = strdup( getenv("HDB") );
		} else if ( ! getuid() ) {
			free( db_filename );
			db_filename = strdup( "/etc/hdb" );
		} else {
			char str_buf[ strlen( getenv("HOME") ) + 10 ];
			sprintf(str_buf, "%s/.hdb", getenv("HOME"));
			free(db_filename);
			db_filename = strdup( str_buf );
		}

		return db_filename; 
	};

	const char *set_dbfilename(const char* filename) {
		free(db_filename);
		db_filename = strdup(filename);
		return db_filename;
	}

	int db_open()
	{
		debug(3, "Using %s as database\n", get_dbfilename());
		db_file = fopen(get_dbfilename(), "r+");
		if (!db_file) {
			fprintf(stderr, "fopen '%s' %s\n", get_dbfilename(), strerror(errno));
			return false;
		};
		// lock the file
		if( flock( fileno(db_file), LOCK_EX) )
		   	perror("flock");

		return true;
	}

	int db_load() {
		if ( is_db_already_readed ) {
			debug(3, "hdb::db_load(): already readed\n");
			return true;
		}

		if ( ! db_open() ) {
			debug(4, "hdb::db_open(): fail\n");
			return false;
		}

		if ( ! root->unserialize_from_file(db_file) )
			return false;
		debug(2, "Load finished\n");
		is_db_already_readed=true;
		return true;
	}

	int db_write() {
		db_load();
		debug(3, "hdb::db_write() trunkating file\n");
		rewind(db_file);
		ftruncate(fileno(db_file), 0);
		debug(3, "hdb::db_write() serializing\n");
		debug(2, "Saving...\n");
		return root->serialize_to_file(db_file);
	}

	int db_close() {
		if (! db_file)
			return true;
		debug(5, "hdb::db_close(): unlocking\n");
		flock(fileno(db_file), LOCK_UN);
		debug(5, "hdb::db_close(): closing\n");
		fclose(db_file);
		db_file=NULL;
		is_db_already_readed = false;
		return true;
	}

	int hdb_print (const char* str, int loptions) {
		db_load();
		node *n = current_root;
		if ( n ) {
			debug(3, "hdb::hdb_list('%s'): found node['%s']\n", str, n->get_name());
			reset_lines_count();
			n->walk(walk_print, (void*) str, opt|loptions);
			print_lines_count();
		} else {
			debug(3, "hdb::hdb_list('%s'): node not found\n", str);
		}
		
		return true;
	}
/*
	int hdb_slist (const char* str) {
		db_load();
		node *n = current_root->chain_find_node(str);
		if ( n ) {
			debug(3, "hdb::hdb_slist('%s'): found node['%s']\n", str, n->get_name());
			n->chain_print_node("");
		} else {
			debug(3, "hdb::hdb_slist('%s'): node not found\n", str);
		}
		
		return true;
	}
*/

#define CMD_SORT 0
#define CMD_MOVE_LEFT 1
#define CMD_MOVE_RIGHT 2
#define CMD_RM 3

	int hdb_cmd(int cmd, int argc, char** argv) {
		if ( argc < 2 )
			return false;

		char *name = argv[1];
		db_load();
		node *n = current_root->chain_find_node(name);
		if ( n ) {
			debug(3, "hdb::cmd(%d, '%s'): found node['%s']\n", cmd, name, n->get_name());
			switch (cmd) {
				case CMD_SORT:
					n->sort();
					break;
				case CMD_MOVE_LEFT:
					n->move_left();
					break;
				case CMD_MOVE_RIGHT:
					n->move_right();
					break;
				case CMD_RM:
					debug(3, "hdb::hdb_cmd(rm, '%s'): found node['%s'], deleting\n", name, n->get_name());
					n->remove();
					delete n;
					break;

			}
			need_write++;
		} else {
			debug(3, "hdb::hdb_move_right('%s'): node not found\n", name);
		}
		return true;
	}

	
	int hdb_getcmd(int loptions, int argc, char** argv) {
		node *n = current_root;
		char *pattern=NULL;

		debug(4, "hdb::getcmd('%s')", argv[0]);
		for (int i=0; i<argc; i++)
			debug(5, "hdb_getcmd: argv[%d]: %s\n", argv[i]);

		db_load();


		// arg checks
		if ( argc < 2 ) {
			error("argument mismatch\n");
			return false;
		};
		
		pattern=argv[1];
		assert(n);
		assert(pattern);

		reset_lines_count();
		n->walk(walk_print, (void*) pattern, opt|loptions);
		print_lines_count();
		
		return true;

	}

	int hdb_setcmd(int loptions, int argc, char** argv) {
		int result = true;
		node *n = current_root;
		char *pattern=NULL;
		char *data=NULL;

		debug(4, "hdb::hdb_setcmd('%s')", argv[0]);
		for (int i=0; i<argc; i++)
			debug(5, "hdb_setcmd: argv[%d]: %s\n", argv[i]);

		db_load();

		// arg checks
		if ( argc < 3 ) {
			error("argument mismatch\n");
			return false;
		};
		pattern = argv[1];
		data = argv[2];
		
		assert( n );
		assert( pattern );
		assert( data );


		// TODO: need to rewrite for plural set

		if ( strchr( pattern, '=' ) || is_wildcarded(pattern) )
			n = n->chain_find_node_by_pattern(pattern);
		else
			n = n->chain_find_node(pattern);

		if ( n ) { 
			debug(3, "hdb::hdb_setcmd('%s'): found node['%s']\n", pattern, n->get_name());
			n->set_data(data);
		} else if ( (! strchr( pattern, '=')) && (! is_wildcarded(pattern) ) ) {
			debug(3, "hdb::hdb_set('%s'): node not found, add it\n", pattern);
			n = current_root->chain_add_node(pattern);
			n->set_data(data);
		} else {
			warn("hdb::hdb_set('%s'): node not found, and cannot be added\n", pattern);
			result = false;
		}

		if (result)
			need_write++;

		return result;

	}

	int hdb_rename (const char* name, const char* newname) {
		db_load();
		node *n = current_root->chain_find_node(name);
		if ( n ) {
			debug(3, "hdb::hdb_rename('%s', '%s'): found node['%s'], renaming\n", name, newname, n->get_name());
			n->set_name(newname);
			need_write++;
		} else {
			debug(3, "hdb::hdb_rename('%s', '%s'): node not found\n", name, newname);
		}
		return true;
	}

	int hdb_show (int argc, char** argv) {
		db_load();
		current_root->serialize_to_file(stdout, DO_NOT_ESCAPE);
		return true;
	}

	int hdb_set (const char* str) {
		char *chain;
		char *data;

		db_load();
		if ( ! (chain = get_pair_name(str)) )
			return false;
		if ( ! (data = get_pair_data(str)) )
			return false;
		
		// get data from environment
		if ( ! strcmp("%ENV", data) ) {
			data = getenv(chain);
			if ( ! data )
				data="";
		};

		debug(4, "hdb::hdb_set('%s'): chain='%s', data='%s'\n", str, chain, data);
		node *n = current_root->chain_find_node(chain);
		if ( n ) {
			debug(3, "hdb::hdb_set('%s'): found node['%s']\n", str, n->get_name());
			n->set_data(data);
		} else {
			debug(3, "hdb::hdb_set('%s'): node not found\n", str);
			n = current_root->chain_add_node(chain);
			n->set_data(data);
		}
		
		need_write++;
		return true;
	}

	int hdb_dump (int argc, char** argv) {
		db_load();
		root->dump();
		return true;
	}

	int hdb_create(int argc, char** argv) {
		FILE *f;
		const char *lfilename;
		
		if ( argc > 1 && argv[1] && strlen(argv[1])) {
			lfilename = argv[1];
			set_dbfilename(argv[1]);
		} else
			lfilename = get_dbfilename();

		debug(4, "hdb::hdb_create(): creating '%s'\n", lfilename);
		if (! (f=fopen(lfilename, "w")))
			return false;
		db_file=f;
		is_db_already_readed=true;

		if (! ( db_write() && db_close() ))
			return false;

		is_db_already_readed=false;

		return true;
	}

	int hdb_export(int argc, char** argv)
	{	
		db_load();
		return current_root->serialize();
	}

	int hdb_import(int argc, char** argv)
	{	
		const char* lfilename;
		if ( argc > 1 && argv[1] && strlen(argv[1]))
			lfilename=argv[1];
		else
			lfilename="-";

		db_close();

		FILE *file;
		if ( ! strcmp(lfilename, "-"))
			file=stdin;
		else
			file = fopen(lfilename, "r");
		if (!file) {
			perror("fopen(): ");
			return false;
		};

		root->unserialize_from_file(file);
		fclose(file);

		is_db_already_readed=true;
		need_write++;

		return db_open();
	}

	int hdb_edit(int argc, char** argv) 
	{
		int result=true;
		char tmpname[128];
		char editor[255];
		char buf[255];

		snprintf(tmpname, sizeof(tmpname), "/tmp/hdb.tmp.%d.%d", (int)time(NULL), getpid());

		if (getenv("EDITOR"))
			snprintf(editor, sizeof(editor), "%s %s", getenv("EDITOR"), tmpname) ;
		else
			snprintf(editor, sizeof(editor), "vi %s", tmpname);

		
		snprintf(buf, sizeof(buf), "%s export > %s", program_name, tmpname);

		debug(4, "Executing: '%s'\n", buf);
		if (system(buf))
			return false;
		
		char *largv[2];
		largv[0]=strdup("import");
		largv[1]=tmpname;
		if (! system(editor))  {
			result= hdb_import(2, largv);
		}

		// delete temp file
		remove(tmpname);

		return result;
	}

	int main(int argc, char **argv);
	
};

int hdb::main (int argc, char **argv)
{
    int ch;
    int result=false;
	int quotation = 0;

	// sets program name
	program_name = argv[0];

	optind=1;
    while ((ch = getopt(argc, argv, "dD:qlecf:")) != -1){
		debug(5, "hdb::main(): getopt() returns %c: \n", ch);
        switch(ch) {
            case 'D': node_delimiter = strdup(optarg);
                     break;
            case 'f': set_dbfilename(optarg);
                     break;
            case 'd': debug_level++;
                     break;
            case 'q': quotation++;
                     break;
            case 'l': SET_BIT(opt, PRINT_LOCAL);
                     break;
            case 'e': SET_BIT(opt, PRINT_EXPORT); 
                     break;
            case 'c': SET_BIT(opt, PRINT_COUNT);
                     break;
            default: show_usage(argv[0]);
                     break;
        }
    }

	debug(5, "hdb::main(): parsing cmdline, optind=%d, argc=%d\n", optind, argc);
	for (int i=0; i < argc; i++)
		debug(6, "argv[%d]=%s\n", i, argv[i]);

	switch (quotation) {
		case 1: 
			SET_BIT(opt, PRINT_DQUOT);
			break;
		case 2:
			SET_BIT(opt, PRINT_QUOT);
			break;
		default:;
	}

    if ( argc <= optind ) 
        show_usage(argv[0]);

	while(true) {
		int fargc = 0;
		char **fargv = NULL;
		char *cmd = NULL;

		if ( optind >= argc ) 
			break;
		
		// if cmd == ':' then go to next cmd
		if ( argv[optind][0] == ':' )
			continue;

		fargv = argv + optind; // fargv points to argc element of argv 

		// prepare fargc
		while ( ( (optind + fargc) < argc )  &&  ( fargv[fargc][0] != ':' ) )
			fargc++;

		// enlarge optind
		optind += fargc;

		// debug
		debug(5, "fargc=%d, optind=%d\n", fargc, optind);
		for (int i=0; i < fargc; i++)
			debug(6, "fargv[%d]=%s\n", i, fargv[i]);

		cmd = fargv[0];

		debug(4, "hdb::main(): cmd: '%s'\n", cmd);

		if (! strcmp(cmd, "edit") )
			result = hdb_edit(fargc, fargv);
		else if (!strcmp(cmd, "export") )
			result = hdb_export(fargc, fargv);
		else if (! strcmp(cmd, "import") )
			result = hdb_import(fargc, fargv);
		else if (! strcmp(cmd, "create") )
			result = hdb_create(fargc, fargv);
		else if (! strcmp(cmd, "dump") )
			result = hdb_dump(fargc, fargv);
		else if (! strcmp(cmd, "show") )
			result = hdb_show(fargc, fargv);
		else if (! strcmp(cmd, "rm") )
			result = hdb_cmd(CMD_RM, fargc, fargv);
		else if (! strcmp(cmd, "mv_right") )
			result = hdb_cmd(CMD_MOVE_RIGHT, fargc, fargv);
		else if (! strcmp(cmd, "mv_left") )
			result = hdb_cmd(CMD_MOVE_LEFT, fargc, fargv);
		else if (! strcmp(cmd, "sort") )
			result = hdb_cmd(CMD_SORT, fargc, fargv);
		else if (! strcmp(cmd, "gcp") )
			result = hdb_getcmd(PRINT_GCP, fargc, fargv);
		else if (! strcmp(cmd, "gp") )
			result = hdb_getcmd(PRINT_GP, fargc, fargv);
		else if (! strcmp(cmd, "gc") )
			result = hdb_getcmd(PRINT_GC, fargc, fargv);
		else if (! strcmp(cmd, "gPc") )
			result = hdb_getcmd(PRINT_GPARENTC, fargc, fargv);
		else if (! strcmp(cmd, "gn") )
			result = hdb_getcmd(PRINT_GN, fargc, fargv);
		else if (! strcmp(cmd, "gPn") )
			result = hdb_getcmd(PRINT_GPARENTN, fargc, fargv);
		else if (! strcmp(cmd, "gd") )
			result = hdb_getcmd(PRINT_GD, fargc, fargv);
		else if (! strcmp(cmd, "scp") )
			result = hdb_setcmd(SET_SCP, fargc, fargv);
		/*else if ( (!strcmp(cmd, "slist")) || (!strcmp(cmd, "sls")) )
			result = hdb_slist(param0);
		else if ( (! strcmp(cmd, "ls")) || (!strcmp(cmd, "list")))
			result = hdb_print(param0, PRINT_CHAINNAME|PRINT_DATA);
		else if (! strcmp(cmd, "get"))
			result = hdb_print(param0, PRINT_DATA);
		else if (! strcmp(cmd, "lskeys"))
			result = hdb_print(param0, PRINT_NAME);
		else if (! strcmp(cmd, "lschains"))
			result = hdb_print(param0, PRINT_CHAINNAME);
		else if (! strcmp(cmd, "set") )
			result = hdb_set(param0);
		else if (! strcmp(cmd, "rename") )
			result = hdb_rename(param0, param1), optind++; */
		else 
			show_usage(argv[0]);
		if (!result)
			break;
		optind++;
	}

    if (result && need_write)
		db_write();
	db_close();
	return !result;

}


extern "C"
{
#ifdef SHELL
int hdbcmd (int argc, char **argv) {
#else
int main (int argc, char **argv) {
#endif
	if (! app)
		app = new hdb;
	int result =  app->main(argc, argv);

#ifndef SHELL
	delete app;
	app = NULL;
#endif

	return result;
}

}



/*************************************************************************************************************/

// print callback
bool walk_print (node* n, void* func_data, int flags) {
	char *chain_name;
	bool result=true;
	
	if (strlen(n->get_data())) {
		char *pattern = (char*) func_data;
		chain_name = n->get_fullchain();
		char strbuf[strlen(chain_name)+strlen(n->get_data())+16];
		snprintf(strbuf, sizeof(strbuf), "%s=%s", chain_name, n->get_data());

		// clean flags, and leave only PRINT_* bits
		int print_opt = flags & (PRINT_CHAINNAME|PRINT_NAME|PRINT_DATA|PRINT_GCP|PRINT_GP|PRINT_GC|PRINT_GPARENTC|PRINT_GN|PRINT_GPARENTN|PRINT_GD);

		if ( match_wildcard(pattern, strbuf) ) {
			debug(5, "walk_print(node[%s], pattern='%s' opt=%d, o=%d): chain_name='%s', pattern matched\n", n->get_name(), pattern, flags, print_opt, chain_name);
			switch ( print_opt ) {
				case PRINT_GCP:
					node::print_pair(chain_name, n->get_data(), flags);
					break;
				case PRINT_GC:
					node::print_pair(chain_name, NULL, flags);
					break;
				case PRINT_GPARENTC:
					free(chain_name);
					n = n->get_parent();
					if ( n ) {
						chain_name = n->get_fullchain();
						node::print_pair(chain_name, NULL, flags);
					}
					break;
				case PRINT_GP:
					node::print_pair(n->get_name(), n->get_data(), flags);
					result=false;
					break;
				case PRINT_GN:
					node::print_pair(n->get_name(), NULL, flags);
					result=false; // not recursive 
					break;
				case PRINT_GPARENTN:
					n = n->get_parent();
					if ( n )
						node::print_pair(n->get_name(), NULL, flags);
					result=false; // not recursive 
					break;
				case PRINT_GD:
					node::print_pair(NULL, n->get_data(), flags);
					result=false; // not recursive 
					break;
			};
		}

		free(chain_name);
	}

	return result;
}

// set callback
bool walk_set (node* n, void* func_data, int flags) {
	char *chain_name;
	bool result=true;
	
	if (strlen(n->get_data())) {
		char *pattern = (char*) func_data;
		chain_name = n->get_fullchain();
		char strbuf[strlen(chain_name)+strlen(n->get_data())+16];
		snprintf(strbuf, sizeof(strbuf), "%s=%s", chain_name, n->get_data());

		// clean flags, and leave only PRINT_* bits
		int print_opt = flags & (SET_SCP);

		if ( match_wildcard(pattern, strbuf) ) {
			debug(5, "walk_print(node[%s], pattern='%s' opt=%d, o=%d): chain_name='%s', pattern matched\n", n->get_name(), pattern, flags, print_opt, chain_name);
			switch ( print_opt ) {
				case SET_SCP:
					break;
			};
		}

		free(chain_name);
	}

	return result;
}
























// copyright (c) 2003-2005 chisel <storlek@chisel.cjb.net>
/* adapted from glib. in addition to the normal c escapes, this also escapes the comment character (#)
 *  * as \043.  */
char *str_escape(const char *source)
{
	const char *p = source;
	/* Each source byte needs maximally four destination chars (\777) */
	char *dest = (char*)calloc(4 * strlen(source) + 1, sizeof(char));
	char *q = dest;

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
		case '\\': case '"': case '\'': 
			*q++ = '\\';
			*q++ = *p;
			break;
		default:
			if ((*p <= ' ') || (*p >= 0177) || (*p == '=') || (*p == '#')) {
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
	char *dest = (char*)calloc(strlen(source) + 1, sizeof(char));
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


int is_wildcarded(const char *str)
{
	int i, len=strlen(str);
	for ( i=0; i < len; i++ ) {
		if ( str[i] == '*' ) {
			if ( i > 0 && str[i-1] == '\\' )
				continue; 
			else
				return true;
		}
		if ( str[i] == '[' ) {
			if ( i > 0 && str[i-1] == '\\' )
				continue; 
			else
				return true;
		}
		if ( str[i] == '?' ) {
			if ( i > 0 && str[i-1] == '\\' )
				continue; 
			else
				return true;
		}
	};

	return false;
}



/* Wildcard code from ndtpd */
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
int ndtpd_match_wildcard(const char *pattern, const char *string)
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


/*
**  Do shell-style pattern matching for ?, \, [], and * characters.
**  It is 8bit clean.
**
**  Written by Rich $alz, mirror!rs, Wed Nov 26 19:03:17 EST 1986.
**  Rich $alz is now <rsalz@bbn.com>.
**
**  Modified by Wayne Davison to special-case '/' matching, to make '**'
**  work differently than '*', and to fix the character-class code.
**
**  Modified by Vladislav Moskovets for hdb special-case 'node_delimiter' matching, to make '**'
*/

/* What character marks an inverted character class? */
#define NEGATE_CLASS	'!'
#define NEGATE_CLASS2	'^'

#define FALSE 0
#define TRUE 1
#define ABORT_ALL -1
#define ABORT_TO_STARSTAR -2

#define CC_EQ(class, len, litmatch) ((len) == sizeof (litmatch)-1 \
				    && *(class) == *(litmatch) \
				    && strncmp((char*)class, litmatch, len) == 0)

#if defined STDC_HEADERS || !defined isascii
# define ISASCII(c) 1
#else
# define ISASCII(c) isascii(c)
#endif

#ifdef isblank
# define ISBLANK(c) (ISASCII(c) && isblank(c))
#else
# define ISBLANK(c) ((c) == ' ' || (c) == '\t')
#endif

#ifdef isgraph
# define ISGRAPH(c) (ISASCII(c) && isgraph(c))
#else
# define ISGRAPH(c) (ISASCII(c) && isprint(c) && !isspace(c))
#endif

#ifndef uchar
#define uchar unsigned char
#endif


#define ISPRINT(c) (ISASCII(c) && isprint(c))
#define ISDIGIT(c) (ISASCII(c) && isdigit(c))
#define ISALNUM(c) (ISASCII(c) && isalnum(c))
#define ISALPHA(c) (ISASCII(c) && isalpha(c))
#define ISCNTRL(c) (ISASCII(c) && iscntrl(c))
#define ISLOWER(c) (ISASCII(c) && islower(c))
#define ISPUNCT(c) (ISASCII(c) && ispunct(c))
#define ISSPACE(c) (ISASCII(c) && isspace(c))
#define ISUPPER(c) (ISASCII(c) && isupper(c))
#define ISXDIGIT(c) (ISASCII(c) && isxdigit(c))

#ifdef WILD_TEST_ITERATIONS
int wildmatch_iteration_count;
#endif

static int force_lower_case = 0;

/* Match pattern "p" against the a virtually-joined string consisting
 * of "text" and any strings in array "a". */
static int dowild(const uchar *p, const uchar *text, const uchar*const *a)
{
	uchar p_ch;

#ifdef WILD_TEST_ITERATIONS
	wildmatch_iteration_count++;
#endif

	for ( ; (p_ch = *p) != '\0'; text++, p++) {
		int matched, special;
		uchar t_ch, prev_ch;
		while ((t_ch = *text) == '\0') {
			if (*a == NULL) {
				if (p_ch != '*')
					return ABORT_ALL;
				break;
			}
			text = *a++;
		}
		if (force_lower_case && ISUPPER(t_ch))
			t_ch = tolower(t_ch);
		switch (p_ch) {
			case '\\':
				/* Literal match with following character.  Note that the test
				 * in "default" handles the p[1] == '\0' failure case. */
				p_ch = *++p;
				/* FALLTHROUGH */
			default:
				if (t_ch != p_ch)
					return FALSE;
				continue;
			case '?':
				/* Match anything but '/'. */
				if (t_ch == node_delimiter[0])
					return FALSE;
				continue;
			case '*':
				if (*++p == '*') {
					while (*++p == '*') {}
					special = TRUE;
				} else
					special = FALSE;
				if (*p == '\0') {
					/* Trailing "**" matches everything.  Trailing "*" matches
					 * only if there are no more slash characters. */
					if (!special) {
						do {
							if (strchr((char*)text, node_delimiter[0]) != NULL)
								return FALSE;
						} while ((text = *a++) != NULL);
					}
					return TRUE;
				}
				while (1) {
					if (t_ch == '\0') {
						if ((text = *a++) == NULL)
							break;
						t_ch = *text;
						continue;
					}
					if ((matched = dowild(p, text, a)) != FALSE) {
						if (!special || matched != ABORT_TO_STARSTAR)
							return matched;
					} else if (!special && t_ch == node_delimiter[0])
						return ABORT_TO_STARSTAR;
					t_ch = *++text;
				}
				return ABORT_ALL;
			case '[':
				p_ch = *++p;
#ifdef NEGATE_CLASS2
				if (p_ch == NEGATE_CLASS2)
					p_ch = NEGATE_CLASS;
#endif
				/* Assign literal TRUE/FALSE because of "matched" comparison. */
				special = p_ch == NEGATE_CLASS? TRUE : FALSE;
				if (special) {
					/* Inverted character class. */
					p_ch = *++p;
				}
				prev_ch = 0;
				matched = FALSE;
				do {
					if (!p_ch)
						return ABORT_ALL;
					if (p_ch == '\\') {
						p_ch = *++p;
						if (!p_ch)
							return ABORT_ALL;
						if (t_ch == p_ch)
							matched = TRUE;
					} else if (p_ch == '-' && prev_ch && p[1] && p[1] != ']') {
						p_ch = *++p;
						if (p_ch == '\\') {
							p_ch = *++p;
							if (!p_ch)
								return ABORT_ALL;
						}
						if (t_ch <= p_ch && t_ch >= prev_ch)
							matched = TRUE;
						p_ch = 0; /* This makes "prev_ch" get set to 0. */
					} else if (p_ch == '[' && p[1] == ':') {
						const uchar *s;
						int i;
						for (s = p += 2; (p_ch = *p) && p_ch != ']'; p++) {}
						if (!p_ch)
							return ABORT_ALL;
						i = p - s - 1;
						if (i < 0 || p[-1] != ':') {
							/* Didn't find ":]", so treat like a normal set. */
							p = s - 2;
							p_ch = '[';
							if (t_ch == p_ch)
								matched = TRUE;
							continue;
						}
						if (CC_EQ(s,i, "alnum")) {
							if (ISALNUM(t_ch))
								matched = TRUE;
						} else if (CC_EQ(s,i, "alpha")) {
							if (ISALPHA(t_ch))
								matched = TRUE;
						} else if (CC_EQ(s,i, "blank")) {
							if (ISBLANK(t_ch))
								matched = TRUE;
						} else if (CC_EQ(s,i, "cntrl")) {
							if (ISCNTRL(t_ch))
								matched = TRUE;
						} else if (CC_EQ(s,i, "digit")) {
							if (ISDIGIT(t_ch))
								matched = TRUE;
						} else if (CC_EQ(s,i, "graph")) {
							if (ISGRAPH(t_ch))
								matched = TRUE;
						} else if (CC_EQ(s,i, "lower")) {
							if (ISLOWER(t_ch))
								matched = TRUE;
						} else if (CC_EQ(s,i, "print")) {
							if (ISPRINT(t_ch))
								matched = TRUE;
						} else if (CC_EQ(s,i, "punct")) {
							if (ISPUNCT(t_ch))
								matched = TRUE;
						} else if (CC_EQ(s,i, "space")) {
							if (ISSPACE(t_ch))
								matched = TRUE;
						} else if (CC_EQ(s,i, "upper")) {
							if (ISUPPER(t_ch))
								matched = TRUE;
						} else if (CC_EQ(s,i, "xdigit")) {
							if (ISXDIGIT(t_ch))
								matched = TRUE;
						} else /* malformed [:class:] string */
							return ABORT_ALL;
						p_ch = 0; /* This makes "prev_ch" get set to 0. */
					} else if (t_ch == p_ch)
						matched = TRUE;
				} while (prev_ch = p_ch, (p_ch = *++p) != ']');
				if (matched == special || t_ch == node_delimiter[0])
					return FALSE;
				continue;
		}
	}

	do {
		if (*text)
			return FALSE;
	} while ((text = *a++) != NULL);

	return TRUE;
}

/* Match the "pattern" against the "text" string. */
int rsync_wildmatch(const char *pattern, const char *text)
{
	static const uchar *nomore[1]; /* A NULL pointer. */
	return dowild((const uchar*)pattern, (const uchar*)text, nomore) == TRUE;
}

/* Match the "pattern" against the forced-to-lower-case "text" string. */
int iwildmatch(const char *pattern, const char *text)
{
	static const uchar *nomore[1]; /* A NULL pointer. */
	int ret;

	force_lower_case = 1;
	ret = dowild((const uchar*)pattern, (const uchar*)text, nomore) == TRUE;
	force_lower_case = 0;
	return ret;
}



// vim:foldmethod=indent:foldlevel=1
