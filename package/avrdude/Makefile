# $Id: Makefile 3622 2006-04-12 00:34:26Z nico $

include $(TOPDIR)/rules.mk

PKG_NAME:=avrdude
PKG_VERSION:=5.5
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://ftp.de.debian.org/debian/pool/main/a/avrdude/
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,AVRDUDE,avrdude,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include" \
		LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib -L$(PKG_BUILD_DIR)" \
		ac_cv_func_malloc_0_nonnull=yes \
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
		  --infodir=/usr/share/info \
		  --libdir=/usr/lib \
		  --libexecdir=/usr/lib \
		  --localstatedir=/var \
		  --mandir=/usr/share/man \
		  --sbindir=/usr/sbin \
		  --sysconfdir=/etc \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR)
	touch $@

$(IPKG_AVRDUDE):
	install -d -m0755 $(IDIR_AVRDUDE)/etc/avrdude
	#install -m0644 ./files/ $(IDIR_AVRDUDE)/etc/avrdude/
	install -d -m0755 $(IDIR_AVRDUDE)/usr/sbin
	$(RSTRIP) $(IDIR_AVRDUDE)
	$(IPKG_BUILD) $(IDIR_AVRDUDE) $(PACKAGE_DIR)
	touch $@
