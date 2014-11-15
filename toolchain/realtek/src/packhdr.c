/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * $Header: /home/bcm_cvsroot/WRT54GX_ALL_Realtek/tools/packbin.src/packhdr.c,v 1.1.1.1 2005/09/30 07:32:47 shixiang Exp $
 *
 * $Author: shixiang $
 *
 * Abstract:
 *
 *   Append image header, calculate checksum and padding.
 *
 * $Log: packhdr.c,v $
 * Revision 1.1.1.1  2005/09/30 07:32:47  shixiang
 * Ken, for All in one
 *
 * Revision 1.1.1.1  2005/07/15 04:13:26  kevin
 * Kevin, for merge 2x2&2x3
 *
 * Revision 1.1.1.1  2005/07/01 11:59:44  kevin
 * For merge code
 *
 * Revision 1.3  2005/01/10 05:26:56  rupert
 * * :  rename the tmpname path  to the current directory
 *
 * Revision 1.2  2005/01/06 12:22:47  yjlou
 * *: fixed the bug of temp file naming
 *    tmpFilename[50] = "/tmp/packbin.XXXXXX";
 *
 * Revision 1.1  2004/12/01 07:34:21  yjlou
 * *** empty log message ***
 *
 * Revision 1.1  2002/07/19 05:50:00  danwu
 * Create file.
 *
 *
 * 
 */

#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <sys/time.h>
#include    <time.h>
/* integration with Loader. */
#include    "rtl_types.h"
#include    "rtl_image.h"

const char logo_msg[] = {
	"(c)Copyright Realtek, Inc. 2002\n" 
	"Project ROME\n\n"
};
#define ENDIAN_SWITCH32(x) (((x) >> 24) | (((x) >> 8) & 0xFF00) | \
                            (((x) << 8) & 0xFF0000) | (((x) << 24) &0xFF000000))
#define ENDIAN_SWITCH16(x) ((((x) >> 8) & 0xFF) | (((x) << 8) & 0xFF00))

int
main(int argc, char *argv[])
{
    FILE *              fp1;
    FILE *              fp2;
    char                tmpFilename[50] = "packbin.XXXXXX";
    unsigned long       imgSize,dateofday,timeofday;
    struct stat         fileStat;
    unsigned long       i;
    unsigned char       ch;
    //unsigned long       boardMagic;
    long boardMagic;
    
	struct timeval timeval;
	struct tm *ut;
	time_t tt;

	tt = time(NULL);
	gettimeofday(&timeval,NULL);
	//gmtime_r(&tt,&ut);
	ut=localtime(&tt);
	printf("date %04d/%02d/%02d %02d:%02d:%02d \n",
			1900+ut->tm_year,
			ut->tm_mon+1,
			ut->tm_mday,
			ut->tm_hour,
			ut->tm_min,
			ut->tm_sec
			);
    	dateofday=((ut->tm_year+1900)<<16)+((ut->tm_mon+1)<<8)+(ut->tm_mday);
    	timeofday=((ut->tm_hour)<<24)+((ut->tm_min)<<16)+((ut->tm_sec)<<8);
	printf("%08lx %08lx\n",dateofday,timeofday);
    /* Check arguments */
    if (argc < 4)
    {
        printf("Usage: PACKHDR board_magic input_file output_file\n");
	printf("    board_magic   Hardware board's magic id.\n");
        printf("    input_file    Name of the input binary image file.\n");
        printf("    output_file   Name of the output binary image file.\n");
        
        return 0;
    }
    
    /* Get board_magic */
    boardMagic = ENDIAN_SWITCH32(strtol(argv[1], NULL, 0));
    if(boardMagic == 0x7fffffff || boardMagic == 0xffffff7f)
    	boardMagic = 0xffffffff;
    printf("boardMagic=%x\n", boardMagic);
    /* Open file */
     if ((fp1=fopen(argv[2], "r+b")) == NULL)
    {
        printf("Cannot open %s !!\n", argv[1]);
        return -1;
    }
    
    /* Get file size */
    if (stat(argv[2], &fileStat) != 0)
    {
        printf("Cannot get file statistics !!\n");
        fclose(fp1);
        exit(-1);
    }
    imgSize = fileStat.st_size;
    
    printf("Image Original Size = 0x%lx\n", imgSize);
    
    /* Temparay file */
    //tmpnam(tmpFilename);
    mkstemp(tmpFilename);
    if ((fp2=fopen(tmpFilename, "w+b")) == NULL)
    {
        printf("Cannot open temprary file !!\n");
        return -2;
    }
    
    /* Copy image */
    fseek(fp1, 0L, SEEK_SET);
    fseek(fp2, 0L, SEEK_SET);
    fwrite((void*)&boardMagic, sizeof(boardMagic), 1, fp2);
    for (i=0; i<imgSize; i++) {
        ch = fgetc(fp1);
	fputc(ch, fp2);
    }
    
    /* Close file and exit */
    fclose(fp1);
    fclose(fp2);
    
    printf("Binary image %s generated!\n", argv[3]);
    remove(argv[3]);
    rename(tmpFilename, argv[3]);

	return 0;
}

