# $Id: Makefile 3112 2007-10-25 23:53:19Z zig $

include $(TOPDIR)/rules.mk

PKG_NAME:=gmov-snmp
PKG_VERSION:=0.1z
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://gmov-snmp.sourceforge.net/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,GMOV-SNMP,gmov-snmp,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	  $(TARGET_CONFIGURE_OPTS) \
	  DESTDIR="$(PKG_INSTALL_DIR)" \
	  all
	touch $@

$(IPKG_GMOV-SNMP):
	install -d -m0755 $(IDIR_GMOV-SNMP)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/snmpd $(IDIR_GMOV-SNMP)/usr/sbin/
	mkdir -p $(IDIR_GMOV-SNMP)/etc/snmpd
	$(CP) $(PKG_BUILD_DIR)/etc/* $(IDIR_GMOV-SNMP)/etc/snmpd/
	$(RSTRIP) $(IDIR_GMOV-SNMP)
	$(IPKG_BUILD) $(IDIR_GMOV-SNMP) $(PACKAGE_DIR)

