# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
#include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=firmware
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).$(PKG_RELEASE)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install



include $(INCLUDE_DIR)/package.mk

PKG_BIN_SRC=$(TOPDIR)/dl-bin/firmware-1.1.tar.gz

ifneq ("$(wildcard $(PKG_BIN_SRC))","")
	FILE_EXISTS = 1
else
	FILE_EXISTS = 0
endif

define Package/firmware
	SECTION:=Freescale Packages
	CATEGORY:=Freescale Packages
	TITLE:=firmware
endef

define Package/firmware/description
	RV16X_26X Firmware Update Manager.
endef

TARGET_CPPFLAGS:= \
	$(TARGET_CPPFLAGS) \
	-DSPI_OPERATION \
	-D_LITTLE_ENDIAN

MAKE_FLAGS += \
	FPIC="$(FPIC)" \
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

ifeq ($(FILE_EXISTS), 1)
define Build/Prepare
	$(call Build/Prepare/Default)
	tar -zxf $(PKG_BIN_SRC) -C $(BUILD_DIR)/
endef

else
define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) -a src $(PKG_BUILD_DIR)/
endef
endif

ifeq ($(FILE_EXISTS), 1)
define Build/Compile
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR_HOST)/include -include endian.h  $(TARGET_CFLAGS_EXTRA)"
endef
else
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR_HOST)/include -include endian.h  $(TARGET_CFLAGS_EXTRA)"          
endef
endif

#define Build/Compile
#	true
#endef
ifeq ($(FILE_EXISTS), 1)
define Package/firmware/install
  echo "Firmware Manager install !!"
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/firmware.config $(1)/etc/config/firmware
	$(INSTALL_BIN) ./files/firmware.init $(1)/etc/init.d/firmware
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/bin/postupgrade
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/boardutil $(1)/usr/bin/
	# $(INSTALL_BIN) ./files/rv16x_26x_fw_unpack.sh $(1)/usr/bin/
	$(INSTALL_BIN) ./files/rv16x_26x_led.sh $(1)/usr/bin/
	$(INSTALL_BIN) ./files/kill_led.sh $(1)/usr/bin/
	$(INSTALL_BIN) ./files/postupgrade.sh $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/libboardinfo.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/rv_boardinfo_api.h $(STAGING_DIR)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/libboardinfo.so $(STAGING_DIR)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/51-led.hotplug $(1)/etc/hotplug.d/iface/51-led
	$(CP) ./files/postupgrade/* $(1)/usr/bin/postupgrade/
endef
else
define Package/firmware/install
  echo "Firmware Manager install !!"
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/firmware.config $(1)/etc/config/firmware
	$(INSTALL_BIN) ./files/firmware.init $(1)/etc/init.d/firmware
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/bin/postupgrade
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/boardutil $(1)/usr/bin/
	# $(INSTALL_BIN) ./files/rv16x_26x_fw_unpack.sh $(1)/usr/bin/
	$(INSTALL_BIN) ./files/rv16x_26x_led.sh $(1)/usr/bin/
	$(INSTALL_BIN) ./files/kill_led.sh $(1)/usr/bin/
	$(INSTALL_BIN) ./files/postupgrade.sh $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/libboardinfo.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/rv_boardinfo_api.h $(STAGING_DIR)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/libboardinfo.so $(STAGING_DIR)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/51-led.hotplug $(1)/etc/hotplug.d/iface/51-led
	$(CP) ./files/postupgrade/* $(1)/usr/bin/postupgrade/
endef
endif

$(eval $(call BuildPackage,firmware))
