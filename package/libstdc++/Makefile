# $Id: Makefile 3112 2008-03-26 23:53:19Z zig $

include $(TOPDIR)/rules.mk

PKG_NAME:=libstdc++
PKG_VERSION:=0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,LIBSTDCXX,libstdc++,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured: $(PKG_BUILD_DIR)/.prepared
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	touch $@

$(IPKG_LIBSTDCXX): $(PKG_BUILD_DIR)/.built
	mkdir -p $(IDIR_LIBSTDCXX)/lib
	$(CP) $(STAGING_DIR)/lib/libstdc++.so.* $(IDIR_LIBSTDCXX)/lib/
	$(CP) $(STAGING_DIR)/lib/libstdc++.so $(IDIR_LIBSTDCXX)/lib/
	$(RSTRIP) $(IDIR_LIBSTDCXX)
	$(IPKG_BUILD) $(IDIR_LIBSTDCXX) $(PACKAGE_DIR)

