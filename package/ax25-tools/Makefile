# $Id: Makefile 001 2006-03-29 00:00:19Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=ax25-tools
PKG_VERSION:=0.0.8
PKG_RELEASE:=1
PKG_MD5SUM:=

#PKG_SOURCE_URL:=@SF/ax25
PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/software/linux/archive/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,AX25-CALL,ax25-call,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,BEACON,beacon,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,KISSATTACH,kissattach,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))                                                                      
$(eval $(call PKG_template,KISSPARMS,kissparms,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))                                                                      
$(eval $(call PKG_template,RXECHO,rxecho,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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


$(IPKG_AX25-CALL):
	mkdir -p $(IDIR_AX25-CALL)/usr/sbin
	cp $(PKG_BUILD_DIR)/user_call/ax25_call $(IDIR_AX25-CALL)/usr/sbin/
	$(STRIP) $(IDIR_AX25-CALL)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_AX25-CALL) $(PACKAGE_DIR)

$(IPKG_BEACON):
	mkdir -p $(IDIR_BEACON)/usr/sbin
	cp $(PKG_BUILD_DIR)/ax25/beacon $(IDIR_BEACON)/usr/sbin/
	$(STRIP) $(IDIR_BEACON)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_BEACON) $(PACKAGE_DIR)

$(IPKG_KISSATTACH):
	mkdir -p $(IDIR_KISSATTACH)/usr/sbin
	cp $(PKG_BUILD_DIR)/kiss/kissattach $(IDIR_KISSATTACH)/usr/sbin/
	$(STRIP) $(IDIR_KISSATTACH)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_KISSATTACH) $(PACKAGE_DIR)

$(IPKG_KISSPARMS):
	mkdir -p $(IDIR_KISSPARMS)/usr/sbin
	cp $(PKG_BUILD_DIR)/kiss/kissparms $(IDIR_KISSPARMS)/usr/sbin/
	$(STRIP) $(IDIR_KISSPARMS)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_KISSPARMS) $(PACKAGE_DIR)

$(IPKG_RXECHO):
	mkdir -p $(IDIR_RXECHO)/usr/sbin
	install -m0755 -d $(IDIR_RXECHO)/etc/ax25
	cp $(PKG_BUILD_DIR)/ax25/rxecho $(IDIR_RXECHO)/usr/sbin/
	install -m0644 $(PKG_BUILD_DIR)/ax25/rxecho.conf $(IDIR_RXECHO)/etc/ax25/   
	$(STRIP) $(IDIR_RXECHO)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_RXECHO) $(PACKAGE_DIR)
		