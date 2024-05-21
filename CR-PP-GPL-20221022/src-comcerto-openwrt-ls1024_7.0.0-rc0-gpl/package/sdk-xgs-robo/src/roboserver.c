/*
 * $Id: socdiag.c,v 1.25 Broadcom SDK $
 * $Copyright: Copyright 2016 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 * robodiag: UDP server to configure the switch.

 */
/** NXP changes **/
#include <stdio.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <getopt.h>
#include <shared/bsl.h>
#include <shared/bslenum.h>

#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <soc/l2x.h>
#include <soc/drv.h>

#include <bcm/error.h>
#include <bcm/init.h>
#include <bcm/link.h>
#include <bcm/trunk.h>
#include <bcm/auth.h>
#include <bcm/l2.h>
#include <bcm/types.h>
#include <bcm/stat.h>
#include <bcm/cosq.h>
#include <bcm/qos.h>

#include <appl/diag/dport.h>
#include <bcm_int/robo/vlan.h>
#include <bcm_int/robo/port.h>

#include <syslog.h>

#define DEFAULT_UNIT_INIT_ID 0
#define INT32_MAX_BYTES 10
#define CONFIG_ERROR 1


/* Memory for TRUNK  */
bcm_trunk_member_t *group_0_member_array = NULL;
bcm_trunk_member_t *group_1_member_array = NULL;

/* Link monitor changes for 802.1x */
#define NXP_LINK_MON_LOOPBACK_SOCK_FD  45678

int32_t nxp_port_linkMon_callback(int unit, bcm_port_t port, bcm_port_info_t *info);
extern int bcm_robo_linkscan_register(int unit, bcm_linkscan_handler_t f);
extern int bcm_robo_linkscan_unregister(int unit, bcm_linkscan_handler_t f);


/* Char Driver changes starts */
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <linux/unistd.h>
#include <string.h>

/** For IOCTLS **/
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#define ROBO_DRIVER_TYPE 'R'
typedef struct tag_ROBO_OBJ_CMD_MSG ROBO_OBJ_CMD_MSG;

typedef enum Cmdmsg
{
DeviceRead,
DeviceWrite,
QosStats,
IMP_Enable,
IMP_Disable,
Default_Reg
}Cmdmsg;

struct tag_ROBO_OBJ_CMD_MSG
{
Cmdmsg       commandMsg;
unsigned int Page_addr;
unsigned int Reg_addr;
unsigned int Reg_value;
unsigned int port;
unsigned int length;
uint32_t Qos_stats[4];
};

/** The combination is made keeping in mind the unique IOC CMD otherwise other device may do something **/
#define ROBO_IOC_CMD       _IOW( ROBO_DRIVER_TYPE, 1, ROBO_OBJ_CMD_MSG )

#define DEVNAME  "RoboDev"

#define ROBODRV   "/dev/" DEVNAME

/* Char Driver changes ends */

//802.1x related
#include <bcm_int/robo/auth.h>
#define AUTH_SEC_EXTEND_MODE   5 
#define AUTH_SEC_SIMPLFY_MODE  6

#define BUFFER_SIZE 1024
#define SERVER_PORT 27876 

#define BCM53128_MIN_PORT 0
#define BCM53128_MAX_PORT 7


#define BCM53134_MIN_PORT 0
#define BCM53134_MAX_PORT 3

#define BCM_SWITCH_QUEUE_NO 4
#define BCM_MIN_QUEUE_NO    1
#define BCM_MAX_QUEUE_NO    4

#define BCM_SW_ALL_PORTS -1

uint16_t sp_flag[8];
uint16_t sp_queue[8];

uint16_t total_ports_g;
uint8_t rv_prod_type_g;

enum {
 PROD_TYPE_RV_160,
 PROD_TYPE_RV_260,
};

/** NXP Changes finished ***/

#include <unistd.h>
#include <stdlib.h>

#include <sal/core/boot.h>
#include <sal/appl/sal.h>
#include <sal/appl/config.h>
#include <sal/appl/pci.h>
#include <soc/debug.h>
#include <shared/shr_bprof.h>

#include <appl/diag/system.h>

#include <linux-bde.h>

#if defined(MEMLOG_SUPPORT) && defined(__GNUC__)
#include <sal/core/memlog.h>
#include <sys/stat.h>
#include <fcntl.h>
int memlog_fd = 0;
char memlog_buf[MEM_LOG_BUF_SIZE];
sal_mutex_t memlog_lock = NULL;
#endif

#define SYSTEM_INIT_CHECK(action, description) \
    if ((rv = (action)) < 0) { \
        msg = (description); \
        goto done; \
    }

void usage(const char *name) {
    char *pStr;

    syslog(LOG_DEBUG,"Usage: %s [-hilsvADFITLMSQV]\n\n", (pStr = strrchr(name, '/'))? pStr + 1: name);
}

extern int vlan_info[BCM_MAX_NUM_UNITS];


extern int bcm_robo_init(int);
extern int bcm_robo_port_config_get(int,bcm_port_config_t *);
extern int bcm_robo_port_stp_set(int,bcm_port_t,int);
extern int bcm_port_autoneg_set(int,bcm_port_t,int);
extern int bcm_robo_linkscan_mode_set(int,bcm_port_t,int);
extern int bcm_robo_port_learn_set(int,bcm_port_t,uint32);
extern int bcm_robo_linkscan_enable_set(int,int);

extern int soc_robo_misc_init(int);
extern int soc_robo_mmu_init(int unit);
extern int soc_l2x_start(int unit, uint32 flags, sal_usecs_t interval);
extern int bcm_robo_init_selective(int,uint32);



extern int bcm_robo_vlan_init(int);
extern int bcm_robo_vlan_create(int,bcm_vlan_t);
extern int bcm_robo_vlan_destroy(int,bcm_vlan_t);
extern int bcm_robo_vlan_port_add(int,bcm_vlan_t,bcm_pbmp_t,bcm_pbmp_t);
extern int bcm_robo_vlan_gport_add(int,bcm_vlan_t,bcm_gport_t,int);
extern int bcm_robo_port_gport_get(int unit, bcm_port_t port, bcm_gport_t *gport);


/*** Linkset API's ***/
extern int bcm_robo_port_speed_get(int,bcm_port_t,int *);
extern int bcm_robo_port_speed_set(int,bcm_port_t,int);

/*** Broadcom switch LOOP DETECTION feature ***/
int soc_robo_loop_detect_enable_set(int unit, int enable);
#define BCM_LOOP_DET_ENABLE 1
#define BCM_LOOP_DET_DISABLE 0

/* Set, get, or delete Fabric unicast table entries. */
extern int bcm_stk_ucbitmap_set(
    int unit,
    int port,
    int modid,
    bcm_pbmp_t pbmp);


extern int bcm_robo_port_vlan_member_set(int,bcm_port_t,uint32);

ibde_t *bde;

/* The bus properties are (currently) the only system specific
 * settings required. 
 * These must be defined beforehand 
 */

#ifndef SYS_BE_PIO
#error "SYS_BE_PIO must be defined for the target platform"
#endif
#ifndef SYS_BE_PACKET
#error "SYS_BE_PACKET must be defined for the target platform"
#endif
#ifndef SYS_BE_OTHER
#error "SYS_BE_OTHER must be defined for the target platform"
#endif

#if !defined(SYS_BE_PIO) || !defined(SYS_BE_PACKET) || !defined(SYS_BE_OTHER)
#error "platform bus properties not defined."
#endif

#ifdef INCLUDE_KNET

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <soc/knet.h>
#include <uk-proxy-kcom.h>
#include <bcm-knet-kcom.h>

/** NXP changes **/
int32_t robodiag_start(void);
void usage(const char *name);


extern int bcm_robo_vlan_port_add(int,bcm_vlan_t,bcm_pbmp_t,bcm_pbmp_t);
extern int bcm_robo_vlan_port_get(int,bcm_vlan_t,bcm_pbmp_t *,bcm_pbmp_t *);

/* --------------Port related ------------------- */
extern int bcm_robo_port_init(int);
extern int bcm_robo_port_speed_get(int,bcm_port_t,int *);


/*  NXP Added header files END **/


/* Function defined in linux-user-bde.c */
extern int
bde_irq_mask_set(int unit, uint32 addr, uint32 mask);
extern int
bde_hw_unit_get(int unit, int inverse);

static soc_knet_vectors_t knet_vect_uk_proxy = {
    {
        uk_proxy_kcom_open,
        uk_proxy_kcom_close,
        uk_proxy_kcom_msg_send,
        uk_proxy_kcom_msg_recv
    },
    bde_irq_mask_set,
    bde_hw_unit_get
};

static soc_knet_vectors_t knet_vect_bcm_knet = {
    {
        bcm_knet_kcom_open,
        bcm_knet_kcom_close,
        bcm_knet_kcom_msg_send,
        bcm_knet_kcom_msg_recv
    },
    bde_irq_mask_set,
    bde_hw_unit_get
};

static void
knet_kcom_config(void)
{
    soc_knet_vectors_t *knet_vect;
    char *kcom_name;
    int procfd;
    char procbuf[128];

    /* Direct IOCTL by default */
    knet_vect = &knet_vect_bcm_knet;
    kcom_name = "bcm-knet";

    if ((procfd = open("/proc/linux-uk-proxy", O_RDONLY)) >= 0) {
        if ((read(procfd, procbuf, sizeof(procbuf))) > 0 &&
            strstr(procbuf, "KCOM_KNET : ACTIVE") != NULL) {
            /* Proxy loaded and active */
            knet_vect = &knet_vect_uk_proxy;
            kcom_name = "uk-proxy";
        }
        close(procfd);
    }

    soc_knet_config(knet_vect);
    var_set("kcom", kcom_name, 0, 0);
}

#endif /* INCLUDE_KNET */


#ifdef BCM_INSTANCE_SUPPORT
static int
_instance_config(const char *inst)
{
    const char *ptr;
#ifndef NO_SAL_APPL
    char *estr;
#endif
    unsigned int dev_mask, dma_size;

    if (inst == NULL) {
#ifndef NO_SAL_APPL
        estr = "./bcm.user -i <dev_mask>[:dma_size_in_mb] \n";
        sal_console_write(estr, sal_strlen(estr) + 1);
#endif
        return -1;
    }
    dev_mask = strtol(inst, NULL, 0);
    if ((ptr = strchr(inst,':')) == NULL) {
        dma_size = 4;
    } else {
        ptr++;
        dma_size = strtol(ptr, NULL, 0);
    }

    if (dma_size < 4) {
#ifndef NO_SAL_APPL
        estr = "dmasize must be > 4M and a power of 2 (4M, 8M etc.)\n";
        sal_console_write(estr, sal_strlen(estr) + 1);
#endif
        return -1;
    } else {
        if ( (dma_size >> 2) & ((dma_size >> 2 )-1)) {
#ifndef NO_SAL_APPL
            estr = "dmasize must be a power of 2 (4M, 8M etc.)\n";
            sal_console_write(estr, sal_strlen(estr) + 1);
#endif
            return -1;
        }
    }
    
    return linux_bde_instance_attach(dev_mask, dma_size);
}
#endif

int
bde_create(void)
{
    linux_bde_bus_t bus;
    bus.be_pio = SYS_BE_PIO;
    bus.be_packet = SYS_BE_PACKET;
    bus.be_other = SYS_BE_OTHER;

    return linux_bde_create(&bus, &bde);

}

unsigned char PortsInLag_g;
unsigned char PortsInLag0_g;
unsigned char PortsInLag1_g;

inline void bcm_lag_portbit_set(int portnum )
{
    /* to be called when a port is added to the LAG. */
    PortsInLag_g |= (1 << (portnum));
}

inline void bcm_lag_portbit_set_trunk(int tid )
{
    /* Ports are copied to LAG 0 or LAG1 based on trunk id */
    if(tid==0){
	    PortsInLag0_g = PortsInLag_g;
	    PortsInLag_g =0;
    }
    else
    {
	    PortsInLag1_g = PortsInLag_g;
	    PortsInLag_g =0;
    }
}

inline char bcm_lag_is_portbit_set(int portnum)
{
   /*  To check a port is present in a LAG or not. Returns . 1 : present, 0 : not present. call in linkmonitor*/
    if ((PortsInLag0_g &  (1 << portnum)) || (PortsInLag1_g &  (1 << portnum)))
	    return 1;
    return 0;
}

inline void  bcm_lag_reset_lag(int tid)
{
    /* This function resets all the ports in the LAG0/LAG1 based on trunkid.*/
    if (tid ==0) {
	    PortsInLag0_g = 0;
    }
    else
    {
	    PortsInLag1_g = 0;
    }
}

/* The below code in #if 0 block is for debugging purpose.
 * It can be used to log Port values in LAG0/LAG1
 */ 
#if 0
void DisplayLagAndPorts(int lagid);
void displayportbinary(int lagid);
void displayportmap(int lagid);

void displayportmap(int lagid)
{
    unsigned char n, pos;
    displayportbinary(lagid);
    if(0 == lagid)
        n = PortsInLag0_g;
    else
        n = PortsInLag1_g;

    for (pos = 0; n != 0; n >>= 1, ++pos)
        if (n & 0x01)
            syslog(LOG_DEBUG,"Port : %d is set\n", pos);
}

void displayportbinary(int lagid)
{
    unsigned char n;
    int pos;
    if(0 == lagid)
        n = PortsInLag0_g;
    else
        n = PortsInLag1_g;

    syslog(LOG_DEBUG,"Portmap in Binary : ");
    for (pos = 7; pos >= 0 ; --pos)
        if(n & (1 << pos))
            syslog(LOG_DEBUG,"1 ");
        else
            syslog(LOG_DEBUG,"0 ");
    syslog(LOG_DEBUG,"\n");
}

void DisplayLagAndPorts(int lagid)
{
    syslog(LOG_DEBUG,"****** LAG ID : %d ***************\n", lagid);
    displayportmap(lagid);
}

#endif

/*********************************************************************
 * Function: nxp_port_linkMon_callback
 * Arguments: command line arguments
 * Return value: 
 *      0 for success
 *      -1 for failure
 * Description:
 * This routine is called through bcmssdk command, When 
 * a link change has been detected on LAN ports.
 *********************************************************************/
int32 nxp_port_linkMon_callback(int unit, bcm_port_t port, bcm_port_info_t *info)
{
	struct sockaddr_in addr;
	int                sock;
	unsigned int       link_info[2];
	char cmd_read[64];
	char cmd_write[64];
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, 17)) < 0)
	{
		perror("Callback socket open failed");
		return -1;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(NXP_LINK_MON_LOOPBACK_SOCK_FD);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	link_info[0]  = port;
	link_info[1]  = info->linkstatus;

	if (sendto(sock, (char *)&link_info, sizeof(link_info), 0, (struct sockaddr*)&addr,sizeof(struct sockaddr)) <= 0)
	{
		perror("Link Monitor Sendto Failed");
		close(sock);
		return -1;
	}
	sprintf(cmd_read,"bcmssdk -R -p 0 -a %d -r 1",port);
	system(cmd_read);

	if (rv_prod_type_g  == PROD_TYPE_RV_160)
	{
		syslog(LOG_INFO,"LAN%d interface is now %s",(4-port), info->linkstatus?"UP":"DOWN");
	}
	else if (rv_prod_type_g  == PROD_TYPE_RV_260)
	{
		syslog(LOG_INFO,"LAN%d interface is now %s",(port+1), info->linkstatus?"UP":"DOWN");
		if (info->linkstatus == 0){
			/*check if the linkdown port is present in LAG or not. 
			If yes write a0 into port control register*/
			if(bcm_lag_is_portbit_set(port) == 1){
				sprintf(cmd_write,"bcmssdk -R -p 0 -a %d -v 160 -w 1",port);
				system(cmd_write);
			}
		}
	}
	else
	{
		syslog(LOG_ERR," Invalid PID ");
		return -1;
	}
	system(cmd_read);
	close(sock);
	return 0;
}

