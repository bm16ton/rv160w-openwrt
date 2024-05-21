#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/bccsdk.h"
#include "lib/bc_url.h"
#include "lib/bc_lcp.h"
#include "lib/bc_string.h"
#include "lib/bc_net.h"
#include "lib/bc_proto.h"
#include "lib/bc_queue.h"
#include "lib/bc_db.h"
#include "lib/bc_rtu.h"
#include "lib/bc_alloc.h"
#include "lib/bc_stats.h"
#include "lib/bc_cache.h"

#include "lc_api.h"

#define LC_IPV4_FMT "%u.%u.%u.%u"

#define LC_IPV6_FMT "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x"

#define LC_IPV4(addr) \
		((unsigned char *)&(addr))[0], ((unsigned char *)&(addr))[1], ((unsigned char *)&(addr))[2], ((unsigned char *)&(addr))[3]

#define LC_IPV6(addr6) \
		((unsigned char *)addr6)[0],  ((unsigned char *)addr6)[1],  ((unsigned char *)addr6)[2],  ((unsigned char *)addr6)[3],  \
		((unsigned char *)addr6)[4],  ((unsigned char *)addr6)[5],  ((unsigned char *)addr6)[6],  ((unsigned char *)addr6)[7],  \
		((unsigned char *)addr6)[8],  ((unsigned char *)addr6)[9],  ((unsigned char *)addr6)[10], ((unsigned char *)addr6)[11], \
		((unsigned char *)addr6)[12], ((unsigned char *)addr6)[13], ((unsigned char *)addr6)[14], ((unsigned char *)addr6)[15]

static struct lc_api_info* g_handle = NULL;

static void signal_handler(int signo)
{
	if(signo == SIGTERM || signo == SIGKILL || signo == SIGINT)
	{
		printf("Shutdown agent");
		if(g_handle)
		{
			lc_api_exit(g_handle);
		}
		exit(0);
	}
}

static void init_signal()
{
	struct sigaction act;

	act.sa_handler = signal_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGKILL, &act, NULL);
	sigaction(SIGINT, &act, NULL);
}

static int bc_init(void)
{
	struct bc_init_params params = { 0 };

	params.max_cache_mb = 4;
	params.lcp_loop_limit = 6;
	params.timeout = 60;
	params.keep_alive = 60;
	params.ssl = 0;
	params.log_stderr = 1;
	params.enable_stats = 0;
	params.enable_rtu = 0;
	params.rtu_poll = 300;
	params.pool_size = 2;
	params.queue_poll = 8;
	params.poller = 0;

	bc_cpystrn(params.device, "DeviceId_cisco", sizeof(params.device));
	bc_cpystrn(params.oem, "BrightCloudSdk", sizeof(params.oem));
	bc_cpystrn(params.uid, "cisco_freescale_98uw", sizeof(params.uid));
	bc_cpystrn(params.server, "bcap15.brightcloud.com", 256);
	bc_cpystrn(params.dbpath, "/tmp", 2048);
	bc_cpystrn(params.dbserver, "database.brightcloud.com", 2048);
	bc_cpystrn(params.cert, "thawteCA.pem", 2048);

	return bc_initialize(&params);
}

static void test_api(const char* req_info, int* category_array)
{
	static unsigned int serial = 0;
	int res;
	struct bc_url_request req;
	int i;

	unsigned char  *mac = lc_api_get_mac(req_info);
	unsigned short *ip_type = lc_api_get_iptype(req_info);
	unsigned int *sip = lc_api_get_sip(req_info);
	unsigned int *dip = lc_api_get_dip(req_info);
	char *os   = lc_api_get_os(req_info);
	char *type = lc_api_get_type(req_info);
	char *host = lc_api_get_url(req_info);

	// For testing
	if((*ip_type) == 0)
	{
		printf("here %02X:%02X:%02X:%02X:%02X:%02X " LC_IPV4_FMT " " LC_IPV4_FMT " %s %s %s\n", 
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], LC_IPV4(sip[0]), LC_IPV4(dip[0]), os, type, host);
	}
	else
	{
		printf("here %02X:%02X:%02X:%02X:%02X:%02X " LC_IPV6_FMT " " LC_IPV6_FMT " %s %s %s\n", 
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], LC_IPV6(sip), LC_IPV6(dip), os, type, host);
	}

#if 1
	// Put RES_PASS into the array[0] and we will bypass this url
	category_array[0] = 2;
#else 

	if (bc_init_request(&req, host, 1, ++serial) == -1)
	{
		fprintf(stderr, "Error initializing request for %s\n", host);
		return;
	}

	res = bc_cache_lookup(&req);

	if (res == 0)
	{
		if (bc_cloud_lookup(&req, 0) == -1)
		{
			printf("Cloud lookup for %s failed\n", req.url);
			return;
		}
	}

	// Put category into the array and we will block this url
	for (i = 0; i < REP_IDX; ++i)
	{
		category_array[i] = req.data.cc[i].category;
	}

	// Put reputation into the array[REP_INDEX]
	category_array[REP_IDX] = req.data.reputation;
#endif
}

int main(int argc, char **argv)
{
	int i;

	if (bc_init() == -1)
		return 1;

	init_signal();
	
	g_handle = lc_api_init();

	while (g_handle)
	{
		if (lc_api_hook(g_handle) == -1)
		{
			break;
		}

		for (i = 0; i < g_handle->query_info.query_num; ++i)
		{
			test_api(g_handle->query_info.query_blocks[i].req_info, g_handle->query_info.query_blocks[i].result);
		}
	}

	return 0;
}

