
#CFLAGS=-g -Wall

#all: alpha_pack packbin packbin2 packhdr kfspad
all: packbin packbin2 packhdr kfspad

alpha_pack:
	$(CC) $(CFLAGS) -o alpha_pack alpha_pack.c

packbin:
	$(CC) $(CFLAGS) -o packbin packbin.c

packbin2:
	$(CC) $(CFLAGS) -o packbin2 packbin2.c

packhdr:
	$(CC) $(CFLAGS) -o packhdr packhdr.c

kfspad:
	$(CC) $(CFLAGS) -o kfspad kfspad.c

clean:
	rm -f *.o
	rm -f packbin packbin2 packhdr kfspad
