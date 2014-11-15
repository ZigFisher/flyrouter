EXEC = flash
OBJS = flash.o utils.o bbutils.o board_flash.o board_flash_common.o
EXTRA_CFLAGS += -I./

all: $(EXEC)

BOARD = sky

# sinus (not completed)
ifeq ($(BOARD),bcm)
	OBJS += bcm/board_api.o 
endif

# skyrouter (bcm5354)
ifeq ($(BOARD),sky)
endif


$(EXEC): $(OBJS) board_flash.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS-$(@)))

board_flash.o:
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -c board_flash_$(BOARD).c -o board_flash.o

.c.o:
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -c $<

clean:
	rm -f $(EXEC) *.o
