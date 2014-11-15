# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=sigrand-adm5120sw
PKG_RELEASE:=1

CFLAGS:=
ifeq ($(BR2_PACKAGE_SIGRAND_ADM5120SW_LINKCTRL),y)
    CFLAGS+=-DADM5120_FORCE_LINK_CONTROL
endif


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_ADM5120SW,sigrand-adm5120sw,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R adm5120sw-drv $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	rm --preserve-root -f $(PKG_BUILD_DIR)/adm5120sw-drv/Makefile
	sed <$(PKG_BUILD_DIR)/adm5120sw-drv/Makefile.tmpl \
	> $(PKG_BUILD_DIR)/adm5120sw-drv/Makefile \
	-e "s%@KERNSRC@%$(LINUX_DIR)%g" \
	-e "s%@CROSS_PREFIX@%$(KERNEL_CROSS)%g" \
	-e "s%@CFLAGS@%$(CFLAGS)%g" \
	-e "s%@KERNVER@%2.6.16%g" \
	-e "s%@INST_PATH@%$(IDIR_SIGRAND_ADM5120SW)%g"	
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR)/adm5120sw-drv && make
	touch $@

$(IPKG_SIGRAND_ADM5120SW):
	install -d -m0755 $(IDIR_SIGRAND_ADM5120SW)/lib/modules/2.6.16
	install -d -m0755 $(IDIR_SIGRAND_ADM5120SW)/etc/init.d
	cd $(PKG_BUILD_DIR)/adm5120sw-drv && make install
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_ADM5120SW)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_ADM5120SW) $(PACKAGE_DIR)	
