# 2017-03-13: li.zhang <li.zhang@deltaww.com>
# The mobile related script and driver packages

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=usb-modem
PKG_VERSION=0.0.00.05
PKG_RELEASE:=5
PKG_TARGETS:=ipkg bin
PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/usb-modem
  SECTION:=DNI
  CATEGORY:=DNI Packages
  TITLE:=3G/4G USB Modem Driver
  DEPENDS:=+usb-modeswitch +encode +kmod-mii

  DRIVERS:= \
		$(LINUX_DIR)/drivers/usb/class/cdc-acm.ko \
		$(LINUX_DIR)/drivers/usb/class/cdc-wdm.ko \
		$(LINUX_DIR)/drivers/usb/serial/usbserial.ko \
		$(LINUX_DIR)/drivers/usb/serial/usb_wwan.ko \
		$(LINUX_DIR)/drivers/usb/serial/qcserial.ko \
		$(LINUX_DIR)/drivers/usb/serial/sierra.ko \
		$(LINUX_DIR)/drivers/usb/serial/option.ko \
		$(LINUX_DIR)/drivers/net/usb/usbnet.ko \
		$(LINUX_DIR)/drivers/net/usb/qmi_wwan.ko \
		$(LINUX_DIR)/drivers/net/usb/sierra_net.ko \
		$(LINUX_DIR)/drivers/net/usb/cdc_ncm.ko \
		$(LINUX_DIR)/drivers/net/usb/huawei_cdc_ncm.ko
endef

define Package/usb-modem/description
  Kernel support for 3G/4G USB Modem Driver
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile
	@true
endef

