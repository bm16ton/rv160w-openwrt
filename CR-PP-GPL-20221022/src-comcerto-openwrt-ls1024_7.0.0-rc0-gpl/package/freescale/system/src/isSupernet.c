/*
Copyright 2020 NXP.

NXP Confidential. This software is owned or controlled by NXP and may only be used strictly in accordance
 with the applicable license terms.  By expressly accepting such terms or by downloading, installing, activating
and/or otherwise using the software, you are agreeing that you have read, and that you agree to comply with and are
bound by, such license terms.  If you do not agree to be bound by the applicable license terms, then you may not
retain, install, activate or otherwise use the software.
*/

#include<stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define MAX_IP_ADDRS_PER_GROUP	50
#define MAX_IP_ADDR_SIZE	19

//#define DEBUG 1

#ifdef DEBUG
#define debug_print(fmt, arg...) \
		printf(fmt, ##arg)
#else
#define debug_print(fmt, arg...)
#endif

typedef struct network_node_s {
	char ipaddr[MAX_IP_ADDR_SIZE];
	struct in_addr network_addr;
	struct in_addr netmask_addr;
}network_node_t;

network_node_t gRemote_networks[MAX_IP_ADDRS_PER_GROUP];
int num_remote_networks=0;

void help(char *program)
{
	printf("Help:\r\n");
	printf("%s <left-networks> <right-networks>\n",
		program);
}

void load_remote_networks(char *remote_networks)
{
	char network[MAX_IP_ADDR_SIZE], mask[4];
	char *p, *tmp;
	unsigned int full_mask = 0xFFFFFFFF;

	debug_print("Remote Networks:\r\n");
	while (p=strsep(&remote_networks, ","))
	{
		memset(network, 0, sizeof(network));
		memset(mask, 0, sizeof(mask));
		debug_print("Network:%18s\t", p);
		strcpy(gRemote_networks[num_remote_networks].ipaddr, p);
		tmp = strchr(p, '/');

		if(tmp)
		{
			strncpy(network, p, tmp-p);
			network[tmp-p+1]='\0';
			strcpy(mask, p+strlen(network)+1);
		}
		else
		{
			strcpy(network,p);
			strcpy(mask, "32");
		}

		gRemote_networks[num_remote_networks].netmask_addr.s_addr = htonl(full_mask << (32 - atoi(mask)));
		inet_aton(network, &gRemote_networks[num_remote_networks].network_addr);

		gRemote_networks[num_remote_networks].network_addr.s_addr &= gRemote_networks[num_remote_networks].netmask_addr.s_addr;
		debug_print("network:%s(0x%8X) mask:%s(0x%8X)\r\n", network, gRemote_networks[num_remote_networks].network_addr.s_addr,mask, gRemote_networks[num_remote_networks].netmask_addr.s_addr);
		num_remote_networks++;
	}
}

void parse_local_networks(char *local_networks)
{
	char network[MAX_IP_ADDR_SIZE], mask[4];
	char *p, *tmp;
	unsigned int full_mask = 0xFFFFFFFF;
	struct in_addr local_network, local_network_mask;
	int i;

	debug_print("Local Networks:\r\n");
	while (p=strsep(&local_networks, ","))
	{
		memset(network, 0, sizeof(network));
		memset(mask, 0, sizeof(mask));
		memset(&local_network, 0, sizeof(local_network));
		memset(&local_network_mask, 0, sizeof(local_network_mask));
		debug_print("Network:%18s\t", p);
		tmp = strchr(p, '/');

		if(tmp)
		{
			strncpy(network, p, tmp-p);
			network[tmp-p+1]='\0';
			strcpy(mask, p+strlen(network)+1);
		}
		else
		{
			strcpy(network,p);
			strcpy(mask, "32");
		}

		local_network_mask.s_addr = htonl(full_mask << (32 - atoi(mask)));
		inet_aton(network, &local_network);

		local_network.s_addr &= local_network_mask.s_addr;

		debug_print("network:%s(0x%8X) mask:%s(0x%8X)\r\n", network, local_network.s_addr,mask, local_network_mask.s_addr);
		for(i=0; i<num_remote_networks; i++)
			if((local_network.s_addr & gRemote_networks[i].network_addr.s_addr) == gRemote_networks[i].network_addr.s_addr)
			{
				printf("1");
				exit(1);
				debug_print("Local network 0x%X/0x%X is subnet of 0x%X/0x%X\r\n",local_network.s_addr,local_network_mask.s_addr, gRemote_networks[i].network_addr.s_addr, gRemote_networks[i].netmask_addr.s_addr);
			}
	}
}


int main(int argc, char *argv[])
{
	if (argc != 3){
		help(argv[0]);
		return 0;
	}
	debug_print("Local Networks:%s  Remote Networks:%s\n", argv[1], argv[2]);
	load_remote_networks(argv[2]);

	parse_local_networks(argv[1]);
	printf("0");
	return 0;
}
