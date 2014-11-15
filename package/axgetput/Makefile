# $Id: Makefile 001 2006-03-29 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=axgetput
PKG_VERSION:=0.0.1
PKG_RELEASE:=1
PKG_MD5SUM:=

#PKG_SOURCE_URL:=http://x-berg.in-berlin.de/cgi-bin/viewcvs.cgi/ampr/axgetput/
PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/software/linux/archive/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,AXGETPUT,axgetput,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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

$(IPKG_AXGETPUT):
	install -m0755 -d $(IDIR_AXGETPUT)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/axgetput $(IDIR_AXGETPUT)/usr/bin/
	ln -fs $(PKG_BUILD_DIR)/axgetput $(IDIR_AXGETPUT)/usr/bin/bget                                                                                  
	ln -fs $(PKG_BUILD_DIR)/axgetput $(IDIR_AXGETPUT)/usr/bin/bput
	$(STRIP) $(IDIR_AXGETPUT)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_AXGETPUT) $(PACKAGE_DIR)
