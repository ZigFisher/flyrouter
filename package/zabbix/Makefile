# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=zabbix
#PKG_VERSION:=1.8.2
PKG_VERSION:=1.6.3
PKG_RELEASE:=1
#PKG_MD5SUM:=9b7dc52656f5cbec846a7ba3299f73bd

#PKG_SOURCE_URL:=http://downloads.sourceforge.net/project/zabbix/ZABBIX%20Latest%20Stable/1.8.9
#PKG_SOURCE_URL:=@SF/zabbix
PKG_SOURCE_URL:=http://prdownloads.sourceforge.net/zabbix
PKG_SOURCE:=zabbix-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/zabbix-$(PKG_VERSION)
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,ZABBIX-AGENTD,zabbix-agentd,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,ZABBIX-SENDER,zabbix-sender,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured: 
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include" \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib" \
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
		  $(DISABLE_LARGEFILE) \
		  --enable-agent \
		  --disable-server \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -Wall -D_REENTRANT" \
		DESTDIR=$(PKG_INSTALL_DIR) \
		all install
	touch $@

$(IPKG_ZABBIX-AGENTD):
	install -d -m0755 $(IDIR_ZABBIX-AGENTD)/etc/zabbix
	install -d -m0755 $(IDIR_ZABBIX-AGENTD)/etc/init.d
	install -m0644 ./files/zabbix_agentd.conf $(IDIR_ZABBIX-AGENTD)/etc/zabbix/
	install -m0755 ./files/zabbix_agentd.init $(IDIR_ZABBIX-AGENTD)/etc/init.d/S97zabbix_agentd
	install -d -m0755 $(IDIR_ZABBIX-AGENTD)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/src/zabbix_agent/zabbix_agentd $(IDIR_ZABBIX-AGENTD)/usr/sbin
	$(RSTRIP) $(IDIR_ZABBIX-AGENTD)
	$(IPKG_BUILD) $(IDIR_ZABBIX-AGENTD) $(PACKAGE_DIR)

$(IPKG_ZABBIX-SENDER):
	install -d -m0755 $(IDIR_ZABBIX-SENDER)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/src/zabbix_sender/zabbix_sender $(IDIR_ZABBIX-SENDER)/usr/sbin
	$(RSTRIP) $(IDIR_ZABBIX-SENDER)
	$(IPKG_BUILD) $(IDIR_ZABBIX-SENDER) $(PACKAGE_DIR)
