# 03-03-2017: harry.lin <harry.lin@deltaww.com>
# Delete bcm-53128M-switch and bcm-53134M-switch, change to be both use bcm-switch, and install into FW to be rv16x-26x-switch.
# 2017-02-06: harry.lin <harry.lin@deltaww.com>
# Add BRCM SWITCH sdk-xgs-robo package support.
# 
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk


PKG_NAME:=sdk-xgs-robo
PKG_VERSION:=6.5.4
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install
	
include $(INCLUDE_DIR)/package.mk
PKG_BIN_SRC=$(TOPDIR)/dl/sdk-xgs-robo-6.5.4.tar.gz

define KernelPackage/sdk-xgs-robo
  CATEGORY:=Broadcom Packages
  SECTION:=Broadcom Packages
  TITLE:=Broadcom BDE Kernel Module
  FILES:= $(PKG_BUILD_DIR)/build/linux/user/dni-4_1_8/linux-kernel-bde.$(LINUX_KMOD_SUFFIX) $(PKG_BUILD_DIR)/build/linux/user/dni-4_1_8/linux-user-bde.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/sdk-xgs-robo/description
 This package is for Broadcom Robo switch BDE kernel module  
endef

define Package/sdk-xgs-robo
  CATEGORY:=Broadcom Packages
  SECTION:=Broadcom Packages
  TITLE:=Broadcom Userspace Module
endef

define Package/sdk-xgs-robo/description
 This package is for Broadcom Robo switch userspace API
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	tar -zxf $(PKG_BIN_SRC) -C $(KERNEL_BUILD_DIR)/
	mkdir -p $(PKG_BUILD_DIR)/systems/linux/user/switch
	cp ./src/roboserver.c $(PKG_BUILD_DIR)/systems/linux/user/switch
	cp ./src/diag_client.c $(PKG_BUILD_DIR)/systems/linux/user/switch
	cp ./src/Makefile $(PKG_BUILD_DIR)/systems/linux/user/switch
	find $(PKG_BUILD_DIR)/ -name 'CVS' -o -name '.git' -o -name '.svn' -o -name '*~' | $(XARGS) rm -rf
	$(call Build/Prepare/Default)
endef

define Build/Compile
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR)/systems/linux/user/dni-4_1_8
endef

define Package/sdk-xgs-robo/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/linux/user/dni-4_1_8/robodiag $(1)/usr/sbin/robodiag
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/linux/user/dni-4_1_8/bcmssdk $(1)/usr/sbin/bcmssdk
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/bcm-switch $(1)/etc/init.d/switch
	$(INSTALL_BIN) ./files/etc/init.d/bcm-mod $(1)/etc/init.d/zswitch
	$(INSTALL_BIN) ./files/etc/bcm_init.sh $(1)/etc/init.d/bcm_init.sh

endef

$(eval $(call KernelPackage,sdk-xgs-robo))
$(eval $(call BuildPackage,sdk-xgs-robo))
