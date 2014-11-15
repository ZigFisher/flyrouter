# $Id: Makefile 001 2007-04-14 23:42:52Z nbd $

include $(TOPDIR)/rules.mk
include ../../rules.mk

PKG_NAME:=mmc-ng
PKG_VERSION:=1.3.4
PKG_RELEASE:=1
PKG_MD5SUM:=

#PKG_SOURCE_URL:=http://programmingstuff.free.fr/files/openwrt/whiterussian/mmc/src/
PKG_SOURCE_URL:=http://flyrouter.net/sources/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,KMOD_MMC-NG,kmod-mmc-ng,$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE),$(ARCH),kernel ($(LINUX_VERSION)-$(BOARD)-$(LINUX_RELEASE))))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	( cd $(PKG_BUILD_DIR); \
		$(TARGET_CC) -I$(STAGING_DIR)/include \
		-I$(STAGING_DIR)/usr/include \
		-I$(LINUX_DIR)/include \
		-W -Wall -Wstrict-prototypes -Wmissing-prototypes -fno-strict-aliasing -mips32 -O2 \
		-DMODULE -D__KERNEL__ \
		-fno-pic -mno-abicalls -mlong-calls -c  \
		-o mmc-ng.o mmc.c; \
	)
	touch $@

$(IPKG_KMOD_MMC-NG):
	install -m0755 -d $(IDIR_KMOD_MMC-NG)/etc/modules.d
	echo "mmc-ng" > $(IDIR_KMOD_MMC-NG)/etc/modules.d/98-mmc-ng
	install -m0755 -d $(IDIR_KMOD_MMC-NG)/lib/modules/$(LINUX_VERSION)
	$(CP) $(PKG_BUILD_DIR)/mmc-ng.$(LINUX_KMOD_SUFFIX) \
	$(IDIR_KMOD_MMC-NG)/lib/modules/$(LINUX_VERSION)/
	$(IPKG_BUILD) $(IDIR_KMOD_MMC-NG) $(PACKAGE_DIR)
	$(RSTRIP_KMOD) $(IDIR_KMOD_MMC-NG)
