# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=irda-utils
PKG_VERSION:=0.9.18
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=@SF/irda-utils
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,IRDA_UTILS,irda-utils,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	V=1 \
	CC="$(TARGET_CC)" \
	LD="$(TARGET_LD)" \
	CFLAGS="$(TARGET_CFLAGS) -I$(PKG_BUILD_DIR)/include" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/glib-1.2 -I$(STAGING_DIR)/usr/lib/glib/include/"
	touch $@

$(IPKG_IRDA_UTILS):
	install -m0755 -d $(IDIR_IRDA_UTILS)/etc/irda
	install -m0644 ./files/irda.conf $(IDIR_IRDA_UTILS)/etc/irda/irda.conf
	install -m0755 -d $(IDIR_IRDA_UTILS)/etc/init.d
	install -m0755 ./files/irda.rc $(IDIR_IRDA_UTILS)/etc/init.d/irda.rc
	install -d -m0755 $(IDIR_IRDA_UTILS)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/irdaping/irdaping $(IDIR_IRDA_UTILS)/usr/bin/
	install -d -m0755 $(IDIR_IRDA_UTILS)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/irattach/irattach $(IDIR_IRDA_UTILS)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/irnetd/irnetd $(IDIR_IRDA_UTILS)/usr/sbin/
	$(RSTRIP) $(IDIR_IRDA_UTILS)
	$(IPKG_BUILD) $(IDIR_IRDA_UTILS) $(PACKAGE_DIR)

