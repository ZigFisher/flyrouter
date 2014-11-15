flags=-g

all: icmptx

icmptx: it.o icmptx.o tun_dev.o
	/home/builder/trunk/staging_dir_mipsel/bin/mipsel-linux-uclibc-gcc $(flags) -o icmptx icmptx.o it.o tun_dev.o
	/home/builder/trunk/staging_dir_mipsel/bin/sstrip icmptx

it.o: it.c
	/home/builder/trunk/staging_dir_mipsel/bin/mipsel-linux-uclibc-gcc $(flags) -c it.c

icmptx.o: icmptx.c
	/home/builder/trunk/staging_dir_mipsel/bin/mipsel-linux-uclibc-gcc $(flags) -c icmptx.c

tun_dev.o: tun_dev.c
	/home/builder/trunk/staging_dir_mipsel/bin/mipsel-linux-uclibc-gcc $(flags) -c tun_dev.c

clean:
	rm -f tun_dev.o it.o icmptx.o icmptx
