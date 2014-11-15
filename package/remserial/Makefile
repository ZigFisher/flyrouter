# $Id: Makefile 001 2006-03-29 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=remserial
PKG_VERSION:=1.4
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://lpccomp.bc.ca/remserial/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,REMSERIAL,remserial,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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

$(IPKG_REMSERIAL):
	install -m0755 -d $(IDIR_REMSERIAL)/usr/sbin
	install -m0755 -d $(IDIR_REMSERIAL)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/remserial $(IDIR_REMSERIAL)/usr/sbin/
	$(STRIP) $(IDIR_REMSERIAL)/usr/sbin/*
	install -m0755 ./files/S90remserial $(IDIR_REMSERIAL)/etc/init.d/
	$(IPKG_BUILD) $(IDIR_REMSERIAL) $(PACKAGE_DIR)