/*********************************************************************
 * Function: debug
 * Arguments: command line arguments
 * Return value: 
 *      0 for success
 *      1 for incorrect command line arguments
 *      BCM_E_XXX for switch configuration failures
 * Description:
 * This routine is called through bcmssdk command. This would write  
 * debug information to a file(/tmp/techreport/port_statics) for 
 * analysis on BCM53128M and BCM53134M chipset.
 * [ Use : bcmssdk -Z 1 -d 1]
 *
 *********************************************************************/
int debug(int argc, char *argv[])
{
    int32 retval = 0;
    char *cptr = NULL;
    uint32 portnum = -1,sw_port=-1,i;
    char str[200];
    FILE *fp = NULL;
    int32 speed = 0, duplex = 0,linkStatus = -1, autoneg = -1,opt, jam = -1, txpause, rxpause;

    optind = 0;
    while ((opt = getopt(argc, argv, "d:p:h")) != -1) 
    {
        switch (opt) 
	{
	        case 'p':
	 	errno = 0;
            	portnum = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
               	if (rv_prod_type_g  == PROD_TYPE_RV_160)
                {
                        if (errno != 0 ||  *cptr != '\0' || (portnum < BCM53134_MIN_PORT)  || (portnum > (BCM53134_MAX_PORT + 1)))
                        {
                                syslog(LOG_ERR,"[RV160]Invalid port = %d configuration [Use Port between 1 to 4]", portnum);
                                retval = CONFIG_ERROR;
                                return -1;
                        }
                        sw_port = (4 - portnum);
                }
                else if (rv_prod_type_g  == PROD_TYPE_RV_260)
                {
                        if (errno != 0 ||  *cptr != '\0' || (portnum < BCM53128_MIN_PORT) || (portnum > (BCM53128_MAX_PORT + 1)))
                        {
                                syslog(LOG_ERR,"[RV260]Invalid port = %d configuration [Use Port between 1 to 8]", portnum);
                                retval = CONFIG_ERROR;
                                return -1;
                        }
                        sw_port = (portnum - 1);
                }
                else
                {
                        syslog(LOG_ERR," Invalid PID ");
			return -1;
                }
            	syslog(LOG_DEBUG, "LAN%d=switch port %d", portnum,sw_port);
		break;
		case 'd':
		if (rv_prod_type_g  == PROD_TYPE_RV_160)
                {
			fp = fopen("/tmp/techreport/port_statics","w");
			if(fp == NULL)
			{
				syslog(LOG_ERR,"Could not open file /tmp/techreport/port_statics");
				return -1;
			}
			else
			{	
				fprintf(fp, "Port\tLinkstatus\tspeed\tduplex\tAutoneg\tTxPause\tRxPause\n");
				for(i=0;i<total_ports_g;i++)
				{
					retval = bcm_port_link_status_get(DEFAULT_UNIT_INIT_ID, i, &linkStatus);
                			if(retval != BCM_E_NONE)
                        			syslog(LOG_ERR,"bcm_port_link_status_get failed with retval = %d", retval);
                			
	
					retval = bcm_port_speed_get(DEFAULT_UNIT_INIT_ID, i, &speed);
					if(retval != BCM_E_NONE)
						syslog(LOG_ERR,"bcm_port_speed_get failed with retval = %d", retval);
					
					retval = bcm_port_duplex_get(DEFAULT_UNIT_INIT_ID, i, &duplex);
					if(retval != BCM_E_NONE)
						syslog(LOG_ERR,"bcm_port_duplex_get failed with retval = %d", retval);
					
					retval = bcm_port_autoneg_get(DEFAULT_UNIT_INIT_ID, i, &autoneg);
					if(retval != BCM_E_NONE)
						syslog(LOG_ERR,"bcm_port_autoneg_get failed with retval = %d", retval);
					
					retval = bcm_port_jam_get(DEFAULT_UNIT_INIT_ID, i, &jam);
					if(retval != BCM_E_NONE)
						syslog(LOG_ERR,"bcm_port_jam_set failed with retval = %d", retval);

					retval = bcm_port_pause_get(DEFAULT_UNIT_INIT_ID,i, &txpause, &rxpause);
					if(retval != BCM_E_NONE)
						syslog(LOG_ERR,"switch : bcm_port_pause_set failed with retval = %d", retval);
				
				syslog(LOG_DEBUG,"Port%d:Status=%d,SPEED=%d,duplex=%d,autoneg=%d,jam=%d,txpause=%d,rxpause=%d",i, linkStatus,speed, duplex, autoneg, jam, txpause, rxpause);
				fprintf(fp,"%d\t %d\t\t %d\t %d\t %d\t %d\t %d\t\n",i,linkStatus,speed,duplex,autoneg,txpause,rxpause);
				}
			}
			fclose(fp);
                }
                else if (rv_prod_type_g  == PROD_TYPE_RV_260)
                {
			fp = fopen("/tmp/techreport/port_statics","w");
			if(fp == NULL)
			{
				syslog(LOG_ERR,"Could not open file /tmp/techreport/port_statics");
				return -1;
			}
			else
			{
				fprintf(fp, "Port\tLinkstatus\tspeed\tduplex\tAutoneg\tTxPause\tRxPause\n");
				for(i=0;i<total_ports_g;i++)
				{	
					retval = bcm_port_link_status_get(DEFAULT_UNIT_INIT_ID, i, &linkStatus);
                			if( retval != BCM_E_NONE)
                        			syslog(LOG_ERR,"bcm_port_link_status_get failed with retval = %d", retval);
					
					retval = bcm_port_speed_get(DEFAULT_UNIT_INIT_ID, i, &speed);
					if(retval != BCM_E_NONE)
						syslog(LOG_ERR,"bcm_port_speed_get failed with retval = %d", retval);
					
					retval = bcm_port_duplex_get(DEFAULT_UNIT_INIT_ID, i, &duplex);
					if(retval != BCM_E_NONE)
						syslog(LOG_ERR,"bcm_port_duplex_get failed with retval = %d", retval);
					
					retval = bcm_port_autoneg_get(DEFAULT_UNIT_INIT_ID,i, &autoneg);
					if(retval != BCM_E_NONE)
						syslog(LOG_ERR,"bcm_port_autoneg_get failed with retval = %d", retval);
					
					retval = bcm_port_jam_get(DEFAULT_UNIT_INIT_ID, i, &jam);
					if(retval != BCM_E_NONE)
						syslog(LOG_ERR,"bcm_port_jam_set failed with retval = %d", retval);

					retval = bcm_port_pause_get(DEFAULT_UNIT_INIT_ID, i, &txpause, &rxpause);
					if(retval != BCM_E_NONE)
						syslog(LOG_ERR,"switch : bcm_port_pause_set failed with retval = %d", retval);
						
				syslog(LOG_DEBUG,"Port%d:status=%d,SPEED=%d,duplex=%d,autoneg=%d,jam=%d,txpause=%d,rxpause=%d",i, linkStatus, speed, duplex, autoneg, jam, txpause, rxpause);
				fprintf(fp,"%d\t %d\t\t %d\t %d\t %d\t %d\t %d\t\n",i,linkStatus,speed,duplex,autoneg,txpause,rxpause);
				}
			}
			fclose(fp);
                }
//sprintf(str,"echo LAN %d,%d,%d,%d,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu >>/tmp/techreport/port_statics",portnum, pLinkStatus, pSpeed, pDuplex, pAutoNegotiation,In_Ucast, In_Bcast, In_Mcast, In_Octets, pkt_error_In,Out_Ucast, Out_Bcast, Out_Mcast, Out_Octets,pkt_error_Out);
                else
                {
                        syslog(LOG_ERR," Invalid PID ");
                                     return -1;
                }
            break;
        case 'h':
            syslog(LOG_DEBUG, "Options:\ndebug : bcmssdk -Z 1 -d 1 ");
	break;
        default:
            retval = CONFIG_ERROR;
            break;
        }
    }
return retval;
}

/*********************************************************************
 * Function: qos
 * Arguments: command line arguments
 * Return value: 
 *      0 for success
 *      1 for incorrect command line arguments
 *      BCM_E_XXX for switch configuration failures
 * Description:
 * This routine is called through bcmssdk command. This would configure 
 * QoS settings on BCM53128M and BCM53134M chipset.
 * There are 3 modes supported:  
 * i) COS-based  ii) DSCP-based  iii) Port-based
 *********************************************************************/

int qos(int argc, char *argv[])
{
    int32 retval = 0;
    char *cptr = NULL;
    int32 opt, pri_src=-1, queue_num=-1, init_status=-1, strict_queue_num= -1, strict_queue_flag= -1;
    int32 weight_Q1=-1, weight_Q2=-1, weight_Q3=-1, weight_Q4=-1,cos_value=-1, dscp_value=-1;
    int32 switch_queuing=-1,switch_classification =-1;
    uint32 portnum = -1, cnt;
    bcm_pbmp_t pbmp;
    uint8_t int_pri=-1;
    int p = -1,prio, qweights[8], mode=0, delay = 0,priority, i;

    optind = 0;
    while ((opt = getopt(argc, argv, "1:2:3:4:c:d:i:p:q:r:s:o:t:g:h")) != -1) {
        switch (opt) {
        case '1':
            errno = 0;
            weight_Q1 = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0')
	    {
                retval = CONFIG_ERROR;
            }
            break;
        case '2':
            errno = 0;
            weight_Q2 = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0')
	    {
                retval = CONFIG_ERROR;
            }
            break;
        case '3':
	    errno = 0;
            weight_Q3 = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0')
	    {
                retval = CONFIG_ERROR;
            }
            break;
        case '4':
            errno = 0;
            weight_Q4 = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0'){
                retval = CONFIG_ERROR;
            }
            break;
        case 'c':
            errno = 0;
            cos_value = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0')
	    {
                retval = CONFIG_ERROR;
            }
            break;
        case 'd':
            errno = 0;
            dscp_value = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0')
	    {
                retval = CONFIG_ERROR;
            }
            break;
        case 'i':
            errno = 0;
            init_status =
                (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            	if (errno != 0 || *cptr != '\0' || init_status != 1)
	    	{
                	retval = CONFIG_ERROR;
            	}
		retval = bcm_cosq_init (DEFAULT_UNIT_INIT_ID);	
		if(retval != BCM_E_NONE)
			syslog(LOG_ERR,"Switch cosq Initialization failed with retval=%d", retval);
		else
			syslog(LOG_DEBUG, "Switch cosq Initialization successful with retval=%d", retval);
        break;
        case 'p':
            errno = 0;
            portnum = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
	       if (rv_prod_type_g  == PROD_TYPE_RV_160)
                {
                        if (errno != 0 ||  *cptr != '\0' || (portnum < BCM53134_MIN_PORT)  || (portnum > (BCM53134_MAX_PORT + 1)))
                        {
                                syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 1 to 4]", portnum);
                                retval = CONFIG_ERROR;
                                return -1;
                        }
			portnum = (4 - portnum);
                }
                else if (rv_prod_type_g  == PROD_TYPE_RV_260)
                {
                        if (errno != 0 ||  *cptr != '\0' || (portnum < BCM53128_MIN_PORT) || (portnum > (BCM53128_MAX_PORT + 1)))
                        {
                                syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 1 to 8]", portnum);
                                retval = CONFIG_ERROR;
                                return -1;
                        }
			portnum = (portnum - 1);
                }
                else
                {
                        syslog(LOG_ERR," Invalid PID ");
			return -1;
                }
            	//syslog(LOG_DEBUG, "switch port %d will be configured for qos", portnum);
            break;
       case 'q':
            errno = 0;
            queue_num = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0' || queue_num < BCM_MIN_QUEUE_NO || queue_num > BCM_MAX_QUEUE_NO) {
                retval = CONFIG_ERROR;
            }
	queue_num = (queue_num - 1);

	if (rv_prod_type_g  == PROD_TYPE_RV_160)
        {
		if( (sp_flag[0] == 1) || (sp_flag[1] == 1) || (sp_flag[2] == 1) || (sp_flag[3] == 1)) 
		{
			if(sp_queue[0] == queue_num || sp_queue[1] == queue_num || sp_queue[2] == queue_num || sp_queue[3] == queue_num)
			{
				queue_num = 7;
				syslog(LOG_DEBUG," ---- SP is enabled ---");
			}
		}
			
                if(pri_src==1)
                {
                        retval = bcm_port_control_set(DEFAULT_UNIT_INIT_ID, portnum, bcmPortControlTCPriority, queue_num);
                        if (retval != BCM_E_NONE)
                        {
                                syslog(LOG_ERR,"TC2COS mapping failed for Port mode [port %d retval = %d]",portnum, retval);
                                return retval;
                        }
                        syslog(LOG_DEBUG,"[port %d = %d queue]", portnum, queue_num);
                }
                else if(pri_src==2)
                {
                        int mapcp = dscp_value;
                        retval = bcm_port_dscp_map_set(DEFAULT_UNIT_INIT_ID, BCM_SW_ALL_PORTS, dscp_value, mapcp, queue_num);
                        if (retval != BCM_E_NONE)
                        {
                             syslog(LOG_DEBUG,"TC2COS mapping failed [dscp=%d queue=%d] ret=%d",dscp_value, queue_num, retval);
                             return retval;
                        }
			syslog(LOG_DEBUG,"[DSCP %d = %d queue]", dscp_value, queue_num);
                }
                else if(pri_src==3)
                {
                       	retval = bcm_cosq_port_mapping_set(DEFAULT_UNIT_INIT_ID, BCM_SW_ALL_PORTS, cos_value, queue_num);
			if(retval != BCM_E_NONE)
                        {
                        	syslog(LOG_ERR,"TC2COS mapping failed [cos %d, queue=%d],ret=%d",cos_value,queue_num,retval);
                                return -1;
                        }
                        syslog(LOG_DEBUG,"[CoS %d = %d queue]", cos_value,queue_num);
		}
        }
        else if (rv_prod_type_g  == PROD_TYPE_RV_260)
        {
		if( (sp_flag[0] == 1) || (sp_flag[1] == 1) || (sp_flag[2] == 1) || (sp_flag[3] == 1) || (sp_flag[4] == 1) || (sp_flag[5] == 1) || (sp_flag[6] == 1) || (sp_flag[7] == 1)) 
	{
		if(sp_queue[0] == queue_num || sp_queue[1] == queue_num || sp_queue[2] == queue_num || sp_queue[3] == queue_num || sp_queue[4] == queue_num || sp_queue[5] == queue_num || sp_queue[6] == queue_num || sp_queue[7] == queue_num)
		{
			queue_num = 3;
			syslog(LOG_DEBUG," ---- SP is enabled ---");
		}
	}
   		
		if(queue_num == 0)
                        priority=0;
                if(queue_num == 1)
                        priority=2;
                if(queue_num == 2)
                        priority=4;
                if(queue_num == 3)
                        priority=6;

                if(pri_src==1)
                {
                        retval = bcm_port_control_set(DEFAULT_UNIT_INIT_ID, portnum, bcmPortControlTCPriority, priority);
                        if (retval != BCM_E_NONE)
                        {
                                syslog(LOG_ERR,"TC2COS mapping failed for Port mode [port %d retval = %d]",portnum, retval);
                                return retval;
                        }
                        syslog(LOG_DEBUG,"[port %d = %d queue]", portnum, queue_num);
                }
                else if(pri_src==2)
                {
                        int mapcp = dscp_value;
                        retval = bcm_port_dscp_map_set(DEFAULT_UNIT_INIT_ID, BCM_SW_ALL_PORTS, dscp_value, mapcp, priority);
                        if (retval != BCM_E_NONE)
                        {
                                syslog(LOG_DEBUG,"TC2COS mapping failed [dscp=%d queue=%d] ret= %d",dscp_value, queue_num, retval);
                                return retval;
                        }
                        syslog(LOG_DEBUG,"[DSCP %d = %d queue]", dscp_value, queue_num);
                }
                else if(pri_src==3)
                {
                        retval = bcm_cosq_port_mapping_set(DEFAULT_UNIT_INIT_ID, BCM_SW_ALL_PORTS, cos_value, queue_num);
                        if(retval != BCM_E_NONE)
                        {
                                syslog(LOG_ERR,"TC2COS mapping failed [cos %d, queue=%d],ret=%d",cos_value,queue_num,retval);
                                return -1;
                        }
                        syslog(LOG_DEBUG,"[CoS %d = %d queue]", cos_value,queue_num);
                }
        }
        else
        {
                syslog(LOG_ERR, "Invalid PID");
                return -1;
        }
     	break;
        case 's':
            errno = 0;
            pri_src =
                (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0'){
                retval = CONFIG_ERROR;
            }
            //syslog(LOG_DEBUG, "priority source selected [%d] < Port based[1],DSCP[2],CoS[3] >", pri_src);
            break;
        case 'o':
	errno = 0;
        switch_queuing = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
        if (errno != 0 || *cptr != '\0' || switch_queuing != 1) 
	{
               	retval = CONFIG_ERROR;
               	break;
	}
	syslog(LOG_DEBUG,"switch Port %d, Weight of Q1=%d,Q2=%d,Q3=%d,Q4=%d", portnum, weight_Q1, weight_Q2,weight_Q3,weight_Q4);
