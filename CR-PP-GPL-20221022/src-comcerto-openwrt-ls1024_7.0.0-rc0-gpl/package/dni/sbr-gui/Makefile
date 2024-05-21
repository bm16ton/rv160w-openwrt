# 12-30-2021: Vinson.Yang <Vinson.Yang@deltaww.com>
# 12/30/2021 sbr-gui drop.

include $(TOPDIR)/rules.mk

PKG_NAME:=sbr-gui
PKG_VERSION:=v1.4.39
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE).tar.xz

include $(INCLUDE_DIR)/package.mk

define Package/sbr-gui
  SECTION:=DNI
  CATEGORY:=DNI Packages
  DEPENDS:=+confd +libjson-c +libuci +libpcre +libxml2
  TITLE:=Cisco SBR GUI
endef

define Package/sbr-gui/description
 DNI Release.
endef

define Build/Prepare
	@true
endef

define Build/Compile
	@true
endef

define Package/sbr-gui/install
	$(TAR) -Jxf $(DL_DIR)/$(PKG_SOURCE) -C$(1)
endef

$(eval $(call BuildPackage,sbr-gui))
