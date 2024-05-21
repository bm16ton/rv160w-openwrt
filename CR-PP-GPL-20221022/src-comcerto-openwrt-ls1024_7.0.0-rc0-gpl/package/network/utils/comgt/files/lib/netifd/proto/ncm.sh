#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_ncm_init_config() {
	no_device=1
	available=1
	proto_config_add_string "device:device"
	proto_config_add_string apn
	proto_config_add_string auth
	proto_config_add_string username
	proto_config_add_string password
	proto_config_add_string pincode
	proto_config_add_string delay
	proto_config_add_string mode
}

proto_ncm_setup() {
	local interface="$1"

	local manufacturer initialize setmode connect ifname devname devpath
	local device apn auth username password pincode delay mode
	json_get_vars device apn auth username password pincode delay mode
	
	if [ "$proto" = "qmi" ];then
		sleep 1
		`gcom info -d "/dev/ttyUSB0" > /tmp/stats/4g/MANUFACTURE_modem1` >/dev/null 2>&1
		imei=`cat /tmp/stats/4g/MANUFACTURE_modem1 | grep IMEI: | head -n 1 | cut -d : -f 2 |  sed "s/[\r\n]*//g" | sed -e "s/^ *//g"`
		imei=`echo ${imei:0:6}`
	fi

	#The e3276s-920 donle deal with
	if [ "$imei" = "867497" ];then
		device="ttyUSB1"
	else
		device=`cat /tmp/TTY_USB1 | head -n 1`
	fi
	device="/dev/"$device
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
              
	[ -n "$apn" ] || {
		apn="internet"
	}

	devname="$(basename "$device")"
	case "$devname" in
	'tty'*)
		devpath="$(readlink -f /sys/class/tty/$devname/device)"
		ifname="$( ls "$devpath"/../../*/net )"
		;;
	*)
		devpath="$(readlink -f /sys/class/usbmisc/$devname/device/)"
		ifname="$( ls "$devpath"/net )"
		;;
	esac
	[ -n "$ifname" ] || {
		logger -t mobile -p local0.error "The interface could not be found."
		proto_notify_error "$interface" NO_IFACE
		proto_set_available "$interface" 0
		return 1
	}

	[ -n "$delay" ] && sleep "$delay"

	manufacturer=`gcom -d "$device" -s /etc/gcom/getcardinfo.gcom | awk '/Manufacturer/ { print tolower($2) }'`
	[ $? -ne 0 ] && {
		logger -t mobile -p local0.error "Failed to get modem information"
		proto_notify_error "$interface" GETINFO_FAILED
		return 1
	}
	json_load "$(cat /etc/gcom/ncm.json)"
	json_select "$manufacturer"
	[ $? -ne 0 ] && {
		logger -t mobile -p local0.error "Unsupported modem"
		proto_notify_error "$interface" UNSUPPORTED_MODEM
		proto_set_available "$interface" 0
		return 1
	}
	json_get_values initialize initialize
	for i in $initialize; do
		eval COMMAND="$i" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
			logger -t mobile -p local0.error "Failed to initialize modem"
			proto_notify_error "$interface" INITIALIZE_FAILED
			return 1
		}
	done

	[ -n "$pincode" ] && {
		PINCODE="$pincode" gcom -d "$device" -s /etc/gcom/setpin.gcom || {
			logger -t mobile -p local0.error "Unable to verify PIN"
			proto_notify_error "$interface" PIN_FAILED
			proto_block_restart "$interface"
			return 1
		}
	}
	[ -n "$mode" ] && {
		json_select modes
		json_get_var setmode "$mode"
		COMMAND="$setmode" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
			logger -t mobile -p local0.error "Failed to set operating mode"
			proto_notify_error "$interface" SETMODE_FAILED
			return 1
		}
	}

	json_get_vars connect
	eval COMMAND="$connect" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
		logger -t mobile -p local0.error "Failed to connect"
		proto_notify_error "$interface" CONNECT_FAILED
		return 1
	}

	logger -t mobile -p local0.debug "$interface: Starting DHCP"

	proto_init_update "$ifname" 1
	proto_send_update "$interface"
	
	update_ncm_device_info

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
}

