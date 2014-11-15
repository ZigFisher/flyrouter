include $(TOPDIR)/rules.mk

KDIR:=$(BUILD_DIR)/linux-$(KERNEL)-$(BOARD)

REV:=$(shell LANG=C cat ../../../../Revision | head -n 1 | awk '/^Revision:/ { print $$2 }' )
ifeq ($(REV),)
REV:=0
endif

ifeq ($(KERNEL),2.4)
FSNAME:=$(patsubst jffs2-%,jffs2,$(FS))

ifeq ($(FS),squashfs)
brcm6345-fs-prepare:

SINUS_TARGET=Sinus_1054_DSL

$(BIN_DIR)/openwrt-brcm6345-$(KERNEL)-$(FSNAME).bin:
	rm -f $(KDIR)/root.squashfs
	mkdir -p $(KDIR)/root/ram
	test -f delete-files-$(KERNEL)-$(FS) && \
			for f in `cat delete-files-$(KERNEL)-$(FS)`; do \
				rm -rf $(KDIR)/root/$$f;	\
			done; 
	( cd ../../package/base-files/files/$(BOARD)-$(KERNEL) && \
		tar cf - --exclude .svn --exclude var . | tar xvf - -C $(KDIR)/root/; \
		 $(SED) 's,$$R,r$(REV),g' $(KDIR)/root/etc/banner; \
		 [ -n "$(SINUS_CONFIG)" ] && echo "Firmware: $(SINUS_CONFIG)" >>$(KDIR)/root/etc/banner; \
		 echo "Builded in ZFT Lab. at: `date '+%Y%m%d %H:%M'`" >>$(KDIR)/root/etc/banner; \
		 echo "" >>$(KDIR)/root/etc/banner; \
	)
	# sinus.conf
	( \
		echo "SINUS_REVISION=$(REV)"; \
		echo "SINUS_CONFIG=$(SINUS_CONFIG)"; \
		echo "SINUS_BUILD_DATE=`date '+%Y%m%d'`"; \
		echo "SINUS_BUILD_TIME=`date '+%H:%M'`"; \
		echo "SINUS_TYPE=tmpfs"; \
		echo "SINUS_TARGET=$(SINUS_TARGET)"; \
		echo "SINUS_HELPDESK=http://flyrouter.net/downloads/software/sinus1054/config"; \
		echo "SINUS_WPAPASS=sinus1054"; \
		echo "SINUS_LEDMODE=Blink"; \
	) >$(KDIR)/root/etc/sinus.conf

	# Create firmware:
	#
	mkdir -p $(KDIR)/root/usr/sbin
	cp ../../brcm6345-2.4/broadcom-programs/adslctl $(KDIR)/root/usr/sbin/
	cp ../../brcm6345-2.4/broadcom-programs/adsl_phy.bin $(KDIR)/root/usr/sbin/
	cp ../../brcm6345-2.4/broadcom-programs/atmctl $(KDIR)/root/usr/sbin/
	cp ../../brcm6345-2.4/broadcom-programs/nas $(KDIR)/root/usr/sbin/
	cp ../../brcm6345-2.4/broadcom-programs/netctl $(KDIR)/root/usr/sbin/
	cp ../../brcm6345-2.4/broadcom-programs/br2684ctl $(KDIR)/root/usr/sbin/
	cp ../../brcm6345-2.4/broadcom-programs/pvc2684ctl $(KDIR)/root/usr/sbin/
	cp ../../brcm6345-2.4/broadcom-programs/pvc2684d $(KDIR)/root/usr/sbin/
	cp ../../brcm6345-2.4/broadcom-programs/wlctl $(KDIR)/root/usr/sbin/
	mkdir -p $(KDIR)/root/lib/modules/2.4.17
	cp ../../brcm6345-2.4/broadcom-modules/adsldd.o $(KDIR)/root/lib/modules/2.4.17/
	cp ../../brcm6345-2.4/broadcom-modules/atmapi.o $(KDIR)/root/lib/modules/2.4.17/
	cp ../../brcm6345-2.4/broadcom-modules/bcm_enet.o $(KDIR)/root/lib/modules/2.4.17/
	cp ../../brcm6345-2.4/broadcom-modules/bcmprocfs.o $(KDIR)/root/lib/modules/2.4.17/
	cp ../../brcm6345-2.4/broadcom-modules/bcm_usb.o $(KDIR)/root/lib/modules/2.4.17/
	cp ../../brcm6345-2.4/broadcom-modules/blaa.o $(KDIR)/root/lib/modules/2.4.17/
	cp ../../brcm6345-2.4/broadcom-modules/wl.o $(KDIR)/root/lib/modules/2.4.17/
	#
	sudo chown -Rh root.root $(KDIR)/root
	sudo bin/mkcramfs -e 0 -r $(KDIR)/root $(KDIR)/root_fs
	sudo chown -Rh builder.builder $(KDIR)/root $(KDIR)/root_fs
	#
	cp -f $(KDIR)/linux-2.4.17/vmlinux $(KDIR)/
	$(STAGING_DIR)/bin/mips-linux-uclibc-strip --remove-section=.note --remove-section=.comment $(KDIR)/vmlinux
	bin/cmplzma.sh $(KDIR)/vmlinux $(KDIR)/kernel
	#
	cp $(KDIR)/root_fs .
	cp $(KDIR)/kernel .
	bin/recompose -z "ZFT Lab. BCM6345" -v 0.1
	#
	#
	# Check size of rootfs (size should be less than FLASH_SIZE - bootloader - kernel_size - 64k (datafs)
	#( \
	#	if [ `stat -c "%s" $(KDIR)/root.sqfs` -gt $$(($(SINUS_FLASH_SIZE)-$(SINUS_KERNEL_START)-$(SINUS_KERNEL_SIZE)-0x10000)) ]; then \
	#		echo "ROOTFS is oversized!!!"; \
	#		exit 1; \
	#	else \
	#		echo "Size is ok"; \
	#	fi; \
	#)

install: $(BIN_DIR)/openwrt-brcm6345-$(KERNEL)-$(FSNAME).bin
endif

endif

clean:
prepare: brcm6345-fs-prepare
compile:
install: $(BIN_DIR)/openwrt-$(BOARD)-$(KERNEL)-$(FS).bin

