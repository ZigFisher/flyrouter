
#ifndef _BOARD_FLASH_H_
#define _BOARD_FLASH_H_

int   board_flash_get_datafs_size();
char* board_flash_read_datafs(char* buf, int buf_size);
int   board_flash_write_datafs(char* buf, int buf_size);
int   board_flash_erase_datafs();
int   board_flash_reboot();
#endif
