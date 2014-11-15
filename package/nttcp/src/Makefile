VERSION=	1.47
prefix=		/usr/local/opt/nttcp-$(VERSION)
CFLAGS =	$(ARCH) $(DBG) $(DEF) $(INC) $(OPT) -DVERSION=\"$(VERSION)\"
LFLAGS = 	$(DBG) 

nttcp:		nttcp.o support.o
		$(CC) $(LFLAGS) nttcp.o -o nttcp support.o $(LIB)

support.o:	support.c support.h

dist:
		if [ -d nttcp-$(VERSION) ]; then \
		    rm -f nttcp-$(VERSION)/*; \
		else \
		    mkdir nttcp-$(VERSION); \
		fi
		ln nttcp.c nttcp.1 support.c support.h \
		   README Makefile nttcp-$(VERSION)
		tar cvf - nttcp-$(VERSION) | gzip > /tmp/nttcp-$(VERSION).tar.gz; \
		zip -lgr /tmp/nttcp`echo $(VERSION)|tr -d .`.zip nttcp-$(VERSION)

install:	nttcp
		-mkdir -p $(prefix)/bin
		-mkdir -p $(prefix)/man/man1
		cp nttcp $(prefix)/bin
		cp nttcp.1 $(prefix)/man/man1

clean:
		rm -f *.o

clobber:	clean
		rm -rf core nttcp nttcp-$(VERSION)
