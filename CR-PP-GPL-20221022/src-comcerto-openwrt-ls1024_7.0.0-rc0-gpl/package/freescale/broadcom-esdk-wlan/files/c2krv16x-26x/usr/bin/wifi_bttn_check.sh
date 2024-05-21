#!/bin/sh

wifi_button="$(cat /sys/class/gpio/gpio6/value)"

WIFI_ON="0"
WIFI_OFF="1"

if [ "$wifi_button" = "$WIFI_ON" ];then
	echo "1"
else
	echo "0"
fi
