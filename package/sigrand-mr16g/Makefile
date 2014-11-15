# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=sigrand-mr16g
PKG_RELEASE:=1


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_MR16G,sigrand-mr16g,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R mr16g-drv $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	rm --preserve-root -f $(PKG_BUILD_DIR)/mr16g-drv/Makefile
	sed <$(PKG_BUILD_DIR)/mr16g-drv/Makefile.tmpl \
	> $(PKG_BUILD_DIR)/mr16g-drv/Makefile \
	-e "s%@KERNSRC@%$(LINUX_DIR)%g" \
	-e "s%@CROSS_PREFIX@%$(KERNEL_CROSS)%g" \
	-e "s%@CFLAGS@%$(CFLAGS)%g" \
	-e "s%@KERNVER@%2.6.16%g" \
	-e "s%@INST_PATH@%$(IDIR_SIGRAND_MR16G)%g"	
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR)/mr16g-drv && make
	touch $@

$(IPKG_SIGRAND_MR16G):
	install -d -m0755 $(IDIR_SIGRAND_MR16G)/sbin
	install -d -m0755 $(IDIR_SIGRAND_MR16G)/lib/modules/2.6.16
	install -d -m0755 $(IDIR_SIGRAND_MR16G)/lib/firmware
	cd $(PKG_BUILD_DIR)/mr16g-drv && make install
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_MR16G)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_MR16G) $(PACKAGE_DIR)	