/* 
 * Logic for strict priority due to switch hardware limitation 
 * Limitation: 
 * BCM53128 support only cos3 as SP as per requirement
 * BCM53134 support only cos7 as SP as per requirement
 *
 */

	int usrq[4];
	sp_flag[portnum] = 0;	
	sp_queue[portnum] = -1;	

	usrq[0]=weight_Q1;
	usrq[1]=weight_Q2;
	usrq[2]=weight_Q3;
	usrq[3]=weight_Q4;

	for(i=0; i<4; i++)
	{
		/* Check if SP needs to be set or not */
		if(usrq[i] == 0)
		{
			strict_queue_flag=1;
			sp_flag[portnum] = 1;
			sp_queue[portnum] = i;
			syslog(LOG_DEBUG,"Strict priority flag enabled usrq [%d:%d:%d:%d]",usrq[0],usrq[1],usrq[2],usrq[3]);
			break;
		}
	}

	BCM_PBMP_CLEAR(pbmp);
	BCM_PBMP_PORT_ADD(pbmp, portnum);
	if(strict_queue_flag == 1)
	{
        	if (rv_prod_type_g  == PROD_TYPE_RV_160)
        	{
                	for (cnt = 0; cnt <= 3; cnt++)
                	{
                        	if(0 == usrq[cnt])
                        	{
                                	qweights[cnt] = 1;
                        	}
                        	else
                        	{
                                	qweights[cnt] = usrq[cnt];
                        	}
                	}
			qweights[4]=1;
        		qweights[5]=1;
        		qweights[6]=1;
			qweights[7]=0;

                	syslog(LOG_DEBUG,"DRR weights [%d %d %d %d %d %d %d %d]",qweights[0],qweights[1],qweights[2],qweights[3],qweights[4],qweights[5],qweights[6],qweights[7]);

                	/* scheduling queues based on user configured weights */
                	retval=bcm_cosq_port_sched_set(DEFAULT_UNIT_INIT_ID, pbmp, BCM_COSQ_DEFICIT_ROUND_ROBIN,qweights,0);
                	if (retval != BCM_E_NONE)
                	{
                        	syslog(LOG_DEBUG,"DRR Switch Queueing set failed [RV160] and returned %d",retval);
                        	return retval;
                	}
                	syslog(LOG_INFO,"BCM_COSQ_DEFICIT_ROUND_ROBIN set Successfully");
		}
        	else if (rv_prod_type_g  == PROD_TYPE_RV_260)
		{
			qweights[0]=weight_Q1;
			qweights[1]=weight_Q2;
			qweights[2]=weight_Q3;
			qweights[3]=0;
                	
			syslog(LOG_DEBUG,"SP weights [%d %d %d %d ]",qweights[0],qweights[1],qweights[2],qweights[3]);
			/* scheduling queues based on user configured weights */
                	retval=bcm_cosq_port_sched_set(DEFAULT_UNIT_INIT_ID, pbmp, BCM_COSQ_WEIGHTED_ROUND_ROBIN,qweights,0);
			if (retval != BCM_E_NONE)
			{
                        	syslog(LOG_DEBUG,"SP Switch Queueing failed [RV260] and returned %d",retval);
				return retval;
			}
                	syslog(LOG_INFO,"BCM_COSQ_STRICT set Successfully");
		}
        }
	else
	{
        	if (rv_prod_type_g  == PROD_TYPE_RV_160)
		{
			qweights[0]=weight_Q1;
			qweights[1]=weight_Q2;
			qweights[2]=weight_Q3;
			qweights[3]=weight_Q4;
			qweights[4]=1;
			qweights[5]=1;
			qweights[6]=1;
			qweights[7]=1;

			syslog(LOG_DEBUG,"DRR Queue weights [ %d %d %d %d %d %d %d %d]", qweights[0], qweights[1],qweights[2],qweights[3],qweights[4],qweights[5],qweights[6],qweights[7]);
			/* scheduling queues based on user configured weights */
                	retval=bcm_cosq_port_sched_set(DEFAULT_UNIT_INIT_ID, pbmp, BCM_COSQ_DEFICIT_ROUND_ROBIN,qweights,0);
                	if (retval != BCM_E_NONE)
			{
				syslog(LOG_ERR,"Switch Queueing failed [RV160]and returned %d",retval);
                        	return retval;
			}
                	syslog(LOG_INFO,"BCM_COSQ_DEFICIT_ROUND_ROBIN set Successfully");
                }
        	else if (rv_prod_type_g  == PROD_TYPE_RV_260)
		{
			qweights[0]=weight_Q1;
                	qweights[1]=weight_Q2;
                	qweights[2]=weight_Q3;
                	qweights[3]=weight_Q4;
                	/* scheduling queues based on user configured weights */
                	retval=bcm_cosq_port_sched_set(DEFAULT_UNIT_INIT_ID, pbmp, BCM_COSQ_WEIGHTED_ROUND_ROBIN,qweights,0);
                	if (retval != BCM_E_NONE)
			{
                        	syslog(LOG_DEBUG,"Switch Queueing failed [RV260] and returned %d",retval);
                       		return retval;
                        }
                	syslog(LOG_INFO,"BCM_COSQ_WEIGHTED_ROUND_ROBIN set Successfully");
               }
       }
        break;
        case 't':
            errno = 0;
            switch_classification =
                (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            	if (errno != 0 || *cptr != '\0' || switch_classification != 1) 
		{
                	retval = CONFIG_ERROR;
                	break;
             	}
        if(pri_src==1)
	{
		retval = bcm_cosq_init(DEFAULT_UNIT_INIT_ID);
            	if(retval != BCM_E_NONE)
            	{
            		syslog(LOG_ERR,"bcm_cosq_init failed with retval=%d",retval);
                	return retval;
            	}
		/* Enable Port-based Qos */
		retval = bcm_switch_control_set(DEFAULT_UNIT_INIT_ID, bcmSwitchPortBasedQos, 1);
		if(retval != BCM_E_NONE) 
		{
			syslog(LOG_ERR,"Port based QoS mode set failed returned with retval = %d",retval);
			return retval;
		}
	 	syslog(LOG_DEBUG, "Port Based QoS mode enabled successfully");
        }
        else if(pri_src==2)
	{
		retval = bcm_cosq_init(DEFAULT_UNIT_INIT_ID);
            if(retval != BCM_E_NONE)
            {
            	syslog(LOG_ERR,"bcm_cosq_init failed with retval=%d",retval);
                return retval;
            }
	   /* Enable DSCP */
           retval = bcm_port_dscp_map_mode_set(DEFAULT_UNIT_INIT_ID, BCM_SW_ALL_PORTS, BCM_PORT_DSCP_MAP_ALL);
           if (retval != BCM_E_NONE)
           {
 	          syslog(LOG_ERR, "DSCP based QoS mode set failed with retval =%d",retval);
                  return retval;
           }
           syslog(LOG_DEBUG,"DSCP Mode set Successful");
	}
	else if(pri_src==3)
        {   
		/* Enable CoS */
                retval = bcm_cosq_init (DEFAULT_UNIT_INIT_ID);
                if(retval != BCM_E_NONE)
                {
                        syslog(LOG_ERR,"bcm_cosq_init failed with retval=%d",retval);
                        return retval;
        }
        syslog(LOG_DEBUG,"CoS based QoS enabled Succefully");
        }
        break;
	case 'r':
                retval = bcm_stat_init(DEFAULT_UNIT_INIT_ID);
                if (retval != BCM_E_NONE) 
                {
                        syslog(LOG_ERR,"Switch Qos stat reset failed and returned %d. Please check the settings",retval);
                        return retval;
                }

		if (rv_prod_type_g  == PROD_TYPE_RV_160)
                {
                        for (portnum = 0;portnum <= 3;portnum++)
                        {
                                retval = bcm_stat_clear(DEFAULT_UNIT_INIT_ID,portnum);
                                if (retval != BCM_E_NONE)
                                {
                                        syslog(LOG_ERR,"bcm_stat_clear failed for port(%d) with error(%d),returning",portnum,retval);
                                	return retval;
                                }
                        }
                }
                else if (rv_prod_type_g  == PROD_TYPE_RV_260)
                {
                        for (portnum = 0;portnum <= 7;portnum++)
                        {
                                retval = bcm_stat_clear(DEFAULT_UNIT_INIT_ID,portnum);
                                if (retval != BCM_E_NONE)
                                {
                                        syslog(LOG_ERR,"bcm_stat_clear failed for port_no %d with error %d, returning",portnum,retval);
                        	        return retval;
                                }
                        }
                }
                syslog(LOG_ERR,"Switch Qos stat reset Succefully and returned %d",retval);
        	return 0;
	case 'g':
		BCM_PBMP_CLEAR(pbmp);
		BCM_PBMP_PORT_ADD(pbmp, portnum);
		bcm_cos_queue_t     cosq;
		bcm_cos_t           prio;

		syslog(LOG_DEBUG,"Priority to queue mappings:");
		for (prio = 0; prio < 8; prio++) 
		{
			if ((retval = bcm_cosq_mapping_get(DEFAULT_UNIT_INIT_ID, prio, &cosq)) < 0) 
			{
				syslog(LOG_DEBUG, "Failed to get mapping for prio =%d retval=%d", prio, retval);
				return -1;
			}
			syslog(LOG_DEBUG,"PRIO %d ==> COSQ %d", prio, cosq);
		}

	   	for (cosq = 0; cosq < 4; cosq++) 
		{

			/* scheduling queues based on user configured weights */
			retval=bcm_cosq_port_sched_get(DEFAULT_UNIT_INIT_ID, pbmp, &mode,qweights, &delay);
			if (retval != BCM_E_NONE) {
			     syslog(LOG_ERR,"Switch Queueing set failed and returned %d. Please check the settings",retval);
			    return retval;
			}
                        /* show init cos queues weights status */
                        syslog(LOG_DEBUG,"COSQ %d = %d packets", cosq, qweights[cosq]);
		    }
	break;
        case 'h':
            syslog(LOG_DEBUG, "Options:\n"
                       "\tqos init:\n\t bcmssdk -Q 1 -i 1 \n"
                       "\tswitch queueing:\n\t bcmssdk -Q 1 -p <portnum> -1 <wt_q1> -2 <wt_q2> -3 <wt_q3> -4 <wt_q4> -o 1\n"
                       "\tswitch classification_port based:\n\t bcmssdk -Q 1 -s 1 -p <portnum> -q <queue_no> \n"
                       "\tswitch classification_dscp based:\n\t bcmssdk -Q 1 -s 2 -d <dscp_no> -q <queue_no> \n"
                       "\tswitch classification_cos based:\n\t bcmssdk -Q 1 -s 3 -c <cos_no> -q <queue_no> \n"
                       "\tswitch classification- setting decision table:\n\t bcmssdk -Q 1 -s <1/2/3> -t 1\n ");
        /* fall-through to default case since there is no more action */
        default:
            retval = CONFIG_ERROR;
            break;
        }
    }
    return retval;
}

/* ********************************************************************
 * Function: Register
 * Arguments: command line arguments
 * Return value: 
 *      0 for success
 *      1 for incorrect command line arguments
 *      BCM_E_NONE for switch configuration failures
 *      other: for return value of get command
 * Description:
 * Get/Set the switch register values using ioctl. This command will set/get the 
 * switch registers
 * ********************************************************************/
