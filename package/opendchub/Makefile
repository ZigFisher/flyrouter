# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=opendchub
PKG_VERSION:=0.7.15
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=@SF/opendchub
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,OPENDCHUB,opendchub,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.source: $(DL_DIR)/$(PKG_SOURCE)
	zcat $(DL_DIR)/$(PKG_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	touch $(BLUEZLIBS_DIR)/.source

$(PKG_BUILD_DIR)/.configured: 
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib" \
		./configure \
			--disable-perl \
			--enable-switch_user \
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
			--enable-static \
			--disable-rpath \
	)
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
	touch $@

$(IPKG_OPENDCHUB):
	install -m0755 -d $(IDIR_OPENDCHUB)/usr/sbin $(IDIR_OPENDCHUB)/etc/init.d                                                               
	install -m0755 ./files/S93opendchub $(IDIR_OPENDCHUB)/etc/init.d/S93opendchub
	$(CP) $(PKG_BUILD_DIR)/src/opendchub $(IDIR_OPENDCHUB)/usr/sbin/                                                    
	$(STRIP) $(IDIR_OPENDCHUB)/usr/sbin/*                                                                      
	$(IPKG_BUILD) $(IDIR_OPENDCHUB) $(PACKAGE_DIR)
