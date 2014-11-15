# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=vnstati
PKG_VERSION:=beta3
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://humdi.net/vnstat
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,VNSTATI,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS)
	touch $@

$(IPKG_VNSTATI):
	install -d -m0755 $(IDIR_VNSTATI)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/src/vnstati $(IDIR_VNSTATI)/usr/bin/
	$(STRIP) $(IDIR_VNSTATI)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_VNSTATI) $(PACKAGE_DIR)
