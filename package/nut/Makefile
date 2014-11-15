# $Id: Makefile 001 2006-03-29 00:00:19Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=nut
PKG_VERSION:=2.4.1
PKG_RELEASE:=1
PKG_MD5SUM:=

#PKG_SOURCE_URL:=@SF/nut
PKG_SOURCE_URL:=http://www.networkupstools.org/source/2.4/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,NUT-SERVER,nut-server,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,NUT-CLIENTS,nut-clients,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include" \
		LDFLAGS="-lm -L$(STAGING_DIR)/usr/lib" \
		./configure \
		  --with-ipv6=no --with-ssl=no --with-snmp=no \
		  --with-usb=yes --with-serial=no \
		  --with-drivers=usbhid-ups \
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
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install 
	touch $@

$(IPKG_NUT-SERVER):
	# Config:
	mkdir -p $(IDIR_NUT-SERVER)/etc/init.d
	install -m0600 ./files/upsd* $(IDIR_NUT-SERVER)/etc/
	install -m0644 ./files/ups.conf $(IDIR_NUT-SERVER)/etc/
	install -m0755 ./files/S90upsd $(IDIR_NUT-SERVER)/etc/init.d/
	# Drivers:
	mkdir -p $(IDIR_NUT-SERVER)/usr/bin
	cp $(PKG_INSTALL_DIR)/usr/bin/usbhid-ups $(IDIR_NUT-SERVER)/usr/bin/
	cp $(PKG_INSTALL_DIR)/usr/bin/upsdrvctl $(IDIR_NUT-SERVER)/usr/bin/
	$(STRIP) $(IDIR_NUT-SERVER)/usr/bin/*
	# Server:
	mkdir -p $(IDIR_NUT-SERVER)/usr/sbin
	cp $(PKG_INSTALL_DIR)/usr/sbin/upsd $(IDIR_NUT-SERVER)/usr/sbin/
	$(STRIP) $(IDIR_NUT-SERVER)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_NUT-SERVER) $(PACKAGE_DIR)

$(IPKG_NUT-CLIENTS):
	mkdir -p $(IDIR_NUT-CLIENTS)/usr/lib
	cp -a $(PKG_INSTALL_DIR)/usr/lib/lib* $(IDIR_NUT-CLIENTS)/usr/lib/
	mkdir -p $(IDIR_NUT-CLIENTS)/usr/bin
	cp $(PKG_INSTALL_DIR)/usr/bin/upsc $(IDIR_NUT-CLIENTS)/usr/bin/
	$(STRIP) $(IDIR_NUT-CLIENTS)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_NUT-CLIENTS) $(PACKAGE_DIR)
