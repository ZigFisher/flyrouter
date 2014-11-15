# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=pjproject
PKG_VERSION:=1.8.5
PKG_RELEASE:=1
#PKG_MD5SUM:=

PKG_SOURCE_URL:=http://www.pjsip.org/release/1.8.5
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_CAT:=bzcat
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,PJPROJECT,pjproject,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured: $(PKG_BUILD_DIR)/.prepared 
	(cd $(PKG_BUILD_DIR); rm -rf config.cache; \
	       $(TARGET_CONFIGURE_OPTS) \
	       CFLAGS="$(TARGET_CFLAGS)" \
	       CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include" \
	       LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib" \
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
	       --disable-l16-codec --disable-g722-codec --disable-g7221-codec --disable-ipp \
	       --disable-ilbc-codec --disable-speex-aec --disable-speex-codec \
	       --disable-ssl --enable-gsm-codec --enable-libsamplerate \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
	       $(TARGET_CONFIGURE_OPTS)
	touch $@

$(IPKG_PJPROJECT):
	mkdir -p $(IDIR_PJPROJECT)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/arpd $(IDIR_PJPROJECT)/usr/sbin/
	$(STRIP) $(IDIR_PJPROJECT)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_PJPROJECT) $(PACKAGE_DIR)
