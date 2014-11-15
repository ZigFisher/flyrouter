# $Id: Makefile 001 2006-03-29 00:00:19Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=obexftp
#PKG_VERSION:=0.20
PKG_VERSION:=0.21
#PKG_VERSION:=0.22-rc9
PKG_RELEASE:=1
PKG_MD5SUM:=

#PKG_SOURCE_URL:=@SF/openobex
PKG_SOURCE_URL:=http://triq.net/obexftp
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_CAT:=bzcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-0.21
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,OBEXFTP-LIB,obexftp-lib,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,OBEXFTP-CLIENT,obexftp-client,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,OBEXFTP-SERVER,obexftp-server,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -lbluetooth" \
		OPENOBEX_CFLAGS="-I$(STAGING_DIR)/usr/include" \
		OPENOBEX_LIBS="-L$(STAGING_DIR)/usr/lib -lopenobex" \
		PKG_CONFIG_PATH="$(STAGING_DIR)/usr/lib/pkgconfig" \
		./configure \
		  --disable-python \
		  --disable-perl \
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


$(IPKG_OBEXFTP-LIB):
	install -m0755 -d $(IDIR_OBEXFTP-LIB)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.{a,la,so*} $(IDIR_OBEXFTP-LIB)/usr/lib/
	$(IPKG_BUILD) $(IDIR_OBEXFTP-LIB) $(PACKAGE_DIR)

$(IPKG_OBEXFTP-CLIENT):
	mkdir -p $(IDIR_OBEXFTP-CLIENT)/usr/bin
	cp $(PKG_INSTALL_DIR)/usr/bin/obexftp $(IDIR_OBEXFTP-CLIENT)/usr/bin/
	$(STRIP) $(IDIR_OBEXFTP-CLIENT)/usr/bin/* 
	$(IPKG_BUILD) $(IDIR_OBEXFTP-CLIENT) $(PACKAGE_DIR)

$(IPKG_OBEXFTP-SERVER):
	mkdir -p $(IDIR_OBEXFTP-SERVER)/usr/sbin
	cp $(PKG_INSTALL_DIR)/usr/bin/obexftpd $(IDIR_OBEXFTP-SERVER)/usr/sbin/
	$(STRIP) $(IDIR_OBEXFTP-SERVER)/usr/sbin/* 
	$(IPKG_BUILD) $(IDIR_OBEXFTP-SERVER) $(PACKAGE_DIR)
