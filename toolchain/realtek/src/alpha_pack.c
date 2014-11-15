#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

/*
0000000 3741 5a4b 0c00 d457 0000 0100 0000 9001
0000010 0000 0000 0000 0000 0000 0000 0000 0000
0000020 0000 0000 bd27 d8ff bfaf 2000 b3af 1c00
*/

#define ALPHA_PACK_MAGIC 0x41374b5a
#define MAX_FILES 5

struct alpha_pack_header {
	unsigned int magic;
	unsigned int version;
	unsigned int kernel;
	unsigned int num_files;
	unsigned int files[MAX_FILES];
	unsigned int pad;
	//unsigned int pad;
}__attribute__((packed));

#define BUFFER_SIZE (64 * 1024)


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

void bwap_header(struct alpha_pack_header *h)
{
	int i;
	
	h->magic = bwap32(h->magic);
	h->kernel = bwap32(h->kernel);
	h->num_files = bwap32(h->num_files);
	
	for (i = 0; i < MAX_FILES; i++)
		h->files[i] = bwap32(h->files[i]);
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

#define VERSION_MINOR_SHIFT	16
#define VERSION_MINOR_MASK	0x00ff0000
#define VERSION_MAJOR_SHIFT	8
#define VERSION_MAJOR_MASK	0x0000ff00
#define VERSION_PATCH_SHIFT	0
#define VERSION_PATCH_MASK	0x000000ff

#define VERSION_MINOR(v)	(((v) & VERSION_MINOR_MASK) >> VERSION_MINOR_SHIFT)
#define VERSION_MAJOR(v)	(((v) & VERSION_MAJOR_MASK) >> VERSION_MAJOR_SHIFT)
#define VERSION_PATCH(v)	(((v) & VERSION_PATCH_MASK) >> VERSION_PATCH_SHIFT)

void pack_alpha_print_help() {
	char *usage = "Usage: PACKBIN input_file image_type pad_to output_file\n"
		      "    input_file    Name of the input binary image file.\n"
		      "    image_type    [r|b|d] for [runtime|boot|root directory].\n"
		      "    pad_to        Size of the output binary image. (0 means no padding)\n"
		      "    output_file   Name of the output binary image file.\n";
	fprintf(stderr, "%s", usage);
}

void print_header(struct alpha_pack_header *h)
{
	int i;
	
	printf("      Magic Number = 0x%08x\n", h->magic);
	printf("version major %d, minor %d, patch %d\n",
		VERSION_MAJOR(h->version),
		VERSION_MINOR(h->version),
		VERSION_PATCH(h->version));

	printf("kernel (%lu)%08x\n", h->kernel, h->kernel);
	printf("num files %lu\n", h->num_files);

	for (i = 0; i < MAX_FILES; i++) {
		printf("%d: (%d)(%08x)\n", i, h->files[i], h->files[i]);
	}
	
	/*if (h->pad != 0x0) {
		fprintf(stderr, "Invalid pad!\n");
		exit(1);
	}*/
}

#define FILE_KERNEL	0
#define FILE_FS0	1
#define FILE_OUT	7

int pack_alpha(int argc, char *argv[])
{
	FILE *fin, *fout;
	struct alpha_pack_header h = { 0 };
	long pad = 0;
	int ret;
	char *buffer;
	int chunk;
	int c;
	char *files[MAX_FILES + 1] = { 0 };
	int fi = 0;
	int i;
	char *sign;
	
	while ((c = getopt(argc, argv, ":k:f:s:p:t:")) != -1) {
		switch (c) {
		case 'k':
			files[FILE_KERNEL] = optarg;
		break;
		
		case 'f':
			files[FILE_FS0 + fi++] = optarg;
		break;
		
		case 's':
			if (strcmp(optarg, "A7KZ") == 0 ||
			    strcmp(optarg, "A7LZ") == 0 ||
			    strcmp(optarg, "AGKZ") == 0 ||
			    strcmp(optarg, "AGLZ") == 0) {
				sign = optarg;
			} else {
				pack_alpha_print_help();
				return -1;
			}
		break;
		
		case 'p':
			pad = strtol(optarg, NULL, 0);
		break;
		
		case 't':
			files[FILE_OUT] = optarg;
		break;

		case '?':
		case ':':
			pack_alpha_print_help();
			return -1;
		break;
		}
	}
	
	for (i = 0; i < MAX_FILES + 1; i++)
		if (files[i]) {	
			printf("%d = %s\n", i, files[i]);
		}

	h.magic = ALPHA_PACK_MAGIC;
	
	return 0;
}

void unpackbin_print_help() {

}



int write_to_fd(FILE *fout, FILE *fin, int size)
{
	char *buffer;
	int chunk;
	size_t ret;
	
	buffer = malloc(BUFFER_SIZE);

	while (size > 0) {
		chunk = size < BUFFER_SIZE ? size : BUFFER_SIZE;
		ret = fread(buffer, 1, chunk,  fin);
		if (ret != chunk) { /* FIXME */
			fprintf(stderr, "%d vs. %d\n", ret, chunk);
			perror("fread");
			return -1;
		}
		ret = fwrite(buffer, 1, chunk, fout);
		if (ret != chunk) {
			perror("fwrite");
			return -1;
		}
		
		size -= chunk;
	}

	return 0;
}

int write_file(char *filename, FILE *fin, int size)
{
	FILE *fout;
	int ret;
	
	fout = fopen(filename, "w+");
	if (fout == NULL) {
		fprintf(stderr, "Failed to open file %s for writing!\n", filename);
		return -1;
	}
	
	ret = write_to_fd(fout, fin, size);
	
	fclose(fout);
	return ret;
}

int unpackbin(int argc, char *argv[])
{
	FILE *fin;
	struct alpha_pack_header h = { 0 };
	int ret;
	int i;
	char filename[200];
	int bytes_read = 0;
	
	fprintf(stderr, "sizeof header %d\n", sizeof(h));
	if (argc != 2) {
		unpackbin_print_help();
		return -1;
	}

	fin = fopen(argv[1], "r");
	if (fin == NULL) {
		fprintf(stderr, "Failed to open input file\n");
		return -1;
	}
	fread(&h, 1, sizeof(h) - sizeof(h.pad), fin);
	bytes_read += sizeof(h) - sizeof(h.pad);

	bwap_header(&h);

	if (h.num_files <= MAX_FILES) {
		fread(&h.pad, 1, sizeof(h.pad), fin);
		bytes_read += sizeof(h.pad);
		
		//memcpy(&h, &h.num_files, sizeof(h) - sizeof(h_new.version));
	} else {
		fprintf(stderr, "TODO\n");
		exit(1);
	}
	
	printf("%08x\n", h.magic);
	//printf("pad1 %08x\n", h.pad1);
	
	if (h.magic != ALPHA_PACK_MAGIC) {
		fprintf(stderr, "Invalid magic!\n");
		return -1;
	}
	
	print_header(&h);
	if (h.pad != 0x0) {
		fprintf(stderr, "Pad not 0x0!\n");
		return -1;
	}
	
	printf("Wrote root of size %d\n", h.kernel & 0xffffff);
	ret = write_file("root", fin, h.kernel & 0xffffff);
	if (ret)
		return ret;
	bytes_read += h.kernel;

	
	for (i = 0; i < h.num_files; i++) {
		int size = h.files[i] & 0xffffff;
		if (h.files[i] == 0)
			continue;
		
		sprintf(filename, "fs%d", i);
		ret = write_file(filename, fin, size);
		if (ret)
			return ret;
		bytes_read += size;
		
		printf("Wrote %s of size %d\n", filename, size);
	}
	
	if (get_file_size(fin) != bytes_read) {
		fprintf(stderr, "Extra data at the end of input!(%d vs. %d)\n",
			get_file_size(fin), bytes_read);
		return -1;
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
	if (match_suffix(argv[0], "unpack_alpha") == 0)
		return unpackbin(argc, argv);

	return pack_alpha(argc, argv);
}
