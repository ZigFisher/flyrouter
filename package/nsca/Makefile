# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=nsca
PKG_VERSION:=2.7.2
PKG_RELEASE:=1
#PKG_MD5SUM:=9b7dc52656f5cbec846a7ba3299f73bd

#PKG_SOURCE_URL:=@SF/nagios
PKG_SOURCE_URL:=http://prdownloads.sourceforge.net/nagios
PKG_SOURCE:=nsca-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/nsca-$(PKG_VERSION)
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,NSCA,nsca,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,SEND-NSCA,send-nsca,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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
		  $(DISABLE_LARGEFILE) \
		  --with-nsca-user="root" \
		  --with-nsca-grp="root" \
		  --with-libmcrypt-prefix="$(STAGING_DIR)/usr/lib" \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -Wall -D_REENTRANT" \
		DESTDIR=$(PKG_INSTALL_DIR) \
		all install
	touch $@

$(IPKG_NSCA):
	install -d -m0755 $(IDIR_NSCA)/etc/nsca
	install -d -m0755 $(IDIR_NSCA)/etc/init.d
	install -m0644 ./files/nsca.cfg $(IDIR_NSCA)/etc/nsca/
	install -m0755 ./files/nsca.init $(IDIR_SEND-NSCA)/etc/init.d/S97nsca
	install -d -m0755 $(IDIR_NSCA)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/src/nsca $(IDIR_NSCA)/usr/sbin
	$(RSTRIP) $(IDIR_NSCA)
	$(IPKG_BUILD) $(IDIR_NSCA) $(PACKAGE_DIR)

$(IPKG_SEND-NSCA):
	install -d -m0755 $(IDIR_SEND-NSCA)/etc/nsca
	install -d -m0755 $(IDIR_SEND-NSCA)/etc/init.d
	install -m0644 ./files/send_nsca.cfg $(IDIR_SEND-NSCA)/etc/nsca/
	install -m0755 ./files/send_nsca_wanip.init $(IDIR_SEND-NSCA)/etc/init.d/S97send_nsca_wanip
	install -d -m0755 $(IDIR_SEND-NSCA)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/src/send_nsca $(IDIR_SEND-NSCA)/usr/sbin
	$(RSTRIP) $(IDIR_SEND-NSCA)
	$(IPKG_BUILD) $(IDIR_SEND-NSCA) $(PACKAGE_DIR)
