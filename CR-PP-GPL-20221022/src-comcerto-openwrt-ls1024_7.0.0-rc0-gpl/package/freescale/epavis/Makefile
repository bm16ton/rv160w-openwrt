#
#
#
#
#
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=epavis-dpi
PKG_VERSION:=devel.mainline
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/epavis-dpi
  SECTION:=Freescale Packages
  CATEGORY:=Freescale Packages
  TITLE:=epavis-dpi wcf infra
  DEPENDS:= +libuci +libxml2 +kmod-nf-conntrack
endef

define Package/epavis-dpi/description
  Lionic WCF Infra module for Webfilter application
endef

define Build/Configure
	echo "Lionic WCF infra Configuring !!"
endef

define Build/Compile
	echo "Lionic WCF infra Compiling !!"
endef

define Package/epavis-dpi/install
	echo "Lionic WCF infra Installing !!"
	$(CP) ./files/etc $(1)/
	$(CP) ./files/lib $(1)/
	$(CP) ./files/usr $(1)/
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/lionic.hotplug $(1)/etc/hotplug.d/iface/26-lionic
endef

$(eval $(call BuildPackage,epavis-dpi))
