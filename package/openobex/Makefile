# $Id: Makefile 001 2006-03-29 00:00:19Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=openobex
PKG_VERSION:=1.3
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=@SF/openobex
#PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/software/linux/archive/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,OPENOBEX,openobex,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib" \
		BLUEZ_CFLAGS="-I$(STAGING_DIR)/usr/include" \
		BLUEZ_LIBS="-L$(STAGING_DIR)/usr/lib -lbluetooth" \
		PKG_CONFIG_PATH="$(STAGING_DIR)/usr/lib/pkgconfig" \
		./configure \
		  --disable-usb \
		  --target=$(GNU_TARGET_NAME) \
		  --host=$(GNU_TARGET_NAME) \
		  --build=$(GNU_HOST_NAME) \
		  --program-prefix="" \
		  --program-suffix="" \
		  --prefix=/usr \
		  --exec-prefix=/usr \
		  --bindir=/usr/bin \
		  --datadir=/usr/share \
		  --includedir=$(STAGING_DIR)/usr/include \
		  --infodir=/usr/share/info \
		  --libdir=/usr/lib \
		  --libexecdir=/usr/lib \
		  --localstatedir=/var \
		  --mandir=/usr/share/man \
		  --sbindir=/usr/sbin \
		  --sysconfdir=/etc \
		  $(DISABLE_NLS) \
		  $(DISABLE_LARGEFILE) \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CCOPT="$(TARGET_CFLAGS) -I$(BUILD_DIR)/linux/include" \
	DESTDIR="$(PKG_INSTALL_DIR)" \
	all install
	touch $@

$(IPKG_OPENOBEX):
	install -m0755 -d $(IDIR_OPENOBEX)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopenobex.{a,la,so*} $(IDIR_OPENOBEX)/usr/lib/
	$(RSTRIP) $(IDIR_OPENOBEX)
	$(IPKG_BUILD) $(IDIR_OPENOBEX) $(PACKAGE_DIR)

$(STAGING_DIR)/usr/lib/libopenobex.so: $(PKG_BUILD_DIR)/.built
	mkdir -p $(STAGING_DIR)/usr/include/openobex/
	$(CP) $(PKG_BUILD_DIR)/include/*.h $(STAGING_DIR)/usr/include/openobex/
	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopenobex.{a,la,so*} $(STAGING_DIR)/usr/lib/
	touch $(STAGING_DIR)/usr/lib/libopenobex.so

install-dev: $(STAGING_DIR)/usr/lib/libopenobex.so

uninstall-dev:
	rm -rf $(STAGING_DIR)/usr/include/openobex/*.h
	rm -rf $(STAGING_DIR)/usr/lib/libopenobex.{a,la,so*}

compile-targets: install-dev
clean-targets: uninstall-dev
