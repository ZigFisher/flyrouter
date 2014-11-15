#include <stdio.h>
#include <wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include "cryptfunc.c"


#define CRYPT_OFFSET (64*1024)
#define dbg if(1)printf

int main(int argc, char** argv)
{
	char *infilename;
	char *outfilename;
	if ( argc == 3 ) {
		FILE *infile = NULL;
		FILE *outfile = NULL;
		int fsize;
		infilename  = argv[1];
		outfilename = argv[2];
		dbg("infilename: %s, outfilename: %s\n", infilename, outfilename);
		char* big_buf = NULL;

		if ( ! (infile = fopen(infilename, "r")) ) {
			perror(infilename);
			return 1;
		}
		if ( ! (outfile = fopen(outfilename, "w")) ) {
			perror(outfilename);
			return 1;
		}
		dbg("open ok\n");
		fseek (infile , 0 , SEEK_END);
		fsize = ftell(infile);
		dbg("size: %d\n", fsize);
		big_buf = malloc(fsize+8);
		if (! big_buf) {
			perror("malloc");
			return 1;
		}

		memset(big_buf, 0, fsize);

		fseek (infile , 0 , SEEK_SET);
		if(errno) perror("fseek");
		fread(big_buf, fsize, 1, infile);
		if(errno) perror("fread");
		fclose(infile);

		simplecrypt(big_buf + CRYPT_OFFSET, fsize-CRYPT_OFFSET);
		fwrite(big_buf, fsize, 1, outfile);
		fclose(outfile);
		return 0;

	} else 
		fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
	return 1;
}


