EXEC = flash
OBJS = flash.o flashdrv.o

EXTRA_CFLAGS += -I./

all: $(EXEC) gpio bixcrypt

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS-$(@)))

gpio:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

bixcrypt:
	$(HOSTCC) $@.c -o $@

.c.o:
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -c $<

clean:
	rm -f $(EXEC) *.o
