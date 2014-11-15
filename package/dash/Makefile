# $Id: Makefile 3588 2006-04-05 02:09:22Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=dash
PKG_VERSION:=0.5.3
PKG_RELEASE:=1
PKG_MD5SUM:=1a3cd6669459be4344ec55ec9d4914f8

PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/main/d/dash
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_CAT:=zcat

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,DASH,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
ifeq ($(BR2_COMPILE_DASH_WITH_KDB),y)
	(cd $(PKG_BUILD_DIR) && cp ../../package/webface/kdb.c src/bltin/ && cat ../../package/dash/dash_with_kdb.patch | patch -p1 );
endif
ifeq ($(BR2_COMPILE_DASH_WITH_HDB),y)
	(cd $(PKG_BUILD_DIR) && cp ../../package/webface/hdb/hdb.cpp src/bltin/ && cat ../../package/dash/dash_with_hdb.patch | patch -p1 );
endif
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		./configure \
		  --target=$(GNU_TARGET_NAME) \
		  --host=$(GNU_TARGET_NAME) \
		  --build=$(GNU_HOST_NAME) \
		  --program-prefix="" \
		  --program-suffix="" \
		  --prefix=/usr \
		  --exec-prefix=/ \
		  --bindir=/bin \
		  --sysconfdir=/etc \
	)
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC=$(TARGET_CC) CFLAGS="$(TARGET_CFLAGS)" \
		BINDIR=/bin
	touch $@

$(IPKG_DASH):
	install -d -m0755 $(IDIR_DASH)/bin
	$(CP) $(PKG_BUILD_DIR)/src/$(PKG_NAME) $(IDIR_DASH)/bin/
	$(STRIP) $(IDIR_DASH)/bin/*
	$(IPKG_BUILD) $(IDIR_DASH) $(PACKAGE_DIR)