int Register(int argc, char *argv[])
{
	int32 opt, rv;
    	char *cptr = NULL;
   	uint32_t data, portnum; 
	char str[200];
        FILE *fp = NULL;
	int ret = 0, i, fd = -1;
	ROBO_OBJ_CMD_MSG vidMsg;
	memset(&vidMsg,0,sizeof(vidMsg));
    	optind = 0;
	syslog(LOG_DEBUG,"Function < %s > called", __FUNCTION__);
    	while ((opt = getopt(argc, argv, "o:p:a:i:v:w:r:q:e:d:h")) != -1) {
        switch (opt) {
	case 'o':
	  errno = 0;
             vidMsg.port = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0') {
                ret= CONFIG_ERROR;
            }
	syslog(LOG_DEBUG,"User entered Port %x", vidMsg.port);
	break;
	case 'p':
	  errno = 0;
             vidMsg.Page_addr = (int32)strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0') {
                ret= CONFIG_ERROR;
            }
	syslog(LOG_DEBUG,"User entered Page %x", vidMsg.Page_addr);
	break;
	case 'a':
	  errno = 0;
            vidMsg.Reg_addr = (int32)strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0') {
                ret= CONFIG_ERROR;
            }
	syslog(LOG_DEBUG,"User entered Reg %x", vidMsg.Reg_addr);
	break;
	case 'v':
	  errno = 0;
            vidMsg.Reg_value = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0') {
                ret= CONFIG_ERROR;
            }
	syslog(LOG_DEBUG,"User entered Value %x", vidMsg.Reg_value);
	break;
	case 'w':
		vidMsg.commandMsg = DeviceWrite;
		vidMsg.length = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
		if( (fd = open(ROBODRV,O_RDONLY)) <= 0)
		{
			syslog(LOG_ERR,"DeviceWrite Device Open failed!!");
			return -1;
		}
		syslog(LOG_DEBUG,"IOCTL call to DeviceWrite");
		ret = ioctl(fd,ROBO_IOC_CMD,&vidMsg);
		if( ret != 0)
		{
			syslog(LOG_ERR,"Error in IOCTL ..DeviceWrite command");
		}

	close(fd);
	 break;
	case 'r':
		vidMsg.commandMsg = DeviceRead;
		vidMsg.length = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
		if( (fd = open(ROBODRV,O_RDONLY)) <=0 )
		{
			syslog(LOG_ERR,"DeviceRead Device Open failed!!");
			return -1;
		}
		syslog(LOG_DEBUG,"IOCTL call to DeviceRead");
		ret = ioctl(fd,ROBO_IOC_CMD,&vidMsg);
		if( ret != 0)
		{
			syslog(LOG_ERR,"Error in IOCTL ..DeviceRead command");
		}

		close(fd);
	 break;
	case 'q':
		vidMsg.commandMsg = QosStats;
                if( (fd = open(ROBODRV,O_RDONLY)) <=0 )
                {
                        syslog(LOG_DEBUG,"QosStats Device Open failed!!");
                        return -1;
                }
		if (rv_prod_type_g  == PROD_TYPE_RV_160)
                {
			for(i = 0;i < 4; i++)
			{
				vidMsg.port=i;
				syslog(LOG_DEBUG,"IOCTL call to QosStats");
				ret = ioctl(fd,ROBO_IOC_CMD,&vidMsg);
				if( ret != 0)
				{
					syslog(LOG_ERR,"Error in IOCTL ..DeviceRead command");
				}
				ret = sprintf(str,"echo %u,%u,%u,%u >/tmp/stats/tmp_sw_qos_stats_port%d",vidMsg.Qos_stats[0],vidMsg.Qos_stats[1], vidMsg.Qos_stats[2],vidMsg.Qos_stats[3],i);
				if(NULL != (fp = popen(str,"r")))
				{
					pclose(fp);
				}
				else
				{
					syslog(LOG_ERR,"port_stats write failed to /tmp/stats/tmp_sw_qos_stats_port");
					pclose(fp);
				}
                	}
                }
                else if (rv_prod_type_g  == PROD_TYPE_RV_260)
                {
			for(i = 0;i < 8; i++)
			{
				vidMsg.port=i;
				syslog(LOG_DEBUG,"IOCTL call to QosStats");
				ret = ioctl(fd,ROBO_IOC_CMD,&vidMsg);
				if( ret != 0)
				{
					syslog(LOG_ERR,"Error in IOCTL ..DeviceRead command");
				}
				ret = sprintf(str,"echo %u,%u,%u,%u >/tmp/stats/tmp_sw_qos_stats_port%d",vidMsg.Qos_stats[0],vidMsg.Qos_stats[1], vidMsg.Qos_stats[2],vidMsg.Qos_stats[3],i);
				if(NULL != (fp = popen(str,"r")))
				{
					pclose(fp);
				}
				else
				{
					syslog(LOG_ERR,"port_stats write failed to /tmp/stats/tmp_sw_qos_stats_port");
					pclose(fp);
				}
			}
		}
                else
                {
                	syslog(LOG_ERR," Invalid PID ");
                }
        close(fd);
	break;
	case 'e':
		vidMsg.commandMsg = IMP_Enable;
		if((fd = open(ROBODRV,O_RDONLY)) <=0)
                {
                        syslog(LOG_ERR,"IMP_Enable Device Open failed!!");
                        return -1;
                }
                syslog(LOG_DEBUG,"IOCTL call to IMP_Enable");
                ret = ioctl(fd,ROBO_IOC_CMD,&vidMsg);
                if( ret != 0)
                {
                        syslog(LOG_ERR,"Error in IOCTL ..DeviceRead command");
                }
                close(fd);
	break;
	case 'd':
		vidMsg.commandMsg = IMP_Disable;
		if( (fd = open(ROBODRV,O_RDONLY)) <=0 )
                {
                        syslog(LOG_ERR,"IMP_Disable Device Open failed!!");
                        return -1;
                }
                syslog(LOG_DEBUG,"IOCTL call to IMP_Disable");
                ret = ioctl(fd,ROBO_IOC_CMD,&vidMsg);
                if( ret != 0)
                {
                        syslog(LOG_ERR,"Error in IOCTL ..DeviceRead command");
                }
                close(fd);
	break;
	case 'i':
		vidMsg.commandMsg = Default_Reg;
		if( (fd = open(ROBODRV,O_RDONLY)) <=0 )
		{
			syslog(LOG_ERR,"Default_Reg Device Open failed!!");
			return -1;
		}
		syslog(LOG_DEBUG,"IOCTL call to Default_Reg");
		ret = ioctl(fd,ROBO_IOC_CMD,&vidMsg);
		if( ret != 0)
		{
			syslog(LOG_ERR,"Error in IOCTL ..Default_Reg command");
		}
		close(fd);
	 break;
	case 'h':
                syslog(LOG_DEBUG,"Options:\n"
                           "\t-o <portnum>\n"
                           "\t-p <page>\n"
                           "\t-a <register>\n"
                           "\t-v <value>\n"
                           "\t-q <Switch QoS stats>\n"
                           "\t-r <read> IOCTL read call\n"
                           "\t-w <write> IOCTL write call\n"
                           "\t-e <IMP enable>\n"
                           "\t-d <IMP disable\n\t");
                rv = CONFIG_ERROR;
                syslog(LOG_ERR,"rv =%d", rv);
                break;

            }
       }
        return 0;
}






/* ********************************************************************
 * Function: feature
 * Arguments: command line arguments
 * Return value: 
 *      0 for success
 *      1 for incorrect command line arguments
 *      BCM_E_NONE for switch configuration failures
 *      other: for return value of get command
 * Description:
 * Get/Set the switch feature status. This command will set/get the 
 * switch features such as dot1x (802.1x) and LLDP
 * ********************************************************************/
int feature(int argc, char *argv[])
{
    int32 rv = 0, opt;
    char *cptr = NULL, ii;
    int32 dot1x_status= -1, lldp_status = -1;
    uint32 portnum;

    	optind = 0;
    	while ((opt = getopt(argc, argv, "p:d:l:h")) != -1) {
        switch (opt) {
	case 'p':
	    errno = 0;
	    portnum = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
	    if (rv_prod_type_g  == PROD_TYPE_RV_160)
            {
            	if (errno != 0 ||  *cptr != '\0' || (portnum < BCM53134_MIN_PORT)  || (portnum > BCM53134_MAX_PORT))
            	{
                	syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 0 to 3]", portnum);
			rv = CONFIG_ERROR;
            		return -1;
	    	}
	    }
            else if (rv_prod_type_g  == PROD_TYPE_RV_260)
            {
                if (errno != 0 || *cptr != '\0' || (portnum < BCM53128_MIN_PORT) || (portnum > BCM53128_MAX_PORT))
                {
			syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 0 to 7]", portnum);
			rv = CONFIG_ERROR;
			return -1;
                }
            }
            else
            {
                    syslog(LOG_DEBUG," Invalid PID ");
		    return -1;
            }
	    syslog(LOG_DEBUG,"dot1x called success for 'p' %d", portnum);
	 break;
        case 'd':
            errno = 0;
            dot1x_status = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0') {
                rv = CONFIG_ERROR;
            }
	    syslog(LOG_DEBUG,"dot1x status = %d", dot1x_status);
#if 0
            if (dot1x_status == 1) 
	    {
            	syslog(LOG_DEBUG,"Register link monitor function");
		if(bcm_linkscan_register(DEFAULT_UNIT_INIT_ID,(bcm_linkscan_handler_t) nxp_port_linkMon_callback) != BCM_E_NONE)
		{
                        syslog(LOG_ERR,"802.1x: bcm_robo_linkscan_register failed");
                        return -1;
		}

	    	if (rv_prod_type_g  == PROD_TYPE_RV_160)
            	{
			for(ii = 0;ii < 4;ii++)	
	      		{
				if(bcm_linkscan_mode_set(DEFAULT_UNIT_INIT_ID, ii , BCM_LINKSCAN_MODE_SW) != BCM_E_NONE)
				{
                        		syslog(LOG_ERR,"802.1x: bcm_linkscan_mode_set failed for port %d", ii);
                        		return -1;
                		}
			}
	    	}
            	else if (rv_prod_type_g  == PROD_TYPE_RV_260)
            	{
			for(ii = 0;ii < 8;ii++)	
	      		{
				if(bcm_linkscan_mode_set(DEFAULT_UNIT_INIT_ID, ii , BCM_LINKSCAN_MODE_SW) != BCM_E_NONE)
				{
                        		syslog(LOG_ERR,"802.1x: bcm_linkscan_mode_set failed for port %d", ii);
                        		return -1;
                		}
			}
            	}
            	else
            	{
                	syslog(LOG_DEBUG," Invalid PID ");
			return -1;
            	}
	
	    	if(bcm_linkscan_enable_set(DEFAULT_UNIT_INIT_ID, 1000000) != BCM_E_NONE)
	    	{
            		syslog(LOG_ERR,"802.1x: bcm_robo_linkscan_enable_set failed");
                	return -1;
	    	}
            	syslog(LOG_DEBUG,"Register link monitor function successfully");
            }
            else if(dot1x_status == 0)
            {
		syslog(LOG_DEBUG,"Un-Register link monitor function");
		if(bcm_linkscan_unregister(DEFAULT_UNIT_INIT_ID,(bcm_linkscan_handler_t)nxp_port_linkMon_callback) != BCM_E_NONE)
		{
			syslog(LOG_ERR,"802.1x: bcm_robo_linkscan_unregister failed");
			return -1;
		}
		syslog(LOG_DEBUG,"Un-Register link monitor function successfully");
	    }
#endif
        break;
        case 'l':
            	errno = 0;
            	lldp_status = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            	if (errno != 0 || *cptr != '\0') 
		{
                	rv = CONFIG_ERROR;
            	}
		 syslog(LOG_DEBUG,"lldp status = %d", lldp_status);
                if (lldp_status == 1) 
		{
                        for (portnum = 0;portnum < 8;portnum++) 
			{
                          if(rv != BCM_E_NONE)
                          {
                          	 syslog(LOG_ERR,"LLDP CPUTAG trap setting failed rv=%d", rv);
                                return rv;
                          }
                        }
                         syslog(LOG_DEBUG,"LLDP CPUTAG trap setting SUCCESS");
                }
                else if  (lldp_status == 0) 
		{
                        for (portnum = 0; portnum < 8;portnum++) 
			{
                        	if(rv != BCM_E_NONE)
                                {
                                	 syslog(LOG_ERR,"LLDP CPUTAG disable failed rv=%d", rv);
                                	return rv;
                                }
                         }
                }
                syslog(LOG_ERR,"LLDP CPUTAG disable SUCCESS");
            	break;
        }
   }
   return rv;
}

/* ********************************************************************
 * Function: dot1x
 * Arguments: command line arguments
 * Return value: 
 *      0 for success
 *      1 for incorrect command line arguments
 *      BCM_E_XXX for switch configuration failures
 *      other: for return value of get command
 * Description:
 * This routine is called through bcmssdk command. This would configure 
 * 802.1X port setting on BCM53128M chipset. There are
 * two possible modes on each port. ForceAuth & ForceUnauth. 
 * When a switch port is configured as ForceAuth, all the packets are
 * allowed to go to CPU port or other ports. When switch port is 
 * configured for ForceUnauth, all the packets are dropped at the port.
 * ********************************************************************/
int dot1x(int argc, char *argv[])
{
    int32 rv = -1, opt, portnum = -1;
    char *cptr = NULL;
    int32 auth = -1, force = -1;
    uint32         mode;
    pbmp_t  pbmp;
    optind = 0;
    while ((opt = getopt(argc, argv, "f:p:a:e:h")) != -1) 
    {
       	switch (opt) 
	{
       		case 'p':
		errno = 0;
		portnum = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
		if (rv_prod_type_g  == PROD_TYPE_RV_160)
		{
			if (errno != 0 ||  *cptr != '\0' || (portnum < BCM53134_MIN_PORT) || (portnum > BCM53134_MAX_PORT))
			{
				syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 1 to 4]", portnum);
				rv = CONFIG_ERROR;
				return -1;
			}
		}
		else if (rv_prod_type_g  == PROD_TYPE_RV_260)
		{
			if (errno != 0 || *cptr != '\0' || (portnum < BCM53128_MIN_PORT) || (portnum > BCM53128_MAX_PORT))
			{
				syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 1 to 8]", portnum);
				rv = CONFIG_ERROR;
				return -1;
			}
		}
		else
		{
			syslog(LOG_ERR," Invalid PID ");
			return -1;
		}
		syslog(LOG_DEBUG, "dot1x called success for 'p' %d", (portnum+1));
		 break;
		 case 'f':
		    errno = 0;
		    force = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
		    if (errno != 0 || *cptr != '\0' || force > 1) {
			rv = CONFIG_ERROR;
		    }
		    syslog(LOG_DEBUG,"dot1x called for 'f' %d", force);
		    if (force == 1) 
		    {
			syslog(LOG_DEBUG,"Forcing port%d to ForceAuth", (portnum + 1));
			rv = bcm_auth_mode_set(DEFAULT_UNIT_INIT_ID, portnum, BCM_AUTH_MODE_UNCONTROLLED);
			if (rv != BCM_E_NONE) 
			{
				syslog(LOG_ERR,"Setting ForceAuth failed with %d", rv);
				return rv;
			}
			else
				syslog(LOG_ERR,"Setting ForceAuth successful for LAN%d", (portnum+1));
		    }
		 break;
		 case 'a':
		    errno = 0;
		    auth = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
		    if (errno != 0 || *cptr != '\0' || auth > 1) {
			rv = CONFIG_ERROR;
		    }
		    if (auth == 1) 
		    {

			syslog(LOG_DEBUG,"Forcing LAN%d to Auto", (portnum + 1));
			rv = bcm_auth_mode_set(DEFAULT_UNIT_INIT_ID, portnum, (BCM_AUTH_SEC_SIMPLIFY_MODE | BCM_AUTH_MODE_AUTH));
			if (rv != BCM_E_NONE)
			{
				syslog(LOG_ERR,"Auto mode setting failed with %d", rv);
				return rv;
			}
			syslog(LOG_DEBUG,"Auto mode setting success for port LAN%d", (portnum + 1));
			rv = bcm_auth_egress_set(DEFAULT_UNIT_INIT_ID, portnum, 0);
			if (rv != BCM_E_NONE)
			{
				syslog(LOG_ERR,"Traffic set failed with %d", rv);
				return rv;
			}
			syslog(LOG_DEBUG,"Traffic enabled on LAN%d successfully", (portnum+1));

		    }		 
		    else if (auth == 0)
		    {
			syslog(LOG_DEBUG,"Forcing LAN%d to Auto", (portnum + 1));
			rv = bcm_auth_mode_set(DEFAULT_UNIT_INIT_ID, portnum, (BCM_AUTH_SEC_SIMPLIFY_MODE | BCM_AUTH_MODE_UNAUTH));
			if (rv != BCM_E_NONE)
			{
				syslog(LOG_ERR,"dot1x port based Auth setting failed with %d", rv);
				return rv;
			}
			syslog(LOG_DEBUG,"Auth mode setting success for LAN%d", (portnum+1));
			rv = bcm_auth_egress_set(DEFAULT_UNIT_INIT_ID, portnum, 1);
			if (rv != BCM_E_NONE)
			{
				syslog(LOG_ERR,"Traffic set failed with %d", rv);
				return rv;
			}
			syslog(LOG_DEBUG,"Traffic DISABLED on LAN%d successfully", (portnum+1));

		   }
		   else
                        syslog(LOG_DEBUG,"DOT1X setting failed for port #%d", (portnum + 1));
			break;
			case 'h':
			    syslog(LOG_DEBUG,"Options:\n"
                		       "\t-p <port-number>\n"
                		       "\t-f <force Auth>\n"
                       			"\t-a 0/1 <Auto mode>\n");
            				rv = 1;
         		break;
        }
    }
        return rv;
}

