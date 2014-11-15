#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h> 
#include <sys/types.h>
#include <string.h>
#include "board_flash.h"
#include "utils.h"

#define MTD_DEVICE "/dev/mtdblock/4"

int   board_flash_get_datafs_size()
{
    // TODO
    return 256*1024;
}

char* board_flash_read_datafs(char* buf, int buf_size)
{
    int size = board_flash_get_datafs_size();
	FILE* file = NULL;
    
    if( buf_size > size )
        dbg("warn: buf_size > size");

    file = fopen(MTD_DEVICE, "r");
	if ( ! file ) {
		perror(NULL);
		return NULL;
	}
	fread(buf, size, 1, file);
	fclose(file);

    return buf;
}
int   board_flash_write_datafs(char* buf, int buf_size)
{
    int fd = open(MTD_DEVICE,  O_WRONLY | O_CREAT);

    if (fd == -1) {
        dbg("error: file does not exist\n");
        return 0;
    }

    size_t written = write(fd, buf, buf_size);
    dbg("written %d bytes\n", written);

    close(fd);

    //return write_file(MTD_DEVICE, buf, board_flash_get_datafs_size());
}

int   board_flash_erase_datafs()
{
    return board_flash_erase_datafs_common();
}

int   board_flash_reboot()
{
    return board_flash_reboot_common();
}
