# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=poe
PKG_VERSION:=1.0
PKG_RELEASE:=0

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install
PKG_BIN_SRC=$(TOPDIR)/package/freescale/$(PKG_NAME)/src
PKG_BIN_FILES=$(TOPDIR)/package/freescale/$(PKG_NAME)/files

include $(INCLUDE_DIR)/package.mk

define KernelPackage/poe
	CATEGORY:=Freescale Packages
	SECTION:=Freescale Packages
	TITLE:=PoE Kernel Module
	FILES:= $(PKG_BUILD_DIR)/poedrv.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/poe/description
	Power Over Ethernet Module.
endef

define Package/poe
	CATEGORY:=Freescale Packages
	SECTION:=Freescale Packages
	TITLE:=PoE Userspace Module
	FILES:= $(PKG_BUILD_DIR)/poedrv.$(LINUX_KMOD_SUFFIX)
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	mkdir -p $(PKG_BUILD_DIR)
	cp -r $(PKG_BIN_SRC)/* $(PKG_BUILD_DIR)/
	find $(PKG_BUILD_DIR) -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
endef

define Build/Compile
	$(call Build/Compile/Default, CROSS_COMPILE="$(TARGET_CROSS)" KERNEL_SOURCE="$(LINUX_DIR)" CCOPT="$(TARGET_CFLAGS)" DESTDIR="$(PKG_INSTALL_DIR)")
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	KERNEL_SOURCE="$(LINUX_DIR)" \
	CCOPT="$(TARGET_CFLAGS)" \
	DESTDIR="$(PKG_INSTALL_DIR)" all 
endef

define Package/poe/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BIN_FILES)/poe.init $(1)/etc/init.d/poeinit
	$(INSTALL_BIN) $(PKG_BIN_FILES)/startpoeled $(1)/etc
	$(INSTALL_BIN) $(PKG_BIN_FILES)/stoppoeled $(1)/etc
	mkdir -p $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BIN_FILES)/poe.conf $(1)/etc/config/poeconf
endef

$(eval $(call KernelPackage,poe))
$(eval $(call BuildPackage,poe))
