#
# Copyright (C) 2020 DNI
#

include $(TOPDIR)/rules.mk

PKG_NAME:=license_check
PKG_VERSION:=1.1
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE).tar.gz


include $(INCLUDE_DIR)/package.mk


define Package/license_check
  SECTION:=DNI
  CATEGORY:=DNI Packages
  DEPENDS:=+confd +libjson-c +libuci
  TITLE:=Check Web Filtering Subscription license
endef

define Package/license_check/description
 This package contains an daemon to check license status periodically, disable Web Filtering if license is expired.
endef

define Build/Prepare
	@true
endef

define Build/Compile
	@true
endef

define Package/license_check/install
	$(TAR) -zxf $(DL_DIR)/$(PKG_SOURCE) -C$(1)
endef

$(eval $(call BuildPackage,license_check))
