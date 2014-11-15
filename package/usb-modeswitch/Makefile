# $Id: Makefile 001 2006-03-29 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=usb_modeswitch
PKG_VERSION:=1.2.5
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,USB-MODESWITCH,usb-modeswitch,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib"
	touch $@

$(IPKG_USB-MODESWITCH):
	install -m0755 -d $(IDIR_USB-MODESWITCH)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/usb_modeswitch $(IDIR_USB-MODESWITCH)/usr/sbin/
	#mkdir -p $(IDIR_USB-MODESWITCH)/etc
	#$(CP) files/usb_modeswitch.conf $(IDIR_USB-MODESWITCH)/etc/
	$(STRIP) $(IDIR_USB-MODESWITCH)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_USB-MODESWITCH) $(PACKAGE_DIR)
