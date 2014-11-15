# $Id: Makefile 3112 2006-02-01 23:53:19Z mbm $

include $(TOPDIR)/rules.mk

PKG_NAME:=mc
PKG_VERSION:=4.6.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.ibiblio.org/pub/Linux/utils/file/managers/mc/
PKG_MD5SUM:=18b20db6e40480a53bac2870c56fc3c4
PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,MC,$(PKG_NAME),$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	(cd $(PKG_BUILD_DIR); rm -rf config.{cache,status} ; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include " \
		LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib" \
		PKG_CONFIG_PATH="$(STAGING_DIR)/usr/lib/pkgconfig" \
		PKG_CONFIG_LIBDIR="$(STAGING_DIR)/usr/lib/pkgconfig" \
		CC_FOR_BUILD="$(HOSTCC)" \
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
		--sysconfdir=/etc \
		$(DISABLE_LARGEFILE) \
		$(DISABLE_NLS) \
		--disable-rpath \
		--with-included-gettext \
		--with-ncurses \
		--without-sco \
		--without-sunos-curses \
		--without-osf1-curses \
		--without-vcurses \
		--without-gpm-mouse \
		--without-hsc \
		--without-termnet \
		--without-debug \
		--without-efence \
		--without-terminfo \
		--without-termcap \
		--without-slang \
		--without-vfs \
		--without-netrc \
		--without-ext2undel \
		--without-catgets \
		--without-x \
		--without-tk \
		--without-xview \
		--without-subshell \
		--disable-glibtest \
		--with-glib12 \
		--with-glib-prefix="$(STAGING_DIR)/usr" \
		--without-libiconv-prefix \
		--without-libintl-prefix \
	);
	touch $@

$(PKG_BUILD_DIR)/.built:
	$(MAKE) -C $(PKG_BUILD_DIR) \
		SUBDIRS="m4 slang edit lib src" \
		all
	touch $@

$(IPKG_MC):
	install -d -m0755 $(IDIR_MC)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/src/mc $(IDIR_MC)/usr/bin/
	ln -sf mc $(IDIR_MC)/usr/bin/mcedit
	ln -sf mc $(IDIR_MC)/usr/bin/mcview
	$(RSTRIP) $(IDIR_MC)
	$(IPKG_BUILD) $(IDIR_MC) $(PACKAGE_DIR)
