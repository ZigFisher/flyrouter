PROG        = usb_modeswitch
VERS        = 0.9.6
RM          = /bin/rm -f
OBJS        = usb_modeswitch.c

all:        $(PROG)
$(PROG):    $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -lusb -o $(PROG) $(OBJS)

