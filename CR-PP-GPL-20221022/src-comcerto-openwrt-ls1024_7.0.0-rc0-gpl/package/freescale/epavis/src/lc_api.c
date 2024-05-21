#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "lc_api.h"

#define LC_API_DEV_NAME         "se_uf"

#define UF_IOCTL_INIT           20
#define UF_IOCTL_HOOK           21
#define UF_IOCTL_HOOK_STATUS 	22

#define AGT_HOOK_STATE_HOOK     2
#define AGT_HOOK_STATE_UNHOOK   3

static const unsigned int g_state_hook = AGT_HOOK_STATE_HOOK;
static const unsigned int g_state_unhook = AGT_HOOK_STATE_UNHOOK;

struct lc_api_info* lc_api_init(void)
{
	char dev_name[40];
	struct lc_api_info* handle = NULL;

	handle = malloc(sizeof(struct lc_api_info));
	if(!handle)
	{
		goto init_error;
	}
	memset(handle, 0, sizeof(struct lc_api_info));

	snprintf(dev_name, 40, "/dev/%s", LC_API_DEV_NAME);
	handle->drv_fd = open(dev_name, O_RDWR | O_SYNC);

	if(handle->drv_fd == -1)
	{
		printf("[%s]: Failed to open %s (%d)\n", __FUNCTION__, dev_name, errno);
		goto init_error;
	}

	if(ioctl(handle->drv_fd, UF_IOCTL_INIT, NULL) == -1)
	{
		printf("[%s]: Failed to ioctl %d\n", __FUNCTION__, UF_IOCTL_INIT);
		goto init_error;
	}

	if(ioctl(handle->drv_fd, UF_IOCTL_HOOK_STATUS, &g_state_hook) == -1)
	{
		goto init_error;
	}

	return handle;

init_error:

	if(handle)
	{
		if (handle->drv_fd > 0)
		{
			close(handle->drv_fd);
		}
		free(handle);
	}

	return NULL;
}

int lc_api_hook(struct lc_api_info* handle)
{
	if(!handle || handle->drv_fd < 0)
 	{
		printf("[%s]: handle is NULL or dev not opened\n", __FUNCTION__);
		return -1;
	}

	if(ioctl(handle->drv_fd, UF_IOCTL_HOOK, &(handle->query_info)) == -1)
	{
		printf("[%s]: Failed to ioctl %d\n", __FUNCTION__, UF_IOCTL_HOOK);
		return -1;
	}

	return 0;
}

void lc_api_exit(struct lc_api_info* handle)
{
	if (handle)
	{
		if(handle->drv_fd > 0)
		{
			ioctl(handle->drv_fd, UF_IOCTL_HOOK_STATUS, &g_state_unhook);
			close(handle->drv_fd);
		}
		free(handle);
	}
}

unsigned char *lc_api_get_mac(const char* req_info)
{
	return req_info + REQ_INFO_OFFSET_MAC;
}

unsigned short *lc_api_get_iptype(const char* req_info)
{
	return req_info + REQ_INFO_OFFSET_IPTYPE;
}

unsigned int *lc_api_get_sip(const char* req_info)
{
	return req_info + REQ_INFO_OFFSET_SIP;
}

unsigned int *lc_api_get_dip(const char* req_info)
{
	return req_info + REQ_INFO_OFFSET_DIP;
}

char* lc_api_get_os(const char* req_info)
{
	return req_info + REQ_INFO_OFFSET_OS;
}

char* lc_api_get_type(const char* req_info)
{
	return req_info + REQ_INFO_OFFSET_TYPE;
}

char* lc_api_get_url(const char* req_info)
{
	return req_info + REQ_INFO_OFFSET_URL;
}