/* ********************************************************************
 * Function: port_stats
 * Arguments: command line arguments
 * Return value: 
 *      0 for success
 *      1 for incorrect command line arguments
 *      BCM_E_XXX for switch configuration failures
 *      other: for return value of get command
 * Description:
 * This routine is called through bcmssdk command. This would get 
 * the switch port statistics
 * ********************************************************************/
int port_stats(int argc, char *argv[])
{
    int32 rv = -1;
    char *cptr = NULL;
    char str[200];
    FILE *fp = NULL;

    int pSpeed, pDuplex,pLinkStatus, autoneg; 
    int32 opt, portnum = -1, pAutoNegotiation = -1;
    uint64 In_Ucast = 0, In_Mcast = 0, In_Bcast = 0, Out_Ucast =
        0, Out_Bcast = 0, Out_Mcast = 0, In_Octets = 0, Out_Octets =
        0, pkt_error_In = 0, pkt_error_Out = 0;

    	optind = 0;
    	while ((opt = getopt(argc, argv, "p:g:r:h")) != -1) 
	{
        switch (opt) 
	{
            case 'p':
                errno = 0;
                portnum = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
		if (rv_prod_type_g  == PROD_TYPE_RV_160)
                {
                        if (errno != 0 || *cptr != '\0' || (portnum < BCM53134_MIN_PORT) || (portnum > BCM53134_MAX_PORT))
                        {
                                syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 1 to 4]", portnum);
                    		rv = CONFIG_ERROR;
                                return -1;
                	}
                }
                else if (rv_prod_type_g  == PROD_TYPE_RV_260)
                {
                        if (errno != 0 || *cptr != '\0' || (portnum < BCM53128_MIN_PORT) || (portnum > BCM53128_MAX_PORT))
                        {
                                syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 1 to 8]", portnum);
                                rv = CONFIG_ERROR;
                                return -1;
                        }
                }
                else
                {
                        syslog(LOG_ERR," Invalid PID ");
			return -1;
                }
                break;
     case 'g':

		rv = bcm_port_link_status_get(DEFAULT_UNIT_INIT_ID, portnum, &pLinkStatus);
		if( rv != BCM_E_NONE)
		{
                        syslog(LOG_ERR,"bcm_port_link_status_get failed with retval = %d", rv);
                	return -1;
		}
		
	        rv = bcm_port_speed_get(DEFAULT_UNIT_INIT_ID, portnum, &pSpeed);
                if(rv != BCM_E_NONE)
                {
                        syslog(LOG_ERR,"bcm_port_speed_get failed with retval = %d", rv);
                }

                rv = bcm_port_duplex_get(DEFAULT_UNIT_INIT_ID, portnum, &pDuplex);
                if(rv != BCM_E_NONE)
                {
                        syslog(LOG_ERR,"bcm_port_duplex_get failed with retval = %d", rv);
                }
                rv = bcm_port_autoneg_get(DEFAULT_UNIT_INIT_ID, portnum, &autoneg);
                if(rv != BCM_E_NONE)
                {
                        syslog(LOG_ERR,"bcm_port_autoneg_get failed with retval = %d", rv);
                }
                //syslog(LOG_DEBUG," port = %d , SPEED=%d duplex=%d, autoneg=%d", (portnum+1), pSpeed, pDuplex, autoneg);

		if(autoneg == 1)
                    pAutoNegotiation = 1;
		else
                    pAutoNegotiation = 0;

  	        COMPILER_64_ZERO(In_Octets);
                COMPILER_64_ZERO(In_Ucast);
                COMPILER_64_ZERO(In_Mcast);
                COMPILER_64_ZERO(In_Bcast);
                COMPILER_64_ZERO(Out_Octets);
                COMPILER_64_ZERO(Out_Ucast);
                COMPILER_64_ZERO(Out_Bcast);
                COMPILER_64_ZERO(Out_Mcast);
                COMPILER_64_ZERO(pkt_error_Out);
                COMPILER_64_ZERO(pkt_error_In);

		bcm_stat_sync(DEFAULT_UNIT_INIT_ID);

		bcm_stat_get(DEFAULT_UNIT_INIT_ID, portnum, snmpIfInOctets, &In_Octets);
		bcm_stat_get(DEFAULT_UNIT_INIT_ID, portnum, snmpIfInUcastPkts, &In_Ucast);
		bcm_stat_get(DEFAULT_UNIT_INIT_ID, portnum, snmpIfInMulticastPkts, &In_Mcast);
		bcm_stat_get(DEFAULT_UNIT_INIT_ID, portnum, snmpIfInBroadcastPkts, &In_Bcast);
		bcm_stat_get(DEFAULT_UNIT_INIT_ID, portnum, snmpIfOutOctets, &Out_Octets);
		bcm_stat_get(DEFAULT_UNIT_INIT_ID, portnum, snmpIfOutUcastPkts, &Out_Ucast);
		bcm_stat_get(DEFAULT_UNIT_INIT_ID, portnum, snmpIfOutBroadcastPkts, &Out_Bcast);
		bcm_stat_get(DEFAULT_UNIT_INIT_ID, portnum, snmpIfOutMulticastPkts, &Out_Mcast);

		bcm_stat_get(DEFAULT_UNIT_INIT_ID, portnum, snmpIfOutErrors, &pkt_error_Out);
		bcm_stat_get(DEFAULT_UNIT_INIT_ID, portnum, snmpIfInErrors, &pkt_error_In);

		//syslog(LOG_DEBUG,"\nIn_Ucast = %llu\nIn_Mcast = %llu\nIn_Bcast = %llu\nOut_Ucast =%llu\nOut_Bcast = %llu\nOut_Mcast = %llu\nIn_Octets = %llu\nOut_Octets =%llu\npkt_error_In = %llu\npkt_error_Out = %llu\n", In_Ucast,In_Mcast,In_Bcast,Out_Ucast,Out_Bcast,Out_Mcast,In_Octets,Out_Octets,pkt_error_In,pkt_error_Out);
                sprintf(str,"echo %d,%d,%d,%d,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu > /tmp/stats/tmp_stats_port%d",
			pLinkStatus, pSpeed, pDuplex, pAutoNegotiation, 
			In_Ucast, In_Bcast, In_Mcast, In_Octets, pkt_error_In,
                     	Out_Ucast, Out_Bcast, Out_Mcast, Out_Octets,
                     	pkt_error_Out,portnum);
                if(NULL != (fp = popen(str,"r")))
		{
                	pclose(fp);
		}
                else 
		{
                	syslog(LOG_ERR,"port_stats write failed to /tmp/stats/tmp_stats_port");
                	pclose(fp);
		}
                break;
            case 'r':
		if (rv_prod_type_g  == PROD_TYPE_RV_160)
                {
			for (portnum = 0;portnum <= 3;portnum++) 
			{
                		rv = bcm_stat_clear(DEFAULT_UNIT_INIT_ID,portnum);
                		if (rv != BCM_E_NONE) 
				{
                    			syslog(LOG_ERR,"bcm_stat_clear failed for port %d,error %d so return",portnum,rv);
                    			return rv;
                		}
            		}
                }
                else if (rv_prod_type_g  == PROD_TYPE_RV_260)
                {
			for (portnum = 0;portnum <= 7;portnum++) 
			{
                		rv = bcm_stat_clear(DEFAULT_UNIT_INIT_ID,portnum);
                		if (rv != BCM_E_NONE) 
				{
                    			syslog(LOG_ERR,"bcm_stat_clear failed for port %d,error %d so return",portnum,rv);
                    		return rv;
                		}
            		}
                }
                else
                {
                        syslog(LOG_ERR,"Invalid PID ");
                        return -1;
                }
            break;
            case 'h':
                syslog(LOG_DEBUG,"Options:\n"
                           "\t-p <portnum>  -g 1 to get port statistics for this port\n"
                           "\t-r 1 to reset global MIB counter to 0 \n\t");
                rv = CONFIG_ERROR;
                break;

            }
       }
        return 0;
}

/*********************************************************************
 * Function: lag
 * Arguments: command line arguments
 * Return value: 
 *      0 for success
 *      1 for incorrect command line arguments
 *      BCM_E_XXX for switch configuration failures
 * Description:
 * This routine is called through bcmssdk command. This would configure 
 * LAG membership on BCM53128M chipset. 
 * Two groups can be configured on RV260. These groups have 
 * their respective port mapping within the switch. The mapping should 
 * follow this guideline when configurng a LAG group.
 *********************************************************************/
int lag(int argc, char *argv[])
{
	int lag_portmask, rv = -1, i=0,j = 0;
	bcm_trunk_t         tid = -1, trunk_id;
	bcm_trunk_info_t    t_add_info;
    	bcm_trunk_member_t *member_array = NULL;
    	int member_count = 0, psc;
	char *cptr = NULL;
	int32 opt, portnum = -1, groupid = -1, init_status=-1;

	optind = 0;
    	while ((opt = getopt(argc, argv, "d:g:i:m:p:h")) != -1) {
        	switch (opt) {
        	case 'd':
		errno = 0;
	        trunk_id = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
                if (errno != 0 || *cptr != '\0') 
		{
                   rv = CONFIG_ERROR;
                   break;
                }

		trunk_id = trunk_id - 1;
		rv = bcm_trunk_destroy(DEFAULT_UNIT_INIT_ID, trunk_id);
		if(rv == BCM_E_NONE)
			 syslog(LOG_DEBUG,"Deletion of trunk_id = %d Successful with rv = %d", trunk_id, rv);
		else
			 syslog(LOG_ERR,"Deletion of trunk_id = %d failed with rv = %d", trunk_id, rv);
		/* Reset ports of particular trunk in case if trunk is deleted */
		bcm_lag_reset_lag(trunk_id);
		
		return 0;
		case 'm':
		if(trunk_id == 0)
		{	/* Release the old memory for LAG 0 and allocate new one */
			if(group_0_member_array != NULL)
			{
		        	sal_free(group_0_member_array);
	    			syslog(LOG_DEBUG,"TRUNK 0 (group_0_member_array) memory released");
			}
			group_0_member_array=NULL;
			
	    		syslog(LOG_DEBUG,"TRUNK 0 (group_0_member_array) memory allocation");
			group_0_member_array = sal_alloc(sizeof(bcm_trunk_member_t) * 4, "member array1");
                        if (NULL == group_0_member_array)
			{
		        	syslog(LOG_ERR," ERROR: sa_alloc failed for TRUNK 0");
				return -1;
            		}
			member_array=group_0_member_array;
		}
		else if(trunk_id == 1)
		{	/* Release the old memory for LAG 1 and allocate new one */
			if(group_1_member_array != NULL)
			{
	    			syslog(LOG_DEBUG,"TRUNK 1 (group_1_member_array) memory released");
		        	sal_free(group_1_member_array);
			}
			group_1_member_array=NULL;
	    		syslog(LOG_DEBUG,"TRUNK 1 (group_1_member_array) memory allocation");
			group_1_member_array = sal_alloc(sizeof(bcm_trunk_member_t) * 4, "member array1");
                        if (NULL == group_1_member_array)
			{
		        	syslog(LOG_ERR," ERROR: sa_alloc failed for TRUNK 1");
				return -1;
            		}
			member_array=group_1_member_array;
		}
	break;
        case 'p':
            errno = 0;
	     if (rv_prod_type_g  == PROD_TYPE_RV_160)
                {
                        portnum = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
                        if (errno != 0 ||  *cptr != '\0' || portnum < BCM53134_MIN_PORT  || portnum > BCM53134_MAX_PORT)
                        {
                                syslog(LOG_ERR,"RV16x Invalid port = %d configuration [Use sw Port between 0 to 3]", portnum);
                                rv = CONFIG_ERROR;
                                return -1;
			}

                }
                else if (rv_prod_type_g  == PROD_TYPE_RV_260)
		{
                        portnum = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
                        if (errno != 0 ||  *cptr != '\0' || portnum < BCM53128_MIN_PORT  || portnum > BCM53128_MAX_PORT)
                        {
                                syslog(LOG_ERR,"RV26x Invalid port = %d configuration [Use sw Port between 0 to 7]", portnum);
                                rv = CONFIG_ERROR;
                                return -1;
                        }
		}
		else
		{
                        syslog(LOG_ERR,"Invalid PID");
		}
	        syslog(LOG_DEBUG,"LAG member port = %d" , portnum);
		/* memory already created for TRUNK's so just using it directly and adding ports to it */
		member_array[member_count].gport = portnum;
		member_count++;
		/* Set all configured port numbers in temporary port variable.*/
		bcm_lag_portbit_set(portnum);
            break;
        case 'g':
            errno = 0;
            groupid = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0') {
                rv = CONFIG_ERROR;
                break;
            }
		tid = (groupid-1);
		/* Creating a TRUNK */
		rv = bcm_trunk_create(DEFAULT_UNIT_INIT_ID, BCM_TRUNK_FLAG_WITH_ID, &tid);
		if(rv == BCM_E_EXISTS)
		{
			 syslog(LOG_DEBUG,"BCM_E_EXISTS (trunk ID %d exist) So just modify it", tid);
		}
		else if(rv == BCM_E_NONE)
		{
			 syslog(LOG_DEBUG,"bcm_trunk_create Successful with rv=%d", rv);
		}
		else
		{
			syslog(LOG_ERR,"bcm_trunk_create failed with rv=%d", rv);
			return -1;
		}

		bcm_trunk_info_t_init(&t_add_info);
		t_add_info.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
		t_add_info.mc_index = BCM_TRUNK_UNSPEC_INDEX;
		t_add_info.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
		t_add_info.psc = 1; //Spread based on source MAC address 

	    	syslog(LOG_DEBUG,"LAG Member count = %d before setting LAG",member_count);
		/* Based on trunk ID, reset LAG and then add ports to corresponding LAG based on trunkid*/
		if(tid ==0){
			PortsInLag0_g = 0;
		}
		else
		{
			PortsInLag1_g = 0;
		}
		bcm_lag_portbit_set_trunk(tid);
		/* Setting the TRUNK with number of ports and count */
		rv = bcm_trunk_set(DEFAULT_UNIT_INIT_ID, tid, &t_add_info, member_count, member_array);
		if (rv < 0) 
		{
		     syslog(LOG_ERR,"bcm_trunk_set failed with rv = %d so destroying the TRUNK %d", rv, tid);
		     bcm_trunk_destroy(DEFAULT_UNIT_INIT_ID, tid);
		     if(member_array)
			     sal_free(member_array);

		     return -1;
		}
		syslog(LOG_DEBUG,"LAG setting successful with rv = %d", rv);
	break;
        case 'i':
		for(i = 0; i < 2; i++)
		{
			 syslog(LOG_DEBUG,"Going to create TRUNK with ID = %d", i);
			rv = bcm_trunk_create(DEFAULT_UNIT_INIT_ID, BCM_TRUNK_FLAG_WITH_ID, &i);
			if(rv != BCM_E_NONE)
			{
				 syslog(LOG_ERR,"bcm_trunk_create failed with rv=%d", rv);
				return rv;
			}
		}
            return 0;
            case 'h':
             syslog(LOG_DEBUG,"Options:\n"
                       "\t-g <LAG group_id>  1 for LAG1,2 for LAG2\n"
                       "\t-p <port_num>  port that will be part of this LAG,LAG is down if no ports configured for a LAG group\n");
            rv = 1;
            break;
        }
    }
    return rv;
}

