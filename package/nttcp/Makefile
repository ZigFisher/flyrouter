# $Id: Makefile 2386 2005-11-07 17:57:52Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=nttcp
PKG_VERSION:=1.47
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/nttcp

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,NTTCP,nttcp,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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
	CFLAGS="$(TARGET_CFLAGS) -I $(STAGING_DIR)/usr/include -DVERSION=\"$(PKG_VERSION)\""
	touch $@												

$(IPKG_NTTCP):
	install -d -m0755 $(IDIR_NTTCP)/usr/bin
	install -m0755 $(PKG_BUILD_DIR)/nttcp $(IDIR_NTTCP)/usr/bin/
	$(RSTRIP) $(IDIR_NTTCP)
	$(IPKG_BUILD) $(IDIR_NTTCP) $(PACKAGE_DIR)
