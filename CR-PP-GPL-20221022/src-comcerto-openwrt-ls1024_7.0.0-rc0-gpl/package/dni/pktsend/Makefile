# 03-03-2017: harry.lin <harry.lin@deltaww.com>
# Add burn in test for facotry test require
#
# Copyright (C) 20016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=pktsend
PKG_RELEASE:=1

PKG_BUILD_DIR := $(KERNEL_BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/pktsend
  SECTION:=DNI
  CATEGORY:=DNI Packages
  TITLE:=PACKETS SENDING
endef

define Package/pktsend/description
 This package contains an utility useful to do packets sending.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef

define Package/pktsend/install
	@true
endef

$(eval $(call BuildPackage,pktsend))
