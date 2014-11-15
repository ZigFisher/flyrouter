#include "rtl_types.h"
#include "rtl_flashdrv.h"
#include "flashdrv.h"

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
#include "cryptfunc.c"

#define dbg if(0)_dbg
#define CRYPT_OFFSET (64*1024)

void _dbg(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

/*
0x00000000-0x00020000 : "ldr"
0x00020000-0x00030000 : "alphafs"
0x00030000-0x00117cd0 : "kernel"
0x00117cd0-0x0035fcd0 : "squashfs"
0x00360000-0x00400000 : "rw"
*/
#ifndef BEEROUTER_FLASH_SIZE
#define BEEROUTER_FLASH_SIZE (0x200000)
#endif

#define RW_PARTITION_SIZE (64*1024)
#define RW_PARTITION_OFFSET (BEEROUTER_FLASH_SIZE-RW_PARTITION_SIZE)
#define ROOTFS_PARTITION_OFFSET 0x000cf000
#define ROOTFS_PARTITION_MAX_SIZE (1156*1024)
#define FLASH_SAVE_FILENAME "/etc/flash.save"

uint32 __flash_base;

int  rtl865x_mmap(int base,int length) // for flash , register mapping
{
        int sg_fd;
        int new_base;
        if ((sg_fd = open("/dev/mem", O_RDWR)) < 0)
               printf("open fail\n");
        new_base = (int) mmap((void*)0x0, length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, sg_fd, base);//>>12);
        close(sg_fd);
        if (new_base==(int)MAP_FAILED)
        {
                fprintf(stderr,"MAP FAIL\n");
                return 0;
        }
        return new_base;
}

char* read_datafs_from_flash()
{
	char* big_buf = malloc(RW_PARTITION_SIZE);
	memset(big_buf, 0, RW_PARTITION_SIZE);
	flashdrv_read((void *)big_buf, (void *)__flash_base + RW_PARTITION_OFFSET, RW_PARTITION_SIZE);
	return big_buf;
}

char* read_from_flash(const char* stroffset, int len)
{
	char* big_buf = malloc(len);
	unsigned int offset = 0;
	dbg("stroffset: %s\n", stroffset);
	sscanf(stroffset, "%x", &offset);
	fprintf(stderr, "offset: 0x%x\n", offset);
	memset(big_buf, 0, len);
	flashdrv_read((void *)big_buf, (void *)__flash_base + offset, len);
	return big_buf;
}

int write_flash(char* buf)
{
	dbg("RW_PARTITION_OFFSET: %x\n", RW_PARTITION_OFFSET);
	dbg("RW_PARTITION_SIZE: %x\n", RW_PARTITION_SIZE);
	dbg("flashing...\n");
	if ( flashdrv_updateImg((void*) buf, (void *)__flash_base + RW_PARTITION_OFFSET, RW_PARTITION_SIZE) ) {
		fprintf(stderr, "flashing fail\n");
		return 0;
	}
	return 1;
}

int write_file(char *filename, char* buf)
{
	FILE *file;
	
	if ( ! strcmp(filename, "-") ) 
		file = stdout;
	else
		file = fopen(filename, "w");

	if ( ! file ) {
		perror(NULL);
		return 0;
	}
	fwrite(buf, RW_PARTITION_SIZE, 1, file);
	fclose(file);
	return 1;
}

int write_to_file(char *filename, char* buf, int size)
{
	FILE *file;
	
	if ( ! strcmp(filename, "-") ) 
		file = stdout;
	else
		file = fopen(filename, "w");

	if ( ! file ) {
		perror(NULL);
		return 0;
	}
	fwrite(buf, size, 1, file);
	fclose(file);
	return 1;
}

int get_rootfs_partition_offset()
{
	FILE* conf;
	char buf[256];
	if (! (conf = fopen("/etc/beerouter.conf", "r"))){
		perror("fopen");
		exit(1);
	}
	while (fgets(buf, sizeof(buf), conf)) {
		int len = strlen("INITFS_PARTITION_OFFSET=");
		if (!strncmp("INITFS_PARTITION_OFFSET=", buf, len)){
			int offset = atoi(buf+len);
			//fprintf(stderr, "INITFS_PARTITION_OFFSET=%d\n", offset);
			if (offset > 1024*300) 
				return offset;
		};
	}
	fprintf(stderr, "Cannot find INITFS_PARTITION_OFFSET\n");
	exit(1);
	return -1;
}

int full_upgrade(char* filename)
{
	FILE *file;
	int fsize = 0;
	char* big_buf = NULL;
	int start_offset;
	unsigned int readed_magic = 0;
	unsigned int magic = BEEROUTER_TARGET_MAGIC;
	if (magic == 0x59a0e842) { // dlink
		dbg("compiled for DLINK\n");
		start_offset=0x30000;
	} else if (magic == 0x59a0e845) { // edimax
		dbg("compiled for EDIMAX\n");
		start_offset=0x20000;
	} else {
		dbg("unknown target");
		exit(1);
	}

	int firmware_max_size;
	firmware_max_size = (BEEROUTER_FLASH_SIZE - start_offset - BEEROUTER_DATAFS_SIZE);
	dbg("start_offset: 0x%x\n", start_offset);
	dbg("magic: 0x%x\n", magic);
	dbg("firmware_max_size: %d\n", firmware_max_size);

	if ( ! (file = fopen(filename, "r")) ) {
		perror(filename);
		goto fail;
	}
	fseek (file , 0 , SEEK_END);
	fsize = ftell(file);
	dbg("file size: %d (0x%x)\n", fsize, fsize);
	if ( fsize > firmware_max_size ) {
		fprintf(stderr, "file too big: %d, max size %d\n", fsize, firmware_max_size);
		goto fail;
	}
	
	big_buf = malloc(fsize);
	if (! big_buf) {
		perror("malloc");
		goto fail;
	}

	memset(big_buf, 0, fsize);

	fseek (file , 0 , SEEK_SET);
	fread(big_buf, fsize, 1, file);
	dbg("readed\n");
	fclose(file);

	memcpy(&readed_magic, big_buf, sizeof(readed_magic));
	if ( readed_magic != magic ) {
		fprintf(stderr, "invalid firmware\n");
		goto fail;
	}

	simplecrypt(big_buf+CRYPT_OFFSET, fsize-CRYPT_OFFSET);
	if ( flashdrv_updateImg((void*) big_buf, (void *)__flash_base + start_offset, fsize) ) {
		fprintf(stderr, "flashing fail\n");
		goto fail;
	}

	dbg("flashed ok\n");
	return 0;

fail:
	return 127;
}

int upgrade_rootfs(char* filename)
{
	FILE *file;
	int fsize = 0;
	char* big_buf = NULL;
	int rootfs_partition_offset=0;

	if ( ! (file = fopen(filename, "r")) ) {
		perror(filename);
		goto fail;
	}
	fseek (file , 0 , SEEK_END);
	fsize = ftell(file);
	if ( fsize-8 > ROOTFS_PARTITION_MAX_SIZE ) {
		fprintf(stderr, "file too big: %d, max size %d\n", fsize, ROOTFS_PARTITION_MAX_SIZE);
		goto fail;
	}
	
	big_buf = malloc(fsize);
	if (! big_buf) {
		perror("malloc");
		goto fail;
	}

	rootfs_partition_offset = get_rootfs_partition_offset();

	memset(big_buf, 0, fsize);

	fseek (file , 0 , SEEK_SET);
	fread(big_buf, fsize, 1, file);
	fclose(file);

	char header[8];
	memcpy(header, big_buf, 7);
	header[7] = '\0';

// 0 - crypted only, 1 - both crypted/uncrypted
#if 1
	if ( ! memcmp(big_buf, "sqsh", 4) ) {
		if ( flashdrv_updateImg((void*) big_buf, (void *)__flash_base + rootfs_partition_offset, fsize) ) {
			fprintf(stderr, "flashdrv_updateImg() fail\n");
			goto fail;
		}
	} else 
#endif
	if ( big_buf[0] == 'b' && big_buf[1] == 'e' && big_buf[3] == 'c' ) {
		simplecrypt(big_buf+8, fsize-8);
		if ( flashdrv_updateImg((void*) big_buf+8, (void *)__flash_base + rootfs_partition_offset, fsize-8) ) {
			fprintf(stderr, "flashdrv_updateImg() fail\n");
			goto fail;
		}
	} else {
		fprintf(stderr, "Invalid image: '%s'\n", header);
		goto fail;
	}


	return 0;

fail:
	return 1;
}

char* read_file(char *filename)
{
	char* big_buf = malloc(RW_PARTITION_SIZE);
	FILE* file = NULL;

	memset(big_buf, 0, RW_PARTITION_SIZE);

	if ( ! strcmp(filename, "-") )
		file = stdin;
	else
		file = fopen(filename, "r");

	if ( ! file ) {
		perror(NULL);
		return NULL;
	}

	dbg("reading from %s\n", filename);
	dbg("RW_PARTITION_SIZE: %d\n", RW_PARTITION_SIZE);
	fread(big_buf, RW_PARTITION_SIZE, 1, file);
	fclose(file);

	return big_buf;
}

int init()
{
	//dbg("init()\n");

	__flash_base=rtl865x_mmap(0x1e000000,0x1000000); /* 0xbe000000~0xbeffffff */
	flashdrv_init();
#if 0	
	dbg("flash_base=0x%x\n", __flash_base);
	dbg("FLASH_MAP_BOARD_INFO_ADDR=0x%x\n", FLASH_MAP_BOARD_INFO_ADDR);
	dbg("BoardInfoAddr: %x\n", FLASH_MAP_BOARD_INFO_ADDR);
	dbg("CcfgImageAddr: %x\n", FLASH_MAP_CCFG_IMAGE_ADDR);
	dbg("RunImageAddr:  %x\n", FLASH_MAP_RUN_IMAGE_ADDR);
#endif

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
	FILE *file = NULL;
	int fsize = 0;
	int result = 1; // default value: shell false
	int exitcode = 0;
	char *tmpdir=NULL;
	if ( ! ( tmpdir = getenv("TMPDIR") ) )
		tmpdir = "/tmp";

	if ( ! read_config() )
		return 1;

	fprintf(stderr, "Creating archive...\n");
	snprintf(filename, sizeof(filename), "%s/flash_save_%d.tar.gz", tmpdir, getpid());
	snprintf(buf1, sizeof(buf1), "tar czf %s %s >/dev/null 2>&1", filename, flash_save_files);
	if ( exitcode = system(buf1) ) {
		fprintf(stderr, "Error: tar exited with %d code\n", exitcode);
		goto end;
	};

	if ( ! (file = fopen(filename, "r")) ) {
		perror(filename);
		goto end;
	}
	fseek (file , 0 , SEEK_END);
	fsize = ftell(file);
	dbg("file size: %d\n", fsize);
	
	if ( fsize > RW_PARTITION_SIZE ) {
		fprintf(stderr, "Error: %s oversized: %d\n", filename, fsize);
		goto end;
	}
	
	pbuf = read_file(filename);
	fprintf(stderr, "Flashing...\n");
	write_flash(pbuf);

	result = 0;
end:
	if ( file ) 
		fclose(file);
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
	if ( ! write_file(filename, pbuf)) {
		exitcode = 127;
		goto end;
	}
	free(pbuf);
	snprintf(buf1, sizeof(buf1), "tar xzf %s -C %s 2>/dev/null", filename, dest?dest:"/ram");
	dbg("system: '%s'\n", buf1);
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
			write_file(filename, buf);
			free(buf);
		} else if ( ! strcmp(argv[1], "write_datafs") ) {
			char * buf = read_file(filename);
			write_flash(buf);
			free(buf);
		} else if ( (! strcmp(argv[1], "read")) && optarg2 && optarg3 ) {
			char * buf = read_from_flash(optarg1, atol(optarg2));
			write_to_file(optarg3, buf, atol(optarg2));
			free(buf);
		} else if ( ! strcmp(argv[1], "save") ) {
			result = save();
		} else if ( ! strcmp(argv[1], "load") ) {
			result = load(optarg);
		} else if ( ! strcmp(argv[1], "erase") ) {
			result = erase();
		} else if ( ! strcmp(argv[1], "upgrade") ) {
			// result = upgrade_rootfs(optarg);
			result = full_upgrade(optarg);
			if ( 0 == result)
				reboot_system();
		} else 
			usage();
	} else 
		usage();

	return result;
}

