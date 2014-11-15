# $Id:  $

include $(TOPDIR)/rules.mk

PKG_NAME:=sigrand-mr17h
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
CFLAGS:=
DEBUG := no
ifeq ($(BR2_PACKAGE_SIGRAND_MR17H_DEBUG),y)
    CFLAGS +=-DSYSFS_DEBUG
    DEBUG := yes
endif

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,SIGRAND_MR17H,sigrand-mr17h,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.prepared:
	@echo "PKG_DIR="$(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp -R mr17h-drv $(PKG_BUILD_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	rm --preserve-root -f $(PKG_BUILD_DIR)/mr17h-drv/Makefile
	@echo "CFLAGS=$(CFLAGS)"
	sed <$(PKG_BUILD_DIR)/mr17h-drv/Makefile.tmpl \
	> $(PKG_BUILD_DIR)/mr17h-drv/Makefile \
	-e "s%@KERNSRC@%$(LINUX_DIR)%g" \
	-e "s%@CROSS_PREFIX@%$(KERNEL_CROSS)%g" \
	-e "s%@CFLAGS@%$(CFLAGS)%g" \
	-e "s%@KERNVER@%2.6.16%g" \
	-e "s%@INST_PATH@%$(IDIR_SIGRAND_MR17H)%g" \
	-e "s%@DEBUG@%$(DEBUG)%g" 
	@echo "KERNVER="$(LINUX_VERSION)" INST_PATH="$(IDIR_SIGRAND_MR17H)
	touch $@

$(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	cd $(PKG_BUILD_DIR)/mr17h-drv && make
	touch $@

$(IPKG_SIGRAND_MR17H):
	install -d -m0755 $(IDIR_SIGRAND_MR17H)/sbin
	install -d -m0755 $(IDIR_SIGRAND_MR17H)/lib/modules/2.6.16
	install -d -m0755 $(IDIR_SIGRAND_MR17H)/lib/firmware
	cd $(PKG_BUILD_DIR)/mr17h-drv && make install
	echo "Depends: $(PKG_DEPEND)" >> $(IDIR_SIGRAND_MR17H)/CONTROL/control
	$(IPKG_BUILD) $(IDIR_SIGRAND_MR17H) $(PACKAGE_DIR)	
	@echo "INSTALLLLLLLLLLL"

local_build: $(PKG_BUILD_DIR)/.prepared
	cd $(PKG_BUILD_DIR)/mr17h-drv && make KERN_PATH=$(LINUX_DIR) CROSS_PREFIX=$(STAGING_DIR)/bin/mipsel-linux-
