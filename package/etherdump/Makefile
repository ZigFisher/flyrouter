# $Id: Makefile 3112 2008-03-19 23:53:19Z zig $

include $(TOPDIR)/rules.mk

PKG_NAME:=etherdump
PKG_VERSION:=2.01
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://freshmeat.net/redir/etherdump/50584/url_tgz
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,ETHERDUMP,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
	    CC="$(TARGET_CC)" \
	    CFLAGS="$(TARGET_CFLAGS)" \
	    LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	    CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include"
	touch $@




$(IPKG_ETHERDUMP):
	install -d -m0755 $(IDIR_ETHERDUMP)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/etherdump $(IDIR_ETHERDUMP)/usr/bin/
	$(STRIP) $(IDIR_ETHERDUMP)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_ETHERDUMP) $(PACKAGE_DIR)
