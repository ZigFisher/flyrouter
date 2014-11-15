# $Id: Makefile 001 2006-03-29 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=digitemp
PKG_VERSION:=3.6.0
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://www.digitemp.com/software/linux/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,DIGITEMP,digitemp,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) ds9097 ds9097u -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) -DLINUX" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include -I$(PKG_BUILD_DIR)/src -I$(PKG_BUILD_DIR)/userial"
	touch $@

$(IPKG_DIGITEMP):
	install -m0755 -d $(IDIR_DIGITEMP)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/digitemp_DS9097 $(IDIR_DIGITEMP)/usr/sbin/
	#$(CP) $(PKG_BUILD_DIR)/digitemp_DS9097U $(IDIR_DIGITEMP)/usr/sbin/
	install -m0755 -d $(IDIR_DIGITEMP)/etc
	install -m0644 ./files/digitemp.conf $(IDIR_DIGITEMP)/etc/
	$(STRIP) $(IDIR_DIGITEMP)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_DIGITEMP) $(PACKAGE_DIR)
