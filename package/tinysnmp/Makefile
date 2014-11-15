# $Id: Makefile 3449 2008-03-27 12:26:02Z zig $

include $(TOPDIR)/rules.mk

PKG_NAME:=tinysnmp
PKG_VERSION:=0.8.4
PKG_RELEASE:=1
PKG_MD5SUM:=892ddb02b74705e299ff0c69315b8943

PKG_SOURCE_URL:=http://oasis.frogfoot.net/code/tinysnmp/download/$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install
PKG_CAT:=zcat

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,TINYSNMP-TOOLS,tinysnmp-tools,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,TINYSNMP-AGENT,tinysnmp-agent,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,TINYSNMP-MODULE-RESOURCES,tinysnmp-module-resources,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,TINYSNMP-MODULE-INTERFACES,tinysnmp-module-interfaces,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	    CC="$(TARGET_CC)" \
	    CFLAGS="$(TARGET_CFLAGS)" \
	    LDFLAGS="-L$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib -L$(PKG_BUILD_DIR)/manager" \
	    CPPFLAGS="-I$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include -I$(PKG_BUILD_DIR)/include" \
	    CROSS="$(TARGET_CROSS)" LD=$(TARGET_CROSS)ld ARCH=$(ARCH) \
	    DESTDIR="$(PKG_INSTALL_DIR)" \
	    prefix="$(PKG_INSTALL_DIR)/usr" \
	    all
	touch $@

$(IPKG_TINYSNMP-TOOLS):
	mkdir -p $(IDIR_TINYSNMP-TOOLS)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/tools/snmp/tinysnmpget $(IDIR_TINYSNMP-TOOLS)/usr/bin/
	ln -s -f tinysnmpget $(IDIR_TINYSNMP-TOOLS)/usr/bin/tinysnmpgetnext
	ln -s -f tinysnmpget $(IDIR_TINYSNMP-TOOLS)/usr/bin/tinysnmpwalk
	$(STRIP) $(IDIR_TINYSNMP-TOOLS)/usr/bin/*
	$(IPKG_BUILD) $(IDIR_TINYSNMP-TOOLS) $(PACKAGE_DIR)

$(IPKG_TINYSNMP-AGENT):
	install -d -m0755 $(IDIR_TINYSNMP-AGENT)/etc
	install -m0644 $(PKG_BUILD_DIR)/agent/tinysnmp.conf $(IDIR_TINYSNMP-AGENT)/etc/tinysnmp.conf
	install -d -m0755 $(IDIR_TINYSNMP-AGENT)/usr/share/tinysnmp/mibs
	install -m0644 $(PKG_BUILD_DIR)/mibs/SNMPv2-SMI.txt $(IDIR_TINYSNMP-AGENT)/usr/share/tinysnmp/mibs
	install -m0644 $(PKG_BUILD_DIR)/mibs/SNMPv2-TC.txt $(IDIR_TINYSNMP-AGENT)/usr/share/tinysnmp/mibs
	install -m0644 $(PKG_BUILD_DIR)/mibs/SNMPv2-CONF.txt $(IDIR_TINYSNMP-AGENT)/usr/share/tinysnmp/mibs
	install -m0644 $(PKG_BUILD_DIR)/mibs/SNMPv2-MIB.txt $(IDIR_TINYSNMP-AGENT)/usr/share/tinysnmp/mibs
	mkdir -p $(IDIR_TINYSNMP-AGENT)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/agent/tinysnmpd $(IDIR_TINYSNMP-AGENT)/usr/sbin/
	$(STRIP) $(IDIR_TINYSNMP-AGENT)/usr/sbin/*
	$(IPKG_BUILD) $(IDIR_TINYSNMP-AGENT) $(PACKAGE_DIR)

$(IPKG_TINYSNMP-MODULE-RESOURCES):
	install -d -m0755 $(IDIR_TINYSNMP-MODULE-RESOURCES)/usr/lib/tinysnmp
	install -m0644 $(PKG_BUILD_DIR)/modules/resources/resources.so $(IDIR_TINYSNMP-MODULE-RESOURCES)/usr/lib/tinysnmp
	install -d -m0755 $(IDIR_TINYSNMP-MODULE-RESOURCES)/usr/share/tinysnmp/mibs
	install -m0644 $(PKG_BUILD_DIR)/mibs/FROGFOOT-RESOURCES-MIB.txt $(IDIR_TINYSNMP-MODULE-RESOURCES)/usr/share/tinysnmp/mibs
	$(STRIP) $(IDIR_TINYSNMP-MODULE-RESOURCES)/usr/lib/tinysnmp/*
	$(IPKG_BUILD) $(IDIR_TINYSNMP-MODULE-RESOURCES) $(PACKAGE_DIR)

$(IPKG_TINYSNMP-MODULE-INTERFACES):
	install -d -m0755 $(IDIR_TINYSNMP-MODULE-INTERFACES)/usr/lib/tinysnmp
	install -m0644 $(PKG_BUILD_DIR)/modules/interfaces/interfaces.so $(IDIR_TINYSNMP-MODULE-INTERFACES)/usr/lib/tinysnmp
	install -d -m0755 $(IDIR_TINYSNMP-MODULE-INTERFACES)/usr/share/tinysnmp/mibs
	install -m0644 $(PKG_BUILD_DIR)/mibs/IF-MIB.txt $(IDIR_TINYSNMP-MODULE-INTERFACES)/usr/share/tinysnmp/mibs
	install -m0644 $(PKG_BUILD_DIR)/mibs/IANAifType-MIB.txt $(IDIR_TINYSNMP-MODULE-INTERFACES)/usr/share/tinysnmp/mibs
	$(STRIP) $(IDIR_TINYSNMP-MODULE-INTERFACES)/usr/lib/tinysnmp/*
	$(IPKG_BUILD) $(IDIR_TINYSNMP-MODULE-INTERFACES) $(PACKAGE_DIR)
