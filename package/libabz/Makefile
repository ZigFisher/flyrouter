# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=libabz
PKG_VERSION:=0.6.2
PKG_RELEASE:=1
PKG_MD5SUM:=a44c7a1c8c23e1059b98b1968c3d5d29

PKG_SOURCE_URL:=http://oasis.frogfoot.net/code/libabz/download/$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,LIBABZ,libabz,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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

$(IPKG_LIBABZ):
	install -m0755 -d $(IDIR_LIBABZ)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/libabz.so.* $(IDIR_LIBABZ)/usr/lib/
	$(RSTRIP) $(IDIR_LIBABZ)
	$(IPKG_BUILD) $(IDIR_LIBABZ) $(PACKAGE_DIR)

$(STAGING_DIR)/usr/lib/libabz.so: $(PKG_BUILD_DIR)/.built
	mkdir -p $(STAGING_DIR)/usr/include/abz
	$(CP) -r $(PKG_BUILD_DIR)/include/abz/* $(STAGING_DIR)/usr/include/abz/
	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/libabz.{a,so} $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/libabz.so.* $(STAGING_DIR)/usr/lib/
	touch $@

install-dev: $(STAGING_DIR)/usr/lib/libabz.so

uninstall-dev:
	rm -rf \
		$(STAGING_DIR)/usr/include/abz \
		$(STAGING_DIR)/usr/lib/libabz.{a,so} \
		$(STAGING_DIR)/usr/lib/libabz.so.*

compile-targets: install-dev
clean-targets: uninstall-dev

