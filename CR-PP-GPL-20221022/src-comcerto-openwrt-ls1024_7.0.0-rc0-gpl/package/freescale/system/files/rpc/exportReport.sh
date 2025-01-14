#!/bin/sh

LOG_DIR="/tmp/log"
LOG="$LOG_DIR/messages"
STATS_DIR="/tmp/stats/"
TECH_REPORT_TMP_DIR="/tmp/techreport"
CONFD_LOGS_DIR="/tmp/confd"
DNS_LOOKUP="/tmp/utility-dnslookup"
PING="/tmp/utility-ping"
TRACE_ROUTE="/tmp/utility-traceroute"
IFACE_STATS=interface-stats
IPTABLES_CMM_ENTRIES=iptables-cmm-entries
VAR_STATE_DIR=/var/state
CONFIG_DIR=/tmp/etc/config
WIRELESS_COUNTERS=wireless-counters
PORT_STATISTICS=port_statics


pid=`uci get systeminfo.sysinfo.pid`
current_time=`date +%Y%m%d`
dev_pid=`echo $pid | cut -f 1 -d -`

tech_report_file1="TechReport_$pid"
tech_report_file2="_$current_time"
TECH_REPORT_FILE="$tech_report_file1$tech_report_file2"

if [ ! -d "$LOG_DIR" ];then
        mkdir $LOG_DIR
fi

if [ ! -d "$TECH_REPORT_TMP_DIR" ];then
        mkdir $TECH_REPORT_TMP_DIR
	touch $TECH_REPORT_TMP_DIR/$PORT_STATISTICS
fi

if [ -f "$PING" ]; then
        cp  -rf $PING $TECH_REPORT_TMP_DIR
fi

if [ -f "$TRACE_ROUTE" ]; then
        cp  -rf $TRACE_ROUTE $TECH_REPORT_TMP_DIR
fi

if [ -f "$DNS_LOOKUP" ]; then
        cp  -rf $DNS_LOOKUP $TECH_REPORT_TMP_DIR
fi

cp -rf $STATS_DIR $TECH_REPORT_TMP_DIR
cp -rf $CONFD_LOGS_DIR/confd.log $TECH_REPORT_TMP_DIR
cp -rf $CONFD_LOGS_DIR/devel.log $TECH_REPORT_TMP_DIR
rm -rf $TECH_REPORT_TMP_DIR/confd/cdb

