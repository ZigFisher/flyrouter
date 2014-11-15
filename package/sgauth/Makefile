# $Id: Makefile 001 2006-03-29 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=sgauth
PKG_VERSION:=2
PKG_RELEASE:=1
PKG_MD5SUM:=

#PKG_SOURCE_URL:=http://drool-linux.pisem.net/sgauth-2-alt5.drool.src.rpm
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SGAUTH,sgauth,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	STAGING_DIR="$(STAGING_DIR)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include"
	touch $@

$(IPKG_SGAUTH):
	install -m0755 -d $(IDIR_SGAUTH)/etc/init.d
	install -m0755 ./files/sgauth $(IDIR_SGAUTH)/etc/init.d
	ln -f -s sgauth $(IDIR_SGAUTH)/etc/init.d/S95sgauth
	install -m0755 -d $(IDIR_SGAUTH)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/sgauth $(IDIR_SGAUTH)/usr/sbin/
	$(STRIP) $(IDIR_SGAUTH)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_SGAUTH) $(PACKAGE_DIR)
