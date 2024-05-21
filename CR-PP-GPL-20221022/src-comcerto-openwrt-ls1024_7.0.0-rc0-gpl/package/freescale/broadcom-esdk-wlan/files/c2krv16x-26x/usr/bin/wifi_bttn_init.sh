#!/bin/sh

WIFI_BUTTON="gpio6"
WIFI_BUTTON_VALUE="6"

[ ! -d "/sys/class/gpio/${WIFI_BUTTON}" ] && {
	echo ${WIFI_BUTTON_VALUE} >/sys/class/gpio/export;
	echo in >/sys/class/gpio/${WIFI_BUTTON}/direction
}
