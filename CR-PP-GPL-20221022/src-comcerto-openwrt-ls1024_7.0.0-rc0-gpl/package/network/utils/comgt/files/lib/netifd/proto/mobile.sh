#!/bin/sh
# 2017-08-22: li.zhang <li.zhang@deltaww.com>
# Porting the E3276
# 2017-07-21: li.zhang <li.zhang@deltaww.com>
# Add the special nat dongle
# 2017-05-11: li.zhang <li.zhang@deltaww.com>
# fix bug72571 huawei e161 can't connect to internet and adjuct code structure
# 2017-05-03: li.zhang <li.zhang@deltaww.com>
# fix bug of the mobile.sh getting called repeatedly when booting
# 2017-04-01: li.zhang <li.zhang@deltaww.com>
# fix bug of some carrier is network identification number and change the handled of auto authentication
# 2017-03-24: li.zhang <li.zhang@deltaww.com>
# modify the sig_strength value
# 2017-03-13: li.zhang <li.zhang@deltaww.com>
# the initial auto connection need files
INCLUDE_ONLY=1

. ../netifd-proto.sh
. ../nat.script
. ./ppp.sh
. ./qmi.sh
. ./directip.sh
. ./ncm.sh
init_proto "$@"

USB_CONN_STATUS=/var/USBCONNSTATUS

proto_mobile_init_config() {
	no_device=1
	available=1
	ppp_generic_init_config
	proto_config_add_string "device:device"
	proto_config_add_string "apn"
	proto_config_add_string "service"
	proto_config_add_string "pincode"
	proto_config_add_string "dialnumber"
	proto_config_add_string "global_enable"
	proto_config_add_string "auth"
}
proto_mobile_setup() {
	local interface="$1"
	if [ ! -f "/var/USBCONNSTATUS" ];then
		proto_set_available "$interface" 0
		return 1
	fi

	#The e3276s-920 dongle with china unicom sim card 
	temp=`lsusb | grep "Bus 003" | grep -v "Device 001" | cut -d " " -f 6`
	if [ "$temp" = "12d1:1506" ];then
		sleep 1
		`gcom info -d "/dev/ttyUSB0" > /tmp/stats/4g/MANUFACTURE_modem1` >/dev/null 2>&1
		`gcom -d "/dev/ttyUSB0" -s /etc/gcom/getimsi.gcom > /tmp/stats/4g/IMSI_modem1` >/dev/null 2>&1
		imei=`cat /tmp/stats/4g/MANUFACTURE_modem1 | grep IMEI: | head -n 1 | cut -d : -f 2 |  sed "s/[\r\n]*//g" | sed -e "s/^ *//g"`
		imsi=`cat /tmp/stats/4g/IMSI_modem1`
		imei=`echo ${imei:0:6}`
		imsi=`echo ${imsi:0:5}`
		#The e3276s-920 dongle
		if [ "$imei" = "867497" ];then
			#china unicom
			if [ "$imsi" = "46001" ] || [ "$imsi" = "46006" ];then
				mobile_3g_setup $1
				return $?
			fi
		fi
		proto_qmi_setup $1
		return $?
	fi

        detect=$(cat /var/USBCONNSTATUS | awk -F: '{printf $4}')
	if [ "$detect" = "3G" ];then
		mobile_3g_setup $1
		return $?
	elif [ "$detect" = "4G" ];then
		proto_qmi_setup $1
		return $?
	fi
	proto_set_available "$interface" 0
	return 1
}

