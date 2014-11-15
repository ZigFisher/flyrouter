# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=dbus
PKG_VERSION:=1.1.1.orig
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://ftp.de.debian.org/debian/pool/main/d/dbus
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-1.1.1
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,DBUS,dbus,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.source: $(DL_DIR)/$(PKG_SOURCE)
	zcat $(DL_DIR)/$(PKG_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	touch $(DBUS_DIR)/.source

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib" \
		PKG_CONFIG_PATH=$(STAGING_DIR)/usr/lib \
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
			$(DISABLE_NLS) \
	)
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
	touch $@

$(IPKG_DBUS):
	install -d -m0755 $(IDIR_DBUS)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(IDIR_DBUS)/usr/bin/
	$(RSTRIP) $(IDIR_DBUS)
	$(IPKG_BUILD) $(IDIR_DBUS) $(PACKAGE_DIR)

$(STAGING_DIR)/usr/lib/libdbus-1.so: $(PKG_BUILD_DIR)/.built
	$(CP) $(PKG_BUILD_DIR)/dbus-1.pc $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(STAGING_DIR)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/dbus-1.0/include/dbus/* $(STAGING_DIR)/usr/include/dbus-1.0/dbus/


install-dev: $(STAGING_DIR)/usr/lib/libdbus-1.so

compile-targets: install-dev

