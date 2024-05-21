# 2017-04-01: li.zhang <li.zhang@deltaww.com>
# Add the 3g_carrierlist script
# 2017-03-13: li.zhang <li.zhang@deltaww.com>
# Add some 3g need script 
#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=comgt
PKG_VERSION:=0.32
PKG_RELEASE:=25

PKG_SOURCE:=$(PKG_NAME).$(PKG_VERSION).tgz
PKG_SOURCE_URL:=@SF/comgt
PKG_MD5SUM:=db2452680c3d953631299e331daf49ef

PKG_MAINTAINER:=Felix Fietkau <nbd@openwrt.org>
PKG_LICENSE:=GPL-2.0+

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME).$(PKG_VERSION)
PKG_CHECK_FORMAT_SECURITY:=0

include $(INCLUDE_DIR)/package.mk

define Package/comgt/Default
  SECTION:=utils
  CATEGORY:=Utilities
endef

define Package/comgt
$(call Package/comgt/Default)
  TITLE:=Option/Vodafone 3G/GPRS control tool
  DEPENDS:=+chat
  URL:=http://www.pharscape.org/comgt.html
endef

define Package/comgt-directip
$(call Package/comgt/Default)
  TITLE:=Sierra Wireless Direct-IP support
  DEPENDS:=+comgt
endef

define Package/comgt-ncm
$(call Package/comgt/Default)
  TITLE+=NCM 3G/4G Support
  DEPENDS:=+comgt +wwan
endef

define Package/comgt/description
 comgt is a scripting language interpreter useful for establishing 
 communications on serial lines and through PCMCIA modems as well as GPRS 
 and 3G datacards.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="" \
		comgt
endef

define Package/comgt/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/comgt $(1)/usr/bin/
	$(LN) comgt $(1)/usr/bin/gcom
	$(INSTALL_DIR) $(1)/etc/
	ln -sf /mnt/packages/mobile/etc/chatscripts/ $(1)/etc/
	ln -sf /mnt/packages/mobile/etc/gcom/ $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/hotplug.d/
	ln -sf /mnt/packages/mobile/etc/hotplug.d/tty/ $(1)/etc/hotplug.d/

	$(INSTALL_DIR) $(1)/lib/netifd/proto
	ln -sf /mnt/packages/mobile/lib/netifd/proto/3g.sh $(1)/lib/netifd/proto/3g.sh
	ln -sf /mnt/packages/mobile/lib/netifd/proto/mobile.sh $(1)/lib/netifd/proto/mobile.sh
endef

define Package/comgt-directip/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	ln -sf /mnt/packages/mobile/lib/netifd/proto/directip.sh $(1)/lib/netifd/proto/directip.sh

endef

define Package/comgt-ncm/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	ln -sf /mnt/packages/mobile/lib/netifd/proto/ncm.sh $(1)/lib/netifd/proto/ncm.sh
endef

$(eval $(call BuildPackage,comgt))
$(eval $(call BuildPackage,comgt-directip))
$(eval $(call BuildPackage,comgt-ncm))
