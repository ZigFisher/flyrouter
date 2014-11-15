# $Id: Makefile 3112 2008-03-19 23:53:19Z zig $

include $(TOPDIR)/rules.mk

PKG_NAME:=nrpe
PKG_VERSION:=2.12
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=@SF/nagios
PKG_MD5SUM:=b2d75e2962f1e3151ef58794d60c9e97
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,NRPE,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured: $(PKG_BUILD_DIR)/.prepared
	(cd $(PKG_BUILD_DIR); \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS)" \
	CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include" \
	LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib" \
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
	$(DISABLE_LARGEFILE) \
	--disable-ssl \
	--enable-command-args \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
	    CC="$(TARGET_CC)" \
	    CFLAGS="$(TARGET_CFLAGS)" \
	    LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	    CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include"
	touch $@

$(IPKG_NRPE):
	install -d -m0755 $(IDIR_NRPE)/usr/sbin
	install -d -m0755 $(IDIR_NRPE)/etc
	install -d -m0755 $(IDIR_NRPE)/etc/init.d
	install -m0755 $(PKG_BUILD_DIR)/src/nrpe $(IDIR_NRPE)/usr/sbin/
	install -m0644 ./files/nrpe.cfg $(IDIR_NRPE)/etc/
	install -m0755 ./files/nrpe.init $(IDIR_NRPE)/etc/init.d/S90nrpe
	$(STRIP) $(IDIR_NRPE)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_NRPE) $(PACKAGE_DIR)
