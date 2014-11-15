# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=gpioctl
PKG_RELEASE:=0.0.1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,GPIOCTL,gpioctl,$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@


$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
		HOSTCC=$(HOSTCC) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I $(STAGING_DIR)/usr/include "
	touch $@

$(IPKG_GPIOCTL):
	mkdir -p $(IDIR_GPIOCTL)/sbin
	$(CP) $(PKG_BUILD_DIR)/gpioctl $(IDIR_GPIOCTL)/sbin
	$(RSTRIP) $(IDIR_GPIOCTL)/sbin/*
	$(IPKG_BUILD) $(IDIR_GPIOCTL) $(PACKAGE_DIR)
