# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=sigrand-doc
PKG_RELEASE:=1
PKG_SOURCE_URL:=http://sigrand.ru/temp/
PKG_SOURCE:=sigrand-doc.tar.bz2
PKG_CAT:=bzcat
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_DOC,sigrand-doc,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	touch $@

$(IPKG_SIGRAND_DOC):
	install -d -m0755 $(IDIR_SIGRAND_DOC)/www/help
	install -d -m0755 $(IDIR_SIGRAND_DOC)/www/help/img
	install -m0755 $(PKG_BUILD_DIR)/*.html $(IDIR_SIGRAND_DOC)/www/help
	install -m0755 $(PKG_BUILD_DIR)/img/* $(IDIR_SIGRAND_DOC)/www/help/img
	#$(RSTRIP) $(IDIR_SIGRAND_DOC)/www/help/*
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_DOC)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_DOC) $(PACKAGE_DIR)
