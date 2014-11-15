# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=module-init-tools
PKG_VERSION:=3.2
PKG_RELEASE:=1
PKG_MD5SUM:=87e8f8681dd53cbc41375a91c595c907

PKG_SOURCE_URL:=http://www.kernel.org/pub/linux/utils/kernel/module-init-tools/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,MODULEINITTOOLS,module-init-tools,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -DMAX_UPLOAD_KB=8192" \
		./configure \
		  --target=$(GNU_TARGET_NAME) \
		  --host=$(GNU_TARGET_NAME) \
		  --build=$(GNU_HOST_NAME) \
		  --program-prefix="" \
		  --program-suffix="" \
		  --prefix=/ \
		  --exec-prefix=/ \
		  --bindir=/bin \
		  --datadir=/usr/share \
		  --includedir=/usr/include \
		  --infodir=/usr/share/info \
		  --libdir=/usr/lib \
		  --libexecdir=/usr/lib \
		  --localstatedir=/var \
		  --mandir=/usr/share/man \
		  --sbindir=/sbin \
		  --sysconfdir=/etc \
		  $(DISABLE_LARGEFILE) \
		  $(DISABLE_NLS) \
	)
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
	touch $@

$(IPKG_MODULEINITTOOLS):
	install -m0755 -d $(IDIR_MODULEINITTOOLS)/sbin
	install -m0755 -d $(IDIR_MODULEINITTOOLS)/bin
	$(CP) $(PKG_INSTALL_DIR)/bin/lsmod $(IDIR_MODULEINITTOOLS)/bin
	$(CP) $(PKG_INSTALL_DIR)/sbin/insmod $(IDIR_MODULEINITTOOLS)/sbin
	$(CP) $(PKG_INSTALL_DIR)/sbin/rmmod $(IDIR_MODULEINITTOOLS)/sbin
	$(CP) $(PKG_INSTALL_DIR)/sbin/depmod $(IDIR_MODULEINITTOOLS)/sbin
	$(CP) $(PKG_INSTALL_DIR)/sbin/modprobe $(IDIR_MODULEINITTOOLS)/sbin
	$(CP) $(PKG_INSTALL_DIR)/sbin/generate-modprobe.conf $(IDIR_MODULEINITTOOLS)/sbin
	$(RSTRIP) $(IDIR_MODULEINITTOOLS)
	$(IPKG_BUILD) $(IDIR_MODULEINITTOOLS) $(PACKAGE_DIR)