for f in /tmp/*.core
do
   cp -v "$f" $TECH_REPORT_TMP_DIR
done

#IPTABLES
touch $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n##################output of iptables -L -nv ##############\n" > $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
iptables -L -nv >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES

echo -e "\n##################output of iptables -L -nv -t nat ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
iptables -L -nv -t nat >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES

echo -e "\n##################output of iptables -L -nv -t mangle ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
iptables -L -nv -t mangle >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES

#CMM ENTRIES
echo -e "\n################## nf_conntract entries ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cat /proc/net/nf_conntrack >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c query sa" ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c query sa >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "##### xfrm state" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
ip xfrm state >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "##### xfrm polocy" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
ip xfrm policy >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c query connections" ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c query connections >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c query tunnels" ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c query tunnels >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c query pppoe" ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c query pppoe >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c query qm " ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c query qm >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c query swqos " ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c query swqos >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c show stat ipsec query " ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c show stat ipsec query >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES

# PoE related infomration
if [ "$dev_pid" == "RV345P" ] || [ "$dev_pid" == "RV260P" ]
then
cp /proc/poe/showallreg -f $TECH_REPORT_TMP_DIR/PoEPortReg
cp /proc/poe/showallstats -f $TECH_REPORT_TMP_DIR/PoEStatistics
fi

#INTERFACE STATS
echo -e "\n################## inface statistics ##############\n" > $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## ifconfig entries ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
/sbin/ifconfig >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of /proc/net/dev entries ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
cat /proc/net/dev >>$TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ip -s link entries ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ip -s link >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ip -s link entries ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ip -s link >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of /etc/hosts ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
cat /etc/hosts >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of /etc/resolv.conf ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
cat /etc/resolv.conf >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of /tmp/dhcp.leases ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
cat /tmp/dhcp.leases >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of route -n ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
route -n >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ip route show ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ip route show  >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ip route show table 200 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ip route show table 220 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of mwan3 status ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
mwan3 status >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of arp -n ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
arp -n >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ip neigh show ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ip neigh show >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of /etc/nsswitch.conf ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
cat /etc/nsswitch.conf >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ip addr show ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ip addr show >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ps -w ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ps -w >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ubus list ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ubus list >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ifstatus wan1 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ifstatus wan1 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ifstatus usb1 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ifstatus usb1 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ifstatus usb1_4 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ifstatus usb1_4 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ethtool -d eth2 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ethtool -d eth2 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ethtool -d eth0 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ethtool -d eth0 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS

echo -e "\n################## Port stats and regs ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
if [ -f "/tmp/wan1reg" ]; then
	echo "wan1reg" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
	cat /tmp/wan1reg >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
fi
if [ -f "/tmp/eth0_eee_dbg" ]; then
	echo "eth0_eee_dbg" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
	cat /tmp/eth0_eee_dbg >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
fi
if [ -f "/tmp/wan2reg" ]; then
	echo "wan2reg" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
	cat /tmp/wan2reg >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
fi
if [ -f "/tmp/eth2_eee_dbg" ]; then
	echo "eth2_eee_dbg" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
	cat /tmp/eth2_eee_dbg >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
fi
echo -e "\n################## Output of mmiotool -R 0x9c200000 0x400 for eth0/MAC1 ############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
mmiotool -R 0x9c200000 0x400 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## Output of mmiotool -R 0x9c220000 0x400 for eth2/MAC2 ############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
mmiotool -R 0x9c220000 0x400 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS

echo -e "\n################## out put of ipsec statusall ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ipsec statusall >> $TECH_REPORT_TMP_DIR/$IFACE_STATS

#WIRELESS STATS
if [ "$dev_pid" == "RV340W" ] || [ "$dev_pid" == "RV260W" ] || [ "$dev_pid" == "RV160W" ]
then
echo -e "\n################## out put of wl -i wl0 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

wl -i wl0 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0.1 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.1 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.2 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.2 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.3 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.3 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.1 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.1 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.2 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.2 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.3 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.3 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.1 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.1 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.2 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.2 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.3 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.3 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.1 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.1 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.2 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.2 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.3 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.3 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.1 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.1 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.2 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.2 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.3 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.3 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.1 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.1 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.2 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.2 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.3 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.3 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.1 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.1 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.2 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.2 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.3 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.3 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.1 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.1 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.2 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.2 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.3 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.3 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.1 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.1 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.2 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.2 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.3 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.3 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.1 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.1 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.2 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.2 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.3 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.3 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.1 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.1 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.2 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.2 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.3 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.3 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.1 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.1 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.2 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.2 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.3 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.3 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 wsec ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 wsec >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 wsec ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 wsec >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 rate ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 rate >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0 maxrate ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 maxrate >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 rate ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 rate >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 maxrate ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 maxrate >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 keys ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 keys >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 keys ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 keys >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 counters ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 counters >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 counters ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 counters >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

for radio in 0 1
do
  iface=""
  for i in 0 1 2 3
  do
    if [ $i == 0 ]
    then
      iface="wl$radio"
    else
      iface="wl$radio.$i"
    fi
    echo -e "\n################## out put of wl -i $iface sta_info <mac> ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

    for j in $(wl -i $iface assoclist | cut -d " " -f 2)
    do
      wl -i $iface sta_info $j >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
    done
  done
done
echo -e "\n################## out put of nvram show ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
nvram show >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of uci show wireless ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
uci show wireless >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of uci show lobby ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
uci show lobby >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

#Exit if the interface is not up
if [ "$(wl -i wl0 isup)" = "1" ];then
   echo -e "\n################## output of wl -i wl0 dump all ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
   wl -i wl0 dump all 2&>1 >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
fi
if [ "$(wl -i wl1 isup)" = "1" ];then
   echo -e "\n################## output of wl -i wl1 dump all ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
   wl -i wl1 dump all 2&>1 >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
fi

# To enable wireless debug logs, Enable one of below debugging snippets by altering the hard coded '! TRUE' conditions.
# This can be achieved by simply removing '!' from the below hard coded conditions.
if [ ! TRUE ]; then
   #Debugging snippet1: This snippet performs more detailed wifi logging+restart infra for complex debugging
   cp /tmp/wl0_cfg* $TECH_REPORT_TMP_DIR
   cp /tmp/wl0_stats* $TECH_REPORT_TMP_DIR

   if [ "$(ps -w | grep debug_wireless | grep -v grep)" == "" ];then
	/usr/bin/debug_wireless_24g.sh
   fi
elif [ ! TRUE ]; then
	#Debugging snippet2: This snippet performs only minimal log collection of 5 mins.
	cp /tmp/wl0_debug_logs $TECH_REPORT_TMP_DIR
	cp /tmp/wl1_debug_logs $TECH_REPORT_TMP_DIR

	if [ "$(ps -w | grep debug_wireless | grep -v grep)" == "" ];then
		/usr/bin/debug_wireless.sh
	fi
fi

## RV260W block end
fi

#LAN PORT STATS
/usr/sbin/bcmssdk -Z 1 -d 1 &

cp -rf $VAR_STATE_DIR $TECH_REPORT_TMP_DIR
cp -rf $CONFIG_DIR  $TECH_REPORT_TMP_DIR
cp -rf /tmp/update.sh $TECH_REPORT_TMP_DIR
cp -rf /tmp/etc/ipsec.conf $TECH_REPORT_TMP_DIR
cp -rf /etc/strongswan.d/charon.conf $TECH_REPORT_TMP_DIR
kill -SIGUSR1 `ps | grep vpnTimer | grep -v grep | awk '{print $1}'`

if [ -f "$LOG" ]; then
        cp  -f $LOG $TECH_REPORT_TMP_DIR
        cp  -f $LOG_DIR/messagesBK $TECH_REPORT_TMP_DIR
fi

sh /usr/bin/guiReport.sh >/dev/null 2>&1
mv $LOG_DIR/guiReport* $TECH_REPORT_TMP_DIR

df -h > $TECH_REPORT_TMP_DIR/df_output

cd $LOG_DIR

tar czf $TECH_REPORT_FILE.tar.gz $TECH_REPORT_TMP_DIR

# Encrypting with password protection
openssl enc -aes-256-cbc -salt -in $TECH_REPORT_FILE.tar.gz -out $TECH_REPORT_FILE.bin -pass pass:C$bTsrd@T^
 
# Decrypting with password
#openssl enc -aes-256-cbc -d -in TechReport_RV160W-I-K9_20180330.bin -out TechReport_RV160W-I-K9_20180330.tar.gz -pass pass:C$bTsrd@T^

rm -f $TECH_REPORT_FILE.tar.gz
rm -rf $TECH_REPORT_TMP_DIR
