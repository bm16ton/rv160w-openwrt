#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ds-lite
PKG_VERSION:=7
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/ds-lite
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+kmod-ipv6 +kmod-ip6-tunnel +resolveip
  TITLE:=Dual-Stack Lite (DS-Lite) configuration support
  MAINTAINER:=Steven Barth <steven@midlink.org>
  PKGARCH:=all
endef

define Package/ds-lite/description
Provides support for Dual-Stack Lite in /etc/config/network.
Refer to http://wiki.openwrt.org/doc/uci/network for
configuration details.
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/ds-lite/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DATA) ./files/dslite.config $(1)/etc/config/dslite
	$(INSTALL_BIN) ./files/dslite.init $(1)/etc/init.d/dslite
endef

$(eval $(call BuildPackage,ds-lite))
