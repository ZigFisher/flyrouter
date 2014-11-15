# $Id: Makefile 3129 2006-02-04 21:55:53Z florian $

include $(TOPDIR)/rules.mk

PKG_NAME:=thttpd
PKG_VERSION:=2.25b
PKG_RELEASE:=1
PKG_MD5SUM:=156b249b3b0bcd48b06badd2db0d56c5

PKG_SOURCE_URL:=http://www.acme.com/software/thttpd/
PKG_SOURCE:=thttpd-$(PKG_VERSION).tar.gz
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/thttpd-$(PKG_VERSION)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,THTTPD,thttpd,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,THTTPD_HTPASSWD,thttpd-htpasswd,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,THTTPD_MATRIXSSL,thttpd-matrixssl,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))
$(eval $(call PKG_template,THTTPD_OPENSSL,thttpd-openssl,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))


$(PKG_BUILD_DIR)/.configured:
	    (cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
        BUILD_CC=$(TARGET_CC) HOSTCC=$(HOSTCC) \
        $(TARGET_CONFIGURE_OPTS) \
        CFLAGS="$(TARGET_CFLAGS)" \
        CPPFLAGS="-I$(STAGING_DIR)/usr/include" \
        LDFLAGS="-L$(STAGING_DIR)/usr/lib" \
        ac_cv_linux_vers=2 \
        td_cv_buggygetaddrinfo="no" \
        ./configure \
        --target=$(GNU_TARGET_NAME) \
        --host=$(GNU_TARGET_NAME) \
        --build=$(GNU_HOST_NAME) \
        --program-prefix="" \
        --program-suffix="" \
        --prefix=/usr \
        --exec-prefix=/usr \
        --bindir=/usr/bin \
        --datadir=/usr/share \
        --includedir=/usr/include \
        --infodir=/usr/share/info \
        --libdir=/usr/lib \
        --libexecdir=/usr/lib \
        --localstatedir=/var \
        --mandir=/usr/share/man \
        --sbindir=/usr/sbin \
        --sysconfdir=/etc )
	touch $@

$(PKG_BUILD_DIR)/.built:
	# with MatrixSSL
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC=$(TARGET_CC) \
		OFLAGS="$(TARGET_CFLAGS)" \
		all
	touch $@

$(IPKG_THTTPD):
	install -d -m0755 $(IDIR_THTTPD)/etc
	install -m0644 ./files/thttpd.conf $(IDIR_THTTPD)/etc/thttpd.conf
	install -d -m0755 $(IDIR_THTTPD)/usr/sbin
	install -m0755 $(PKG_BUILD_DIR)/thttpd $(IDIR_THTTPD)/usr/sbin/
	$(RSTRIP) $(IDIR_THTTPD)
	$(IPKG_BUILD) $(IDIR_THTTPD) $(PACKAGE_DIR)

$(IPKG_THTTPD_HTPASSWD):
	install -d -m0755 $(IDIR_THTTPD_HTPASSWD)/usr/sbin
	install -m0755 $(PKG_BUILD_DIR)/extras/htpasswd $(IDIR_THTTPD_HTPASSWD)/usr/sbin/
	$(RSTRIP) $(IDIR_THTTPD_HTPASSWD)
	$(IPKG_BUILD) $(IDIR_THTTPD_HTPASSWD) $(PACKAGE_DIR)

