# $Id: Makefile 001 2006-04-03 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=empty
PKG_VERSION:=0.6.9b
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=@SF/empty
#PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/linux/source/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,EMPTY,empty,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include"
	touch $@

$(IPKG_EMPTY):
	install -m0755 -d $(IDIR_EMPTY)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/empty $(IDIR_EMPTY)/usr/bin/
	$(STRIP) $(IDIR_EMPTY)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_EMPTY) $(PACKAGE_DIR)
