# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=shc
PKG_VERSION:=3.8.7
PKG_RELEASE:=1

#PKG_SOURCE_URL:=http://www.datsi.fi.upm.es/~frosal/sources
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
#PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SHC,shc,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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

$(IPKG_SHC):
	install -m0755 -d $(IDIR_SHC)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/shc $(IDIR_SHC)/usr/sbin/
	$(RSTRIP) $(IDIR_SHC)
	$(IPKG_BUILD) $(IDIR_SHC) $(PACKAGE_DIR)

