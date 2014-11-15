include $(TOPDIR)/rules.mk

LINUX_VERSION:=2.4.18
LINUX_RELEASE:=1
LINUX_KERNEL_MD5SUM:=

include ../rules.mk
include ./config

include ../generic-$(KERNEL)/modules.mk
include ../kernel.mk

$(LINUX_DIR)/.patched: $(LINUX_DIR)/.unpacked
	#[ -d ../generic-$(KERNEL)/patches ] && $(PATCH) $(LINUX_DIR) ../generic-$(KERNEL)/patches $(MAKE_TRACE)
	[ -d ./patches ] && $(PATCH) $(LINUX_DIR) ./patches $(MAKE_TRACE)
	@cp config $(LINUX_DIR)/.config
	@cp ramdisk.gz $(LINUX_DIR)/arch/mips/ramdisk/ramdisk.img
	touch $@

#$(LINUX_DIR)/.unpacked: 
#	touch $@

#$(LINUX_DIR)/.depend_done:
#	touch $@

#$(LINUX_DIR)/.modules_done: 
#	touch $@

#$(STAMP_DIR)/.linux-compile:
#	make -C $(LINUX_DIR)/rtkload bz2
#	touch $@