/*********************************************************************
 * Function: mirror
 * Arguments: command line arguments
 * Return value: 
 *      0 for success
 *      1 for incorrect command line arguments
 *      BCM_E_XXX for switch configuration failures
 * Description:
 * This routine is called through bcmssdk command. This would configure 
 * port mirroring on BCM53128M chipset. 
 * A mirror port can be any port on the switch which obeys the following
 * guideline.
 * Rx & Tx are mirrored to same monitor port on RV260
 * Rx & Tx mask should be same. 
 *********************************************************************/
int mirror(int argc, char *argv[])
{
        int32 rv=-1,opt;
	soc_port_t  port, dport;

        char *cptr = NULL;
        int32 mir_portnum = 0,init_status=-1;
        int mon_portnum=0;

        optind = 0;
        while ((opt = getopt(argc, argv, "d:m:p:s:i:h")) != -1) {
            switch (opt) {
            case 'm':
            	errno = 0;
		mon_portnum = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
	      	if (rv_prod_type_g  == PROD_TYPE_RV_160)
		{
			if (errno != 0 ||  *cptr != '\0' || (mon_portnum < BCM53134_MIN_PORT) || (mon_portnum > BCM53134_MAX_PORT))
			{
				 syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 0 to 3]", mon_portnum);
                		rv = CONFIG_ERROR;
				return -1;
			}
		}
		else if (rv_prod_type_g  == PROD_TYPE_RV_260)
		{
			if (errno != 0 ||  *cptr != '\0' || (mon_portnum < BCM53128_MIN_PORT) || (mon_portnum > BCM53128_MAX_PORT))
			{
				 syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 0 to 7]", mon_portnum);
				rv = CONFIG_ERROR;
				return -1;
			}
		}
		else
		{
			syslog(LOG_ERR, " Invalid PID ");
			return -1;
            	}
            	syslog(LOG_DEBUG, "PORT %d  will be monitor port ", mon_portnum);
		/** Enable Mirroring on BCM Switch **/
		rv = bcm_mirror_mode_set(DEFAULT_UNIT_INIT_ID, BCM_MIRROR_L2);
		if (rv != BCM_E_NONE)
		{
			 syslog(LOG_ERR,"Mirror Mode ENABLE failed with retval = %d", rv);
			return rv;
		}
		else
		{
			 syslog(LOG_INFO,"BCM Mirror Mode ENABLE successfully");
		}
		
		/** mirror-to-port **/
		rv = bcm_mirror_to_set(DEFAULT_UNIT_INIT_ID, mon_portnum);
		if (rv != BCM_E_NONE)
		{
			 syslog(LOG_ERR,"bcm_mirror_to_set  failed with retval = %d", rv);
			 return rv;
		}
		else
		{
			 syslog(LOG_DEBUG,"bcm_mirror_to_set successfully");
		}
            break;
        case 'p':
            errno = 0;
	    mir_portnum = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
	    if (rv_prod_type_g  == PROD_TYPE_RV_160)
            {
		if (errno != 0 ||  *cptr != '\0' || (mir_portnum < BCM53134_MIN_PORT) || (mir_portnum > BCM53134_MAX_PORT))
		{
			 syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 1 to 4]", mir_portnum);
                	rv = CONFIG_ERROR;
			return -1;
            	}
            }
            else if (rv_prod_type_g  == PROD_TYPE_RV_260)
            {
		if (errno != 0 ||  *cptr != '\0' || (mir_portnum < BCM53128_MIN_PORT)  || (mir_portnum > BCM53128_MAX_PORT))
		{
			 syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 1 to 8]", mir_portnum);
			rv = CONFIG_ERROR;
			return -1;
		}
            }
            else
            {
              	 syslog(LOG_ERR," Invalid PID ");
		return -1;
            }

            syslog(LOG_DEBUG,"PORT %d rx & tx will be mirrored", mir_portnum);
	    rv = bcm_mirror_ingress_set(DEFAULT_UNIT_INIT_ID, mir_portnum, 1);
            if (rv < 0) 
	    {
                 syslog(LOG_ERR,"bcm_mirror_ingress_get() failed with retval = %d", rv);
            }

	    rv = bcm_mirror_egress_set(DEFAULT_UNIT_INIT_ID, mir_portnum, 1);
            if (rv < 0) 
	    {
                 syslog(LOG_ERR,"bcm_mirror_ingress_get() failed with retval = %d", rv);
            }
            break;
  	case 'd':
		/** Disable mirroring **/
		rv = bcm_mirror_mode_set(DEFAULT_UNIT_INIT_ID, BCM_MIRROR_DISABLE);
                if (rv != BCM_E_NONE)
                         syslog(LOG_ERR,"Mirroring DISABLE failed with retval = %d", rv);
                else
                         syslog(LOG_INFO,"Mirroring DISABLE successfully");

                return rv;
        
  	case 's':
	
		 rv = bcm_mirror_to_get(DEFAULT_UNIT_INIT_ID, &port);
		if (rv != BCM_E_NONE)
			 syslog(LOG_ERR,"bcm_mirror_to_get  failed with retval = %d", rv);
		else
			 syslog(LOG_INFO,"bcm_mirror_to_get successfully Mirror Port is [%d]", port);
	

	break;
  	case 'i':
            errno = 0;
            init_status = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (errno != 0 || *cptr != '\0'
                || init_status != 1)
            {
                rv = CONFIG_ERROR;
                break;
            }
#if 0
                rv = bcm_mirror_init(DEFAULT_UNIT_INIT_ID);
                if (rv != BCM_E_NONE)
                         syslog(LOG_ERR,"Mirror init failed with retval = %d", rv);
                else
                         syslog(LOG_DEBUG,"BCM Mirror initialize successfully");
#endif			
	 break;
         case 'h':
            syslog(LOG_DEBUG, "Options:\n"
                       "\t-m <monitor portnum>  set port number\n"
                       "\t-p <rx & tx mirror portnum>  set port number\n"
                       "\tex:\n\t bcmssdk -M 1 -m 4 -p 1 -p 2\n\t"
                       "\tex:\n\t bcmssdk -M 1 -m 2 -p 3 -p 4 \n\t");
            /*fall-through to default case since there is no more action */
        default:
            rv = CONFIG_ERROR;
            break;

        }
    }

        return rv;
}

/* ********************************************************************
 * Function: linkset
 * Arguments: command line arguments
 * Return value: 
 *      0 for success
 *      CONFIG_ERROR (= 1) for incorrect command line arguments
 *      BCM_E_XXX for switch configuration failures
 * Description:
 * This routine is called through bcmssdk command. This would configure 
 * follwoing port settings based on command line arguments
 * 1. port ENABLE/DISABLE
 * 2. port flow control status
 * 3. port eee status
 * 4. port mode : half10/half100/full10/full100/full1000/auto
 * ********************************************************************/
