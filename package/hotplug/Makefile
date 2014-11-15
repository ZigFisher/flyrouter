include $(TOPDIR)/rules.mk
PKG_NAME:=hotplug
PKG_VERSION:=2004_09_23
PKG_RELEASE:=1
PKG_MD5SUM:=1542af9cbe1053cfde9421007f702cf2
PKG_SOURCE_URL:= http://freshmeat.net/redir/hotplug/54412/url_tgz/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

INSTALL = install -c
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA  = $(INSTALL) -m 644
INSTALL_SCRIPT = $(INSTALL_PROGRAM)

# System locations
prefix = $(IDIR_HOTPLUG)
exec_prefix = ${prefix}
etcdir = ${prefix}/etc
sbindir = ${exec_prefix}/sbin
mandir = ${prefix}/usr/share/man
srcdir = .


include $(TOPDIR)/package/rules.mk
$(eval $(call PKG_template,HOTPLUG,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(PKG_BUILD_DIR)/.configured: $(PKG_BUILD_DIR)/.prepared
	#Since there is no configure script, we can directly go to the building step
	touch $@
$(PKG_BUILD_DIR)/.built:
	touch $@
$(IPKG_HOTPLUG):
	$(INSTALL) -D $(PKG_BUILD_DIR)/sbin/hotplug $(sbindir)/hotplug
	$(INSTALL) -d $(etcdir)/hotplug/{usb,pci}
	$(INSTALL) -D $(PKG_BUILD_DIR)/etc/hotplug.d/default/default.hotplug $(etcdir)/hotplug.d/default/default.hotplug
	for F in $(PKG_BUILD_DIR)/etc/hotplug/{*.{agent,rc},hotplug.functions} ; do \
		$(INSTALL) $$F $(etcdir)/hotplug ; \
		done
	for F in $(PKG_BUILD_DIR)/etc/hotplug/{blacklist,usb.{user,hand,dist}map} ; do \
		$(INSTALL_DATA) $$F $(etcdir)/hotplug ; \
		done
	$(INSTALL) -D $(PKG_BUILD_DIR)/etc/rc.d/init.d/hotplug $(etcdir)/init.d/hotplug
	$(INSTALL_DATA) -D $(PKG_BUILD_DIR)/hotplug.8  $(mandir)/man8/hotplug.8
	$(INSTALL) -d $(PKG_BUILD_DIR)/$(prefix)/var/log/hotplug $(prefix)/var/run
	$(INSTALL) -d -m 700 $(prefix)/var/run/usb
	mkdir -p $(prefix)/var/lock/subsys
	ln -sf hotplug $(etcdir)/init.d/S11hotplug
	$(IPKG_BUILD) $(IDIR_HOTPLUG) $(PACKAGE_DIR)


