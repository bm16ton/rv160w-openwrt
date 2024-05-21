#
# Copyright (C) 2020 DNI
#

include $(TOPDIR)/rules.mk

PKG_NAME:=configuration_recover
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE).tar.gz


include $(INCLUDE_DIR)/package.mk


define Package/configuration_recover
  SECTION:=DNI
  CATEGORY:=DNI Packages
  DEPENDS:=+confd +libjson-c
  TITLE:=Configuration recover
endef

define Package/configuration_recover/description
 This package contains an daemon to recover the configuration.
endef

define Build/Prepare
	@true
endef

define Build/Compile
	@true
endef

define Package/configuration_recover/install
	$(TAR) -zxf $(DL_DIR)/$(PKG_SOURCE) -C$(1)
endef

$(eval $(call BuildPackage,configuration_recover))
