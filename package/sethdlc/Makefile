# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=sigrand-sethdlc
PKG_RELEASE:=1


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_SETHDLC,sigrand-sethdlc,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/sethdlc sethdlc.c
	touch $@

$(IPKG_SIGRAND_SETHDLC):
	install -d -m0755 $(IDIR_SIGRAND_SETHDLC)/sbin
	install -m0755 $(PKG_BUILD_DIR)/sethdlc $(IDIR_SIGRAND_SETHDLC)/sbin
	$(RSTRIP) $(IDIR_SIGRAND_SETHDLC)/sbin/*
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_SETHDLC)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_SETHDLC) $(PACKAGE_DIR)

