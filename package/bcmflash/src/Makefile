EXEC = flash
OBJS = flash.o board_api.o utils.o bbutils.o

EXTRA_CFLAGS += -I./

all: $(EXEC) ledctrl

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS-$(@)))

ledctrl:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c

.c.o:
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -c $<

clean:
	rm -f $(EXEC) *.o
