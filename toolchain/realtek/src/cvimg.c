/*
 *      Tool to convert ELF image to be the AP downloadable binary
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: cvimg.c,v 1.3 2004/12/17 09:45:32 davidhsu Exp $
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "../APP/wl/apmib.h"

#define HOME_GATEWAY
#define DEFAULT_START_ADDR	0x80300000
#define DEFAULT_BASE_ADDR	0x80000000

static unsigned short calculateChecksum(char *buf, int len);

/////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	char inFile[80]={0}, outFile[80]={0};
	int fh, size;
	struct stat status;
	char *buf;
	IMG_HEADER_Tp pHeader;
	unsigned long startAddr;
	unsigned long burnAddr;
	unsigned short checksum;

	if (argc == 4 && !strcmp(argv[1], "size_chk")) {
		unsigned long total_size;

		sscanf(argv[2], "%s", inFile);
		sscanf(argv[3], "%x", &startAddr);
		if ( stat(inFile, &status) < 0 ) {
			printf("Can't stat file! [%s]\n", inFile );
			exit(1);
		}
		printf("==============================================\n");
		printf("Summary ==>\n");
		printf("Image loading  addr          :0x%x\n", startAddr);
		printf("Image decompress end addr    :0x%x\n", ((unsigned long)DEFAULT_BASE_ADDR)+(unsigned long)status.st_size);

		total_size = startAddr - ((unsigned long)DEFAULT_BASE_ADDR);

		if (status.st_size > (int)total_size)
			printf("Error!!!! : Kernel image decompress will overwirte load image\n");

		else
			printf("Available size               :0x%08x\n",
					total_size - (unsigned long)status.st_size);

		exit(0);
	}

	// parse input arguments
	if ( argc != 6) {
		printf("Usage: cvimg [root|linux|boot|all] input-filename output-filename start-addr burn-addr\n");
		exit(1);
	}
	sscanf(argv[2], "%s", inFile);
	sscanf(argv[3], "%s", outFile);
	sscanf(argv[4], "%x", &startAddr);
	sscanf(argv[5], "%x", &burnAddr);
	// check input file and allocate buffer
	if ( stat(inFile, &status) < 0 ) {
		printf("Can't stat file! [%s]\n", inFile );
		exit(1);
	}
	size = status.st_size + sizeof(IMG_HEADER_T) + sizeof(checksum);
	if (size%2)
		size++; // pad

	buf = malloc(size);
	if (buf == NULL) {
		printf("Malloc buffer failed!\n");
		exit(1);
	}
	memset(buf, '\0', size);
	pHeader = (IMG_HEADER_Tp)buf;
	buf += sizeof(IMG_HEADER_T);

	// Read data and generate header
	fh = open(inFile, O_RDONLY);
	if ( fh == -1 ) {
		printf("Open input file error!\n");
		free( pHeader );
		exit(1);
	}
	lseek(fh, 0L, SEEK_SET);
	if ( read(fh, buf, status.st_size) != status.st_size) {
		printf("Read file error!\n");
		close(fh);
		free(pHeader);
		exit(1);
	}
	close(fh);

	if( !strcmp("root", argv[1]))
		memcpy(pHeader->signature, ROOT_HEADER, SIGNATURE_LEN);
	else if ( !strcmp("boot", argv[1]))
		memcpy(pHeader->signature, BOOT_HEADER, SIGNATURE_LEN);
	else if ( !strcmp("linux", argv[1]))
		memcpy(pHeader->signature, FW_HEADER, SIGNATURE_LEN);
	else if ( !strcmp("all", argv[1]))
		memcpy(pHeader->signature, ALL_HEADER, SIGNATURE_LEN);
	else{
		printf("not supported signature\n");
		exit(1);
	}
	
	pHeader->len = DWORD_SWAP(size-sizeof(IMG_HEADER_T));
	pHeader->startAddr = DWORD_SWAP(startAddr);
	pHeader->burnAddr = DWORD_SWAP(burnAddr);

	checksum = WORD_SWAP(calculateChecksum(buf, status.st_size));
	*((unsigned short *)&buf[size-sizeof(IMG_HEADER_T)-sizeof(checksum)]) = checksum;

	// Write image to output file
	fh = open(outFile, O_RDWR|O_CREAT|O_TRUNC);
	if ( fh == -1 ) {
		printf("Create output file error! [%s]\n", outFile);
		free(pHeader);
		exit(1);
	}
	write(fh, pHeader, size);
	close(fh);
	chmod(outFile, DEFFILEMODE);
	printf("Generate image successfully, length=%d, checksum=0x%x\n", DWORD_SWAP(pHeader->len), checksum);
	free(pHeader);
}

static unsigned short calculateChecksum(char *buf, int len)
{
	int i, j;
	unsigned short sum=0, tmp;

	j = (len/2)*2;

	for (i=0; i<j; i+=2) {
		tmp = *((unsigned short *)(buf + i));
		sum += WORD_SWAP(tmp);
	}

	if ( len % 2 ) {
		tmp = buf[len-1];
		sum += WORD_SWAP(tmp);
	}
	return ~sum+1;
}
