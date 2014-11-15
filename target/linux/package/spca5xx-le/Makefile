include $(TOPDIR)/rules.mk
include ../../rules.mk

PKG_NAME := kmod-usb-spca5xx-le
PKG_VERSION := 00.57.08
PKG_RELEASE := 2

PKG_SOURCE_URL := http://mxhaard.free.fr/spca50x/embedded/KernelPatch
PKG_SOURCE := usb-2.4.31LE06_zft.patch.tar.gz
#PKG_SOURCE := linux-2.4.31-716LE08.patch.tgz
#PKG_MD5SUM := c4e195260ea0bb7b532e279503daf266
PKG_CAT := zcat

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,KMOD_USB_SPCA5XX_LE,$(PKG_NAME),$(LINUX_VERSION)-$(BOARD)-$(PKG_RELEASE),$(ARCH),kernel ($(LINUX_VERSION)-$(BOARD)-$(KERNEL_RELEASE))))

$(PKG_BUILD_DIR)/.prepared:
	# copy patched files
	##mkdir -p $(PKG_BUILD_DIR)/arch/cris/kernel
	##$(CP) $(LINUX_DIR)/arch/cris/kernel/crisksyms.c $(PKG_BUILD_DIR)/arch/cris/kernel/
	##mkdir -p $(PKG_BUILD_DIR)/drivers/net/wireless
	##$(CP) $(LINUX_DIR)/drivers/net/wireless/Config.in $(PKG_BUILD_DIR)/drivers/net/wireless/
	##$(CP) $(LINUX_DIR)/drivers/net/wireless/Makefile $(PKG_BUILD_DIR)/drivers/net/wireless/
	mkdir -p $(PKG_BUILD_DIR)/drivers/usb
	$(CP) $(LINUX_DIR)/drivers/usb/Config.in $(PKG_BUILD_DIR)/drivers/usb/
	$(CP) $(LINUX_DIR)/drivers/usb/Makefile $(PKG_BUILD_DIR)/drivers/usb/
	zcat $(DL_DIR)/$(PKG_SOURCE) | tar -C $(PKG_BUILD_DIR) --wildcards -x -f -
	$(PATCH) $(PKG_BUILD_DIR) $(PKG_BUILD_DIR)
	if [ -d ./patches ]; then $(PATCH) $(PKG_BUILD_DIR) ./patches ; fi
	cp -fpR ./src/* $(PKG_BUILD_DIR)/
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C "$(LINUX_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		PATH="$(TARGET_PATH)" \
		SUBDIRS="$(PKG_BUILD_DIR)/drivers/usb/spca5xx" \
		EXTRA_CFLAGS="" \
		modules
	touch $@

$(IPKG_KMOD_USB_SPCA5XX_LE):
	install -m0755 -d $(IDIR_KMOD_USB_SPCA5XX_LE)/etc/modules.d
	install -m0644 ./files/modules $(IDIR_KMOD_USB_SPCA5XX_LE)/etc/modules.d/90-spca5xx-le
	install -m0755 -d $(IDIR_KMOD_USB_SPCA5XX_LE)/lib/modules/$(LINUX_VERSION)
	cp -fpR $(PKG_BUILD_DIR)/drivers/usb/spca5xx/spca5xx.o \
		$(IDIR_KMOD_USB_SPCA5XX_LE)/lib/modules/$(LINUX_VERSION)/spca5xx-le.o
	$(IPKG_BUILD) $(IDIR_KMOD_USB_SPCA5XX_LE) $(PACKAGE_DIR)