int linkset(int argc, char *argv[])
{

    int32 retval = -1, ii;
    char *cptr = NULL;
    int32 opt, portnum = -1, port_status = -1, eee_status = -1, linkmonitor = -1;
    bcm_port_cfg_t  pcfg;
    bcm_robo_phy_cfg_t  *phy_cfg;
    int rv = BCM_E_NONE, i;
    bcm_port_info_t     *port_info;
    int speed , enable, eee_state, autoneg = 0, duplex = -1, jam = -1, txpause, rxpause,flow_control = -1;
    bcm_port_abil_t ability, remote_ability;
    optind = 0;
    
        while ((opt = getopt(argc, argv, "g:p:f:e:s:m:l:h")) != -1) {
            switch (opt) {
            case 'g':
		retval = bcm_port_speed_get(DEFAULT_UNIT_INIT_ID, portnum, &speed);
		if(retval != BCM_E_NONE)
		{
			syslog(LOG_ERR,"bcm_port_speed_get failed with retval = %d", retval);
		}
		retval = bcm_port_duplex_get(DEFAULT_UNIT_INIT_ID, portnum, &duplex);
		if(retval != BCM_E_NONE)
		{
			syslog(LOG_ERR,"bcm_port_duplex_get failed with retval = %d", retval);
		}
		retval = bcm_port_autoneg_get(DEFAULT_UNIT_INIT_ID, portnum, &autoneg);
		if(retval != BCM_E_NONE)
		{
			syslog(LOG_ERR,"bcm_port_autoneg_get failed with retval = %d", retval);
		}
		retval = bcm_port_jam_get(DEFAULT_UNIT_INIT_ID, portnum, &jam);
                if(retval != BCM_E_NONE)
	                syslog(LOG_ERR,"bcm_port_jam_set failed with retval = %d", retval);

		 retval = bcm_port_pause_get(DEFAULT_UNIT_INIT_ID, portnum, &txpause, &rxpause);
                 if(retval != BCM_E_NONE)
                 	syslog(LOG_DEBUG,"bcm_port_pause_set failed with retval = %d", retval);
		
		syslog(LOG_DEBUG,"LAN%d:Speed=%d,Duplex=%d,Autoneg=%d,jam=%d,TxPAUse=%d,RxPAUse=%d",portnum,speed,duplex,autoneg,jam,txpause,rxpause);
	    	return 0;
            case 'p':
                  errno = 0;
                  portnum = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
		  if (rv_prod_type_g  == PROD_TYPE_RV_160)
                  {
			if (errno != 0 || *cptr != '\0' || (portnum < BCM53134_MIN_PORT)  || (portnum > BCM53134_MAX_PORT))
			{
				syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 1 to 4]", portnum);
				retval = CONFIG_ERROR;
				return -1;
			}
                  } 
                  else if (rv_prod_type_g  == PROD_TYPE_RV_260)
                  {
			if (errno != 0 || *cptr != '\0' || (portnum < BCM53128_MIN_PORT)  || (portnum > BCM53128_MAX_PORT))
			{
				syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 1 to 8]", portnum);
				retval = CONFIG_ERROR;
				return -1;
			}
                  }
                  else
                  {
			syslog(LOG_ERR, " Invalid PID ");
			return -1;
                  }
		syslog(LOG_INFO,"Switch port(%d) configuration", portnum);
                break;

	  case 'f':           /*flow control as passed by user */
                errno = 0;
                if (!strncmp(optarg, "enable", 6)) 
		{
			flow_control = 1;
		} else if (!strncmp(optarg, "disable", 6)) {
			flow_control = 0;
                } else {
                    retval = CONFIG_ERROR;
                    return retval;
                }
		
                if(flow_control == 1)
                {
                        retval = bcm_port_pause_set(DEFAULT_UNIT_INIT_ID, portnum, 1, 1);
			if(retval != BCM_E_NONE)
			{
                        	syslog(LOG_ERR,"bcm_port_pause_set failed with retval = %d", retval);
				return -1;
			}
			syslog(LOG_DEBUG,"Flow control enabled successfully");
			retval = bcm_port_pause_get(DEFAULT_UNIT_INIT_ID, portnum, &txpause, &rxpause);
			if(retval != BCM_E_NONE)
				syslog(LOG_ERR,"switch : bcm_port_pause_get failed with retval = %d", retval);

			syslog(LOG_INFO,"Txpause=%d,Rxpause=%d",txpause,rxpause);
		}
                else if(flow_control == 0)
		{
			retval = bcm_port_pause_set(DEFAULT_UNIT_INIT_ID, portnum, 0, 0);
			if(retval != BCM_E_NONE)
			{
			        syslog(LOG_ERR,"bcm_port_pause_set failed with retval = %d", retval);
				return -1;
			}
			syslog(LOG_INFO,"Flow control disabled successfully");
			retval = bcm_port_pause_get(DEFAULT_UNIT_INIT_ID, portnum, &txpause, &rxpause);
			if(retval != BCM_E_NONE)
				syslog(LOG_ERR,"switch : bcm_port_pause_get failed with retval = %d", retval);

				syslog(LOG_INFO,"Txpause=%d,Rxpause=%d",txpause,rxpause);
		}
		else
		{
                    syslog(LOG_ERR,"[%s:%d] Something Wrong check", __FUNCTION__, __LINE__);
		}
                break;
 	     case 'e':           /*EEE status as passed by user */
                errno = 0;
		eee_status=(int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
		if (eee_status != 0 && eee_status != 1) 
		{
		    retval = CONFIG_ERROR;
		    return retval;
                }
		if (!strncmp(optarg, "enable", 6)) {
			retval = bcm_port_control_set(DEFAULT_UNIT_INIT_ID, portnum, bcmPortControlEEEEnable, 1);
			if(retval != BCM_E_NONE)
				syslog(LOG_ERR," bcm_port_control_set failed with retval %d",retval);
                } else if (!strncmp(optarg, "disable", 6)) 
		{
			retval = bcm_port_control_set(DEFAULT_UNIT_INIT_ID, portnum, bcmPortControlEEEEnable, 0);
			if(retval != BCM_E_NONE)
				syslog(LOG_ERR," bcm_port_control_set failed with retval %d",retval);
                } else {
                    retval = CONFIG_ERROR;
                    return retval;
                }
	
		retval = bcm_port_control_get(DEFAULT_UNIT_INIT_ID, portnum, bcmPortControlEEEEnable, &eee_state);
		if(retval != BCM_E_NONE)
			syslog(LOG_ERR,"bcm_port_control_get failed with retval %d",retval);
		else
			syslog(LOG_INFO,"EEE %d",eee_state);

                break;
	     case 's':
			port_status=(int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
	                if (port_status != 0 && port_status != 1) {
        	            retval = CONFIG_ERROR;
                	    return retval;
                	}
			if(port_status == 1)
			{
				retval = bcm_port_enable_set(DEFAULT_UNIT_INIT_ID, portnum, 1);
				if(retval != BCM_E_NONE)
					syslog(LOG_ERR," bcm_port_enable_set failed with retval %d",retval);
			}
			else if (port_status == 0)
			{
				retval = bcm_port_enable_set(DEFAULT_UNIT_INIT_ID, portnum, 0);
				if(retval != BCM_E_NONE)
					syslog(LOG_ERR," bcm_port_enable_set failed with retval %d",retval);
			}
			retval = bcm_port_enable_get(DEFAULT_UNIT_INIT_ID, portnum, &enable);
			if(retval != BCM_E_NONE)
				syslog(LOG_ERR," bcm_port_enable_get failed with retval %d",retval);
			else
				syslog(LOG_INFO,"port (%d) Enable = %d",portnum, enable);
		break;
	     case 'm':           /*mode as passed by user */
                errno = 0;
                if (!strncmp(optarg, "auto", 4)) 
		{

			if ((retval = bcm_port_autoneg_set(DEFAULT_UNIT_INIT_ID, portnum, 1)) < 0)
			{
                                syslog(LOG_ERR,"bcm_port_autoneg_set failed with retval = %d", retval);
				return -1;
                        }
                        syslog(LOG_INFO,"Auto-negotiation enabled successfully");

		}
		else 
		{
			if (!strcmp(optarg, "half10")) 
			{
				rv = bcm_port_duplex_set(DEFAULT_UNIT_INIT_ID, portnum, BCM_PORT_DUPLEX_HALF);
			  	if(rv != BCM_E_NONE)
					syslog(LOG_ERR,"Failed to set the duplex configuration with rv=%d", rv);

				syslog(LOG_INFO,"Setting speed to half-10");
				rv = bcm_port_speed_set(DEFAULT_UNIT_INIT_ID, portnum, 10);
				if(rv != BCM_E_NONE)
					syslog(LOG_ERR,"Failed to set speed of port configuration with rv=%d", rv);
                	} 
			else if (!strcmp(optarg, "full10")) 
			{
				rv = bcm_port_duplex_set(DEFAULT_UNIT_INIT_ID, portnum, BCM_PORT_DUPLEX_FULL);
			  	if(rv != BCM_E_NONE)
					syslog(LOG_ERR,"Failed to set the duplex configuration with rv=%d", rv);

				syslog(LOG_INFO,"Setting speed to full-10");
				rv = bcm_port_speed_set(DEFAULT_UNIT_INIT_ID, portnum, 10);
				if(rv != BCM_E_NONE)
					syslog(LOG_ERR,"Failed to set the port configuration with rv=%d", rv);
                	} 
			else if (!strcmp(optarg, "half100")) 
			{
				rv = bcm_port_duplex_set(DEFAULT_UNIT_INIT_ID, portnum, BCM_PORT_DUPLEX_HALF);
				if(rv != BCM_E_NONE)
					syslog(LOG_ERR,"Failed to set duplex configuration with rv=%d", rv);

				syslog(LOG_INFO,"Setting speed to half-100");
				rv = bcm_port_speed_set(DEFAULT_UNIT_INIT_ID, portnum, 100);
				if(rv != BCM_E_NONE)
					syslog(LOG_ERR,"Failed to set the port configuration with rv=%d", rv);
                	} 
			else if (!strcmp(optarg, "full100")) 
			{
				syslog(LOG_INFO,"Setting speed to full-100");
				rv = bcm_port_duplex_set(DEFAULT_UNIT_INIT_ID, portnum, BCM_PORT_DUPLEX_FULL);
			  	if(rv != BCM_E_NONE)
					syslog(LOG_ERR,"Failed to set the duplex configuration with rv=%d", rv);

				rv = bcm_port_speed_set(DEFAULT_UNIT_INIT_ID, portnum, 100);
				if(rv != BCM_E_NONE)
					syslog(LOG_ERR,"Failed to set the port configuration with rv=%d", rv);
                	} 
			else if (!strcmp(optarg, "full1000"))
			{
				syslog(LOG_INFO,"Setting speed to full-1000");
				rv = bcm_port_duplex_set(DEFAULT_UNIT_INIT_ID, portnum, BCM_PORT_DUPLEX_FULL);
				if(rv != BCM_E_NONE)
					syslog(LOG_ERR,"Failed to set the duplex configuration with rv=%d", rv);
			
				rv = bcm_port_speed_set(DEFAULT_UNIT_INIT_ID, portnum, 1000);
				if(rv != BCM_E_NONE)
					syslog(LOG_ERR,"Failed to set speed of port configuration with rv=%d", rv);
			}
                	else 
			{
				syslog(LOG_ERR, "Invalid switch case...check it");
                    		retval = CONFIG_ERROR;
                    		return retval;
                	}
		} 
                break;
	     case 'l':
	    errno = 0;
	    linkmonitor=(int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
            if (linkmonitor == 1) 
	    {
            	syslog(LOG_DEBUG,"Register link monitor function");
		if(bcm_linkscan_register(DEFAULT_UNIT_INIT_ID,(bcm_linkscan_handler_t) nxp_port_linkMon_callback) != BCM_E_NONE)
		{
                        syslog(LOG_ERR,"802.1x: bcm_robo_linkscan_register failed");
                        return -1;
		}

	    	if (rv_prod_type_g  == PROD_TYPE_RV_160)
            	{
			for(ii = 0;ii < 4;ii++)	
	      		{
				if(bcm_linkscan_mode_set(DEFAULT_UNIT_INIT_ID, ii , BCM_LINKSCAN_MODE_SW) != BCM_E_NONE)
				{
                        		syslog(LOG_ERR,"802.1x: bcm_linkscan_mode_set failed for port %d", ii);
                        		return -1;
                		}
			}
	    	}
            	else if (rv_prod_type_g  == PROD_TYPE_RV_260)
            	{
			for(ii = 0;ii < 8;ii++)	
	      		{
				if(bcm_linkscan_mode_set(DEFAULT_UNIT_INIT_ID, ii , BCM_LINKSCAN_MODE_SW) != BCM_E_NONE)
				{
                        		syslog(LOG_ERR,"802.1x: bcm_linkscan_mode_set failed for port %d", ii);
                        		return -1;
                		}
			}
            	}
            	else
            	{
                	syslog(LOG_DEBUG," Invalid PID ");
			return -1;
            	}
	
	    	if(bcm_linkscan_enable_set(DEFAULT_UNIT_INIT_ID, 1000000) != BCM_E_NONE)
	    	{
            		syslog(LOG_ERR,"802.1x: bcm_robo_linkscan_enable_set failed");
                	return -1;
	    	}
            	syslog(LOG_DEBUG,"Register link monitor function successfully");
            }
            else if(linkmonitor == 0)
            {
		syslog(LOG_DEBUG,"Un-Register link monitor function");
		if(bcm_linkscan_unregister(DEFAULT_UNIT_INIT_ID,(bcm_linkscan_handler_t)nxp_port_linkMon_callback) != BCM_E_NONE)
		{
			syslog(LOG_ERR,"802.1x: bcm_robo_linkscan_unregister failed");
			return -1;
		}
		syslog(LOG_DEBUG,"Un-Register link monitor function successfully");
	    }
	     break;
             case 'h':
                syslog(LOG_DEBUG,"Options:\n"
                           "\t-p <portnum>  set port number\n"
                           "\t-f <enable/disable>  set flow_control to enable/disable\n"
                           "\t-e <enable/disable>  set eee status to enable/disable\n"
                           "\t-m <mode>  set (half10/full10/half100/full100/full1000/auto).\n");
                retval = CONFIG_ERROR;
	        break;
	    }
	}
	return 0;
}

int init(int argc, char *argv[])
{
	int retval = -1;
	int             mod, rv;
        bcm_port_t      xport;

	 uint32      flags = 0;
        int module_num;
        int found;
        int32_t opt;
        char *cptr = NULL;

	while ((opt = getopt(argc, argv, "m:h")) != -1) {
		switch (opt) {
		    case 'm':
			errno = 0;
			module_num = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
			if (errno != 0 || *cptr != '\0' || module_num < 1 || module_num > 47) 
			{
			    retval = CONFIG_ERROR;
				 syslog(LOG_ERR,"VLANID = %d ERROR",module_num);
			    break;
			}
			syslog(LOG_ERR,"Module id = %d",module_num);
			break;
	
                    if ((retval = bcm_init_selective(DEFAULT_UNIT_INIT_ID, module_num)) < 0) {
                         syslog(LOG_ERR,"Unable to initialize imodule %d", module_num);
                         retval = -1;
                     }
                }
            }

	return 0;
}

/* ********************************************************************
 * Function: vlan
 * Arguments: command line arguments
 * Return value: 
 *      0 for success
 *      1 for incorrect command line arguments
 *      BCM_E_XXX for switch configuration failures
 * Description:
 * This routine is called through bcmssdk command. This would configure 
 * VLAN membership on BCM53128M chipset. There are
 * three possible membership modes on each port. Tagged, Untagged and 
 * Excluded. When a switch port is configured as tagged for this VLAN,
 * the port will tag this VLAN for all outgoing packets from the port.
 * When a switch port is configured as untagged for this VLAN, the port
 * will remove tag for all outgoing packets. When port is configured as
 * excluded for this VLAN, all tagged packets with this VLAN will be 
 * dropped by the port
 **********************************************************************/
int vlan(int argc, char *argv[])
{
	int retval = -1, portnum = -1, i;
	bcm_vlan_t 	vlanid;
	int32_t 	opt;
	char *cptr = 	NULL;
	int exclude = -1;
	uint32   	outer_tag, inner_tag; 
	pbmp_t       	port_pbm;
	pbmp_t          port_ubm;

	BCM_PBMP_CLEAR(port_pbm);
	BCM_PBMP_CLEAR(port_ubm);
	
	while ((opt = getopt(argc, argv, "c:u:s:t:v:d:h")) != -1) {
		switch (opt) {
		    case 'v':
			errno = 0;
			vlanid = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
			if (errno != 0 || *cptr != '\0' || vlanid < 1 || vlanid > 4095) 
			{
			    	retval = CONFIG_ERROR;
				syslog(LOG_ERR,"VLANID = %d ERROR",vlanid);
			    	break;
			}
			syslog(LOG_INFO,"VLAN with vid = %d",vlanid);
		break;
		case 's':
			if(1)
			{	
				bcm_vlan_data_t *list;
				int             count, alloc_pfmtp,i;
				char            *pfmtp = NULL;

				 alloc_pfmtp = SOC_PBMP_FMT_LEN * sizeof(*pfmtp);
				    pfmtp = sal_alloc(alloc_pfmtp, "CLI VLAN show pbmp format");
				    if (NULL == pfmtp) {
					retval = BCM_E_MEMORY;
				    }

				retval = bcm_vlan_list(DEFAULT_UNIT_INIT_ID, &list, &count);
				if(retval < 0)
				    syslog(LOG_ERR,"bcm_vlan_list failed with retval %d", retval);

				for (i = 0; i < count; i++)
				{
					syslog(LOG_DEBUG,"VLAN TAG = %d, VLAN COUNT = %d", list[i].vlan_tag, count);
					syslog(LOG_DEBUG," Port bitmap %s",SOC_PBMP_FMT(list[i].port_bitmap, pfmtp));
					syslog(LOG_DEBUG," Untag bitmap %s",SOC_PBMP_FMT(list[i].ut_port_bitmap, pfmtp));
				}
			}
			return 0;
		case 'c':
			if(vlanid != BCM_VLAN_DEFAULT) 
			{
				retval = bcm_vlan_create(DEFAULT_UNIT_INIT_ID, vlanid);
				if(retval == BCM_E_NONE)
					 syslog(LOG_DEBUG,"VLAN %d created Successfully", vlanid);
				else
					 syslog(LOG_ERR,"VLAN %d creation failed with retval = %d", vlanid, retval);
			}
		break;
		case 't':
			errno = 0;
			portnum = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
			if (rv_prod_type_g  == PROD_TYPE_RV_160) 
			{
				if (errno != 0 ||  *cptr != '\0' || (portnum < BCM53134_MIN_PORT) || (portnum > BCM53134_MAX_PORT))
				{
			    		 syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 0 to 3]", portnum);
			    		retval = CONFIG_ERROR;
			    		return -1;
				}
        		}
        		else if (rv_prod_type_g  == PROD_TYPE_RV_260)
        		{
				if (errno != 0 ||  *cptr != '\0' || (portnum < BCM53128_MIN_PORT) || (portnum > BCM53128_MAX_PORT))
				{
				     syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 0 to 7]", portnum);
				    retval = CONFIG_ERROR;
				    return -1;
				}
        		}
        		else
        		{
                		 syslog(LOG_ERR,"Invalid PID");
				return -1;
        		}
			//syslog(LOG_DEBUG,"[ -t %d]", portnum);
                        BCM_PBMP_PORT_ADD(port_pbm, portnum);
			exclude = 0;
		break;
		case 'u':
			errno = 0;
			portnum = (int32) strtoull(optarg, &cptr, INT32_MAX_BYTES);
			
			if (rv_prod_type_g  == PROD_TYPE_RV_160) 
			{
				if (errno != 0 || *cptr != '\0' || (portnum < BCM53134_MIN_PORT) || (portnum > BCM53134_MAX_PORT))
				{
			    		 syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 1 to 4]", portnum);
			    		retval = CONFIG_ERROR;
			    		return -1;
				}
        		}
        		else if (rv_prod_type_g  == PROD_TYPE_RV_260)
        		{
				if (errno != 0 || *cptr != '\0' || (portnum < BCM53128_MIN_PORT) || (portnum > BCM53128_MAX_PORT))
				{
				     syslog(LOG_ERR,"Invalid port = %d configuration [Use Port between 1 to 8]", portnum);
				    retval = CONFIG_ERROR;
				    return -1;
				}
        		}
        		else
        		{
                		 syslog(LOG_ERR,"Invalid PID");
				return -1;
			}
			//syslog(LOG_DEBUG,"[ -u %d ", portnum);
                        BCM_PBMP_PORT_ADD(port_pbm, portnum);
                        BCM_PBMP_PORT_ADD(port_ubm, portnum);

			exclude = 0;

			retval = drv_port_vlan_pvid_get(DEFAULT_UNIT_INIT_ID, portnum, &outer_tag, &inner_tag);
			if(retval != 0)
				 syslog(LOG_ERR," drv_port_vlan_pvid_get failed with retval = %d", retval);
			retval = drv_port_vlan_pvid_set(DEFAULT_UNIT_INIT_ID, portnum, vlanid, vlanid);

			//printf("Before Get outer_tag = %d, inner_tag = %d", outer_tag, inner_tag);
			if(retval != 0)
				 syslog(LOG_ERR," drv_port_vlan_pvid_set failed with retval = %d", retval);

			retval = drv_port_vlan_pvid_get(DEFAULT_UNIT_INIT_ID, portnum, &outer_tag, &inner_tag);
			if(retval != 0)
				 syslog(LOG_ERR," drv_port_vlan_pvid_get failed with retval = %d", retval);

			 //syslog(LOG_DEBUG,"Outer_tag = %d, inner_tag = %d]", outer_tag, inner_tag);
		break;
		case 'd':
			if(vlanid != BCM_VLAN_DEFAULT) 
			{
                                retval = bcm_vlan_destroy(DEFAULT_UNIT_INIT_ID, vlanid);
                                if(retval == BCM_E_NONE)
				{
                                         syslog(LOG_DEBUG,"VLAN %d deleted Successfully", vlanid);
					return 0;
                        	}
                                else
				{
                                         syslog(LOG_ERR,"VLAN %d deletion failed with retval = %d", vlanid, retval);
					return -1;
				}
			}
			else
			{
                             	syslog(LOG_DEBUG,"Deletion of VLAN=%d not allowed So reset it", vlanid);
				BCM_PBMP_CLEAR(port_pbm);
			        BCM_PBMP_CLEAR(port_ubm);
				retval = bcm_vlan_port_get(DEFAULT_UNIT_INIT_ID, vlanid, &port_pbm, &port_ubm);
				if(retval < 0)
	    				 syslog(LOG_ERR,"bcm_vlan_port_get failed with retval %d", retval);
				else
					 syslog(LOG_DEBUG,"bcm_vlan_port_get Successful for VLAN = %d", vlanid);
				
				retval = bcm_vlan_port_remove(DEFAULT_UNIT_INIT_ID, vlanid, port_pbm);
				if(retval < 0)
	    				 syslog(LOG_ERR,"bcm_vlan_port_remove failed with retval %d", retval);
				else
					 syslog(LOG_DEBUG,"bcm_vlan_port_remove done successfully for VLAN = %d", vlanid);

			     	goto show;
			}
		break;	
		case 'h':
	                 syslog(LOG_DEBUG,"Options:\n"
                           "\t-u <port_num>  range 0-3 for Rv160, 0-7 for RV260. port that untag's this vlan"
                           "\t-t <port_num>  range 0-3 for Rv160, 0-7 for RV260. port that tags this vlan"
                           "\t-i vlan initilization"
                           "\t-d delete this vlan"
                           "\t-v <vlan-id>  range 1-4095. vlan identifier");
        	        retval = 1;
                break;
           	default:
                	 syslog(LOG_DEBUG,"Invalid option %s", optarg);
		}
	}
	if(exclude == 0)
		BCM_PBMP_PORT_ADD(port_pbm, 0x8);
	
	//printf(" before port_pbm[0] = 0x%x",port_pbm.pbits[0]);
	//printf(" before port_pbm[1] = 0x%x",port_pbm.pbits[1]);
	
	retval = bcm_vlan_port_add(DEFAULT_UNIT_INIT_ID, vlanid, port_pbm, port_ubm);
	if(retval < 0)
	    syslog(LOG_ERR,"bcm_vlan_port_add failed with retval %d", retval);
