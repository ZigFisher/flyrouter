#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/reboot.h>

#include <stdio.h>
#include <wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <string.h>
#include <dirent.h>
#include "bbutils.h"
#include "utils.h"

void _dbg(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

int write_file(char *filename, char* buf, int size)
{
	FILE *file;
    size_t written = 0;

    dbg("dbg: Writing %d bytes to '%s'...\n", size, filename);
	
	if ( ! strcmp(filename, "-") ) 
		file = stdout;
	else
		file = fopen(filename, "w");

	if ( ! file ) {
		perror(NULL);
		return 0;
	}
	written = fwrite(buf, size, 1, file);

    dbg("dbg: written %d bytes\n", written*size);

	fclose(file);
	return 1;
}

int file_size(const char* filename)
{
	FILE *file = NULL;
    int result = -1;
	if ( ! (file = fopen(filename, "r")) ) {
		perror(filename);
		return result;
	}
	fseek (file , 0 , SEEK_END);
	result = ftell(file);
	if ( file ) 
		fclose(file);
	dbg("file size: %d\n", result);
    return result;
}

unsigned char* read_file(const char *filename, int* size)
{
    int fsize;
    char* big_buf;
	FILE* file = NULL;

    fsize = file_size(filename);
    *size = fsize;

    if ( fsize < 0 )
        return NULL;

	big_buf = malloc(fsize);
    if ( !big_buf ) {
        perror("malloc");
        return NULL;
    }

	memset(big_buf, 0, fsize);

	if ( ! strcmp(filename, "-") )
		file = stdin;
	else
		file = fopen(filename, "r");

	if ( ! file ) {
		perror(NULL);
		return NULL;
	}

	dbg("reading from %s\n", filename);
	fread(big_buf, fsize, 1, file);
	fclose(file);

	return big_buf;
}

unsigned short crc16_add(unsigned char b, unsigned short acc)
{
    acc ^= b;
    acc  = (acc >> 8) | (acc << 8);
    acc ^= (acc & 0xff00) << 4;
    acc ^= (acc >> 8) >> 4;
    acc ^= (acc & 0xff00) >> 5;
    return acc;
}
/*---------------------------------------------------------------------------*/

unsigned short crc16(unsigned char *p, int size)
{
    int result = 0;
    int i;
    for( i = 0; i < size; i++)
        result = crc16_add(p[i], result);
    return result;
}


int is_file_exists(const char* filename)
{
    struct stat s;
    if (stat(filename, &s) != 0)
        return FALSE;
    return TRUE;
}


static int replace_file(const char *fileName,
		struct stat *statbuf,
		void* userData,
		int depth)
{
    char *dst_dir =  (char*) userData;
    unsigned short src_crc = 0, 
                   dst_crc = 0;
    unsigned char *src_buf = NULL, 
                  *dst_buf = NULL;
    int src_size = 0, 
        dst_size = 0;

    char *src_filename = (char*)fileName;
    char *dst_filename = malloc(strlen(dst_dir) + strlen(src_filename) + 10);

    snprintf(dst_filename, strlen(dst_dir) + strlen(src_filename) + 5, "%s%s", dst_dir, fileName);

    if ( ! is_file_exists(src_filename) || !is_file_exists(dst_filename) )
        goto end;

    if ( S_ISLNK(statbuf->st_mode) )
        goto end;

    if ( strcmp("/etc/passwd", src_filename) == 0 \
            || strcmp("/etc/shadow", src_filename) == 0 \
            || strcmp("/etc/beerouter.conf", src_filename) == 0 \
            || strcmp("/etc/flyrouter.conf", src_filename) == 0 \
            || strcmp("/etc/skyrouter.conf", src_filename) == 0 \
            || strcmp("/etc/router.conf", src_filename) == 0 )
        goto end;

    src_buf = read_file(src_filename, &src_size);
    dst_buf = read_file(dst_filename, &dst_size);

    src_crc = crc16(src_buf, src_size);
    dst_crc = crc16(dst_buf, dst_size);

    dbg("src crc: 0x%x, dst crc: 0x%x    %s\n", src_crc, dst_crc, (src_crc == dst_crc? "EQUAL": "NOT EQUAL"));

    if ( src_crc == dst_crc ) {
        fprintf(stderr, "Replacing %s to symlink to %s\n", src_filename, dst_filename);
        unlink(src_filename);
        if ( symlink(dst_filename, src_filename) )
            perror("symlink");
    }

end:
    free(dst_filename);

    if (src_buf)
        free(src_buf);
    if (dst_buf)
        free(dst_buf);

	return TRUE;
};



int replace_to_symlinks(const char* src_path, const char* dst_path)
{
    //dbg("replace_to_symlinks(%s, %s)\n", src_path, dst_path);
    
    return recursive_action(src_path, ACTION_RECURSE, replace_file, NULL, (void*)dst_path, 0); 
};



