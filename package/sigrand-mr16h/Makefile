# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=sigrand-mr16h
PKG_RELEASE:=1

CFLAGS:=-DAUTOSTART_OFF -DSG16_NORBURST -DSG16_NOWBURST
ifeq ($(BR2_PACKAGE_SIGRAND_MR16H_LINKMAP),y)
    CFLAGS +=-DSG16_USERMODE_EVENTS
endif


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_MR16H,sigrand-mr16h,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R mr16h-drv $(PKG_BUILD_DIR)
	cp -R dsl2eth_map $(PKG_BUILD_DIR)	
	touch $@

$(PKG_BUILD_DIR)/.configured:
	rm --preserve-root -f $(PKG_BUILD_DIR)/mr16h-drv/Makefile
	sed <$(PKG_BUILD_DIR)/mr16h-drv/Makefile.tmpl \
	> $(PKG_BUILD_DIR)/mr16h-drv/Makefile \
	-e "s%@KERNSRC@%$(LINUX_DIR)%g" \
	-e "s%@CROSS_PREFIX@%$(KERNEL_CROSS)%g" \
	-e "s%@CFLAGS@%$(CFLAGS)%g" \
	-e "s%@KERNVER@%2.6.16%g" \
	-e "s%@INST_PATH@%$(IDIR_SIGRAND_MR16H)%g"	
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR)/mr16h-drv && make
	touch $@

$(IPKG_SIGRAND_MR16H):
	install -d -m0755 $(IDIR_SIGRAND_MR16H)/sbin
	install -d -m0755 $(IDIR_SIGRAND_MR16H)/lib/modules/2.6.16
	install -d -m0755 $(IDIR_SIGRAND_MR16H)/lib/firmware
	cd $(PKG_BUILD_DIR)/mr16h-drv && make install
ifeq ($(BR2_PACKAGE_SIGRAND_MR16H_LINKMAP),y)
	install -m0700  dsl2eth_map/__sg16_link_hndl $(IDIR_SIGRAND_MR16H)/sbin
	install -d -m755 $(IDIR_SIGRAND_MR16H)/etc/sigrand
	install -m0600  dsl2eth_map/dsl2eth.map $(IDIR_SIGRAND_MR16H)/etc/sigrand
	install -m0600  dsl2eth_map/dsl2eth.map.example $(IDIR_SIGRAND_MR16H)/etc/sigrand
endif
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_MR16H)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_MR16H) $(PACKAGE_DIR)	
