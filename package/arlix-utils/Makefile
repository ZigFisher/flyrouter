# $Id: Makefile 001 2006-03-29 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=arlix-utils
PKG_VERSION:=0.0.1
PKG_RELEASE:=1
PKG_MD5SUM:=

#PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/linux/source/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,CALIBRATE,calibrate,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,QTH,qth,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,UI-CLUSTER,ui-cluster,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,X-BOT,x-bot,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS=" -L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include"
	touch $@

$(IPKG_CALIBRATE):
	install -m0755 -d $(IDIR_CALIBRATE)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/calibrate $(IDIR_CALIBRATE)/usr/sbin/
	$(STRIP) $(IDIR_CALIBRATE)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_CALIBRATE) $(PACKAGE_DIR)

$(IPKG_QTH):
	install -m0755 -d $(IDIR_QTH)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/qth $(IDIR_QTH)/usr/bin/
	$(STRIP) $(IDIR_QTH)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_QTH) $(PACKAGE_DIR)

$(IPKG_UI-CLUSTER):
	install -m0755 -d $(IDIR_UI-CLUSTER)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/ui-cluster $(IDIR_UI-CLUSTER)/usr/sbin/
	$(STRIP) $(IDIR_UI-CLUSTER)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_UI-CLUSTER) $(PACKAGE_DIR)

$(IPKG_X-BOT):
	install -m0755 -d $(IDIR_X-BOT)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/x-bot $(IDIR_X-BOT)/usr/sbin/
	$(STRIP) $(IDIR_X-BOT)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_X-BOT) $(PACKAGE_DIR)
