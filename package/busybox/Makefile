# $Id: Makefile 3625 2006-04-12 12:07:44Z kaloz $

include $(TOPDIR)/rules.mk

PKG_NAME:=busybox
PKG_RELEASE:=1

ifeq ($(strip $(BR2_PACKAGE_BUSYBOX_SNAPSHOT)),y)
# Be aware that this changes daily....
PKG_VERSION:=0.${shell date +"%G%m%d"}
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_SOURCE:=$(PKG_NAME)-snapshot.tar.bz2
PKG_SOURCE_URL:=http://www.busybox.net/downloads/snapshots
PKG_MD5SUM:=x
else
PKG_VERSION:=1.12.4
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://www.busybox.net/downloads
PKG_MD5SUM:=5b8c427bf596732d6b6156672e6e093f
endif
PKG_CAT:=bzcat

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,BUSYBOX,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	cp config/config $(PKG_BUILD_DIR)/.config
	yes '' | $(MAKE) CC=$(TARGET_CC) CROSS="$(TARGET_CROSS)" -C $(PKG_BUILD_DIR) oldconfig
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) V=1 CROSS_COMPILE=$(TARGET_CROSS) CC=$(TARGET_CC) CROSS="$(TARGET_CROSS)" PREFIX="$(IDIR_BUSYBOX)" \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)" ARCH="$(ARCH)" -C $(PKG_BUILD_DIR)
	touch $@

$(IPKG_BUSYBOX):
	$(MAKE) V=1 CROSS_COMPILE=$(TARGET_CROSS) CC=$(TARGET_CC) CROSS="$(TARGET_CROSS)" CONFIG_PREFIX="$(IDIR_BUSYBOX)" \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)" -C $(PKG_BUILD_DIR) install
	$(STRIP) $(IDIR_BUSYBOX)/bin/busybox
	$(IPKG_BUILD) $(IDIR_BUSYBOX) $(PACKAGE_DIR)

