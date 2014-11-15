# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=libdebug
PKG_VERSION:=0.4.2
PKG_RELEASE:=1
PKG_MD5SUM:=e1fa1d7cb38e4487947a0b2883df57d4

PKG_SOURCE_URL:=http://oasis.frogfoot.net/code/libdebug/download/$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,LIBDEBUG,libdebug,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	    CC="$(TARGET_CC)" \
	    CFLAGS="$(TARGET_CFLAGS)" \
	    LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib -L$(PKG_BUILD_DIR)/src" \
	    CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include -I$(PKG_BUILD_DIR)/include" \
	    CROSS="$(TARGET_CROSS)" LD=$(TARGET_CROSS)ld ARCH=$(ARCH) \
	    DESTDIR="$(PKG_INSTALL_DIR)" \
	    prefix="$(PKG_INSTALL_DIR)/usr" \
	    all
	touch $@

$(IPKG_LIBDEBUG):
	install -m0755 -d $(IDIR_LIBDEBUG)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/libdebug.so.* $(IDIR_LIBDEBUG)/usr/lib/
	$(RSTRIP) $(IDIR_LIBDEBUG)
	$(IPKG_BUILD) $(IDIR_LIBDEBUG) $(PACKAGE_DIR)

$(STAGING_DIR)/usr/lib/libdebug.so: $(PKG_BUILD_DIR)/.built
	mkdir -p $(STAGING_DIR)/usr/include/debug
	$(CP) -r $(PKG_BUILD_DIR)/include/debug/* $(STAGING_DIR)/usr/include/debug/
	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/libdebug.{a,so} $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/libdebug.so.* $(STAGING_DIR)/usr/lib/
	touch $@

install-dev: $(STAGING_DIR)/usr/lib/libdebug.so

uninstall-dev:
	rm -rf \
		$(STAGING_DIR)/usr/include/debug \
		$(STAGING_DIR)/usr/lib/libdebug.{a,so} \
		$(STAGING_DIR)/usr/lib/libdebug.so.*

compile-targets: install-dev
clean-targets: uninstall-dev
