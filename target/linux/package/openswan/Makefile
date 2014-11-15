# $Id: Makefile 3635 2006-04-13 15:41:27Z nbd $

include $(TOPDIR)/rules.mk
include ../../rules.mk

PKG_NAME:=openswan
PKG_VERSION:=2.4.6
PKG_RELEASE:=2
PKG_MD5SUM:=b34d71ca49dedad017879b0e912d40dd

PKG_SOURCE_URL:=http://www.openswan.org/download
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_CAT:=zcat

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,KMOD_OPENSWAN,kmod-openswan,$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE),$(ARCH),kernel ($(LINUX_VERSION)-$(BOARD)-$(LINUX_RELEASE))))

FLAGS := $(TARGET_CFLAGS) -I$(PKG_BUILD_DIR)/linux/include -L$(STAGING_DIR)/usr/lib -I$(STAGING_DIR)/usr/include

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		LINUX_RELEASE="$(LINUX_RELEASE)" \
		KERNELSRC="$(LINUX_DIR)" \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		USERCOMPILE="$(FLAGS)" \
		AS="$(TARGET_CC) -c $(TARGET_CFLAGS)" \
		module

$(IPKG_KMOD_OPENSWAN):
	mkdir -p $(IDIR_KMOD_OPENSWAN)/lib/modules/$(LINUX_VERSION)
	$(CP) $(PKG_BUILD_DIR)/modobj*/ipsec.$(LINUX_KMOD_SUFFIX) $(IDIR_KMOD_OPENSWAN)/lib/modules/$(LINUX_VERSION)/
	$(IPKG_BUILD) $(IDIR_KMOD_OPENSWAN) $(PACKAGE_DIR)
