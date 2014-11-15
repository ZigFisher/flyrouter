# $Id: Makefile 001 2006-03-29 00:00:19Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=libax25
PKG_VERSION:=0.0.11
PKG_RELEASE:=1
PKG_MD5SUM:=

#PKG_SOURCE_URL:=@SF/ax25
PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/software/linux/archive/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,LIBAX25,libax25,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include" \
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

$(IPKG_LIBAX25):
	install -m0755 -d $(IDIR_LIBAX25)/etc/ax25
	install -m0644 $(PKG_BUILD_DIR)/axports $(IDIR_LIBAX25)/etc/ax25/
	install -m0755 -d $(IDIR_LIBAX25)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libax25.{a,la,so*} $(IDIR_LIBAX25)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libax25io.{a,la,so*} $(IDIR_LIBAX25)/usr/lib/
	$(RSTRIP) $(IDIR_LIBAX25)
	$(IPKG_BUILD) $(IDIR_LIBAX25) $(PACKAGE_DIR)


$(STAGING_DIR)/usr/lib/libax25.so: $(PKG_BUILD_DIR)/.built
	mkdir -p $(STAGING_DIR)/usr/include/netax25/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/netax25/*.h $(STAGING_DIR)/usr/include/netax25/
	mkdir -p $(STAGING_DIR)/usr/include/netrose/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/netax25/kernel_rose.h $(STAGING_DIR)/usr/include/netrose/rose.h
	mkdir -p $(STAGING_DIR)/usr/include/netrom/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/netax25/kernel_netrom.h $(STAGING_DIR)/usr/include/netrose/netrom.h
	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libax25.{a,la,so*} $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libax25io.{a,la,so*} $(STAGING_DIR)/usr/lib/
	touch $(STAGING_DIR)/usr/lib/libax25.so


install-dev: $(STAGING_DIR)/usr/lib/libax25.so

uninstall-dev:
	rm -rf $(STAGING_DIR)/usr/include/netax25/*.h
	rm -rf $(STAGING_DIR)/usr/lib/libax25.{a,la,so*}
	rm -rf $(STAGING_DIR)/usr/lib/libax25io.{a,la,so*}

compile-targets: install-dev
clean-targets: uninstall-dev
