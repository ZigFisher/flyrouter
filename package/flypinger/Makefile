# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=flypinger
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,FLYPINGER,flypinger,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	touch $@

$(IPKG_FLYPINGER):
	( tar cf - -C files --exclude .svn . | tar xvf - -C $(IDIR_FLYPINGER) ); 
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_FLYPINGER)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_FLYPINGER) $(PACKAGE_DIR)

