# $Id: Makefile 1410 2005-07-11 20:26:57Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=dbhub
PKG_VERSION:=0.451
PKG_RELEASE:=2-geoka
PKG_MD5SUM:=7d7d7c5bbce9cdb9b000821d687469b8

PKG_SOURCE_URL:=http://mesh.dl.sourceforge.net/sourceforge/dbhub
PKG_SOURCE:=dbhub-$(PKG_VERSION).tgz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,DBHUB,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured: $(PKG_BUILD_DIR)/.prepared
	(cd $(PKG_BUILD_DIR); \
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
		$(DISABLE_LARGEFILE) \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:.
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
	touch $@

$(IPKG_DBHUB):
	install -d -m0755 $(IDIR_DBHUB)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dbhub $(IDIR_DBHUB)/usr/sbin/dbhub
	install -d -m0755 $(IDIR_DBHUB)/etc/dbhub
	$(RSTRIP) $(IDIR_DBHUB)
	$(IPKG_BUILD) $(IDIR_DBHUB) $(PACKAGE_DIR)
