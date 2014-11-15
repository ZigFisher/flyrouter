# $Id: Makefile 3198 2006-02-09 03:21:01Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME := cpu
PKG_RELEASE := 1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,CPU,$(PKG_NAME),$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $@
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/cpu cpu.c
	touch $@

$(IPKG_CPU): 
	install -d -m0755 $(IDIR_CPU)/sbin
	install -m0755 $(PKG_BUILD_DIR)/cpu $(IDIR_CPU)/sbin
	$(RSTRIP) $(IDIR_CPU)/sbin/*
	$(IPKG_BUILD) $(IDIR_CPU) $(PACKAGE_DIR)
