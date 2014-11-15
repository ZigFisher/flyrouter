#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BIX_MAGIC 0x10004260

#define PACKBIN_MAGIC 0x59a0e842
#define PACKBIN_MAGIC1 0x47363134

#define PACKBIN_RUNTIME	0x8dc9
#define PACKBIN_BOOT	0xea43
#define PACKBIN_ROOT	0xb162

#define PACKBIN_TYPE_REM 0x100

#define YEAR_SHIFT	16
#define YEAR_MASK	0xffff0000
#define MONTH_SHIFT	8
#define MONTH_MASK	0x0000ff00
#define DAY_SHIFT	0
#define DAY_MASK	0x000000ff

#define DATE_YEAR(h)	(((h)->date & YEAR_MASK) >> YEAR_SHIFT)
#define DATE_MONTH(h)	(((h)->date & MONTH_MASK) >> MONTH_SHIFT)
#define DATE_DAY(h)	(((h)->date & DAY_MASK) >> DAY_SHIFT)

#define DATE_GEN_YEAR(x)	(((x) << YEAR_SHIFT) & YEAR_MASK)
#define DATE_GEN_MONTH(x)	(((x) << MONTH_SHIFT) & MONTH_MASK)
#define DATE_GEN_DAY(x)		(((x) << DAY_SHIFT) & DAY_MASK)

#define HOUR_SHIFT	24
#define HOUR_MASK	0xff000000
#define MINUTE_SHIFT	16
#define MINUTE_MASK	0x00ff0000
#define SECOND_SHIFT	8
#define SECOND_MASK	0x0000ff00
#define MSECOND_SHIFT	0
#define MSECOND_MASK	0x000000ff

#define TIME_HOUR(h)	(((h)->time & HOUR_MASK) >> HOUR_SHIFT)
#define TIME_MINUTE(h)	(((h)->time & MINUTE_MASK) >> MINUTE_SHIFT)
#define TIME_SECOND(h)	(((h)->time & SECOND_MASK) >> SECOND_SHIFT)
#define TIME_MSECOND(h)	(((h)->time & MSECOND_MASK) >> MSECOND_SHIFT)

#define TIME_GEN_HOUR(x)	(((x) << HOUR_SHIFT) & HOUR_MASK)
#define TIME_GEN_MINUTE(x)	(((x) << MINUTE_SHIFT) & MINUTE_MASK)
#define TIME_GEN_SECOND(x)	(((x) << SECOND_SHIFT) & SECOND_MASK)
#define TIME_GEN_MSECOND(x)	(((x) << MSECOND_SHIFT) & MSECOND_MASK)

#define BUFFER_SIZE (64 * 1024)

struct packbin_header {
	unsigned int magic;
	unsigned int type;
	unsigned int date;
	unsigned int time;
	
	unsigned int data_size;
	
	unsigned short int unk1;
	unsigned char checksum_body;
	unsigned char checksum_header;
}__attribute__((packed));

unsigned int bwap32(unsigned int x)
{
	unsigned int test = 0xff;
	
	if (((unsigned char *)&test)[0] == 0xff)
		return 	(x & 0xff000000) >> 24 |
			(x & 0x00ff0000) >> 8 |
			(x & 0x0000ff00) << 8 |
			(x & 0x000000ff) << 24;
	else
		return x;
}

void bwap_header(struct packbin_header *h)
{
	h->magic = bwap32(h->magic);
	h->type = bwap32(h->type);
	h->date = bwap32(h->date);
	h->time = bwap32(h->time);
	h->data_size = bwap32(h->data_size);
}

/* Return 0 if b is a suffix of a */
int match_suffix(char *a, char *b)
{
	int offset;
	offset = strlen(a) - strlen(b);

	if (offset >= 0 && strcmp(a + offset, b) == 0)
		return 0;

	return -1;
}

long get_file_size(FILE *fin)
{
	long size, pos;

	pos = ftell(fin);
	if (fseek(fin, 0, SEEK_END) != 0) {
		perror("seek");
		return -1;
	}
	
	size = ftell(fin);
	
	if (fseek(fin, pos, SEEK_SET) != 0) {
		perror("seek");
		return -1;
	}
	return size;
}

typedef unsigned char (* check_func)(void *data, int len, unsigned char c);

unsigned char gen_checksum(void *d, int len, unsigned char c)
{
	unsigned char *data = d;
	int i;
	
	for (i = 0; i < len; i++)
		c ^= data[i];

	return c;
}

