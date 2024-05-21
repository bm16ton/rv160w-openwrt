#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_directip_init_config() {
	available=1
	no_device=1
	proto_config_add_string "device:device"
	proto_config_add_string "apn"
	proto_config_add_string "pincode"
	proto_config_add_string "auth"
	proto_config_add_string "username"
	proto_config_add_string "password"
}

proto_directip_setup() {
	local interface="$1"
	local chat devpath devname

	local device apn pincode ifname auth username password
	json_get_vars device apn pincode auth username password

	[ -n "$ctl_device" ] && device=$ctl_device

	if [ "$interface" = "usb1" ];then
		TTY_FILE=/tmp/TTY_USB1
		usb_port=1
	elif [ "$interface" = "usb2" ];then
		TTY_FILE=/tmp/TTY_USB2
		usb_port=2
	fi
	
	while read line
	do
		# Check which tty device can be used for data communication
		logger -t mobile -p local0.info "$interface: check for $line"
		
		dirpath=`find /sys/devices/platform/ -name $line | head -n 1`
		dirpath=`dirname $dirpath`
		InterruptDevice=`find $dirpath | xargs grep "Interrupt"`
		[ -n "$InterruptDevice" ] && {
			InterruptDevice=$line
			got_success=1
			logger -t mobile -p local0.info "$interface: selecting $InterruptDevice for communication"
			break
		}	
	done < $TTY_FILE
	
	[ "$got_success" = 0 ] && {
		InterruptDevice=`cat $TTY_FILE | head -n 1`
		logger -t mobile -p local0.info "$interface: selecting Default $InterruptDevice for communication"	
	}
	# Make link
	ln -sf /dev/$InterruptDevice $device
		
	# Update USB status
	usbconnstatus=`cat $USB_CONN_STATUS | grep USB${usb_port}`
	if [ -n "$usbconnstatus" ];then
		`sed -i "/^USB$usb_port:/c\USB$usb_port:$line:$device:4G:1" $USB_CONN_STATUS`
	else
		echo "USB$usb_port:$line:$device:4G:1" >> $USB_CONN_STATUS
	fi
	
	[ -e "$device" ] || {
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}
	
	modem=$device
	device="$(readlink -f "$device")"
	devname="$(basename "$device")"
	devpath="$(readlink -f /sys/class/tty/$devname/device)"
	ifname="$( ls "$devpath"/../../*/net )"

	[ -n "$ifname" ] || {
		proto_notify_error "$interface" NO_IFNAME
		proto_set_available "$interface" 0
		return 1
	}

	cardinfo=$(gcom -d "$modem" -s /etc/gcom/getcardinfo.gcom)
	[ -n $(echo "$cardinfo" | grep -q "Sierra Wireless") ] || {
		proto_notify_error "$interface" BAD_DEVICE
		proto_block_restart "$interface"
		return 1
	}

	if [ -n "$pincode" ]; then
		PINCODE="$pincode" gcom -d "$modem" -s /etc/gcom/setpin.gcom || {
			proto_notify_error "$interface" PIN_FAILED
			proto_block_restart "$interface"
			return 1
		}
	fi
	# wait for carrier to avoid firmware stability bugs
	gcom -d "$modem" -s /etc/gcom/getcarrier.gcom || return 1

	local auth_type=0
	case $auth in
	pap) auth_type=1;;
	chap) auth_type=2;;
	esac
	
	USE_APN="$apn" USE_USER="$username" USE_PASS="$password" USE_AUTH="$auth_type" \
			gcom -d "$modem" -s /etc/gcom/directip.gcom || {
		proto_notify_error "$interface" CONNECT_FAILED
		proto_block_restart "$interface"
		return 1
	}

	update_mobile_info $interface

	logger -p mobile -t "directip[$$]" "Connected, starting DHCP"
	proto_init_update "$ifname" 1
	proto_send_update "$interface"

	json_init
	json_add_string name "${interface}_4"
	json_add_string ifname "@$interface"
	json_add_string proto "dhcp"
	ubus call network add_dynamic "$(json_dump)"

	json_init
	json_add_string name "${interface}_6"
	json_add_string ifname "@$interface"
	json_add_string proto "dhcpv6"
	ubus call network add_dynamic "$(json_dump)"

	return 0
}

