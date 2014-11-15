# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=webface
PKG_RELEASE:=1


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,WEBFACE,webface,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/kdb kdb.c
	-$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/hdb -luClibc++ hdb/hdb.cpp
	$(TARGET_CC) -lcrypt $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/htpasswd htpasswd.c
	touch $@

$(IPKG_WEBFACE):
	( tar cf - -C files --exclude .svn . | tar xvf - -C $(IDIR_WEBFACE)	 ); 
	install -d -m0755 $(IDIR_WEBFACE)/usr/bin
	install -m0755 $(PKG_BUILD_DIR)/htpasswd $(IDIR_WEBFACE)/usr/bin
	install -m0755 $(PKG_BUILD_DIR)/kdb $(IDIR_WEBFACE)/usr/bin
	-install -m0755 $(PKG_BUILD_DIR)/hdb $(IDIR_WEBFACE)/usr/bin
	$(RSTRIP) $(IDIR_WEBFACE)/usr/bin/*
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_WEBFACE)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_WEBFACE) $(PACKAGE_DIR)

