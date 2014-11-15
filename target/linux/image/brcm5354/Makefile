include $(TOPDIR)/rules.mk
include $(TOPDIR)/.config


KDIR:=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)

REV:=$(shell LANG=C cat ../../../../Revision | head -n 1 | awk '/^Revision:/ { print $$2 }' )
ifeq ($(REV),)
REV:=0
endif

lzma-loader-clean:
	$(MAKE) -C lzma-loader clean

lzma-loader-prepare:
	$(MAKE) -C lzma-loader prepare

lzma-loader-compile: lzma-loader-prepare
	$(MAKE) -C lzma-loader compile

ifeq ($(IB),)
$(KDIR)/vmlinux.lzma: $(KDIR)/vmlinux
	cat $^ | $(STAGING_DIR)/bin/lzma e -si -so -eos -lc1 -lp2 -pb2 > $@ || (rm -f $@ && false)

$(KDIR)/loader.tmp.gz: compile
$(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).trx: $(KDIR)/vmlinux.lzma
endif

ifeq ($(FS),jffs2-8MB)
TRXALIGN:=-a 0x20000
endif
ifeq ($(FS),jffs2-4MB)
TRXALIGN:=-a 0x10000
endif
# see ticket #155
ifeq ($(FS),squashfs)
TRXALIGN:=-a 1024
endif


$(KDIR)/loader.tmp.gz:
	$(CP) $(BUILD_DIR)/loader.gz $@
	echo -ne "\\x00" >> $@

$(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).trx: $(KDIR)/loader.tmp.gz
	rm -f $(KDIR)/root.squashfs
	mkdir -p $(KDIR)/root/ram
	test -f delete-files-$(KERNEL)-$(FS) && \
			for f in `cat delete-files-$(KERNEL)-$(FS)`; do \
				rm -rf $(KDIR)/root/$$f;	\
			done; 
	( cd ../../package/base-files/files/$(BOARD)-$(KERNEL) && \
		tar cf - --exclude .svn --exclude var . | tar xvf - -C $(KDIR)/root/; \
		 $(SED) 's,$$R,r-$(REV),g' $(KDIR)/root/etc/banner; \
		 [ -n "$(SKY_CONFIG)" ] && echo "Firmware: $(SKY_CONFIG)" >>$(KDIR)/root/etc/banner; \
		 echo "Builded in ZFT Lab. at: `date '+%Y%m%d %H:%M'`" >>$(KDIR)/root/etc/banner; \
		 echo "" >>$(KDIR)/root/etc/banner; \
	)
	# skyrouter.conf
	( \
		echo "SKYROUTER_REVISION=$(REV)"; \
		echo "SKYROUTER_CONFIG=$(SKY_CONFIG)"; \
		echo "SKYROUTER_BUILD_DATE=`date '+%Y%m%d'`"; \
		echo "SKYROUTER_BUILD_TIME=`date '+%H:%M'`"; \
		echo "SKYROUTER_TYPE=tmpfs"; \
		echo "KERNEL_PARTITION=/dev/mtd/1"; \
		echo "KERNEL_PARTITION_SIZE=1"; \
		echo "INITFS_PARTITION=/dev/mtd/2"; \
		echo "INITFS_PARTITION_SIZE=1"; \
		echo "DATAFS_PARTITION=/dev/mtd/4"; \
		echo "DATAFS_PARTITION_SIZE=1"; \
	) >$(KDIR)/root/etc/skyrouter.conf
	# Base section:
	#
	# Create squashfs-lzma
	$(STAGING_DIR)/bin/mksquashfs-lzma $(KDIR)/root $(KDIR)/root.squashfs -le -noappend -nopad -all-root $(MAKE_TRACE)
	#
	# Check size of rootfs (size should be less than FLASH_SIZE - bootloader - kernel_size - 64k (datafs)
	#( \
	#	if [ `stat -c "%s" $(KDIR)/root.squashfs` -gt $$(($(SKYROUTER_FLASH_SIZE)-$(SKYROUTER_KERNEL_START)-$(SKYROUTER_KERNEL_SIZE)-0x10000)) ]; then \
	#		echo "ROOTFS is oversized!!!"; \
	#		exit 1; \
	#	else \
	#		echo "Size is ok"; \
	#	fi; \
	#)
#
# D-Link DIR-320  <->  RD-010
ifeq ($(SKYROUTER_TARGET_DIR320),y)
	$(STAGING_DIR)/bin/asustrx -p WL500gpv2 -v 1.1.$(REV) -o $@ $(KDIR)/loader.tmp.gz $(KDIR)/vmlinux.lzma $(KDIR)/root.$(FS)
endif
#
# Asus WL-520GU  <->  RD-011
ifeq ($(SKYROUTER_TARGET_WL520GU),y)
	$(STAGING_DIR)/bin/asustrx -p WL520gu -v 1.1.$(REV) -o $@ $(KDIR)/loader.tmp.gz $(KDIR)/vmlinux.lzma $(KDIR)/root.$(FS)
endif
#
# Asus WL-520GC  <->  RD-012
ifeq ($(SKYROUTER_TARGET_WL520GC),y)
	$(STAGING_DIR)/bin/asustrx -p WL520gc -v 1.1.$(REV) -o $@ $(KDIR)/loader.tmp.gz $(KDIR)/vmlinux.lzma $(KDIR)/root.$(FS)
endif


ifeq ($(IB),)
clean: lzma-loader-clean
prepare: lzma-loader-prepare
compile: lzma-loader-compile
else
clean:
prepare:
compile:
endif
install: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).trx
install-ib:
	mkdir -p $(IB_DIR)/staging_dir_$(ARCH)/bin
	$(CP) $(STAGING_DIR)/bin/addpattern $(STAGING_DIR)/bin/trx $(STAGING_DIR)/bin/motorola-bin $(IB_DIR)/staging_dir_$(ARCH)/bin
	mkdir -p $(IB_DIR)/build_$(ARCH)/linux-$(KERNEL)-$(BOARD)
	$(CP) $(BUILD_DIR)/loader.gz $(IB_DIR)/build_$(ARCH)/
	$(CP) $(BUILD_DIR)/loader.elf $(IB_DIR)/build_$(ARCH)/
	$(CP) $(KDIR)/vmlinux.lzma $(IB_DIR)/build_$(ARCH)/linux-$(KERNEL)-$(BOARD)/

