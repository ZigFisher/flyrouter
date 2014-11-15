# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=alsa-utils
PKG_VERSION:=1.0.13
PKG_RELEASE:=1
PKG_MD5SUM:=dfe4bb5d3217f3ec662b172ce8397cf0

PKG_SOURCE_URL:=ftp://ftp.alsa-project.org/pub/utils/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_CAT:=bzcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,APLAY,aplay,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,AMIXER,amixer,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,ALSAMIXER,alsamixer,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

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
		  --disable-rpath \
		  --disable-alsatest \
		  $(DISABLE_LARGEFILE) \
		  $(DISABLE_NLS) \
	);
	touch $@

#		  --includedir=/usr/include \


$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -Wall -D_REENTRANT" \
		DESTDIR=$(PKG_INSTALL_DIR) \
		all install
	touch $@

$(IPKG_APLAY):
	install -d -m0755 $(IDIR_APLAY)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/aplay $(IDIR_APLAY)/usr/bin/		
	$(RSTRIP) $(IDIR_APLAY)
	$(IPKG_BUILD) $(IDIR_APLAY) $(PACKAGE_DIR)

$(IPKG_AMIXER):
	install -d -m0755 $(IDIR_AMIXER)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/amixer $(IDIR_AMIXER)/usr/bin/
	$(RSTRIP) $(IDIR_AMIXER)
	$(IPKG_BUILD) $(IDIR_AMIXER) $(PACKAGE_DIR)

$(IPKG_ALSAMIXER):
	install -d -m0755 $(IDIR_ALSAMIXER)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/alsamixer $(IDIR_ALSAMIXER)/usr/bin/
	$(RSTRIP) $(IDIR_ALSAMIXER)
	$(IPKG_BUILD) $(IDIR_ALSAMIXER) $(PACKAGE_DIR)
