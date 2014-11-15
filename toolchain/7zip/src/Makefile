
#CFLAGS=-g -Wall

all: 7zip

7zip: LZMAEncoder.o 7zip.o
	$(CXX) $(CFLAGS) -o 7zip LZMAEncoder.o 7zip.o
	
LZMAEncoder.o: LZMA/LZMAEncoder.cpp 
	$(CC) $(CFLAGS) -c LZMA/LZMAEncoder.cpp

7zip.o: 7zip.c
	$(CC) $(CFLAGS) -c 7zip.c
clean:
	rm -f *.o
	rm -f 7zip

make fresh:
	make clean
	make all
