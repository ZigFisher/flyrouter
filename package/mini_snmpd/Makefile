# $Id: Makefile 1269 2005-06-17 06:29:08Z nico $

include $(TOPDIR)/rules.mk

PKG_NAME:=mini-snmpd
PKG_VERSION:=1.0
PKG_RELEASE:=3
PKG_MD5SUM:=13f2202ff01ff6b6463989f34f453063

PKG_SOURCE_URL:=http://members.aon.at/linuxfreak/linux/
PKG_SOURCE:=mini_snmpd.tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/mini_snmpd

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,MINI_SNMPD,mini-snmpd,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))


$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC=$(TARGET_CC) \
		OFLAGS="$(TARGET_CFLAGS) -D__DEMO__ -DSYSLOG" \
		mini_snmpd
	touch $@

$(IPKG_MINI_SNMPD):
	install -d -m0755 $(IDIR_MINI_SNMPD)/usr/sbin
	install -d -m0755 $(IDIR_MINI_SNMPD)/etc/init.d/
	install -m0755 $(PKG_BUILD_DIR)/mini_snmpd $(IDIR_MINI_SNMPD)/usr/sbin/
	install -m0755 ./files/S90snmpd $(IDIR_MINI_SNMPD)/etc/init.d/
	$(RSTRIP) $(IDIR_MINI_SNMPD)
	$(IPKG_BUILD) $(IDIR_MINI_SNMPD) $(PACKAGE_DIR)
