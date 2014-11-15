# $Id: Makefile 001 2006-03-29 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=nmeap
PKG_VERSION:=0.3z
PKG_RELEASE:=1
PKG_MD5SUM:=

#PKG_SOURCE_URL:=@SF/nmeap
PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/software/linux/archive/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,NMEAP,nmeap,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(CP) $(TOPDIR)/package/nmeap/test4.c $(PKG_BUILD_DIR)/tst
	$(MAKE) -C $(PKG_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) -I$(PKG_BUILD_DIR)/inc" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include "
	touch $@

$(IPKG_NMEAP):
	install -m0755 -d $(IDIR_NMEAP)/usr/sbin
	install -m0755 -d $(IDIR_NMEAP)/etc/init.d
	install -m0755 -d $(IDIR_NMEAP)/etc/navigation
	$(CP) $(PKG_BUILD_DIR)/tst/test4 $(IDIR_NMEAP)/usr/sbin/nmeap2kml
	install -m0755 ./files/navigator.sh $(IDIR_NMEAP)/usr/sbin/navigator.sh
	install -m0755 ./files/S94navigation $(IDIR_NMEAP)/etc/init.d/S94navigation
	install -m0644 ./files/nmeap2kml.cfg $(IDIR_NMEAP)/etc/navigation/nmeap2kml.cfg
	$(STRIP) $(IDIR_NMEAP)/usr/sbin/nmeap2kml
	$(IPKG_BUILD) $(IDIR_NMEAP) $(PACKAGE_DIR)
