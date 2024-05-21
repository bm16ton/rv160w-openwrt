#ifndef _LC_API_H_
#define _LC_API_H_

#define REQ_INFO_OFFSET_MAC		0
#define REQ_INFO_OFFSET_IPTYPE	6
#define REQ_INFO_OFFSET_SIP		8
#define REQ_INFO_OFFSET_DIP		24
#define REQ_INFO_OFFSET_OS		40
#define REQ_INFO_OFFSET_TYPE	72
#define REQ_INFO_OFFSET_URL		104
#define REQ_INFO_URL_LENGTH		511
#define MAX_URL_LENGTH			615

#define MAX_QUERY_RESULT 		10		// An URL can belong to 10 categories
#define MAX_BATCH_QUERY_NUM 	10		
#define MAX_QUEUE_SIZE			1000

/* 1 to 83 means block by its category number */
#define RES_PASS                0
#define RES_SYSTEM_ERROR        99
#define RES_BLOCK_WITHOUT_CATEGORY 84

#define REP_IDX                 5
/* Note: Please do not modify all the definitions above */

struct lc_query_block
{
	const int reserved[2];
	int result[MAX_QUERY_RESULT];
	const unsigned char req_info[MAX_URL_LENGTH+1];
};
/* Note: Please do not modify all the definitions above */

struct lc_query_info
{
	const long ts_sec;		// timestamp in seconds, for computing response time
	const long ts_usec;		// timestamp in microseconds, for computing response time
	const int query_num;		// query number in this batch query
	struct lc_query_block query_blocks[MAX_BATCH_QUERY_NUM];	// batch query blocks
};
/* Note: Please do not modify all the definitions above */

struct lc_api_info
{
	int drv_fd;
	struct lc_query_info query_info;
};
/* Note: Please do not modify all the definitions above */

struct lc_api_info* lc_api_init(void);
int lc_api_hook(struct lc_api_info* handle);
void lc_api_exit(struct lc_api_info* handle);

unsigned char *lc_api_get_mac(const char* req_info);
unsigned short *lc_api_get_iptype(const char* req_info);
unsigned int *lc_api_get_sip(const char* req_info);
unsigned int *lc_api_get_dip(const char* req_info);
char* lc_api_get_os(const char* req_info);
char* lc_api_get_type(const char* req_info);
char* lc_api_get_url(const char* req_info);

#endif
