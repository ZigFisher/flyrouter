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
#include "board_api.h"

//#define DEBUG
#include "utils.h"

#define RW_PARTITION_SIZE (1024*16)
#define FLASH_SAVE_FILENAME "/etc/flash.save"

char* read_datafs_from_flash()
{
	char* big_buf  = malloc(RW_PARTITION_SIZE);
	memset(big_buf, 0, RW_PARTITION_SIZE);
    sysPersistentGet(big_buf, RW_PARTITION_SIZE, 0);
	return big_buf;
}

int write_flash(char* buf)
{
	dbg("RW_PARTITION_SIZE: %x\n", RW_PARTITION_SIZE);
	dbg("sysGetPsiSize(): %d\n", sysGetPsiSize());
	dbg("flashing...\n");
    sysPersistentSet(buf, RW_PARTITION_SIZE, 0);
	return 1;
}


int init()
{
	dbg("init()\n");

	return 0;
}

void usage()
{
	// fprintf(stderr, "Usage: flash save|load|read|write [filename]\n");
	fprintf(stderr, "Usage: flash save|load [dest]|erase|upgrade\n");
}

static char flash_save_files[512];

int read_config()
{
	FILE *conf_file;
	char *p = flash_save_files;

	if ( ! (conf_file = fopen(FLASH_SAVE_FILENAME, "r")) ) {
		perror(FLASH_SAVE_FILENAME);
		return 0;
	}


	while ( fgets(p, sizeof(flash_save_files) - (p-flash_save_files), conf_file) ) {
		//dbg("readed: '%s'\n", p);
		p+=strlen(p);
	}

	while ( p = strstr(flash_save_files, "\n") )
		p[0] = ' ';

	//dbg("content of flash_save_files: '%s'\n", flash_save_files);

	fclose(conf_file);
	return 1;
}


int save()
{
	char buf1[256];
	char *pbuf = NULL;
	char filename[128] = "";
	int fsize = 0;
	int result = 1; // default value: shell false
	int exitcode = 0;
	char *tmpdir=NULL;
	if ( ! ( tmpdir = getenv("TMPDIR") ) )
		tmpdir = "/tmp";

	if ( ! read_config() )
		return 1;

    replace_to_symlinks("/etc", "/rom");

	fprintf(stderr, "Creating archive...\n");
	snprintf(filename, sizeof(filename), "%s/flash_save_%d.tar.gz", tmpdir, getpid());
	snprintf(buf1, sizeof(buf1), "tar czf %s %s >/dev/null 2>&1", filename, flash_save_files);
	if ( exitcode = system(buf1) ) {
		fprintf(stderr, "Error: tar exited with %d code\n", exitcode);
		goto end;
	};

	pbuf = read_file(filename, &fsize);
	
	if ( fsize > RW_PARTITION_SIZE ) {
		fprintf(stderr, "Error: %s oversized: %d\n", filename, fsize);
		goto end;
	}
	
	fprintf(stderr, "Flashing...\n");
	write_flash(pbuf);

	result = 0;
end:
	unlink(filename);
	free(pbuf);
	return result;
}

int load(char *dest)
{
	char buf1[256];
	char *pbuf = NULL;
	char filename[128];
	int exitcode = 0;
	char *tmpdir=NULL;
	if ( ! ( tmpdir = getenv("TMPDIR") ) )
		tmpdir = "/tmp";

	snprintf(filename, sizeof(filename), "%s/flash_save_%d.tar.gz", tmpdir, getpid());
	pbuf = read_datafs_from_flash();

    dbg("writing to %s\n", filename);
	if ( ! write_file(filename, pbuf, RW_PARTITION_SIZE)) {
		exitcode = 127;
		goto end;
	}
	free(pbuf);
	snprintf(buf1, sizeof(buf1), "tar xzf %s -C %s", filename, dest?dest:"/ram");
	dbg("system('%s')\n", buf1);
	exitcode = system(buf1);
	if ( WEXITSTATUS(exitcode) ) {
		fprintf(stderr, "Error: tar exited with %d code\n", WEXITSTATUS(exitcode) );
		exitcode = 127;
		goto end;
	};


end:
	unlink(filename);
	return exitcode;
}

int erase()
{
	char *buf = malloc(RW_PARTITION_SIZE);
	memset(buf, 0, RW_PARTITION_SIZE);
	write_flash(buf);
	free(buf);
	return 0;
}

int reboot_system()
{
	fprintf(stderr, "Rebooting...\n");
	sync();
	reboot(RB_AUTOBOOT);
	return 0;
}

int main(int argc, char** argv)
{
	char *filename = "-";
	char *optarg = NULL;
	char *optarg1 = NULL;
	char *optarg2 = NULL;
	char *optarg3 = NULL;
	int result = 0;
	dbg("starting(): argc=%d\n", argc);
	init();
	dbg("argc:%d, argv[0]=%s, argv[1]=%s, argv[2]=%s\n", argc, argv[0], argv[1], argv[2]);
	if ( argc >= 2 ) {
		if ( argc == 3 )
			filename = optarg = argv[2];
		if ( argc == 5 ) {
			optarg1 = argv[2];
			optarg2 = argv[3];
			optarg3 = argv[4];
		}

		dbg("filename=%s\n", filename);
		if ( ! strcmp(argv[1], "read_datafs") ) {
			char * buf = read_datafs_from_flash();
			write_file(filename, buf, RW_PARTITION_SIZE);
			free(buf);
		} else if ( ! strcmp(argv[1], "write_datafs") ) {
            int size = 0;
			char * buf = read_file(filename, &size);
			write_flash(buf);
			free(buf);
		} else if ( ! strcmp(argv[1], "save") ) {
			result = save();
		} else if ( ! strcmp(argv[1], "load") ) {
			result = load(optarg);
		} else if ( ! strcmp(argv[1], "erase") ) {
			result = erase();
		} else 
			usage();
	} else 
		usage();

	return result;
}

