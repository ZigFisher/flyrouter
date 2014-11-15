# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=bluez-utils
PKG_VERSION:=2.25
#PKG_VERSION:=3.24
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://bluez.sourceforge.net/download
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,HCICONFIG,hciconfig,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,HCID,hcid,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,HCITOOL,hcitool,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,L2PING,l2ping,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,RFCOMM,rfcomm,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,SDPD,sdpd,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,PAND,pand,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,SDPTOOL,sdptool,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.source: $(DL_DIR)/$(PKG_SOURCE)
	zcat $(DL_DIR)/$(PKG_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	touch $(BLUEZLIBS_DIR)/.source

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include/dbus-1.0" \
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
			--enable-shared \
			--disable-static \
			--disable-rpath \
			--disable-dbus \
			--disable-fuse \
			--disable-alsa \
			--disable-cups \
			--disable-pcmcia \
			--disable-initscripts \
			--disable-bccmd \
			--disable-avctrl \
			--disable-hid2hci \
			--disable-dfutool \
			--disable-bluepin \
			--with-bluez=$(STAGING_DIR)/usr/include \
			--with-usb=$(STAGING_DIR)/usr/include \
	)
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
	touch $@


$(IPKG_HCICONFIG):
	install -d -m0755 $(IDIR_HCICONFIG)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/hciconfig $(IDIR_HCICONFIG)/usr/sbin/
	$(RSTRIP) $(IDIR_HCICONFIG)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_HCICONFIG) $(PACKAGE_DIR)

$(IPKG_HCID):
	install -d -m0755 $(IDIR_HCID)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/hcid $(IDIR_HCID)/usr/sbin/
	install -d -m0755 $(IDIR_HCID)/etc/bluetooth
	$(CP) $(PKG_INSTALL_DIR)/../hcid/hcid.conf $(IDIR_HCID)/etc/bluetooth/
	install -m0700 ./files/givepin $(IDIR_HCID)/etc/bluetooth/
	install ./files/pin $(IDIR_HCID)/etc/bluetooth/
	$(RSTRIP) $(IDIR_HCID)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_HCID) $(PACKAGE_DIR)

$(IPKG_HCITOOL):
	install -d -m0755 $(IDIR_HCITOOL)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/hcitool $(IDIR_HCITOOL)/usr/bin/
	$(RSTRIP) $(IDIR_HCITOOL)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_HCITOOL) $(PACKAGE_DIR)

$(IPKG_L2PING):
	install -d -m0755 $(IDIR_L2PING)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/l2ping $(IDIR_L2PING)/usr/bin/
	$(RSTRIP) $(IDIR_L2PING)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_L2PING) $(PACKAGE_DIR)

$(IPKG_RFCOMM):
	install -d -m0755 $(IDIR_RFCOMM)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/rfcomm $(IDIR_RFCOMM)/usr/bin/
	install -d -m0755 $(IDIR_RFCOMM)/etc/bluetooth
	$(CP) $(PKG_INSTALL_DIR)/etc/bluetooth/rfcomm.conf $(IDIR_RFCOMM)/etc/bluetooth/
	install -m0700 ./files/givepin $(IDIR_RFCOMM)/etc/bluetooth/
	install ./files/pin $(IDIR_RFCOMM)/etc/bluetooth/
	$(RSTRIP) $(IDIR_RFCOMM)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_RFCOMM) $(PACKAGE_DIR)

$(IPKG_SDPD):
	install -d -m0755 $(IDIR_SDPD)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/sdpd $(IDIR_SDPD)/usr/sbin/
	$(RSTRIP) $(IDIR_SDPD)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_SDPD) $(PACKAGE_DIR)

$(IPKG_PAND):
	install -d -m0755 $(IDIR_PAND)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/pand $(IDIR_PAND)/usr/sbin/
	$(RSTRIP) $(IDIR_PAND)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_PAND) $(PACKAGE_DIR)

$(IPKG_SDPTOOL):
	install -d -m0755 $(IDIR_SDPTOOL)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/sdptool $(IDIR_SDPTOOL)/usr/bin/
	$(RSTRIP) $(IDIR_SDPTOOL)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_SDPTOOL) $(PACKAGE_DIR)


#	install -d -m0755 $(IDIR_BLUEZ_UTILS)/var/lib/bluetooth

