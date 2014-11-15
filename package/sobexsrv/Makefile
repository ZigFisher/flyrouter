# $Id: Makefile 001 2006-03-29 00:00:19Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=sobexsrv
PKG_VERSION:=1.0.1
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://www.mulliner.org/bluetooth
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SOBEXSRV,sobexsrv,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	export STAGING_DIR=$(STAGING_DIR)
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include/iconv/" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/usr/lib/iconv/" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(BUILD_DIR)/linux/include -I$(STAGING_DIR)/usr/include"
	touch $@

$(IPKG_SOBEXSRV):
	mkdir -p $(IDIR_SOBEXSRV)/usr/sbin
	cp $(PKG_BUILD_DIR)/src/sobexsrv $(IDIR_SOBEXSRV)/usr/sbin/
	$(STRIP) $(IDIR_SOBEXSRV)/usr/sbin/* 
	$(IPKG_BUILD) $(IDIR_SOBEXSRV) $(PACKAGE_DIR)
