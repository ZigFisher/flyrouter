include $(TOPDIR)/rules.mk

KDIR:=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)

REV:=$(shell LANG=C cat ../../../../Revision | head -n 1 | awk '/^Revision:/ { print $$2 }' )
ifeq ($(REV),)
REV:=0
endif

export PADFILE=$(STAGING_DIR)/bin/padfile

$(PADFILE):
	$(HOSTCC) src/padfile.c -o $(PADFILE)

ifeq ($(KERNEL),2.4)
FSNAME:=$(patsubst jffs2-%,jffs2,$(FS))

ifeq ($(FS),squashfs)
rtl865xb-fs-prepare:


INITFS_OFFSET=$(shell echo $$(( $(BEEROUTER_KERNEL_START)+$(BEEROUTER_KERNEL_SIZE) )) )
INITFS_SIZE=$(shell   echo $$(( $(BEEROUTER_FLASH_SIZE)-$(BEEROUTER_KERNEL_START)-$(BEEROUTER_KERNEL_SIZE)-$(BEEROUTER_DATAFS_SIZE))) )
DATAFS_OFFSET=$(shell echo $$(( $(INITFS_OFFSET)+$(INITFS_SIZE) )) )

ifeq ($(BEEROUTER_TARGET_EDIMAX),y)
BEEROUTER_TARGET=edimax
BEEROUTER_STATUS_LED=A0
BEEROUTER_USB_LED=A1
BEEROUTER_RESET_BUTTON=A7
endif
ifeq ($(BEEROUTER_TARGET_DLINK),y)
BEEROUTER_TARGET=dlink
BEEROUTER_STATUS_LED=A6
BEEROUTER_USB_LED=A5
BEEROUTER_RESET_BUTTON=A7
endif



$(BIN_DIR)/openwrt-rtl865xb-$(KERNEL)-$(FSNAME).bin: $(PADFILE)
	rm -f $(KDIR)/root.squashfs
	mkdir -p $(KDIR)/root/ram
	test -f delete-files-$(KERNEL)-$(FS) && \
			for f in `cat delete-files-$(KERNEL)-$(FS)`; do \
				rm -rf $(KDIR)/root/$$f;	\
			done; 
	( cd ../../package/base-files/files/$(BOARD)-$(KERNEL) && \
		tar cf - --exclude .svn --exclude var . | tar xvf - -C $(KDIR)/root/; \
		 $(SED) 's,$$R,r$(REV),g' $(KDIR)/root/etc/banner; \
		 [ -n "$(BEE_CONFIG)" ] && echo "Firmware: $(BEE_CONFIG)" >>$(KDIR)/root/etc/banner; \
		 echo "Builded in ZFT Lab. at: `date '+%Y%m%d %H:%M'`" >>$(KDIR)/root/etc/banner; \
		 echo "" >>$(KDIR)/root/etc/banner; \
	)
	# beerouter.conf
	( \
		echo "BEEROUTER_REVISION=$(REV)"; \
		echo "BEEROUTER_CONFIG=$(BEE_CONFIG)"; \
		echo "BEEROUTER_BUILD_DATE=`date '+%Y%m%d'`"; \
		echo "BEEROUTER_BUILD_TIME=`date '+%H:%M'`"; \
		echo "BEEROUTER_TYPE=tmpfs"; \
		echo "BEEROUTER_TARGET=$(BEEROUTER_TARGET)"; \
		echo "STATUS_LED=$(BEEROUTER_STATUS_LED)"; \
		echo "USB_LED=$(BEEROUTER_USB_LED)"; \
		echo "RESET_BUTTON=$(BEEROUTER_RESET_BUTTON)"; \
		echo "KERNEL_PARTITION=/dev/mtd/1"; \
		echo "KERNEL_PARTITION_OFFSET=$$(($(BEEROUTER_KERNEL_START)))"; \
		echo "KERNEL_PARTITION_SIZE=$$(($(BEEROUTER_KERNEL_SIZE)))"; \
		echo "INITFS_PARTITION=/dev/mtd/2"; \
		echo "INITFS_PARTITION_OFFSET=$$(($(BEEROUTER_KERNEL_START)+$(BEEROUTER_KERNEL_SIZE)))"; \
		echo "INITFS_PARTITION_SIZE=$(INITFS_SIZE)"; \
		echo "DATAFS_PARTITION=/dev/mtd/3"; \
		echo "DATAFS_PARTITION_OFFSET=$(DATAFS_OFFSET)"; \
		echo "DATAFS_PARTITION_SIZE=$$(($(BEEROUTER_DATAFS_SIZE)))"; \
	) >$(KDIR)/root/etc/beerouter.conf
	# Base section:
	#
	# Create squashfs-lzma
	$(STAGING_DIR)/bin/mksquashfs-lzma $(KDIR)/root $(KDIR)/root.sqfs -be -noappend -nopad -all-root $(MAKE_TRACE)
	#
	# Check size of rootfs (size should be less than FLASH_SIZE - bootloader - kernel_size - 64k (datafs)
	( \
		if [ `stat -c "%s" $(KDIR)/root.sqfs` -gt $$(($(BEEROUTER_FLASH_SIZE)-$(BEEROUTER_KERNEL_START)-$(BEEROUTER_KERNEL_SIZE)-0x10000)) ]; then \
			echo "ROOTFS is oversized!!!"; \
			exit 1; \
		else \
			echo "Size is ok"; \
		fi; \
	)