update_ncm_device_info() {

	# Manufacture & Firmware                                                      
        `gcom info -d $device > /tmp/stats/4g/MANUFACTURE_modem1` >/dev/null 2>&1   
        # SIM STATUS                                                                                              
       `gcom PIN -d $device > /tmp/stats/4g/PIN_modem1` >/dev/null 2>&1
                                                                           
        # Signal Strength                                                                                 
        `gcom sig -d $device > /tmp/stats/4g/STRENGTH_modem1` >/dev/null 2>&1                                   
                                                                                                          
        # IMSI                                                                      
        `gcom -d $device -s /etc/gcom/getimsi.gcom > /tmp/stats/4g/IMSI_modem1` >/dev/null 2>&1
                                                                                      
        # Carrier                                                                      
        `gcom -d $device -s /etc/gcom/getcarrier.gcom > /tmp/stats/4g/CARRIER_modem1` >/dev/null 2>&1          
                                                                                     
        # Service Type                                                          
        `gcom -d $device -s /etc/gcom/getservicetype.gcom > /tmp/stats/4g/SERVICE_modem1` >/dev/null 2>&1

	  
	manufacture=`cat /tmp/stats/4g/MANUFACTURE_modem1 | grep Manufacturer: | head -n 1 | cut -d : -f 2 |  sed "s/[\r\n]*//g" | sed -e "s/^ *//g"` >/dev/null 2>&1
	card_info=`cat /tmp/stats/4g/MANUFACTURE_modem1 | grep Model: | head -n 1 | cut -d : -f 2 |  sed "s/[\r\n]*//g" | sed -e "s/^ *//g"` >/dev/null 2>&1
	firmware=`cat /tmp/stats/4g/MANUFACTURE_modem1 | grep Revision: | head -n 1 | cut -d : -f 2- | sed "s/[\r\n]*//g" | sed -e "s/^ *//g"` >/dev/null 2>&1
	sim_status=`cat /tmp/stats/4g/PIN_modem1` >/dev/null 2>&1
	carrier=`cat /tmp/stats/4g/CARRIER_modem1 | grep COPS: | cut -d , -f 3 | sed -e 's/"//g'` >/dev/null 2>&1
	imsi=`cat /tmp/stats/4g/IMSI_modem1` >/dev/null 2>&1
	service_name="4G"
	signal=`cat /tmp/stats/4g/STRENGTH_modem1 | grep "Signal Quality:" | cut -d : -f 2 | cut -d , -f 1 | sed -e "s/^ *//g"` >/dev/null 2>&1
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

	# Update the confd          
 	`sh /usr/bin/update_mobile.sh "add" "/dev/modem1" "$manufacture" "$firmware" "$sim_status" "$imsi" "$carrier" "$service_name" "$sig_strength" "$card_info"` >/dev/null 2>&1                                                   
}

proto_ncm_teardown() {
	local interface="$1"

	local manufacturer disconnect

	local device
	imei=`cat /tmp/stats/4g/MANUFACTURE_modem1 | grep IMEI: | head -n 1 | cut -d : -f 2 |  sed "s/[\r\n]*//g" | sed -e "s/^ *//g"`
	imei=`echo ${imei:0:6}`
	if [ "$imei" = "867497" ];then
		device="/dev/ttyUSB1"
	else
		device="/dev/ttyUSB0"
	fi

	echo "Stopping network"
	manufacturer=`cat /tmp/stats/4g/MANUFACTURE_modem1 | grep Manufacturer: | head -n 1 | cut -d : -f 2- |  sed "s/[\r\n]*//g" | sed -e "s/^ *//g"` >/dev/null 2>&1
	[ $? -ne 0 ] && {
		echo "Failed to get modem information"
		proto_notify_error "$interface" GETINFO_FAILED
		return 1
	}

	json_load "$(cat /etc/gcom/ncm.json)"
	json_select "$manufacturer" || {
		echo "Unsupported modem"
		proto_notify_error "$interface" UNSUPPORTED_MODEM
		return 1
	}

	json_get_vars disconnect
	COMMAND="$disconnect" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
		echo "Failed to disconnect"
		proto_notify_error "$interface" DISCONNECT_FAILED
		return 1
	}

	proto_init_update "*" 0
	proto_send_update "$interface"
}
[ -n "$INCLUDE_ONLY" ] || {
	add_protocol ncm
}
