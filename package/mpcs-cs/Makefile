# $Id: Makefile 001 2006-03-29 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=mpcs-cs
PKG_VERSION:=1.0
PKG_RELEASE:=2c
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://www.hamradio.crimea.ua/downloads/software/linux/archive/
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE:=mpcs-$(PKG_VERSION).tgz
PKG_CAT:=zcat

#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/mpcs-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,MPCS-CS,mpcs-cs,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	PATH=$(STAGING_DIR)/bin:$$PATH $(MAKE) -C $(PKG_BUILD_DIR) cross-flyrouter \
	CC="$(TARGET_CC)" \
	DS_CFLAGS="$(TARGET_CFLAGS) -c" \
	DS_LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	DS_CC=$(ARCH)-linux-uclibc-gcc \
	DS_AR=$(ARCH)-linux-uclibc-ar \
	DS_LD=$(ARCH)-linux-uclibc-ld \
	DS_RL=$(ARCH)-linux-uclibc-ranlib \
	DS_ST=$(ARCH)-linux-uclibc-strip \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include" && touch $@

$(IPKG_MPCS-CS):
	install -m0755 -d $(IDIR_MPCS-CS)/usr/sbin
	install -m0755 -d $(IDIR_MPCS-CS)/etc/mpcs
	install -m0755 -d $(IDIR_MPCS-CS)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/Distribution/mpcs-1.0-flyrouter $(IDIR_MPCS-CS)/usr/sbin/mpcs
	install -m0755 ./files/S98mpcs $(IDIR_MPCS-CS)/etc/init.d/
	install -m0644 ./files/mpcs.* $(IDIR_MPCS-CS)/etc/mpcs/
	$(STRIP) $(IDIR_MPCS-CS)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_MPCS-CS) $(PACKAGE_DIR)

