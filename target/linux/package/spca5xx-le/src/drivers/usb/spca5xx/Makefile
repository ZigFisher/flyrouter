# standalone Makefile for spca5xx-le

spca5xx-objs := spcadecoder.o spca_core.o

obj-m := spca5xx.o

include $(TOPDIR)/Rules.make

spca5xx.o: $(spca5xx-objs)
	$(LD) -r -o $@ $(spca5xx-objs)
