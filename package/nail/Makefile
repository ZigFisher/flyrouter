# $Id: Makefile 001 2006-07-18 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=nail
PKG_VERSION:=11.25
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=@SF/nail
#PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/linux/source/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_CAT:=bzcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,NAIL,nail,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	rm -rf $(PKG_BUILD_DIR)/config.h
	$(CP) $(TOPDIR)/package/nail/files/config.h $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	$(MAKE) -C $(PKG_BUILD_DIR) config.h \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	all
	touch $@

$(IPKG_NAIL): 
	install -m0755 -d $(IDIR_NAIL)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/nail $(IDIR_NAIL)/usr/bin/
	install -m0755 -d $(IDIR_NAIL)/etc             
	install -m0600 $(TOPDIR)/package/nail/files/nail.rc $(IDIR_NAIL)/etc/ 
	$(RSTRIP) $(IDIR_NAIL)
	$(IPKG_BUILD) $(IDIR_NAIL) $(PACKAGE_DIR)
