#!/bin/sh
# 2017-08-22: li.zhang <li.zhang@deltaww.com>
# Porting the E3276
# 2017-07-21: li.zhang <li.zhang@deltaww.com>
# Add the special nat dongle
# 2017-06-27: li.zhang <li.zhang@deltaww.com>
# Add the AC320U 4g dongle
# 2017-04-01: li.zhang <li.zhang@deltaww.com>
# Add the repeat dialing when dialing fail
# 2017-03-24: li.zhang <li.zhang@deltaww.com>
# modify the sig_strength value
# 2017-03-13:li.zhang <li.zhang@deltaww.com>
# the initial 4G connection script  
[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	. ../nat.script
	. ./directip.sh
	. ./ncm.sh
	init_proto "$@"
}

proto_qmi_init_config() {
	available=1
	no_device=1
	proto_config_add_string "device:device"
	proto_config_add_string apn
	proto_config_add_string auth
	proto_config_add_string username
	proto_config_add_string password
	proto_config_add_string pincode
	proto_config_add_string delay
	proto_config_add_string modes
	proto_config_add_string global_enable
}

qmi_disconnect() {
	# disable previous autoconnect state using the global handle
	# do not reuse previous wds client id to prevent hangs caused by stale data
	killall -9 uqmi
	uqmi -s -d "$device" \
		--stop-network 0xffffffff \
		--autoconnect > /dev/null
}

qmi_wds_release() {
	[ -n "$cid" ] || return 0

	uqmi -s -d "$device" --set-client-id wds,"$cid" --release-client-id wds
	uci_revert_state network $interface cid
}

_proto_qmi_setup() {
	local interface="$1"

	local device apn auth username password pincode delay modes cid pdh metric
	json_get_vars device apn auth username password pincode delay modes global_enable metric

	[ -n "$ctl_device" ] && device=$ctl_device

	[ -n "$device" ] || {
		logger -t mobile -p local0.error "No control device specified"
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}
	[ -c "$device" ] || {
		logger -t mobile -p local0.error "$interface: The specified control device does not exist"
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}

	modem=$device
	device="$(readlink -f "$device")"
	devname="$(basename "$device")"
	devpath="/sys/class/usbmisc/$devname/device/"
	ifname="$( ls "$devpath"/net )"
	[ -n "$ifname" ] || {
		logger -t mobile -p local0.error "$interface: The interface could not be found."
		proto_notify_error "$interface" NO_IFACE
		proto_set_available "$interface" 0
		return 1
	}

	[ -n "$delay" ] && sleep "$delay"

	while uqmi -s -d "$device" --get-pin-status | grep '"UIM uninitialized"' > /dev/null; do
		sleep 1;
	done

	[ -n "$pincode" ] && {
		uqmi -s -d "$device" --verify-pin1 "$pincode" || {
			logger -t mobile -p local0.error "$interface: Unable to verify PIN"
			proto_notify_error "$interface" PIN_FAILED
			proto_block_restart "$interface"
			return 1
		}
	}

	#AC341 wait wda set data format success
	if [ "$temp" = "1199:9055" ];then
		cid=`uqmi -s -d "$device" --get-client-id wds`
		uqmi -s -d "$device" --set-client-id wds,"$cid" --start-network "internet" --autoconnect &
		sleep 2
		killall -9 uqmi
		uqmi -s -d "$device" --set-client-id wds,"$cid" --release-client-id wds
		uqmi -s -d "$device" --stop-network 0xffffffff --autoconnect
	fi

	qmi_disconnect

	uqmi -s -d "$device" --set-data-format 802.3
	uqmi -s -d "$device" --wda-set-data-format 802.3

	logger -t mobile -p local0.debug "$interface: Waiting for network registration"
	while uqmi -s -d "$device" --get-serving-system | grep '"searching"' > /dev/null; do
		sleep 5;
	done

	[ -n "$modes" ] && uqmi -s -d "$device" --set-network-modes "$modes"

	cid=`uqmi -s -d "$device" --get-client-id wds`
	[ $? -ne 0 ] && {
		logger -t mobile -p local0.error "$interface: Unable to obtain client ID"
		proto_notify_error "$interface" NO_CID
		return 1
	}


	[ -n "$apn" ] || {
		# Try to get the apn from ISP
		apn=`qmicli -d $device --wds-get-default-settings=3gpp | grep "APN:" | cut -d : -f 2 | sed -e "s/^ *//g" | sed -e "s/'//g"` >/dev/null 2>&1
		[ -z "$apn" ] && {
			apn="internet"
		}
	}

	# Update the statistics
	update_device_info $interface "$apn"
	
	logger -t mobile -p local0.notice "$interface: Starting network $apn"

	while [ "$repeat" = "" ];
	do
		pdh=`uqmi -s -d "$device" --set-client-id wds,"$cid" \
			--start-network "$apn" \
			${auth:+--auth-type $auth} \
			${username:+--username $username} \
			${password:+--password $password}`
		repeat=`echo $pdh | sed s/[a-z,A-Z,\",\ ]//g`
		sleep 1
	done

	uci set network.$interface.cid=$cid
	uci set network.$interface.pdh="$pdh"
	uci commit network

	logger -t mobile -p local0.debug "$interface: Starting DHCP"
	proto_init_update "$ifname" 1
	proto_send_update "$interface"

	json_init
	json_add_string name "${interface}_4"
	json_add_string ifname "@$interface"
	json_add_string proto "dhcp"
	json_add_int metric "$metric"
	json_close_object
	ubus call network add_dynamic "$(json_dump)"

	json_init
	json_add_string name "${interface}_6"
	json_add_string ifname "@$interface"
	json_add_string proto "dhcpv6"
	json_close_object
	ubus call network add_dynamic "$(json_dump)"
}

