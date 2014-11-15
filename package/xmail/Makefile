# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=xmail
PKG_VERSION:=1.24
PKG_RELEASE:=1
PKG_MD5SUM:=106730bdbaa4fda9e90a7c22df024114

PKG_SOURCE_URL:=http://www.xmailserver.org/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,XMAIL,xmail,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	$(CP) ./files/SysMachine.h $(PKG_BUILD_DIR)/
	touch $@

$(PKG_BUILD_DIR)/.built:
	CFLAGS="$(TARGET_CFLAGS)" \
	$(MAKE) -C $(PKG_BUILD_DIR) -f Makefile.lnx \
		$(TARGET_CONFIGURE_OPTS) \
		CPPFLAGS="$(TARGET_CPPFLAGS) -I$(STAGING_DIR)/usr/include/uClibc++ -fno-builtin -fno-rtti -nostdinc++" \
		LDFLAGS="$(TARGET_LDFLAGS) -nodefaultlibs -ldl -lm -lpthread" \
		all
	touch $@

$(IPKG_XMAIL):
	mkdir -p $(IDIR_XMAIL)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/bin/* $(IDIR_XMAIL)/usr/bin/
	mkdir -p $(IDIR_XMAIL)/var/MailRoot/
	$(CP) $(PKG_BUILD_DIR)/MailRoot $(IDIR_XMAIL)/var/
	$(IPKG_BUILD) $(IDIR_XMAIL) $(PACKAGE_DIR)