update_mobile_info()
{
	`gcom info -d $modem > /tmp/stats/3g/MANUFACTURE_modem1` >/dev/null 2>&1
        manufacture=`cat /tmp/stats/3g/MANUFACTURE_modem1 | grep Manufacturer: | head -n 1 | cut -d : -f 2 |  sed "s/[\r\n]*//g" | sed -e "s/^ *//g"` >/dev/null 2>&1
	card_info=`cat /tmp/stats/3g/MANUFACTURE_modem1 | grep Model: | head -n 1 | cut -d : -f 2 |  sed "s/[\r\n]*//g" | sed -e "s/^ *//g"` >/dev/null 2>&1
	firmware=`cat /tmp/stats/3g/MANUFACTURE_modem1 | grep Revision: | head -n 1 | cut -d : -f 2 | sed "s/[\r\n]*//g" | sed -e "s/^ *//g"` >/dev/null 2>&1
    
	# SIM STATUS
	`gcom PIN -d $modem > /tmp/stats/3g/PIN_modem1` >/dev/null 2>&1
    
	# Signal Strength
	`gcom sig -d $modem > /tmp/stats/3g/STRENGTH_modem1` >/dev/null 2>&1
	# IMSI
	`gcom -d $modem -s /etc/gcom/getimsi.gcom > /tmp/stats/3g/IMSI_modem1` >/dev/null 2>&1
	# Carrier
	`gcom -d $modem -s /etc/gcom/getcarrier.gcom > /tmp/stats/3g/CARRIER_modem1` >/dev/null 2>&1
	# Service Type
	`gcom -d $modem -s /etc/gcom/getservicetype.gcom > /tmp/stats/3g/SERVICE_modem1` >/dev/null 2>&1

	sim_status=`cat /tmp/stats/3g/PIN_modem1` >/dev/null 2>&1
	imsi=`cat /tmp/stats/3g/IMSI_modem1` >/dev/null 2>&1
	carrier=`cat /tmp/stats/3g/CARRIER_modem1 | grep COPS: | cut -d , -f 3 | sed -e 's/"//g'` >/dev/null 2>&1
	service_name="4G"
	signal=`cat /tmp/stats/3g/STRENGTH_modem1 | grep "Signal Quality:" | cut -d : -f 2 | cut -d , -f 1 | sed -e "s/^ *//g"` >/dev/null 2>&1

	if [ -n "$signal" ];then
	       case $signal in
			[0-1])
				sig_strength="-109 dBm"
		;;
			[3-9][1-9])
				sig_strength="> -51 dBm"
		;;
			*)
				sig_strength=`cat /etc/gcom/3g_signallist | grep "^$signal:" | cut -d : -f 2` >/dev/null 2>&1
		;;
		esac
	else
		sig_strength=""
	fi

	sig_strength=`echo $sig_strength | sed "s/[a-z,A-Z,\>,\ ]//g"`
    
	if [ "`echo $carrier | sed s/[0-9]//g`" = "" ]; then
		carrier=`cat /etc/gcom/3g_carrierlist | grep "$carrier" | cut -d : -f 1` >/dev/null 2>&1
	fi
    
	case $carrier in
		*FFFFFFFF*)
		    carrier="Unknown"
		;;
	esac

	logger -t mobile -p local0.info "$interface: manufacture=$manufacture"
	logger -t mobile -p local0.info "$interface: service=$service"
	logger -t mobile -p local0.info "$interface: card_info=$card_info"
	logger -t mobile -p local0.info "$interface: firmware=$firmware"
	logger -t mobile -p local0.info "$interface: imsi=$imsi"
	logger -t mobile -p local0.info "$interface: sim_status=$sim_status"
	logger -t mobile -p local0.info "$interface: carrier=$carrier"
	logger -t mobile -p local0.info "$interface: service_name=$service_name"
	logger -t mobile -p local0.info "$interface: sig_strength=$sig_strength"
	logger -t mobile -p local0.info "$interface: driver_version=$driver_version"
	logger -t mobile -p local0.info "$interface: dialnumber=$dialnumber"
	logger -t mobile -p local0.info "$interface: apn=$apn"

	# Update the confd
	`sh /usr/bin/update_mobile.sh "add" "$modem" "$manufacture" "$firmware" "$sim_status" "$imsi" "$carrier" "$service_name" "$sig_strength" "$card_info"` >/dev/null 2>&1
}

proto_directip_teardown() {
	local interface="$1"

	local device
	json_get_vars device

	[ -n "$ctl_device" ] && device=$ctl_device

	gcom -d "$device" -s /etc/gcom/directip-stop.gcom || proto_notify_error "$interface" CONNECT_FAILED

	proto_init_update "*" 0
	proto_send_update "$interface"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol directip
}
