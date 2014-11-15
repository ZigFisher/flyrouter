# $Id: Makefile 3638 2006-04-13 18:28:55Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=openswan
PKG_VERSION:=2.4.6
PKG_RELEASE:=1
PKG_MD5SUM:=b34d71ca49dedad017879b0e912d40dd
#PKG_MD5SUM:=a9a8e88313faceebfc5ceb1a9da9a3c4

PKG_SOURCE_URL:=http://www.openswan.org/download
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_CAT:=zcat

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,OPENSWAN,openswan,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

FLAGS := $(TARGET_CFLAGS) -I$(PKG_BUILD_DIR)/linux/include -L$(STAGING_DIR)/usr/lib -I$(STAGING_DIR)/usr/include

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		KERNELSRC="$(LINUX_DIR)" \
		ARCH="mips" \
		USERCOMPILE="$(FLAGS)" \
		EXTRA_INCLUDE="-I$(STAGING_DIR)/usr/include" \
		EXTRA_LIBS="-L$(STAGING_DIR)/usr/lib" \
		IPSECDIR="/usr/lib/ipsec" \
		INC_USRLOCAL="/usr" \
		programs
	touch $@

$(IPKG_OPENSWAN):
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		DESTDIR="$(IDIR_OPENSWAN)" \
		KERNELSRC="$(LINUX_DIR)" \
		ARCH="mips" \
		USERCOMPILE="$(FLAGS)" \
		IPSECDIR="/usr/lib/ipsec" \
		INC_USRLOCAL="/usr" \
		install
	-$(STRIP) $(IDIR_OPENSWAN)/usr/lib/ipsec/*
	-$(STRIP) $(IDIR_OPENSWAN)/usr/libexec/ipsec/*
	rm -rf $(IDIR_OPENSWAN)/usr/share
	rm -rf $(IDIR_OPENSWAN)/usr/man
	rm -rf $(IDIR_OPENSWAN)/var
	mkdir -p $(IDIR_OPENSWAN)/etc/init.d
	ln -sf ipsec $(IDIR_OPENSWAN)/etc/init.d/S60ipsec
	rm -rf $(IDIR_OPENSWAN)/etc/rc*.d
	install -m0755 ./files/ipsec $(IDIR_OPENSWAN)/etc/init.d/
	install -m0600 ./files/ipsec.conf $(IDIR_OPENSWAN)/etc/
	install -m0600 ./files/ipsec.secrets $(IDIR_OPENSWAN)/etc/
	find $(PKG_BUILD_DIR) -name \*.old | xargs rm -rf
	mkdir -p $(PACKAGE_DIR)
	$(IPKG_BUILD) $(IDIR_OPENSWAN) $(PACKAGE_DIR)
