# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=webadmin
PKG_RELEASE:=2

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,WEBADMIN,webadmin,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	touch $@

$(IPKG_WEBADMIN):
	( tar cf - -C files --exclude .svn . | tar xvf - -C $(IDIR_WEBADMIN)	 ); 
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_WEBADMIN)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_WEBADMIN) $(PACKAGE_DIR)

