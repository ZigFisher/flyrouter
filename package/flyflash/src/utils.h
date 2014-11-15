//#define DEBUG

#ifdef DEBUG
#define dbg if(1)_dbg
#else 
#define dbg if(0)_dbg
#endif

void _dbg(const char* format, ...);
int write_file(char *filename, char* buf, int size);
int file_size(const char* filename);
unsigned char* read_file(const char *filename, int* size);

unsigned short crc16_add(unsigned char b, unsigned short crc);
unsigned short crc16(unsigned char *p, int size);


