# $Id: Makefile 1408 2005-07-11 19:07:28Z nico $

include $(TOPDIR)/rules.mk

PKG_NAME:=i2c-tools
PKG_VERSION:=3.0.2
PKG_RELEASE:=1
#PKG_MD5SUM:=

PKG_SOURCE_URL:=http://dl.lm-sensors.org/i2c-tools/releases/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).bz2
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_CAT:=bzcat

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,I2C-TOOLS,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) CC=$(TARGET_CC) CFLAGS="$(TARGET_CFLAGS)" -C $(PKG_BUILD_DIR)
	touch $@

$(IPKG_I2C-TOOLS):
	mkdir -p $(IDIR_I2C-TOOLS)/usr/sbin
	cp $(PKG_BUILD_DIR)/$(PKG_NAME)/tools/i2cdetect $(IDIR_I2C-TOOLS)/usr/sbin/
	cp $(PKG_BUILD_DIR)/$(PKG_NAME)/tools/i2cdump $(IDIR_I2C-TOOLS)/usr/sbin/
	cp $(PKG_BUILD_DIR)/$(PKG_NAME)/tools/i2cget $(IDIR_I2C-TOOLS)/usr/sbin/
	cp $(PKG_BUILD_DIR)/$(PKG_NAME)/tools/i2cset $(IDIR_I2C-TOOLS)/usr/sbin/
	$(STRIP) $(IDIR_I2C-TOOLS)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_I2C-TOOLS) $(PACKAGE_DIR)