ifeq ($(BEEROUTER_TARGET_EDIMAX),y)
	# Edimax BR6214K section:
	# Todo:
	#    * check kernel or rootfs size if bigger than flash size
	#
	# Compress kernel with lzma
	$(STAGING_DIR)/bin/lzma e $(KDIR)/vmlinux $(KDIR)/kernel.edimax.lzma -a1 -d18
	#
	# Write Edimax magic number for kernel
	$(STAGING_DIR)/bin/packbin $(KDIR)/kernel.edimax.lzma r 0 $(KDIR)/kernel.edimax.bix 0x59a0e845
	#
	# Link kernel with root_fs (1mB, 700kB kernel size)
	#$(STAGING_DIR)/bin/kfspad $(KDIR)/kernel.edimax.bix 0x100000 $(KDIR)/root.sqfs $(KDIR)/edimax.bin
	$(STAGING_DIR)/bin/kfspad $(KDIR)/kernel.edimax.bix $(BEEROUTER_KERNEL_SIZE) $(KDIR)/root.sqfs $(KDIR)/edimax.bin
	#
	# Write Edimax magic number for firmware
	$(STAGING_DIR)/bin/packbin $(KDIR)/edimax.bin k 0 $(KDIR)/br6214k.bix 0x59a0e845
	$(STAGING_DIR)/bin/bixcrypt $(KDIR)/br6214k.bix $(KDIR)/br6214k_crypted.bix
endif
ifeq ($(BEEROUTER_TARGET_DLINK),y)
	#
	# D-Link DIR-120 or DI-604UP section
	# 
	$(PADFILE) $(BUILD_DIR)/linux/7linux $$(($(BEEROUTER_KERNEL_SIZE)-64))
	$(STAGING_DIR)/bin/alpha_pack -k $(BUILD_DIR)/linux/7linux -f $(KDIR)/root.sqfs -s A7KZ -p 4 -t $(KDIR)/mtdimg.7z
	$(STAGING_DIR)/bin/bcat --ch "grep kernelunzip $(BUILD_DIR)/linux/7System.map | cut -c 1-8" -t $(KDIR)/mtdimg.7z -p 5
	$(STAGING_DIR)/bin/packbin $(KDIR)/mtdimg.7z r 0 $(KDIR)/dir120.bix 0x59a0e842
	#
	#cp $(KDIR)/7krun.bix $(KDIR)/DI524upImg.bix
	#cp $(KDIR)/7krun.bix $(KDIR)/DI604upImg.bix
	#$(STAGING_DIR)/bin/bcat -x 0x60420010 -t $(KDIR)/DI524upImg.bix	
	#$(STAGING_DIR)/bin/bcat -x 0x60420013 -t $(KDIR)/DI604upImg.bix
	$(STAGING_DIR)/bin/bixcrypt $(KDIR)/dir120.bix $(KDIR)/dir120_crypted.bix
endif


install: $(BIN_DIR)/openwrt-rtl865xb-$(KERNEL)-$(FSNAME).bin
endif

#ifneq ($(FS),jffs2-4MB)
#$(BIN_DIR)/openwrt-br6104k-$(KERNEL)-$(FSNAME).bin: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin
#
#install: $(BIN_DIR)/openwrt-br6104k-$(KERNEL)-$(FSNAME).bin
#endif

endif


clean:
prepare: rtl865xb-fs-prepare
compile:
install: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin

