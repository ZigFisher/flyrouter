# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=flyscript
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,FLYSCRIPT,flyscript,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	touch $@

$(IPKG_FLYSCRIPT):
	( tar cf - -C files --exclude .svn . | tar xvf - -C $(IDIR_FLYSCRIPT) ); 
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_FLYSCRIPT)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_FLYSCRIPT) $(PACKAGE_DIR)

