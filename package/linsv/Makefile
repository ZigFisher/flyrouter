# $Id: Makefile 001 2006-03-29 00:00:18Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=linsv
PKG_VERSION:=0.5.2
PKG_RELEASE:=1
PKG_MD5SUM:=

#PKG_SOURCE_URL:=ftp://ham2.cc.fer.hr/pub/users/9a4gl/openwrt/
PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/software/linux/archive/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,LINSV,linsv,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib"
	touch $@

$(IPKG_LINSV):
	install -m0755 -d $(IDIR_LINSV)/usr/sbin
	install -m0755 -d $(IDIR_LINSV)/etc/ax25/linsv/help
	$(CP) $(PKG_BUILD_DIR)/linsv $(IDIR_LINSV)/usr/sbin/
	install -m0644 $(PKG_BUILD_DIR)/etc/*.sv $(IDIR_LINSV)/etc/ax25/linsv/
	install -m0644 $(PKG_BUILD_DIR)/etc/help/g.hlp $(IDIR_LINSV)/etc/ax25/linsv/help/
	$(STRIP) $(IDIR_LINSV)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_LINSV) $(PACKAGE_DIR)

