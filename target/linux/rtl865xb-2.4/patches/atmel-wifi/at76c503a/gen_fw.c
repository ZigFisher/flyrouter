/* $Id: gen_fw.c,v 1.6 2004/06/15 19:53:42 jal2 Exp $ */

/* This file includes the old style firmwares and outputs new, binary files. */

#include <stdio.h>
#include <assert.h>

typedef unsigned char u8;
typedef unsigned int u32;

#define BOARDTYPE_503_INTERSIL_3861 1
#define BOARDTYPE_503_INTERSIL_3863 2
#define BOARDTYPE_503_RFMD          3
#define BOARDTYPE_503_RFMD_ACC      4
#define BOARDTYPE_505_RFMD          5
#define BOARDTYPE_505_RFMD_2958     6
#define BOARDTYPE_505A_RFMD_2958    7

#include "fw-rfmd-0.90.2-140.h"
u8 intfw_503rfmd_0_90_2[] = FW_503RFMD_INTERNAL;
u8 extfw_503rfmd_0_90_2[] = FW_503RFMD_EXTERNAL;
#undef FW_503RFMD_INTERNAL
#undef FW_503RFMD_EXTERNAL

#include "fw-rfmd-1.101.0-84.h"
#include "fw-rfmd-acc-1.101.0-84.h"
#include "fw-i3861.h"
#include "fw-i3863.h"
#include "fw-r505.h"
#include "fw-505rfmd2958-1.101.0-86.h"

#define atuwi_fw_rfmd2958_smc_int intfw_505a_rfmd2958
#define atuwi_fw_rfmd2958_smc_ext extfw_505a_rfmd2958
#include "atuwi_rfmd2958-smc_fw.h"

/* The struct of the firmware header: */
typedef struct {
        u32 crc;             // CRC32 of the whole image (seed ~0, no post-process)
        u32 board_id;        // BOARDTYPE_xxx
        u32 version;         // firmware version code
        u32 str_offset;      // printable string offset (copyright)
        u32 internal_offset; // internal firmware image offset
        u32 internal_len;    // internal firmawre image len
        u32 external_offset; // external firmware image offset
        u32 external_len;    // external firmware image len
} at76c50x_fw_t __attribute__((packed));

#define cpu_to_le32(x) (x)
// round to next multiple of four
#define QUAD(x) ((x) % 4 ? (x) + (4 - ((x)%4)) : (x))

u8 intfw_503rfmd[] = FW_503RFMD_INTERNAL;
u8 extfw_503rfmd[] = FW_503RFMD_EXTERNAL;
u8 intfw_503rfmd_acc[] = FW_503RFMD_ACC_INTERNAL;
u8 extfw_503rfmd_acc[] = FW_503RFMD_ACC_EXTERNAL;
u8 intfw_i3861[] = FW_I3861_INTERNAL;
u8 extfw_i3861[] = FW_I3861_EXTERNAL;
u8 intfw_i3863[] = FW_I3863_INTERNAL;
u8 extfw_i3863[] = FW_I3863_EXTERNAL;
u8 intfw_505rfmd[] = FW_505RFMD_INTERNAL;
u8 extfw_505rfmd[] = FW_505RFMD_EXTERNAL;
u8 intfw_505rfmd2958[] = FW_505RFMD2958_INTERNAL;
u8 extfw_505rfmd2958[] = FW_505RFMD2958_EXTERNAL;