void packbin_print_help() {
	char *usage = "Usage: PACKBIN input_file image_type pad_to output_file\n"
		      "    input_file    Name of the input binary image file.\n"
		      "    image_type    [r|b|d] for [runtime|boot|root directory].\n"
		      "    pad_to        Size of the output binary image. (0 means no padding)\n"
		      "    output_file   Name of the output binary image file.\n";
	fprintf(stderr, "%s", usage);
}

void print_header(struct packbin_header *h)
{
	printf("date %d/%02d/%02d %02d:%02d:%02d\n",
		DATE_YEAR(h), DATE_MONTH(h), DATE_DAY(h),
		TIME_HOUR(h), TIME_MINUTE(h), TIME_SECOND(h));
	
	printf("%08x %08x\n", h->date, h->time);
	//printf("Image Original Size = 0x%x\n", h->size);
	printf("      Magic Number = 0x%08x\n", h->magic);
	printf("      Body Checksum = 0x%x\n", h->checksum_body);
	printf("      Header Checksum = 0x%x\n", h->checksum_header);
}

int packbin(int argc, char *argv[])
{
	FILE *fin, *fout;
	struct packbin_header h = { 0 };
	long pad;
	int ret;
	char *buffer;
	int chunk;
	struct tm *tm;
	time_t curr_time;
	
	if (argc != 5 ||
	    strlen(argv[2]) != 1) {
		packbin_print_help();
		return -1;
	}
	
	fin = fopen(argv[1], "r");
	if (fin == NULL) {
		fprintf(stderr, "Failed to open input file\n");
		return -1;
	}

	h.data_size = get_file_size(fin);

	pad = strtol(argv[3], NULL, 0);
	if (pad == 0)
		pad = h.data_size + sizeof(h);
	
	if (h.data_size + sizeof(h) > pad) {
		fprintf(stderr, "Padding size error !!\n");
		return -1;
	}
	
	fout = fopen(argv[4], "w+");
	if (fout == NULL) {
		fprintf(stderr, "Failed to open file for writing\n");
		return -1;
	}
	
	if (match_suffix(argv[4], ".bix") == 0) {
		printf("Writing .bix file!\n");
		h.magic = BIX_MAGIC;
		fwrite(&h.magic, 1, sizeof(h.magic), fout);
	}
	/* Write now and again later */
	fwrite(&h, 1, sizeof(h), fout);

	h.magic = PACKBIN_MAGIC;

	h.type = PACKBIN_TYPE_REM;
	switch(*argv[2]) {
	case 'r':
		h.type |= PACKBIN_RUNTIME << 16;
	break;

	case 'b':
		h.type |= PACKBIN_BOOT << 16;
	break;
	
	case 'd':
		h.type |= PACKBIN_ROOT << 16;
	break;

	default:
		fprintf(stderr, "Invalid type!\n");
		return -1;
	}

	if (time(&curr_time) == (time_t)-1) {
		fprintf(stderr, "time failed!\n");
		return -1;
	}
	tm = localtime(&curr_time);
	if (tm == NULL) {
		perror("localtime");
		return -1;
	}
	h.date = DATE_GEN_YEAR(tm->tm_year + 1900) |
		 DATE_GEN_MONTH(tm->tm_mon + 1) |
		 DATE_GEN_DAY(tm->tm_mday);

	h.time = TIME_GEN_HOUR(tm->tm_hour) |
		 TIME_GEN_MINUTE(tm->tm_min) |
		 TIME_GEN_SECOND(tm->tm_sec) |
		 TIME_GEN_MSECOND(/*42*/0);
	
	ret = write_to_fd(fout, fin, h.data_size, gen_checksum);
	if (ret < 0)
		return -1;
	
	h.checksum_body = ret;

	pad -= h.data_size + sizeof(h);
	h.data_size += pad;

	buffer = malloc(BUFFER_SIZE);
	memset(buffer, 0, BUFFER_SIZE);
	
	while (pad > 0) {
		chunk = pad < BUFFER_SIZE ? pad : BUFFER_SIZE;
		ret = fwrite(buffer, 1, chunk, fout);
		if (ret != chunk) {
			perror("fwrite");
			return -1;
		}
		pad -= chunk;
	}

	h.checksum_header = gen_checksum(&h, sizeof(h) - 1, 0);

	print_header(&h);
	
	bwap_header(&h);
	
	if (fseek(fout, 0, SEEK_SET) != 0) {
		perror("seek");
		return -1;
	}
	fwrite(&h, 1, sizeof(h), fout);

	printf("Binary image %s generated!\n", argv[4]);

	return 0;
}

