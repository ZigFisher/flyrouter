# $Id: Makefile vlad 20061211 $

include $(TOPDIR)/rules.mk

PKG_NAME := ifenslave
PKG_RELEASE := 4

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,IFENSLAVE,$(PKG_NAME),$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/ifenslave ifenslave.c
	touch $@

$(IPKG_IFENSLAVE): 
	install -d -m0755 $(IDIR_IFENSLAVE)/sbin
	install -m0755 $(PKG_BUILD_DIR)/ifenslave $(IDIR_IFENSLAVE)/sbin
	$(RSTRIP) $(IDIR_IFENSLAVE)/sbin/*
	$(IPKG_BUILD) $(IDIR_IFENSLAVE) $(PACKAGE_DIR)
