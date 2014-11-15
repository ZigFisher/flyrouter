# $Id: Makefile 3449 2006-03-22 12:26:02Z kaloz $

include $(TOPDIR)/rules.mk

PKG_NAME:=iputils
PKG_VERSION:=20020927
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/main/i/iputils/
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/iputils
PKG_CAT:=zcat

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,PING,ping,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,ARPING,arping,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	#rm -rf $(PKG_INSTALL_DIR)
	#mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include" \
	KERNEL_INCLUDE=$(LINUX_DIR)/include
	touch $@

$(IPKG_PING):
	mkdir -p $(IDIR_PING)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/ping $(IDIR_PING)/usr/bin/ping-iputils
	$(STRIP) $(IDIR_PING)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_PING) $(PACKAGE_DIR)

$(IPKG_ARPING):
	mkdir -p $(IDIR_ARPING)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/arping $(IDIR_ARPING)/usr/bin/arping-iputils
	$(STRIP) $(IDIR_ARPING)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_ARPING) $(PACKAGE_DIR)