define Package/usb-modem/install
	$(INSTALL_DIR) $(1)/mnt/packages/
	$(INSTALL_DIR) $(1)/etc/mobile/
	$(INSTALL_DIR) $(1)/etc/mobile/etc/modules.d
	echo $(foreach file,$(DRIVERS),$(basename $(notdir $(file)))) | tr ' ' '\012' > $(1)/etc/mobile/etc/modules.d/60-usb-modem
	echo $(PKG_VERSION) >> $(1)/etc/mobile/etc/usb-modem.ver
	$(CP) ./files/* $(1)/etc/mobile/
	$(INSTALL_DIR) $(1)/usr/lib/opkg/info
	$(INSTALL_DIR) $(1)/etc/mobile/lib/modules/4.1.8/
	# driver
	$(CP) $(LINUX_DIR)/drivers/usb/class/cdc-acm.ko $(1)/etc/mobile/lib/modules/4.1.8/
	$(CP) $(LINUX_DIR)/drivers/usb/class/cdc-wdm.ko $(1)/etc/mobile/lib/modules/4.1.8/
	$(CP) $(LINUX_DIR)/drivers/usb/serial/usbserial.ko $(1)/etc/mobile/lib/modules/4.1.8/
	$(CP) $(LINUX_DIR)/drivers/usb/serial/usb_wwan.ko $(1)/etc/mobile/lib/modules/4.1.8/
	$(CP) $(LINUX_DIR)/drivers/usb/serial/qcserial.ko $(1)/etc/mobile/lib/modules/4.1.8/
	$(CP) $(LINUX_DIR)/drivers/usb/serial/sierra.ko $(1)/etc/mobile/lib/modules/4.1.8/
	$(CP) $(LINUX_DIR)/drivers/usb/serial/option.ko $(1)/etc/mobile/lib/modules/4.1.8/
	$(CP) $(LINUX_DIR)/drivers/net/usb/usbnet.ko $(1)/etc/mobile/lib/modules/4.1.8/
	$(CP) $(LINUX_DIR)/drivers/net/usb/qmi_wwan.ko $(1)/etc/mobile/lib/modules/4.1.8/
	$(CP) $(LINUX_DIR)/drivers/net/usb/sierra_net.ko $(1)/etc/mobile/lib/modules/4.1.8/
	$(CP) $(LINUX_DIR)/drivers/net/usb/cdc_ncm.ko $(1)/etc/mobile/lib/modules/4.1.8/
	$(CP) $(LINUX_DIR)/drivers/net/usb/huawei_cdc_ncm.ko $(1)/etc/mobile/lib/modules/4.1.8/
	#script
	$(INSTALL_DIR) $(1)/etc/mobile/lib/netifd/proto/
	$(CP) $(TOPDIR)/package/network/utils/comgt/files/* $(1)/etc/mobile/ -rf
	$(CP) $(TOPDIR)/package/network/utils/uqmi/files/lib/netifd/proto/qmi.sh $(1)/etc/mobile/lib/netifd/proto/
	$(CP) $(TOPDIR)/package/network/utils/uqmi/files/lib/netifd/nat.script $(1)/etc/mobile/lib/netifd/
	$(CP) $(TOPDIR)/package/dni/bwmgmt/files/etc/hotplug.d/tty/40-bwmgmt $(1)/etc/mobile/etc/hotplug.d/tty/
	$(INSTALL_DIR) $(1)/etc/mobile/etc/hotplug.d/usbmisc/
	$(CP) $(TOPDIR)/package/dni/bwmgmt/files/etc/hotplug.d/usbmisc/40-4g_bwmgmt $(1)/etc/mobile/etc/hotplug.d/usbmisc/
	$(CP) $(TOPDIR)/package/network/utils/wwan/files/wwan.usbmisc $(1)/etc/mobile/etc/hotplug.d/usbmisc/00_wwan.sh
	$(CP) $(TOPDIR)/package/network/utils/uqmi/files/etc/hotplug.d/usbmisc/30-4g_link $(1)/etc/mobile/etc/hotplug.d/usbmisc/
	$(CP) $(BUILD_DIR)/usbmode*/ipkg-comcerto2000/usb-modeswitch/etc/mobile/etc/usb-mode.json $(1)/etc/mobile/etc/usb-mode.json
	$(INSTALL_DIR) $(1)/etc/mobile/usr/bin/
	$(CP) $(BUILD_DIR)/encode/encode $(1)/etc/mobile/usr/bin/encode
	$(CP) $(TOPDIR)/package/freescale/opsdb/files/update_mobile.sh $(1)/etc/mobile/usr/bin/update_mobile.sh

	[ "$(1)" = "$(BIN_DIR)/$(PKG_NAME)" ] && ($(RSTRIP) "$(1)" && cd "$(1)" && find ! -type d -printf '%P\n' | sort | grep -v '^\(install\.list\|fwsupport\)'> $(1)/install.list && tar -zcf $(PKG_NAME)-$(PKG_VERSION).tar.gz -T $(1)/install.list etc/mobile/fwsupport install.list; ls -A | grep -v "$(PKG_NAME)" | xargs rm -rf) || true

	[ "$(1)" = "$(BIN_DIR)/$(PKG_NAME)" ] && (cd "$(1)" && ${TOPDIR}/build_dir/host/fwutils/encode $(PKG_NAME)-$(PKG_VERSION).tar.gz ${TOPDIR}/package/freescale/fwutils/files/p1.pem  usbcode && tar  -zcf $(PKG_NAME)-en-$(PKG_VERSION).tar.gz usbcode $(PKG_NAME)-$(PKG_VERSION).tar.gz) || true
	# driver and file link
	[ "$(1)" = "$(BIN_DIR)/$(PKG_NAME)" ] || \
	(cd "$(1)" && find ! -type d -printf '%P\n' | sort | grep -v '\(install\.list\|fwsupport\)'> $(1)/etc/mobile/install.list;\
	$(INSTALL_DIR) $(1)/lib/modules/4.1.8/; \
	ln -sf /mnt/packages/mobile/lib/modules/4.1.8/cdc-acm.ko $(1)/lib/modules/4.1.8/cdc-acm.ko; \
	ln -sf /mnt/packages/mobile/lib/modules/4.1.8/cdc-wdm.ko $(1)/lib/modules/4.1.8/cdc-wdm.ko; \
	ln -sf /mnt/packages/mobile/lib/modules/4.1.8/usbserial.ko $(1)/lib/modules/4.1.8/usbserial.ko; \
	ln -sf /mnt/packages/mobile/lib/modules/4.1.8/usb_wwan.ko $(1)/lib/modules/4.1.8/usb_wwan.ko; \
	ln -sf /mnt/packages/mobile/lib/modules/4.1.8/qcserial.ko $(1)/lib/modules/4.1.8/qcserial.ko; \
	ln -sf /mnt/packages/mobile/lib/modules/4.1.8/sierra.ko $(1)/lib/modules/4.1.8/sierra.ko; \
	ln -sf /mnt/packages/mobile/lib/modules/4.1.8/option.ko $(1)/lib/modules/4.1.8/option.ko; \
	ln -sf /mnt/packages/mobile/lib/modules/4.1.8/usbnet.ko $(1)/lib/modules/4.1.8/usbnet.ko; \
	ln -sf /mnt/packages/mobile/lib/modules/4.1.8/qmi_wwan.ko $(1)/lib/modules/4.1.8/qmi_wwan.ko; \
	ln -sf /mnt/packages/mobile/lib/modules/4.1.8/sierra_net.ko $(1)/lib/modules/4.1.8/sierra_net.ko; \
	ln -sf /mnt/packages/mobile/lib/modules/4.1.8/cdc_ncm.ko $(1)/lib/modules/4.1.8/cdc_ncm.ko; \
	ln -sf /mnt/packages/mobile/lib/modules/4.1.8/huawei_cdc_ncm.ko $(1)/lib/modules/4.1.8/huawei_cdc_ncm.ko; \
	ln -sf /mnt/packages/mobile/etc/usb-modem.ver $(1)/etc/usb-modem.ver; \
	$(INSTALL_DIR) $(1)/etc/modules.d; \
	ln -sf /mnt/packages/mobile/etc/modules.d/60-usb-modem $(1)/etc/modules.d/60-usb-modem; \
	$(INSTALL_DIR) $(1)/sbin/; \
	ln -sf /mnt/packages/mobile/sbin/usb-modem $(1)/sbin/usb-modem; \
	ln -sf /mnt/packages/mobile/usr/lib/opkg/info/kmod-usb-modem.list $(1)/usr/lib/opkg/info/kmod-usb-modem.list)|| true
	$(RM) $(1)/etc/mobile/etc/usb-mode.json
	$(RM) $(1)/etc/mobile/fwsupport
endef

$(eval $(call BuildPackage,usb-modem))