/*
	else
		syslog(LOG_DEBUG,"port %d as tagged added to vlanid = %d", portnum , vlanid);
*/	

show :
	syslog(LOG_INFO,"VLAN Tagged Configuration done with the below bitmap");
	bcm_vlan_data_t *list;
	int             count, alloc_pfmtp;
	char            *pfmtp = NULL;

	 alloc_pfmtp = SOC_PBMP_FMT_LEN * sizeof(*pfmtp);
	    pfmtp = sal_alloc(alloc_pfmtp, "CLI VLAN show pbmp format");
	    if (NULL == pfmtp) {
		retval = BCM_E_MEMORY;
	    }

	retval = bcm_vlan_list(DEFAULT_UNIT_INIT_ID, &list, &count);
	if(retval < 0)
	    syslog(LOG_ERR,"bcm_vlan_list failed with retval %d", retval);

	for (i = 0; i < count; i++)
	{
		syslog(LOG_INFO,"VLAN TAG = %d, VLAN COUNT = %d", list[i].vlan_tag, count);
		syslog(LOG_INFO," Port bitmap %s",SOC_PBMP_FMT(list[i].port_bitmap, pfmtp));
		syslog(LOG_INFO," Untag bitmap %s",SOC_PBMP_FMT(list[i].ut_port_bitmap, pfmtp));
	}
	return 0;
}



int _main_(int argc, char** argv)
{
    int32 opt = 0, long_index = 0;
    static struct option long_options[] = {
        {    "help", no_argument, 0,   'h'},
        {    "init",           20, 0,  'I'},
        {    "vlan",           20, 0,  'V'},
        { "linkset",           20, 0,  'L'},
        { "mirror",            20, 0,  'M'},
        {    "lag",            20, 0,  'T'},
        { "port_stats",        20, 0,  'S'},
        {    "acl",            20, 0,  'A'},
        {    "dot1x",          20, 0,  'D'},
        {  "feature",          20, 0,  'F'},
        {  "Register",         20, 0,  'R'},
        { "qos",               20, 0,  'Q'},
        { "debug",             20, 0,  'Z'},
        {         0,           0, 0,     0}
    };

        optind = 0;
        while ((opt = getopt_long(argc, argv, "hIVLMRTSDFQZ", long_options, &long_index)) != -1) 
	{
		switch (opt) {
		    case 'I':
			init(argc-1, argv+1);
			break;
		    case 'V':
			vlan(argc-1, argv+1);
			break;
		    case 'L':
			linkset(argc-1,argv+1);
			break;
		    case 'M':
			mirror(argc-1,argv+1);
			break;
		    case 'T':
			lag(argc-1, argv+1);
			break;
		    case 'S':
			port_stats(argc-1,argv+1);
			break;
		    case 'D':
			dot1x(argc-1, argv+1);
			break;
		    case 'F':
			feature(argc-1, argv+1);
			break;
		    case 'R':
			Register(argc-1, argv+1);
			break;
		    case 'Q':
			qos(argc-1,argv+1);
			break;
		    case 'Z':
			debug(argc-1,argv+1);
			break;
		    case 'h':
		    default:
			usage(argv[0]);
			return -1;
		}
	    }
	return 0;
}

/* ********************************************************************
 * Function: get_PolePosition_PID
 * Arguments: No
 * Return value: 
 *      0 for success
 *     -1 for failure
 * Description:
 * This routine is called from robodiag to get the PP Product ID and
 *  
 **********************************************************************/
int get_PolePosition_PID()
{
  FILE *fp=NULL;
  char cmd[256];

  sprintf(cmd,"uci get systeminfo.sysinfo.pid | awk -F'-' '{print $1}'");

  if((fp = popen(cmd,"r"))!=NULL)
  {
        memset(cmd,0,256);
        fgets(cmd,256,fp);
  }
  pclose(fp);

  if (strncmp(cmd,"RV160",5) == 0)
  {
        total_ports_g = 4;
        rv_prod_type_g = PROD_TYPE_RV_160;
        syslog(LOG_DEBUG,"Board = %s with %d ports", "RV16x", total_ports_g);
        return 0;
  }
  else if (strncmp(cmd,"RV260",5) == 0)
  {
        total_ports_g = 8;
        rv_prod_type_g = PROD_TYPE_RV_260;
        syslog(LOG_DEBUG,"Board = %s with %d ports","RV26x", total_ports_g);
        return 0;
  }
  return -1;
}


/* *************************************
 * Function: robodiag_start
 * Arguments: No
 * Return value: 
 *      0 for success
 *     -1 for failure
 * Description:
 * This routine is robodiag server. 
 **************************************/
int  robodiag_start(void)
{
        int32_t retval =-1;
        int         rv=0;
        char        *msg = NULL;
	int udpSocket, nBytes;
	char *option;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	char* c[100];
	char strp[BUFFER_SIZE];
	char* head = NULL;
	int i;

	retval = get_PolePosition_PID();
	if(retval == 0)
		syslog(LOG_DEBUG,"PolePosition_PID got successfully");
	else
		syslog(LOG_ERR,"get_PolePosition_PID failed , please check port mapping may not work properly");
		

	 /*Create UDP socket*/
	if ((udpSocket = socket(PF_INET, SOCK_DGRAM, 0)) == -1 ) {
		syslog(LOG_ERR,"socket() creation failed");
		return -1;
	}

	/*Configure settings in address struct*/
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	/*Bind socket with address struct*/
	if((retval = bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr))) == -1) {
		syslog(LOG_ERR,"socket bind failed");
		close(udpSocket);
		return -1;
	}

	/*Initialize size variable to be used later on*/
	addr_size = sizeof(serverStorage);

	syslog(LOG_DEBUG,"ROBOSERVER while loop started");
	while(1)
	{
		for(i=0;i<100;i++)
		{
			if((c[i] = calloc(1,100)) == NULL) {
				syslog(LOG_ERR,"calloc(): couldn't allocate memory");
				close(udpSocket);
				return -1;
			}
        	}
		/* Try to receive any incoming UDP datagram. Address and port of 
		   requesting client will be stored on serverStorage variable */
		bzero(strp, BUFFER_SIZE);
		if((nBytes = recvfrom(udpSocket,strp,BUFFER_SIZE,0,(struct sockaddr *)&serverStorage, &addr_size)) == -1) 
		{
			syslog(LOG_ERR,"recvfrom() call failed");
			for(i=0;i<100;i++)
			{
			  free(c[i]);
			}
			close(udpSocket);
			return -1;
		}
		//syslog(LOG_DEBUG,"ROBOSERVER [%s]", strp);
		head=strp;
		for(i=0;head != NULL;i++)
		{
			option = strsep(&head," ");
			strcpy(c[i],option);
			strcat(c[i],"\0");
		}
		retval = _main_(i,c);
		if(retval == 0){
			for(i=0;i<100;i++)
                        {
	                         free(c[i]);
                        }

		}			
		else
			syslog(LOG_ERR,"Call to _main_ function failed with retval = %d", retval);
	}
	return BCM_E_NONE;
}

/*
 * Main loop.
 */
int main(int argc, char *argv[])
{
    int i, len, retval = -1;
    char *envstr;
    char *config_file, *config_temp;
#ifdef BCM_INSTANCE_SUPPORT
    const char *inst = NULL;
#endif

#ifdef BCM_BPROF_STATS
    shr_bprof_stats_time_init();
#endif

    if ((envstr = getenv("BCM_CONFIG_FILE")) != NULL) {
        config_file = envstr;
        len = sal_strlen(config_file);
        if ((config_temp = sal_alloc(len+5, NULL)) != NULL) {
            sal_strcpy(config_temp, config_file);
            sal_strcpy(&config_temp[len], ".tmp");
#ifndef NO_SAL_APPL
            sal_config_file_set(config_file, config_temp);
#endif
            sal_free(config_temp);
        }
    }

#ifdef BCM_INSTANCE_SUPPORT
    for (i = 1; i < argc; i++) {
         if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--instance")) {
            inst = argv[i+1];            
            /*
             * specify the dev_mask and its dma_size (optional,default:4MB)
             * bcm.user -i 0x1[:8]
             */
            if (_instance_config(inst) < 0){
#ifndef NO_SAL_APPL
                char *estr = "config error!\n";
                sal_console_write(estr, sal_strlen(estr) + 1);
#endif
                exit(1);
            }
        }
    }
#endif

    if (sal_core_init() < 0
#ifndef NO_SAL_APPL
        || sal_appl_init() < 0
#endif
        ) {
        /*
         * If SAL initialization fails then printf will most
         * likely assert or fail. Try direct console access
         * instead to get the error message out.
         */
#ifndef NO_SAL_APPL
        char *estr = "SAL Initialization failed\r\n";
        sal_console_write(estr, sal_strlen(estr) + 1);
#endif
        exit(1);
    }

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--reload")) {
            sal_boot_flags_set(sal_boot_flags_get() | BOOT_F_RELOAD);
        }
    }

#ifdef MEMLOG_SUPPORT
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-memlog") && argv[i+1] != NULL) {
            memlog_fd = creat(argv[i+1], S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        }
    }
    if (memlog_fd > 0) {
        memlog_lock = sal_mutex_create("memlog lock");
    }
#endif
#ifdef INCLUDE_KNET
    knet_kcom_config();
#endif
    //printf("Calling diag_shell \n");
    diag_shell();
	
	retval = system_init(DEFAULT_UNIT_INIT_ID);
	if (retval == BCM_E_NONE)
		syslog(LOG_DEBUG,"switch system_init succefully retval = %d", retval);
	else
		syslog(LOG_ERR,"switch system_init failed, retval =%d", retval);
    
    retval = soc_robo_loop_detect_enable_set(DEFAULT_UNIT_INIT_ID, BCM_LOOP_DET_DISABLE);
    if (retval == BCM_E_NONE)
	syslog(LOG_DEBUG,"LOOP DETECTION DISABLE succefully retval = %d", retval);
    else
	syslog(LOG_ERR,"soc_robo_loop_detect_enable_set failed, retval =%d", retval);
 
    robodiag_start(); 

    linux_bde_destroy(bde);
#ifdef MEMLOG_SUPPORT
    if (memlog_lock) {
        sal_mutex_destroy(memlog_lock);
    }
#endif
    return 0;
}




/*
 * These stubs are here for legacy compatability reasons. 
 * They are used only by the diag/test code, not the driver, 
 * so they are really not that important. 
 */

void pci_print_all(void)
{
    int device;

    if (NULL == bde) {
        sal_printf("Devices not probed yet.\n");
        return;
    }

    sal_printf("Scanning function 0 of devices 0-%d\n", bde->num_devices(BDE_SWITCH_DEVICES) - 1);
    sal_printf("device fn venID devID class  rev MBAR0    MBAR1    IPIN ILINE\n");

    for (device = 0; device < bde->num_devices(BDE_SWITCH_DEVICES); device++) {
	uint32		vendorID, deviceID, class, revID;
	uint32		MBAR0, MBAR1, ipin, iline;
	
	vendorID = (bde->pci_conf_read(device, PCI_CONF_VENDOR_ID) & 0x0000ffff);
	
	if (vendorID == 0)
	    continue;
	

#define CONFIG(offset)	bde->pci_conf_read(device, (offset))

	deviceID = (CONFIG(PCI_CONF_VENDOR_ID) & 0xffff0000) >> 16;
	class    = (CONFIG(PCI_CONF_REVISION_ID) & 0xffffff00) >>  8;
	revID    = (CONFIG(PCI_CONF_REVISION_ID) & 0x000000ff) >>  0;
	MBAR0    = (CONFIG(PCI_CONF_BAR0) & 0xffffffff) >>  0;
	MBAR1    = (CONFIG(PCI_CONF_BAR1) & 0xffffffff) >>  0;
	iline    = (CONFIG(PCI_CONF_INTERRUPT_LINE) & 0x000000ff) >>  0;
	ipin     = (CONFIG(PCI_CONF_INTERRUPT_LINE) & 0x0000ff00) >>  8;
	
#undef CONFIG

	sal_printf("%02x  %02x %04x  %04x  "
		   "%06x %02x  %08x %08x %02x   %02x\n",
		   device, 0, vendorID, deviceID, class, revID,
		   MBAR0, MBAR1, ipin, iline);
    }
}
