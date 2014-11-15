# $Id: Makefile 001 2006-04-17 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=sredird
PKG_VERSION:=2.2.2
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://www.ibiblio.org/pub/Linux/system/serial/
#PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/software/linux/archive/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SREDIRD,sredird,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) -Wall -D_POSIX -D_SYSV -D_SELECT -D_HAVE_MACROS" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include"
	touch $@

$(IPKG_SREDIRD):
	install -m0755 -d $(IDIR_SREDIRD)/usr/sbin                                                                                          
	$(CP) $(PKG_BUILD_DIR)/sredird $(IDIR_SREDIRD)/usr/sbin/ 
	$(STRIP) $(IDIR_SREDIRD)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_SREDIRD) $(PACKAGE_DIR)
