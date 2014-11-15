# $Id: Makefile 3139 2006-02-05 17:30:49Z florian $

include $(TOPDIR)/rules.mk

PKG_NAME:=utcpdump
PKG_VERSION:=0.1
PKG_RELEASE:=1
#PKG_MD5SUM:=

#PKG_SOURCE_URL:=@SF/utcpdump
PKG_SOURCE_URL:=http://ftp.chg.ru/pub/Linux/trinux/boot/

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_CAT:=zcat
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,UTCPDUMP,utcpdump,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		BUILD_CC=$(TARGET_CC) HOSTCC=$(HOSTCC) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib" \
		ac_cv_linux_vers=2 \
		td_cv_buggygetaddrinfo="no" \
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
		--enable-shared \
		--disable-static \
		--enable-ipv6 \
		--without-crypto \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CCOPT="$(TARGET_CFLAGS)" INCLS="-I. -I$(STAGING_DIR)/usr/include" \
		DESTDIR="$(PKG_INSTALL_DIR)"
	touch $@

$(IPKG_UTCPDUMP):
	install -m0755 -d $(IDIR_UTCPDUMP)/usr/sbin
	install -m0755 $(PKG_BUILD_DIR)/utcpdump $(IDIR_UTCPDUMP)/usr/sbin/
	$(STRIP) $(IDIR_UTCPDUMP)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_UTCPDUMP) $(PACKAGE_DIR)

