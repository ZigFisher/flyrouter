# $Id: Makefile 001 2006-03-29 00:00:19Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=bluespam
PKG_VERSION:=1.2
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://www.josos.org/index/040-softwares/bluetooth
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,BLUESPAM,bluespam,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include" \
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


$(IPKG_BLUESPAM):
	mkdir -p $(IDIR_BLUESPAM)/usr/sbin
	cp $(PKG_BUILD_DIR)/bluespam $(IDIR_BLUESPAM)/usr/sbin/
	$(STRIP) $(IDIR_BLUESPAM)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_BLUESPAM) $(PACKAGE_DIR)
