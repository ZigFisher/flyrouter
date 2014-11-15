include $(TOPDIR)/rules.mk

KDIR:=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)

REV:=$(shell LANG=C svn info | awk '/^Revision:/ { print$$2 }' )
ifeq ($(REV),)
REV:=0
endif



export PADFILE=$(STAGING_DIR)/bin/padfile

$(SCRIPT_DIR)/padfile:
	$(HOSTCC) src/padfile.c -o $(PADFILE)


ifeq ($(KERNEL),2.6)

ifeq ($(FS),jffs2-32MB)

LINUX_VERSION:=2.6.16
LINUX_BUILD_DIR:=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)
adm5120-fs-prepare:
	test -f delete-files-$(KERNEL)-$(FS) && \
			for f in `cat delete-files-$(KERNEL)-$(FS)`; do \
				rm -rf $(KDIR)/root/$$f;	\
			done; 
	( cd ../../package/base-files/files/$(BOARD)-$(KERNEL) && \
		tar cf - --exclude .svn . | tar xvf - -C $(KDIR)/root/; \
		 $(SED) 's,$$R,r$(REV),g' $(KDIR)/root/etc/banner; \
		 echo "Builded at: `date '+%Y%m%d %H:%M'`" >>$(KDIR)/root/etc/banner; \
		 echo "" >>$(KDIR)/root/etc/banner; \
	)

$(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin: $(SCRIPT_DIR)/padfile
	( 	SG_KERNEL_BLOCKS=$(SG_KERNEL_BLOCKS) \
		SG_FS_BLOCKS=$(SG_FS_BLOCKS) \
		PADFILE=$(PADFILE) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		IMG_NAME=$(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin \
		/bin/sh ./mkimage-jffs2 $(KDIR)/root.jffs2-32MB $(BUILD_DIR)/linux/vmlinux )

endif



endif

clean:
prepare: adm5120-fs-prepare
compile:
install: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin
