# $Id: Makefile 4000 2008-12-05 12:00:19Z alexlp $

include $(TOPDIR)/rules.mk

PKG_NAME:=owfs
PKG_VERSION:=2.7p24
PKG_RELEASE:=3
#PKG_MD5SUM:=8aea6f3bb0dc9fdb52ed3f0e9d84fa79

PKG_SOURCE_URL:=@SF/owfs
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,OWFS,owfs,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,OWCAPI,owcapi,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,OWNETAPI,ownetapi,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,OWLIB,owlib,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,OWSHELL,owshell,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

ifneq ($(BR2_COMPILE_OWFS_WITH_USB),y)
DISABLE_USB:=--disable-usb
else
DISABLE_USB:=--enable-usb
PKG_DEPEND+=", libusb"
endif

ifneq ($(BR2_COMPILE_OWFS_WITH_PARPORT),y)
DISABLE_PARPORT:=--disable-parport
else
DISABLE_PARPORT:=--enable-parport
endif

ifneq ($(BR2_COMPILE_OWFS_WITH_I2C),y)
DISABLE_I2C:=--disable-i2c
else
DISABLE_I2C:=--enable-i2c
endif

ifneq ($(BR2_COMPILE_OWFS_WITH_SWIG),y)
DISABLE_SWIG:=--disable-swig
else
DISABLE_SWIG:=--enable-swig
endif

ifneq ($(BR2_COMPILE_OWFS_WITH_OWPHP),y)
DISABLE_OWPHP:=--disable-owphp
else
DISABLE_OWPHP:=--enable-owphp
endif

ifneq ($(BR2_COMPILE_OWFS_WITH_OWPERL),y)
DISABLE_OWPERL:=--disable-owperl
else
DISABLE_OWPERL:=--enable-owperl
endif

ifneq ($(BR2_COMPILE_OWFS_WITH_OWTCL),y)
DISABLE_OWTCL:=--disable-owtcl
else
DISABLE_OWTCL:=--enable-owtcl
endif

ifneq ($(BR2_COMPILE_OWFS_WITH_OWPYTHON),y)
DISABLE_OWPYTHON:=--disable-owpython
else
DISABLE_OWPYTHON:=--enable-owpython
endif



#--enable-owshell        Enable owshell support (default true)
#--enable-owlib          Enable owlib support (default true)
#--enable-ownetlib       Enable ownetlib support (default true)
#--enable-thermocouple   Enable thermocouple support (default true)
#--enable-owhttpd        Enable owhttpd module (default true)
#--enable-owftpd         Enable owftpd module (default true)
#--enable-owserver       Enable owserver module (default true)
#--enable-ownet          Enable ownet module (default true)
#--enable-owtap          Enable owtap module (default true)
#--enable-owside         Enable owside module (default true)
#--enable-owmon          Enable owmon module (default true)
#--enable-owcapi         Enable owcapi module (default true)
#--enable-swig           Enable swig (default auto)
#--enable-owperl         Enable owperl module (default true)
#--enable-owphp          Enable owphp module (default true)
#--enable-owpython       Enable owpython module (default true)
#--enable-owtcl          Enable owtcl module (default true)
#--enable-profiling      Enable profiling (default false)



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
			$(DISABLE_NLS) \
			$(DISABLE_LARGEFILE) \
			$(DISABLE_USB) \
			$(DISABLE_PARPORT) \
			$(DISABLE_I2C) \
			$(DISABLE_SWIG) \
			$(DISABLE_OWPHP) \
			$(DISABLE_OWPERL) \
			$(DISABLE_OWTCL) \
			$(DISABLE_OWPYTHON) \
			--disable-owfs \
			--disable-owftpd \
			--disable-cache \
			--disable-ownet \
			--disable-zero \
			--disable-w1 \
			--disable-ha7 \
			--disable-debug \
			--disable-debian \
			--disable-tai8570 \
			--enable-owhttpd \
			--enable-owserver \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
	touch $@


$(IPKG_OWFS):
	install -d -m0755 $(IDIR_OWFS)/usr/bin
	#install -m0755 ./files/kill-owfs $(IDIR_OWFS)/usr/bin
	#install -m0755 ./files/run-owfs $(IDIR_OWFS)/usr/bin
	#$(CP) $(PKG_INSTALL_DIR)/usr/bin/owfs $(IDIR_OWFS)/usr/bin
	#$(CP) $(PKG_INSTALL_DIR)/usr/bin/owftpd $(IDIR_OWFS)/usr/bin
	#$(CP) $(PKG_INSTALL_DIR)/usr/bin/owhttpd $(IDIR_OWFS)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/owserver $(IDIR_OWFS)/usr/bin
	#install -d -m0755 $(IDIR_OWFS)/etc/default
	#$(CP) ./files/default/* $(IDIR_OWFS)/etc/default
	install -d -m0755 $(IDIR_OWFS)/etc/init.d
	$(CP) ./files/init.d/S90owfs-micro $(IDIR_OWFS)/etc/init.d
	# Up by Zig
	install -d -m0755 $(IDIR_OWFS)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libow-2.7.so.* $(IDIR_OWFS)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libow.so $(IDIR_OWFS)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/owdir $(IDIR_OWFS)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/owread $(IDIR_OWFS)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/owwrite $(IDIR_OWFS)/usr/bin
	#$(CP) $(PKG_INSTALL_DIR)/usr/bin/owget $(IDIR_OWFS)/usr/bin
	#$(CP) $(PKG_INSTALL_DIR)/usr/bin/owpresent $(IDIR_OWFS)/usr/bin
	#
	$(RSTRIP) $(IDIR_OWFS)
	$(IPKG_BUILD) $(IDIR_OWFS) $(PACKAGE_DIR)


$(IPKG_OWCAPI):
	install -d -m0755 $(IDIR_OWCAPI)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/owcapi.h $(IDIR_OWCAPI)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/owfs_config.h $(IDIR_OWCAPI)/usr/include
	install -d -m0755 $(IDIR_OWCAPI)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libowcapi-2.7.so.* $(IDIR_OWCAPI)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libowcapi.so $(IDIR_OWCAPI)/usr/lib
	$(RSTRIP) $(IDIR_OWCAPI)
	$(IPKG_BUILD) $(IDIR_OWCAPI) $(PACKAGE_DIR)


$(IPKG_OWNETAPI):
	install -d -m0755 $(IDIR_OWNETAPI)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ownetapi.h $(IDIR_OWNETAPI)/usr/include
	install -d -m0755 $(IDIR_OWNETAPI)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libownet-2.7.so.* $(IDIR_OWNETAPI)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libownet.so $(IDIR_OWNETAPI)/usr/lib
	$(RSTRIP) $(IDIR_OWNETAPI)
	$(IPKG_BUILD) $(IDIR_OWNETAPI) $(PACKAGE_DIR)


$(IPKG_OWLIB):
	install -d -m0755 $(IDIR_OWLIB)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libow-2.7.so.* $(IDIR_OWLIB)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libow.so $(IDIR_OWLIB)/usr/lib
	$(RSTRIP) $(IDIR_OWLIB)
	$(IPKG_BUILD) $(IDIR_OWLIB) $(PACKAGE_DIR)


$(IPKG_OWSHELL):
	install -d -m0755 $(IDIR_OWSHELL)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/owdir $(IDIR_OWSHELL)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/owpresent $(IDIR_OWSHELL)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/owread $(IDIR_OWSHELL)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/owwrite $(IDIR_OWSHELL)/usr/bin
	$(RSTRIP) $(IDIR_OWSHELL)
	$(IPKG_BUILD) $(IDIR_OWSHELL) $(PACKAGE_DIR)

