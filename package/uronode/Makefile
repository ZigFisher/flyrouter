# $Id: Makefile 001 2006-03-29 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=uronode
PKG_VERSION:=0.6.5
PKG_RELEASE:=1
PKG_MD5SUM:=

#PKG_SOURCE_URL:=ftp://ftp.n1uro.net/pub/ax25/
PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/software/linux/archive/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,URONODE,uronode,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib"
	touch $@

$(IPKG_URONODE):
	install -m0755 -d $(IDIR_URONODE)/usr/sbin
	install -m0755 -d $(IDIR_URONODE)/etc/ax25
	$(CP) $(PKG_BUILD_DIR)/node $(IDIR_URONODE)/usr/sbin/
	install -m0644 $(PKG_BUILD_DIR)/node.conf $(IDIR_URONODE)/etc/ax25/
	$(STRIP) $(IDIR_URONODE)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_URONODE) $(PACKAGE_DIR)

     
