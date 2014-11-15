# $Id: Makefile 3624 2006-04-12 07:53:11Z kaloz $

include $(TOPDIR)/rules.mk

PKG_NAME:=wireless-tools
PKG_VERSION:=29
PKG_RELEASE:=4
#PKG_MD5SUM:=599c94497f9c9073c7b052d3dcb7cd16
PKG_MD5SUM:=e06c222e186f7cc013fd272d023710cb

PKG_SOURCE_URL:=http://www.hpl.hp.com/personal/Jean_Tourrilhes/Linux
PKG_SOURCE:=wireless_tools.$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/wireless_tools.$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,WIRELESS_TOOLS,wireless-tools,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS) -I." \
		libiw.so.$(PKG_VERSION) iwmulticall
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		PREFIX="$(PKG_INSTALL_DIR)" \
		INSTALL_DIR="$(PKG_INSTALL_DIR)/usr/sbin" \
		INSTALL_LIB="$(PKG_INSTALL_DIR)/usr/lib" \
		install-iwmulticall
	touch $@

$(IPKG_WIRELESS_TOOLS):
	mkdir -p $(IDIR_WIRELESS_TOOLS)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libiw.so.$(PKG_VERSION) $(IDIR_WIRELESS_TOOLS)/usr/lib/
	mkdir -p $(IDIR_WIRELESS_TOOLS)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/iwconfig $(IDIR_WIRELESS_TOOLS)/usr/sbin/
	ln -sf iwconfig $(IDIR_WIRELESS_TOOLS)/usr/sbin/iwgetid
	ln -sf iwconfig $(IDIR_WIRELESS_TOOLS)/usr/sbin/iwlist
	ln -sf iwconfig $(IDIR_WIRELESS_TOOLS)/usr/sbin/iwpriv
	ln -sf iwconfig $(IDIR_WIRELESS_TOOLS)/usr/sbin/iwspy				
	$(RSTRIP) $(IDIR_WIRELESS_TOOLS)
	$(IPKG_BUILD) $(IDIR_WIRELESS_TOOLS) $(PACKAGE_DIR)

install-dev: $(PKG_BUILD_DIR)/.built
	mkdir -p $(STAGING_DIR)/usr/{lib,include}
	$(CP) $(PKG_BUILD_DIR)/libiw.so* $(STAGING_DIR)/usr/lib/
	ln -sf libiw.so.$(PKG_VERSION) $(STAGING_DIR)/usr/lib/libiw.so
	$(CP) $(PKG_BUILD_DIR)/{iwlib,wireless}.h $(STAGING_DIR)/usr/include/
	
uninstall-dev:
	rm -f $(STAGING_DIR)/usr/lib/libiw.*
	rm -f $(STAGING_DIR)/usr/include/{iwlib,wireless}.h

compile-targets: install-dev
clean-targets: uninstall-dev
