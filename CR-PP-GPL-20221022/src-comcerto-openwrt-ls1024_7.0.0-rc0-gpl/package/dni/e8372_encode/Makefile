include $(TOPDIR)/rules.mk

PKG_NAME:=encode
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/encode
    SECTION:=DNI
    CATEGORY:=DNI Packages
    TITLE:=E8372 dongle password encode
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Package/encode/install
	install -d -m0755 $(1)/usr/bin
	ln -sf /mnt/packages/mobile/usr/bin/encode $(1)/usr/bin/encode
endef

$(eval $(call BuildPackage,encode))