update_device_info()
{
	local interface="$1"
	local apn="$apn"

	manufacture=`qmicli -d $modem --dms-get-manufacturer | grep Manufacturer | cut -d : -f 2- | sed -e "s/^ *//g" | sed -e "s/'//g"` >/dev/null 2>&1
	card_info=`qmicli -d $modem --dms-get-model | grep Model: | cut -d : -f 2 | sed -e "s/^ *//g" | sed -e "s/'//g"` >/dev/null 2>&1
	firmware=`qmicli -d $modem --dms-get-revision | grep "Revision" | cut -d : -f 2- | sed -e "s/^ *//g" | sed -e "s/'//g"`>/dev/null 2>&1
	sim_status=`qmicli -d $modem --dms-uim-get-state | grep "State" | cut -d : -f 2- | sed -e "s/^ *//g" | sed -e "s/'//g"` >/dev/null 2>&1
	imsi=`qmicli -d $modem --dms-uim-get-imsi | grep "IMSI:" | cut -d : -f 2- | sed -e "s/^ *//g" | sed -e "s/'//g"` >/dev/null 2>&1

	carrier=`qmicli -d $modem --nas-get-home-network | grep Description: | cut -d : -f 2- | sed -e "s/^ *//g" | sed -e "s/'//g"` >/dev/null 2>&1
	sig_strength=`qmicli -d $modem --nas-get-signal-strength | awk '/Current:/{getline; print}' | cut -d : -f 2 | sed -e "s/^ *//g" | sed -e "s/'//g"` >/dev/null 2>&1
	
	sig_strength=`echo $sig_strength | sed "s/[a-z,A-Z,\>,\ ]//g"`

	service_name="4G"
	driver_version=`cat /etc/usb-modem.ver`

	logger -t mobile -p local0.info "$interface: manufacture=$manufacture" 
	logger -t mobile -p local0.info "$interface: card_info=$card_info"
	logger -t mobile -p local0.info "$interface: firmware=$firmware"
	logger -t mobile -p local0.info "$interface: imsi=$imsi"
	logger -t mobile -p local0.info "$interface: sim_status=$sim_status"
	logger -t mobile -p local0.info "$interface: carrier=$carrier"
	logger -t mobile -p local0.info "$interface: service_name=$service_name"
	logger -t mobile -p local0.info "$interface: sig_strength=$sig_strength"
	logger -t mobile -p local0.info "$interface: driver_version=$driver_version"
	logger -t mobile -p local0.info "$interface: apn=$apn"

	`sh /usr/bin/update_mobile.sh "add" "$modem" "$manufacture" "$firmware" "$sim_status" "$imsi" "$carrier" "$service_name" "$sig_strength" "$card_info"` >/dev/null 2>&1        
}

proto_qmi_setup() {
	local ret

	if [ "$global_enable" = "0" ];then
		return 1
	fi

	# Keeping a sleep because in some cases, device is not available when this is called.
	sleep 5
	
	temp=`lsusb | grep "Bus 003" | grep -v "Device 001" | cut -d " " -f 6`

	# The nat special dongle
	if [ "$temp" = "12d1:14db" ] || [ "$temp" = "12d1:14dc" ] || [ "$temp" = "1410:9022" ];then
		proto_nat_setup $interface
		return $?
	fi

	# The AC320U special dongle
	if [ "$temp" = "0f3d:68aa" ];then
		proto_directip_setup $interface
		return $?
	fi

	# The ncm special dongle
	if [ "$temp" = "12d1:1506" ];then
		proto_ncm_setup $interface
		return $?
	fi

	# normal qmi dongle
	_proto_qmi_setup $@
	ret=$?

	[ "$ret" = 0 ] || {
		logger "qmi bringup failed, retry in 15s"
		sleep 15
	}

	return $ret
}

proto_qmi_teardown() {
	local device
	json_get_vars device

	local cid=$(uci get network.$interface.cid)
	local pdh=$(uci get network.$interface.pdh)
	
	[ -n "$cid" ] && {
		[ -n "$pdh" ] && {
			logger -t mobile -p local0.debug "$interface: Stopping network pdh"
			uqmi -s -d "$device" --set-client-id wds,"$cid" --stop-network "$pdh"
		}
		logger -t mobile -p local0.debug "$interface: Stopping network cid"
		uqmi -s -d "$device" --set-client-id wds,"$cid" --release-client-id wds --set-autoconnect disabled
	}

	pid=`pgrep -f "uqmi -s -d $device"`

	[ -n "$pid" ] && {
		logger -t mobile -p local0.debug "$interface: Stopping network killall uqmi"
		kill -9 $pid
	}

	temp=`lsusb | grep "Bus 003" | grep -v "Device 001" | cut -d " " -f 6`
	if [ "$temp" = "12d1:1506" ];then
		proto_ncm_teardown "$interface"
	fi

	proto_kill_command "$interface"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol qmi
}

