#include <sys/reboot.h>
#include "board_flash.h"

int board_flash_reboot_common()
{
	reboot(RB_AUTOBOOT);
	return 0;
}

int   board_flash_erase_datafs_common()
{
	char *buf = malloc(board_flash_get_datafs_size());
	memset(buf, 0, board_flash_get_datafs_size());
	board_flash_write_datafs(buf, board_flash_get_datafs_size());
	free(buf);
	return 0;
}

