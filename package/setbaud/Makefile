# $Id: Makefile 001 2008-03-08 00:00:01Z zig_fisher $

include $(TOPDIR)/rules.mk

PKG_NAME:=setbaud
PKG_RELEASE:=0.1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SETBAUD,setbaud,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@	

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/setbaud setbaud.c
	touch $@

$(IPKG_SETBAUD):
	install -m0755 -d $(IDIR_SETBAUD)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/setbaud $(IDIR_SETBAUD)/usr/sbin/
	$(STRIP) $(IDIR_SETBAUD)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_SETBAUD) $(PACKAGE_DIR)
