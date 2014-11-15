
#include "board_flash.h"

int   board_flash_get_datafs_size()
{
    return (1024*16);
}


char* board_flash_read_datafs(char* buf, int buf_size)
{
    sysPersistentGet(big_buf, RW_PARTITION_SIZE, 0);
}

int   board_flash_write_datafs(char* buf, int buf_size)
{
	dbg("sysGetPsiSize(): %d\n", sysGetPsiSize());
	dbg("flashing...\n");
    sysPersistentSet(buf, board_flash_get_datafs_size(), 0);
    return 1;
}

int   board_flash_erase_datafs()
{
    return board_flash_erase_datafs_common();
}

int   board_flash_reboot()
{
    return board_flash_reboot_common();
}
