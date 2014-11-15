
ifeq ($(LIBLOG_ENABLED),1)
CFLAGS += -DLIBLOG_ENABLED
endif

all: log.o

log.o:
	$(CC) $(CFLAGS) -c log.c -o $@ 

clean:
	-rm *.o