mobile_3g_setup() {
	local interface="$1"
	local chat
	local got_success=0
	local is_config=0
	local usb_port=0

	json_get_var device device
	json_get_var apn apn
	#json_get_var service service
	json_get_var pincode pincode
	json_get_var dialnumber dialnumber
	json_get_var global_enable global_enable
	json_get_var auth auth

	if [ "$global_enable" = "0" ];then
		return 1
	fi
	proto=3g
	local modem=`basename $device`
	modem="_"$modem
	local cdma_dongle=0
	local evdo_dongle=0
	local temp=

	# Let all the tty* devices come
	sleep 5

	# We have umts/cdma services.
	service="umts"

	if [ "$interface" = "usb1" ];then
		TTY_FILE=/tmp/TTY_USB1
		usb_port=1
		temp=`lsusb | grep "Bus 003" | grep -v "Device 003" | cut -d " " -f 6`
	elif [ "$interface" = "usb2" ];then
		TTY_FILE=/tmp/TTY_USB2
		usb_port=2
		temp=`lsusb | grep "Bus 003" | grep -v "Device 001" | cut -d " " -f 6`
	fi

	while read line
	do
		is_config=1
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

	[ "$is_config" = "0" ] && {
		proto_set_available "$interface" 0
		return 1
	}

	[ "$is_config" = "1" ] && {
		[ "$got_success" = 0 ] && {
			InterruptDevice=`cat $TTY_FILE | head -n 1`
			logger -t mobile -p local0.info "$interface: selecting Default $InterruptDevice for communication"	
		}
		# Make link
		ln -sf /dev/$InterruptDevice $device
		
		# Update USB status
		usbconnstatus=`cat $USB_CONN_STATUS | grep USB${usb_port}`
		if [ -n "$usbconnstatus" ];then
			`sed -i "/^USB$usb_port:/c\USB$usb_port:$line:$device:3G:1" $USB_CONN_STATUS`
		else
			echo "USB$usb_port:$line:$device:3G:1" >> $USB_CONN_STATUS
		fi
	}

	[ -e "$device" ] || {
		proto_set_available "$interface" 0
		return 1
	}

	# Write the other COM PORT to file which we use to get Signal Strength at run time
	other_comport=`cat $TTY_FILE | grep -v "$InterruptDevice" | tail -n 1` >/dev/null 2>&1
	
	echo "/dev/${other_comport}" > /tmp/stats/3g/COMPORT${modem}

	# Manufacture & Firmware
	`gcom info -d $device > /tmp/stats/3g/MANUFACTURE$modem` >/dev/null 2>&1
    
	manufacture=`cat /tmp/stats/3g/MANUFACTURE$modem | grep Manufacturer: | head -n 1 | cut -d : -f 2- |  sed "s/[\r\n]*//g" | sed -e "s/^ *//g"` >/dev/null 2>&1
	card_info=`cat /tmp/stats/3g/MANUFACTURE$modem | grep Model: | head -n 1 | cut -d : -f 2- |  sed "s/[\r\n]*//g" | sed -e "s/^ *//g"` >/dev/null 2>&1
	firmware=`cat /tmp/stats/3g/MANUFACTURE$modem | grep Revision: | head -n 1 | cut -d : -f 2- | sed "s/[\r\n]*//g" | sed -e "s/^ *//g"` >/dev/null 2>&1

	[ "$cdma_dongle" = 1 ] && service="cdma"

	# EC169 dongle
	[ "$card_info" = "EC169" ] && {
		service="evdo"
		manufacture=`echo $manufacture | cut -d : -f 2` >/dev/null 2>&1
		firmware=`echo $firmware | cut -d : -f 2` >/dev/null 2>&1
	}

	logger -t mobile -p local0.info "$interface: Using Device1:$InterruptDevice Device2=$other_comport service=$service"
		
	case "$service" in
		cdma|evdo)
			logger -t mobile -p local0.info "$interface: dailing cdma service"
			chat="/etc/chatscripts/evdo.chat"
			# set the dialnumber and apn for auto mode for cdma dongles.
			[ -z "$apn" ] && apn="internet"
			[ -z "$dialnumber=" ] && dialnumber="#777"
		;;
		*)
			logger -t mobile -p local0.info "$interface: dailing umts service"
			chat="/etc/chatscripts/3g.chat"
			cardinfo=$(gcom -d "$device" -s /etc/gcom/getcardinfo.gcom)
			if echo "$cardinfo" | grep -q Novatel; then
				case "$service" in
					umts_only) CODE=2;;
					gprs_only) CODE=1;;
					*) CODE=0;;
				esac
				export MODE="AT\$NWRAT=${CODE},2"
			elif echo "$cardinfo" | grep -q Option; then
				case "$service" in
					umts_only) CODE=1;;
					gprs_only) CODE=0;;
					*) CODE=3;;
				esac
				export MODE="AT_OPSYS=${CODE}"
			elif echo "$cardinfo" | grep -q "Sierra Wireless"; then
				SIERRA=1
			elif echo "$cardinfo" | grep -qi huawei; then
				case "$service" in
					umts_only) CODE="14,2";;
					gprs_only) CODE="13,1";;
					*) CODE="2,2";;
				esac
				export MODE="AT^SYSCFG=${CODE},3FFFFFFF,2,4"
			fi

			if [ -n "$pincode" ]; then
				PINCODE="$pincode" gcom -d "$device" -s /etc/gcom/setpin.gcom || {
					logger -t mobile -p local0.error "$interface: Unable to verify PIN"
					proto_notify_error "$interface" PIN_FAILED
					proto_block_restart "$interface"
					return 1
				}
			fi
			[ -n "$MODE" ] && gcom -d "$device" -s /etc/gcom/setmode.gcom

			# wait for carrier to avoid firmware stability bugs
			[ -n "$SIERRA" ] && {
				gcom -d "$device" -s /etc/gcom/getcarrier.gcom || return 1
			}

			if [ -z "$dialnumber" ]; then
				dialnumber="*99***1#"
			fi

		;;
	esac

	# Get all the required statistics
	
	# SIM STATUS
	`gcom PIN -d $device > /tmp/stats/3g/PIN$modem` >/dev/null 2>&1
	
	# Signal Strength
	#E161 delay
	if [ "$card_info" = "E161" ];then
	    sleep 3
	fi
	`gcom sig -d $device > /tmp/stats/3g/STRENGTH$modem` >/dev/null 2>&1
	
	# IMSI
	`gcom -d $device -s /etc/gcom/getimsi.gcom > /tmp/stats/3g/IMSI$modem` >/dev/null 2>&1
	
	# Carrier
	`gcom -d $device -s /etc/gcom/getcarrier.gcom > /tmp/stats/3g/CARRIER$modem` >/dev/null 2>&1
	
	# Service Type
	`gcom -d $device -s /etc/gcom/getservicetype.gcom > /tmp/stats/3g/SERVICE$modem` >/dev/null 2>&1
	
	# If apn is null, get from ISP
	if [ -z "$apn" ];then
		apn=`cat /tmp/stats/3g/MANUFACTURE$modem | grep APN: | cut -d , -f 3 | sed -e 's/"//g'`
		[ -z "$apn" ] && apn="internet"
	fi

	# Update the 3G statistics
	sim_status=`cat /tmp/stats/3g/PIN$modem` >/dev/null 2>&1
	imsi=`cat /tmp/stats/3g/IMSI$modem` >/dev/null 2>&1
	carrier=`cat /tmp/stats/3g/CARRIER$modem | grep COPS: | cut -d , -f 3 | sed -e 's/"//g'` >/dev/null 2>&1
	service_name="3G"
	signal=`cat /tmp/stats/3g/STRENGTH$modem | grep "Signal Quality:" | cut -d : -f 2 | cut -d , -f 1 | sed -e "s/^ *//g"` >/dev/null 2>&1
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
	
	[ "$carrier" = "" ] && carrier=`echo $imsi | cut -c 1-5` >/dev/null 2>&1

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
	`sh /usr/bin/update_mobile.sh "add" "$device" "$manufacture" "$firmware" "$sim_status" "$imsi" "$carrier" "$service_name" "$sig_strength" "$card_info"` >/dev/null 2>&1
	
	case $auth in
	PAP)
		auth="+pap"
		;;
	CHAP)
		auth="+chap"
		;;
	MSCHAP)
		auth="+mschap"
		;;
	MSCHAP2)
		auth="+mschap-v2"
		;;
	auto)
		auth="noauth"
		;;
	none)
		auth="noauth"
		;;
	esac

	connect="${apn:+USE_APN=$apn }DIALNUMBER=$dialnumber /usr/sbin/chat -t5 -v -E -f $chat"
	ppp_generic_setup "$interface" \
		noaccomp \
		nopcomp \
		novj \
		nobsdcomp \
		lock \
		crtscts \
		$auth \
		115200 "$device"
	return 0
}

proto_mobile_teardown() {
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

add_protocol mobile
