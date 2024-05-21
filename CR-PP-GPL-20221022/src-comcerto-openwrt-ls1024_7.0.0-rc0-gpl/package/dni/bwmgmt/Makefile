# 03-16-2018: harry.lin <harry.lin@deltaww.com>
# 03/16/2018 bwmgmt drop.

include $(TOPDIR)/rules.mk

PKG_NAME:=bwmgmt
PKG_VERSION:=2018-03-16
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)$(if $(PKG_RELEASE),-$(PKG_RELEASE)).tar.xz

include $(INCLUDE_DIR)/package.mk

define Package/bwmgmt
  SECTION:=DNI
  CATEGORY:=DNI Packages
  TITLE:=BandWidth Management
endef

define Package/bwmgmt/description
 DNI Release.
endef

define Build/Prepare
	@true
endef

define Build/Compile
	@true
endef

define Package/bwmgmt/install
	$(TAR) -Jxf $(DL_DIR)/$(PKG_SOURCE) -C$(1)
endef

$(eval $(call BuildPackage,bwmgmt))
