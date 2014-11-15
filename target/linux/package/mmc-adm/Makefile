# $Id: Makefile 001 2008-04-02 23:42:52Z zig $

include $(TOPDIR)/rules.mk
include ../../rules.mk

PKG_NAME:=mmc
PKG_VERSION:=0.0.1
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://kiel.kool.dk/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-adm-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,KMOD_MMC-ADM,kmod-mmc-adm,$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE),$(ARCH),kernel ($(LINUX_VERSION)-$(BOARD)-$(LINUX_RELEASE))))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(CP) ./mmc-adm.c $(PKG_BUILD_DIR)/
	( cd $(PKG_BUILD_DIR); \
		$(TARGET_CC) -I$(STAGING_DIR)/include \
		-I$(STAGING_DIR)/usr/include \
		-I$(LINUX_DIR)/include \
		-W -Wall -Wstrict-prototypes -Wmissing-prototypes -fno-strict-aliasing -mips32 -O2 \
		-DMODULE -D__KERNEL__ \
		-fno-pic -mno-abicalls -mlong-calls -c  \
		-o mmc-adm.o mmc-adm.c; \
	)
	touch $@

$(IPKG_KMOD_MMC-ADM):
	install -m0755 -d $(IDIR_KMOD_MMC-ADM)/etc/modules.d
	echo "mmc-adm" > $(IDIR_KMOD_MMC-ADM)/etc/modules.d/98-mmc-adm
	install -m0755 -d $(IDIR_KMOD_MMC-ADM)/lib/modules/$(LINUX_VERSION)
	$(CP) $(PKG_BUILD_DIR)/mmc-adm.$(LINUX_KMOD_SUFFIX) \
	$(IDIR_KMOD_MMC-ADM)/lib/modules/$(LINUX_VERSION)/
	$(IPKG_BUILD) $(IDIR_KMOD_MMC-ADM) $(PACKAGE_DIR)
	$(RSTRIP_KMOD) $(IDIR_KMOD_MMC-ADM)