void unpackbin_print_help() {

}



int write_to_fd(FILE *fout, FILE *fin, int size, check_func check)
{
	char *buffer;
	int chunk;
	size_t ret;
	unsigned char c = 0;
	
	buffer = malloc(BUFFER_SIZE);

	while (size > 0) {
		chunk = size < BUFFER_SIZE ? size : BUFFER_SIZE;
		ret = fread(buffer, 1, chunk,  fin);
		if (ret != chunk) { /* FIXME */
			perror("fread");
			return -1;
		}
		c = check(buffer, chunk, c);
		ret = fwrite(buffer, 1, chunk, fout);
		if (ret != chunk) {
			perror("fwrite");
			return -1;
		}
		
		size -= chunk;
	}

	return c;
}

int write_file(char *filename, FILE *fin, int size, check_func check)
{
	FILE *fout;
	int ret;
	
	fout = fopen(filename, "w+");
	if (fout == NULL) {
		fprintf(stderr, "Failed to open file %s for writing!\n", filename);
		return -1;
	}
	
	ret = write_to_fd(fout, fin, size, check);
	
	fclose(fout);
	return ret;
}

int unpackbin(int argc, char *argv[])
{
	FILE *fin;
	struct packbin_header h = { 0 };
	int c;
	unsigned char *buffer;
	int size;
	int ret;
	int is_bix = 0;
	
	if (argc != 2 && argc != 3) { /* TODO: 3rd arg */
		unpackbin_print_help();
		return -1;
	}
	
	buffer = malloc(1024*1024*8);
	
	fin = fopen(argv[1], "r");
	if (fin == NULL) {
		fprintf(stderr, "Failed to open input file\n");
		return -1;
	}
	fread(&h.magic, 1, sizeof(h.magic), fin);
	if (h.magic == BIX_MAGIC) {
		printf("Reading BIX file!\n");
		is_bix = sizeof(h.magic);
		fread(&h, 1, sizeof(h), fin);
	} else {
		fread(&h.type, 1, sizeof(h) - sizeof(h.magic), fin);
	}

	bwap_header(&h);
	switch (h.magic) {
	case PACKBIN_MAGIC:
		printf("d-link magic found!\n");
	break;

	case PACKBIN_MAGIC1:
		printf("netgear magic found!\n");
	break;

	default:
		fprintf(stderr, "Invalid magic!\n");
		return -1;
	break;
	}
	
	print_header(&h);

	c = gen_checksum(&h, sizeof(h) - 1, 0);
	
	if (c != h.checksum_header) {
		fprintf(stderr, "Header checksum failed! (%x vs. %x)\n", c, h.checksum_header);
		return -1;
	}

	if (h.unk1 != 0x0) {
		fprintf(stderr, "unk1 set to %08x\n", h.unk1);
		return -1;
	}
	
	if (h.type & 0xffff != PACKBIN_TYPE_REM) {
		fprintf(stderr, "type_rem differs %08x\n", h.type & 0xffff);
		return -1;
	}
	switch ((h.type >> 16) & 0xffff) {
	case PACKBIN_RUNTIME:
		printf("runtime\n");
		ret = write_file("runtime", fin, h.data_size, gen_checksum);
	break;
	
	case PACKBIN_BOOT:
		printf("boot\n");
		ret = write_file("boot", fin, h.data_size, gen_checksum);
	break;
	
	case PACKBIN_ROOT:
		printf("root\n");
		ret = write_file("root", fin, h.data_size, gen_checksum);
	break;
	
	default:
		printf("unknown type(%x)\n", (h.type >> 16) & 0xffff);
		return -1;
	break;
	}
	
	if (ret != h.checksum_body) {
		fprintf(stderr, "Body checksum failed! (%x vs. %x)\n", c, h.checksum_body);
		return -1;
	}
	
	if (get_file_size(fin) != h.data_size + sizeof(h) + is_bix) {
		fprintf(stderr, "Extra data at the end of input!(%d vs. %d)\n",
			get_file_size(fin), h.data_size + sizeof(h));
		return -1;
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
	if (match_suffix(argv[0], "unpackbin") == 0)
		return unpackbin(argc, argv);
	
	return packbin(argc, argv);
}
