# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=microdc2
PKG_VERSION:=0.15.6
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://corsair626.no-ip.org/microdc/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,MICRODC2,microdc2,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/iconv -I$(STAGING_DIR)/usr/include/libxml2" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib" \
		./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--program-prefix="" \
		--program-suffix="" \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--datadir=/usr/share \
		--includedir=/usr/include \
		--infodir=/usr/share/info \
		--libdir=/usr/lib \
		--libexecdir=/usr/lib \
		--localstatedir=/var \
		--mandir=/usr/share/man \
		--sbindir=/usr/sbin \
		--sysconfdir=/etc \
		$(ENABLE_NLS) \
		$(ENABLE_LARGEFILE) \
		--enable-shared \
		--disable-static \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CCOPT="$(TARGET_CFLAGS)" \
		INCLS="-I. -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/iconv -I$(STAGING_DIR)/usr/include/libxml2" \
		LIBS="-L$(STAGING_DIR)/usr/lib"
	touch $@

$(IPKG_MICRODC2):
	mkdir -p $(IDIR_MICRODC2)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/src/microdc2 $(IDIR_MICRODC2)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/src/tthsum $(IDIR_MICRODC2)/usr/bin/
	$(STRIP) $(IDIR_MICRODC2)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_MICRODC2) $(PACKAGE_DIR)
