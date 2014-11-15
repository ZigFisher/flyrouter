# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=bcmflash
PKG_RELEASE:=0.0.2

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,BCMFLASH,bcmflash,$(PKG_RELEASE),$(ARCH)))

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

$(IPKG_BCMFLASH):
	mkdir -p $(IDIR_BCMFLASH)/sbin
	$(CP) $(PKG_BUILD_DIR)/flash $(IDIR_BCMFLASH)/sbin
	$(CP) $(PKG_BUILD_DIR)/ledctrl $(IDIR_BCMFLASH)/sbin
	$(RSTRIP) $(IDIR_BCMFLASH)
	$(IPKG_BUILD) $(IDIR_BCMFLASH) $(PACKAGE_DIR)
