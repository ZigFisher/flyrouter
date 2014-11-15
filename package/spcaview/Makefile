# $Id: Makefile 001 2006-03-29 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=spcaview
PKG_VERSION:=20061208
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://mxhaard.free.fr/spca50x/Download/
#PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/software/linux/archive/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SPCAVIEW,spcaview,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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

$(IPKG_SPCAVIEW):
	install -m0755 -d $(IDIR_SPCAVIEW)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/spcacat $(IDIR_SPCAVIEW)/usr/sbin/
	$(STRIP) $(IDIR_SPCAVIEW)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_SPCAVIEW) $(PACKAGE_DIR)
