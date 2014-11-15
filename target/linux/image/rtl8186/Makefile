include $(TOPDIR)/rules.mk

KDIR:=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)

REV:=$(shell LANG=C svn info | awk '/^Revision:/ { print$$2 }' )
ifeq ($(REV),)
REV:=0
endif


ifeq ($(KERNEL),2.4)
FSNAME:=$(patsubst jffs2-%,jffs2,$(FS))

ifeq ($(FS),squashfs)
rtl8186-fs-prepare:

$(BIN_DIR)/openwrt-rtl8186-$(KERNEL)-$(FSNAME).bin:
	##rm -f $(KDIR)/root.squashfs
	mkdir -p $(KDIR)/root/ram
	test -f delete-files-$(KERNEL)-$(FS) && \
			for f in `cat delete-files-$(KERNEL)-$(FS)`; do \
				rm -rf $(KDIR)/root/$$f;	\
			done; 
	( cd ../../package/base-files/files/$(BOARD)-$(KERNEL) && \
		tar cf - --exclude .svn --exclude var . | tar xvf - -C $(KDIR)/root/; \
		 $(SED) 's,$$R,r$(REV),g' $(KDIR)/root/etc/banner; \
		 echo "Builded in ZFT Lab. at: `date '+%Y%m%d %H:%M'`" >>$(KDIR)/root/etc/banner; \
		 echo "" >>$(KDIR)/root/etc/banner; \
	)
	#( \
	#	MIDGE_TYPE=tmpfs \
	# 	MIDGE_KERNEL_BLOCKS=$(MIDGE_KERNEL_BLOCKS) \
	#	MIDGE_SQUASHFS_BLOCKS=$(MIDGE_SQUASHFS_BLOCKS) \
	#	MIDGE_DATAFS_BLOCKS=$$((($(MIDGE_FLASH_SIZE)-$(MIDGE_KERNEL_START))/0x10000-$(MIDGE_KERNEL_BLOCKS)-$(MIDGE_SQUASHFS_BLOCKS))) \
	#	/bin/sh ./mkconf $(KDIR)/root/etc/midge.conf \
	#)
	#$(STAGING_DIR)/bin/mksquashfs-lzma $(KDIR)/root $(KDIR)/root.squashfs -nopad -noappend -root-owned -be $(MAKE_TRACE)
	#$(STAGING_DIR)/bin/mksquashfs $(KDIR)/root $(KDIR)/root.squashfs -nopad -noappend -root-owned -be $(MAKE_TRACE)
	#( 	MAKECSYS=yes \
	#	MIDGE_KERNEL_BLOCKS=$(MIDGE_KERNEL_BLOCKS) \
	#	MIDGE_SQUASHFS_BLOCKS=$(MIDGE_SQUASHFS_BLOCKS) \
	#	MIDGE_DATAFS_BLOCKS=$$((($(MIDGE_FLASH_SIZE)-$(MIDGE_KERNEL_START))/0x10000-$(MIDGE_KERNEL_BLOCKS)-$(MIDGE_SQUASHFS_BLOCKS))) \
	#	MIDGE_TYPE=tmpfs \
	#	/bin/sh ./mkimage -k $(LINUX_DIR)/vmlinuz -s $(KDIR)/root.squashfs -o $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin.csys )
	#( 	MIDGE_KERNEL_BLOCKS=$(MIDGE_KERNEL_BLOCKS) \
	#	MIDGE_SQUASHFS_BLOCKS=$(MIDGE_SQUASHFS_BLOCKS) \
	#	MIDGE_DATAFS_BLOCKS=$$((($(MIDGE_FLASH_SIZE)-$(MIDGE_KERNEL_START))/0x10000-$(MIDGE_KERNEL_BLOCKS)-$(MIDGE_SQUASHFS_BLOCKS))) \
	#	MIDGE_TYPE=tmpfs \
	#	/bin/sh ./mkimage -k $(LINUX_DIR)/vmlinuz -s $(KDIR)/root.squashfs -o $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin )
	#
	#
	make -C $(LINUX_DIR)/rtkload bz2
	#
	
install: $(BIN_DIR)/openwrt-rtl8186-$(KERNEL)-$(FSNAME).bin
endif

#ifneq ($(FS),jffs2-4MB)
#$(BIN_DIR)/openwrt-br6104k-$(KERNEL)-$(FSNAME).bin: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin
#
#install: $(BIN_DIR)/openwrt-br6104k-$(KERNEL)-$(FSNAME).bin
#endif

endif


clean:
prepare: rtl8186-fs-prepare
compile:
install: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin
