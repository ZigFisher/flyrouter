# $Id: Makefile 001 2006-03-29 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=mpcs
PKG_VERSION:=tiny
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/software/linux/archive/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,MPCS-TINY,mpcs-tiny,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	PATH=$(STAGING_DIR)/bin:$$PATH $(MAKE) -C $(PKG_BUILD_DIR) cross-$(ARCH)-linux-uclibc \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include" && touch $@

$(IPKG_MPCS-TINY):
	install -m0755 -d $(IDIR_MPCS-TINY)/usr/sbin
	install -m0755 -d $(IDIR_MPCS-TINY)/etc/mpcs
	install -m0755 -d $(IDIR_MPCS-TINY)/etc/init.d
	#$(CP) $(PKG_BUILD_DIR)/Distribution/mpcs-0.9d-$(ARCH)-linux-uclibc $(IDIR_MPCS-TINY)/usr/sbin/mpcs
	#$(CP) $(PKG_BUILD_DIR)/Distribution/mpcs-0.8k-rc13m-$(ARCH)-linux-uclibc $(IDIR_MPCS-TINY)/usr/sbin/mpcs
	$(CP) $(PKG_BUILD_DIR)/mpcs $(IDIR_MPCS-TINY)/usr/sbin/mpcs
	install -m0755 ./files/S98mpcs $(IDIR_MPCS-TINY)/etc/init.d/
	#install -m0644 ./files/mpcs.ac $(IDIR_MPCS-TINY)/etc/mpcs/
	install -m0644 ./files/mpcs.conf $(IDIR_MPCS-TINY)/etc/mpcs/
	install -m0644 ./files/mpcs.guess $(IDIR_MPCS-TINY)/etc/mpcs/
	install -m0644 ./files/mpcs.server $(IDIR_MPCS-TINY)/etc/mpcs/
	install -m0644 ./files/mpcs.srvid $(IDIR_MPCS-TINY)/etc/mpcs/
	install -m0644 ./files/mpcs.user $(IDIR_MPCS-TINY)/etc/mpcs/
	$(STRIP) $(IDIR_MPCS-TINY)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_MPCS-TINY) $(PACKAGE_DIR)

