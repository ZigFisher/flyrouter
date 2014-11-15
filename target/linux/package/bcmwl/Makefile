# $Id: Makefile 3734 2006-05-06 23:40:19Z nbd $

include $(TOPDIR)/rules.mk
include ../../rules.mk

PKG_NAME:=broadcom-wl
PKG_VERSION:=4.150.10.5.3
PKG_RELEASE:=3.2
PKG_MD5SUM:=cc615fd49719eea8bce5b2a8813783f3

PKG_SOURCE_URL:=http://downloads.openwrt.org/sources/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_CAT:=bzcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

WL_WEXT=1

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,KMOD_BCMWL,kmod-bcmwl,$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE),$(ARCH),kernel ($(LINUX_VERSION)-$(BOARD)-$(LINUX_RELEASE))))

$(PKG_BUILD_DIR)/.configured:
	$(CP) src/* $(PKG_BUILD_DIR)/
	touch $@

BCMWL_MAKEOPTS= -C $(PKG_BUILD_DIR) \
		PATH="$(TARGET_PATH)" \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		TARGET="$(HAL_TARGET)" \
		TOOLPREFIX="$(KERNEL_CROSS)" \
		TOOLPATH="$(KERNEL_CROSS)" \
		KERNELPATH="$(LINUX_DIR)" \
		DOMULTI=1 \
		SUBDIRS="$(PKG_BUILD_DIR)/kmod"

$(PKG_BUILD_DIR)/.built:

	# Compile the kernel part
	$(MAKE) -C $(LINUX_DIR)/ \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		SUBDIRS="$(PKG_BUILD_DIR)/driver" \
		modules
	$(MAKE) -C $(LINUX_DIR)/ \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		SUBDIRS="$(PKG_BUILD_DIR)/driver" \
		MOD_NAME="_mimo" \
		modules
	$(MAKE) -C $(LINUX_DIR)/ \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		SUBDIRS="$(PKG_BUILD_DIR)/wlcompat" \
		$(if $(WL_WEXT),WL_WEXT=1) \
		modules
	$(MAKE) -C $(LINUX_DIR)/ \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		SUBDIRS="$(PKG_BUILD_DIR)/wlcompat" \
		DEBUG=1 $(if $(WL_WEXT),WL_WEXT=1) \
		modules
	
	# NVRAM stub
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-c -o $(PKG_BUILD_DIR)/nvram/nvram_stub.o \
		$(PKG_BUILD_DIR)/nvram/nvram_stub.c
	
	# Compile wlc
	$(MAKE) -C $(PKG_BUILD_DIR)/wlc \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		all

	# Compile libshared
	$(MAKE) -C $(PKG_BUILD_DIR)/router/shared \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I. -I$(PKG_BUILD_DIR)/include -Dlinux=1" \
		all
	
	$(TARGET_CC) -o $(PKG_BUILD_DIR)/nas \
		$(PKG_BUILD_DIR)/nas_exe.o \
		$(PKG_BUILD_DIR)/nvram/nvram_stub.o \
		-L$(STAGING_DIR)/usr/lib $(PKG_BUILD_DIR)/router/shared/libshared.a

	$(TARGET_CC) -o $(PKG_BUILD_DIR)/wl $(PKG_BUILD_DIR)/wl_exe.o
	
	touch $@


$(IPKG_KMOD_BCMWL):
	#install -m0755 -d $(IDIR_KMOD_BCMWL)/usr/lib/
	#install -m0755 $(PKG_BUILD_DIR)/router/shared/libshared.a $(IDIR_KMOD_BCMWL)/usr/lib/
	#
	install -m0755 -d $(IDIR_KMOD_BCMWL)/usr/sbin/
	install -m0755 -d $(IDIR_KMOD_BCMWL)/lib/modules/$(LINUX_VERSION)
	install -m0755 $(PKG_BUILD_DIR)/nas $(IDIR_KMOD_BCMWL)/usr/sbin/
	install -m0755 $(PKG_BUILD_DIR)/wl $(IDIR_KMOD_BCMWL)/usr/sbin/
	#install -m0755 $(PKG_BUILD_DIR)/wlc/wlc $(IDIR_KMOD_BCMWL)/usr/sbin/
	$(STRIP) $(IDIR_KMOD_BCMWL)/usr/sbin/*
	ln -sf nas $(IDIR_KMOD_BCMWL)/usr/sbin/nas4not
	ln -sf nas $(IDIR_KMOD_BCMWL)/usr/sbin/nas4wds
	$(CP) $(PKG_BUILD_DIR)/driver/wl.o $(IDIR_KMOD_BCMWL)/lib/modules/$(LINUX_VERSION)
	#$(CP) $(PKG_BUILD_DIR)/wlcompat/wlcompat.o $(IDIR_KMOD_BCMWL)/lib/modules/$(LINUX_VERSION)
	install -m0755 -d $(IDIR_KMOD_BCMWL)/etc/
	$(CP) -a files/etc/modules.d $(IDIR_KMOD_BCMWL)/etc/
	#$(CP) files/* $(IDIR_KMOD_BCMWL)/
	$(IPKG_BUILD) $(IDIR_KMOD_BCMWL) $(PACKAGE_DIR)
