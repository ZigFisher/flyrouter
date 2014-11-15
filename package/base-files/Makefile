# $Id: Makefile 3588 2006-04-05 02:09:22Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=base-files
PKG_RELEASE:=6
PKG_BUILD_DIR:=$(BUILD_DIR)/base-files

BOARD:=$(shell LANG=C grep ^BR2_LINUX_2 $(TOPDIR)/.config|sed -e "s/=y//" \
	-e "s/.*_//"| tr "[:upper:]" "[:lower:]")
KERNEL:=$(shell LANG=C grep ^BR2_LINUX_2 $(TOPDIR)/.config|sed -e "s/=y//" \
	-e "s/BR2_LINUX_//" -e "s/_BRCM//" -e "s/_/./"| tr "[:upper:]" "[:lower:]")

REV:=$(shell cat ../../Revision | head -n 1 | awk '/^Revision:/ { print $2 }' )
ifeq ($(REV),)
REV:=0
endif

include $(TOPDIR)/package/rules.mk

IDIR_BASE:=$(PKG_BUILD_DIR)/base-files
IPKG_BASE:=$(PACKAGE_DIR)/$(PKG_NAME)_$(PKG_RELEASE)_$(ARCH).ipk

UCLIBC_VERSION:=${shell cat $(STAGING_DIR)/uclibc_version}
IDIR_UCLIBC:=$(PKG_BUILD_DIR)/uclibc
IPKG_UCLIBC:=$(PACKAGE_DIR)/uclibc_$(UCLIBC_VERSION)-$(PKG_RELEASE)_$(ARCH).ipk

LIBGCC_VERSION:=${shell cat $(STAGING_DIR)/gcc_version}
IDIR_LIBGCC:=$(PKG_BUILD_DIR)/libgcc
IPKG_LIBGCC:=$(PACKAGE_DIR)/libgcc_$(LIBGCC_VERSION)-$(PKG_RELEASE)_$(ARCH).ipk

PACKAGES:=$(IPKG_BASE) $(IPKG_UCLIBC) 
PACKAGES_INSTALL:=$(IPKG_BASE) $(IPKG_UCLIBC) 

ifeq ($(BR2_PACKAGE_LIBGCC),y)
PACKAGES+=$(IPKG_LIBGCC)
PACKAGES_INSTALL+=$(IPKG_LIBGCC)
endif
ifeq ($(BR2_PACKAGE_LIBGCC),m)
PACKAGES+=$(IPKG_LIBGCC)
endif

$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $(PKG_BUILD_DIR) $(PACKAGE_DIR)
	touch $@

$(PKG_BUILD_DIR)/.configured:
	touch $@

$(PKG_BUILD_DIR)/.built:
	touch $@

