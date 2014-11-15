# $Id: Makefile 3622 2006-04-12 00:34:26Z nico $

include $(TOPDIR)/rules.mk

PKG_NAME:=conversd
PKG_VERSION:=1.62a
PKG_RELEASE:=1
PKG_MD5SUM:=

PKG_SOURCE_URL:=http://x-berg.in-berlin.de/cgi-bin/viewcvs.cgi/ampr/
PKG_SOURCE:=$(PKG_NAME)-saupp.tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-saupp
PKG_CAT:=zcat
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,CONVERSD,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include" \
	-DWANT_LOG -DFORKPTY -DOWNER="daemon" -DGID="daemon" -DCONF_DIR="/etc/conversd" -DDATA_DIR="/var/cache/conversd" -DCONVERSHOST="localhost"
	touch $@							

$(IPKG_CONVERSD):
	mkdir -p $(IDIR_CONVERSD)
	$(CP) $(PKG_INSTALL_DIR)/* $(IDIR_CONVERSD)/
	$(RSTRIP) $(IDIR_CONVERSD)/
	$(IPKG_BUILD) $(IDIR_CONVERSD) $(PACKAGE_DIR)
