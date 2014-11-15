# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=alsa-lib
PKG_VERSION:=1.0.13
PKG_RELEASE:=1
PKG_MD5SUM:=d55a9d7d2a79d738a1b7a511cffda4b6

PKG_SOURCE_URL:=ftp://ftp.alsa-project.org/pub/lib/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_CAT:=bzcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,ALSA-LIB,alsa-lib,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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
		  --enable-shared \
		  --enable-static \
		  --disable-debugging \
		  --disable-profiling \
		  --with-softfloat \
		  --with-versioned=no \
		  $(DISABLE_LARGEFILE) \
		  $(DISABLE_NLS) \
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

$(IPKG_ALSA-LIB):
	install -m0755 -d $(IDIR_ALSA-LIB)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libasound.so.* $(IDIR_ALSA-LIB)/usr/lib/
	install -m0755 -d $(IDIR_ALSA-LIB)/etc
	$(CP) $(PKG_INSTALL_DIR)/usr/share/alsa/alsa.conf $(IDIR_ALSA-LIB)/etc/
	$(RSTRIP) $(IDIR_ALSA-LIB)
	$(IPKG_BUILD) $(IDIR_ALSA-LIB) $(PACKAGE_DIR)			

$(STAGING_DIR)/usr/lib/libax25.so: $(PKG_BUILD_DIR)/.built
	mkdir -p $(STAGING_DIR)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/alsa $(STAGING_DIR)/usr/include/
	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libasound.{la,a,so*} $(STAGING_DIR)/usr/lib/
	mkdir -p $(STAGING_DIR)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/alsa.pc $(STAGING_DIR)/usr/lib/pkgconfig/			
	touch $(STAGING_DIR)/usr/lib/libax25.so

install-dev: $(STAGING_DIR)/usr/lib/libax25.so

uninstall-dev:
	rm -rf $(STAGING_DIR)/usr/include/netax25/*.h
	rm -rf $(STAGING_DIR)/usr/lib/libasound.so.*

compile-targets: install-dev
clean-targets: uninstall-dev