$(IPKG_BASE):
	$(SCRIPT_DIR)/make-ipkg-dir.sh $(IDIR_BASE) ipkg/$(PKG_NAME).control $(PKG_RELEASE) $(ARCH)
	$(CP) ./default/* $(IDIR_BASE)
	#
	# Create empty dir's sceleton - need for GIT usage
	#
	mkdir -p $(IDIR_BASE)/bin
	mkdir -p $(IDIR_BASE)/dev
	mkdir -p $(IDIR_BASE)/etc
	mkdir -p $(IDIR_BASE)/etc/adsl
	mkdir -p $(IDIR_BASE)/etc/crontabs
	mkdir -p $(IDIR_BASE)/etc/dropbear
	mkdir -p $(IDIR_BASE)/etc/init.d
	mkdir -p $(IDIR_BASE)/etc/modules.d
	mkdir -p $(IDIR_BASE)/etc/network
	mkdir -p $(IDIR_BASE)/etc/network/if-down.d
	mkdir -p $(IDIR_BASE)/etc/network/if-post-down.d
	mkdir -p $(IDIR_BASE)/etc/network/if-pre-up.d
	mkdir -p $(IDIR_BASE)/etc/network/if-up.d
	mkdir -p $(IDIR_BASE)/etc/rc.d
	mkdir -p $(IDIR_BASE)/etc/scripts
	mkdir -p $(IDIR_BASE)/lib
	mkdir -p $(IDIR_BASE)/mnt
	mkdir -p $(IDIR_BASE)/mnt/mmc
	mkdir -p $(IDIR_BASE)/mnt/usb
	mkdir -p $(IDIR_BASE)/proc
	mkdir -p $(IDIR_BASE)/ram
	mkdir -p $(IDIR_BASE)/rom
	mkdir -p $(IDIR_BASE)/root
	mkdir -p $(IDIR_BASE)/sbin
	mkdir -p $(IDIR_BASE)/usr
	mkdir -p $(IDIR_BASE)/usr/bin
	mkdir -p $(IDIR_BASE)/usr/lib
	mkdir -p $(IDIR_BASE)/usr/local
	mkdir -p $(IDIR_BASE)/usr/share
	mkdir -p $(IDIR_BASE)/usr/share/udhcpc
	mkdir -p $(IDIR_BASE)/var
	mkdir -p $(IDIR_BASE)/var/cache
	mkdir -p $(IDIR_BASE)/var/lib
	mkdir -p $(IDIR_BASE)/var/lib/bluetooth
	mkdir -p $(IDIR_BASE)/var/lib/vnstat
	mkdir -p $(IDIR_BASE)/var/lock
	mkdir -p $(IDIR_BASE)/var/log
	mkdir -p $(IDIR_BASE)/var/mail
	mkdir -p $(IDIR_BASE)/var/run
	mkdir -p $(IDIR_BASE)/var/spool
	mkdir -p $(IDIR_BASE)/var/state
	mkdir -p $(IDIR_BASE)/www
	#
	-find $(IDIR_BASE) -type d -name CVS | xargs rm -rf
	-find $(IDIR_BASE) -type d -name .svn | xargs rm -rf
	-find $(IDIR_BASE) -name '.#*' | xargs rm -f
	$(IPKG_BUILD) $(IDIR_BASE) $(PACKAGE_DIR)

$(IPKG_UCLIBC):
	$(SCRIPT_DIR)/make-ipkg-dir.sh $(IDIR_UCLIBC) ipkg/uclibc.control $(UCLIBC_VERSION)-$(PKG_RELEASE) $(ARCH)
	mkdir -p $(IDIR_UCLIBC)/lib
	for file in ld-uClibc libc libcrypt libdl libm libnsl libresolv librt libuClibc libutil; do \
		$(CP) $(STAGING_DIR)/lib/$$file.so.* $(IDIR_UCLIBC)/lib/; \
		$(CP) $(STAGING_DIR)/lib/$$file-$(UCLIBC_VERSION).so $(IDIR_UCLIBC)/lib/; \
	done
	-$(STRIP) $(IDIR_UCLIBC)/lib/*
	$(IPKG_BUILD) $(IDIR_UCLIBC) $(PACKAGE_DIR)

$(IPKG_LIBGCC):
	$(SCRIPT_DIR)/make-ipkg-dir.sh $(IDIR_LIBGCC) ipkg/libgcc.control $(LIBGCC_VERSION)-$(PKG_RELEASE) $(ARCH)
	mkdir -p $(IDIR_LIBGCC)/lib
	$(CP) $(STAGING_DIR)/lib/libgcc_s.so.* $(IDIR_LIBGCC)/lib/
	-$(STRIP) $(IDIR_LIBGCC)/lib/*
	$(IPKG_BUILD) $(IDIR_LIBGCC) $(PACKAGE_DIR)

package-clean:
	rm -f $(PACKAGES)
	
clean-targets: package-clean
prepare:
compile-targets: $(PACKAGES)
install-targets: compile
	mkdir -p $(TARGET_DIR)
	$(IPKG) install $(PACKAGES_INSTALL)