struct fw {
	const char *filename;
	u32 board_id;
	u32 version;
	const char *str_id;
	u8 *intfw;
	u32 intfw_sz;
	u8 *extfw;
	u32 extfw_sz;
} fws[] = {
	{ "atmel_at76c503-rfmd-0.90.2-140.bin", BOARDTYPE_503_RFMD,
	  0x005a028c, "0.90.2-140 503 RFMD "
	  "Copyright (c) 1999-2000 by Atmel Corporation",
	  intfw_503rfmd_0_90_2, sizeof(intfw_503rfmd_0_90_2),
          extfw_503rfmd_0_90_2, sizeof(extfw_503rfmd_0_90_2)},

	{ "atmel_at76c503-rfmd.bin", BOARDTYPE_503_RFMD,
	  0x01650054, "1.101.0-84 503 RFMD "
	  "Copyright (c) 1999-2000 by Atmel Corporation",
	  intfw_503rfmd, sizeof(intfw_503rfmd),
	  extfw_503rfmd, sizeof(extfw_503rfmd)},

	{ "atmel_at76c503-rfmd-acc.bin", BOARDTYPE_503_RFMD_ACC,
	  0x01650054, "1.101.0-84 503 RFMD Accton design "
	  "Copyright (c) 1999-2000 by Atmel Corporation",
	  intfw_503rfmd_acc, sizeof(intfw_503rfmd_acc),
	  extfw_503rfmd_acc, sizeof(extfw_503rfmd_acc)},

	{ "atmel_at76c503-i3861.bin", BOARDTYPE_503_INTERSIL_3861,
	  0x005a002c, "0.90.0-44 Intersil 3861 "
	  "Copyright (c) 1999-2000 by Atmel Corporation",
	  intfw_i3861, sizeof(intfw_i3861),
	  extfw_i3861, sizeof(extfw_i3861)},

	{ "atmel_at76c503-i3863.bin", BOARDTYPE_503_INTERSIL_3863,
	  0x005a002c, "0.90.0-44 Intersil 3863 "
	  "Copyright (c) 1999-2000 by Atmel Corporation",
	  intfw_i3863, sizeof(intfw_i3863),
	  extfw_i3863, sizeof(extfw_i3863)},

	{ "atmel_at76c505-rfmd.bin", BOARDTYPE_505_RFMD,
	  0x005b0004, "0.91.0-4 505 RFMD "
	  "Copyright (c) 1999-2000 by Atmel Corporation",
	  intfw_505rfmd, sizeof(intfw_505rfmd),
	  extfw_505rfmd, sizeof(extfw_505rfmd)},

	{ "atmel_at76c505-rfmd2958.bin", BOARDTYPE_505_RFMD_2958,
	  0x01650056, "1.101.0-86 505 RFMD2958 "
	  "Copyright (c) 1999-2000 by Atmel Corporation",
	  intfw_505rfmd2958, sizeof(intfw_505rfmd2958),
	  extfw_505rfmd2958, sizeof(extfw_505rfmd2958)},

	{ "atmel_at76c505a-rfmd2958.bin", BOARDTYPE_505A_RFMD_2958,
	  0x01660071, "1.102.0-113 505A RFMD 2958"
	  "Copyright (c) 1999-2004 by Atmel Corporation",
	  intfw_505a_rfmd2958, sizeof(intfw_505a_rfmd2958),
	  extfw_505a_rfmd2958, sizeof(extfw_505a_rfmd2958)},
};

int nr_fws = sizeof(fws) / sizeof(struct fw);
const u8 zeros[] = {0,0,0};

#define _CRCPOLY_LE 0xedb88320
static u32 crc32 (u32 crc, u8 const *p, size_t len)
{
        int i;
        while (len--) {
                crc ^= *p++;
                for (i = 0; i < 8; i++)
                        crc = (crc >> 1) ^ ((crc & 1) ? _CRCPOLY_LE : 0);
        }
        return crc;
}

int main(void)
{
	int i;
	u32 val, offs;
	FILE *f;
	struct fw *fw;
	at76c50x_fw_t hd;

	for(i=0,fw=fws; i < nr_fws; fw++,i++) {
		if ((f=fopen(fw->filename, "w")) == NULL) {
			fprintf(stderr,"#ERR cannot open %s for writing (errno %m)\n",
				fw->filename);
			continue;
		}

		hd.crc = ~0; /* the initial seed */
		hd.board_id = cpu_to_le32(fw->board_id);
		hd.version = cpu_to_le32(fw->version);
		// string area starts after header
		hd.str_offset = cpu_to_le32(sizeof(at76c50x_fw_t));
		hd.internal_offset = hd.str_offset + strlen(fw->str_id) + 1;
		hd.internal_offset = QUAD(hd.internal_offset);
		hd.internal_len = fw->intfw_sz;
		hd.external_offset = hd.internal_offset + hd.internal_len;
		hd.external_offset = QUAD(hd.external_offset);
		hd.external_len = fw->extfw_sz;

		/* calc crc */
		/* the header */
		hd.crc = crc32(hd.crc, (u8 *)&hd.board_id, 0x20-0x4);
		/* the string */
		hd.crc = crc32(hd.crc, fw->str_id, strlen(fw->str_id) +1);
		/* zeros in gap */
		hd.crc = crc32(hd.crc, zeros, hd.internal_offset - 
			       (hd.str_offset + strlen(fw->str_id)  + 1));
		/* internal fw */
		hd.crc = crc32(hd.crc, fw->intfw, fw->intfw_sz);
		/* zeros in gap */
		hd.crc = crc32(hd.crc, zeros, hd.external_offset - 
			       (hd.internal_offset + fw->intfw_sz));
		/* external fw */
		hd.crc = crc32(hd.crc, fw->extfw, fw->extfw_sz);

#define FWRITE(ptr,len,fp) \
  if (len > 0) {\
    if (fwrite(ptr,len,1,fp) < 1) {\
      fprintf(stderr,"#ERR failed to write %d bytes, errno %m\n", len);\
      fclose(fp);\
      continue;\
    }\
  }

		FWRITE((u8 *)&hd, sizeof(hd),f);
		FWRITE(fw->str_id, strlen(fw->str_id)+1, f);
		FWRITE(zeros, hd.internal_offset - 
		       (hd.str_offset + strlen(fw->str_id)  + 1), f);
		FWRITE(fw->intfw, fw->intfw_sz, f);
		FWRITE(zeros, hd.external_offset - 
		       (hd.internal_offset + fw->intfw_sz), f);
		FWRITE(fw->extfw, fw->extfw_sz, f);

		fclose(f);
	}

	return 0;
}

