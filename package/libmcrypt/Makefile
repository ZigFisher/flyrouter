# $Id: Makefile 3139 2006-02-05 17:30:49Z florian $

include $(TOPDIR)/rules.mk

PKG_NAME:=libmcrypt
PKG_VERSION:=2.5.8
PKG_RELEASE:=1
PKG_MD5SUM:=c4f491dd411a09e9de3b8702ea6f73eb

PKG_SOURCE_URL:=@SF/mcrypt
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_CAT:=bzcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,LIBMCRYPT,libmcrypt,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib" \
		ac_cv_linux_vers=$(BR2_DEFAULT_KERNEL_HEADERS) \
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
		  --enable-shared \
		  --enable-static \
		  --disable-yydebug \
		  --enable-ipv6 \
		  --with-build-cc=$(HOSTCC) \
		  --with-pcap=linux \
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

$(IPKG_LIBMCRYPT):
	install -m0755 -d $(IDIR_LIBMCRYPT)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmcrypt.so.* $(IDIR_LIBMCRYPT)/usr/lib/
	$(RSTRIP) $(IDIR_LIBMCRYPT)
	$(IPKG_BUILD) $(IDIR_LIBMCRYPT) $(PACKAGE_DIR)

$(STAGING_DIR)/usr/lib/libmcrypt.so: $(PKG_BUILD_DIR)/.built
	mkdir -p $(STAGING_DIR)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/mcrypt* $(STAGING_DIR)/usr/include/
	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmcrypt.{a,so*} $(STAGING_DIR)/usr/lib/
	touch $(STAGING_DIR)/usr/lib/libpcap.so

install-dev: $(STAGING_DIR)/usr/lib/libmcrypt.so

uninstall-dev:
	rm -rf $(STAGING_DIR)/usr/include/mcrypt*
	rm -rf $(STAGING_DIR)/usr/lib/libmcrypt.{a,so*}

compile-targets: install-dev
clean-targets: uninstall-dev
