/*
 * This app s to send snmp trap kind of messages to confd
 * Required modules can add their module name,notification msg,variable tothe global array.
 *
 * usage:
 * notifier -o moduleownerid -g msgtype -i notifyid -n no.of.variable  -v variableid -t type(0-int/1-str) -m msg
 *
 * Ex:
 * notifier -o 0 -g 1 -i 1 -n 1 -v 2 -t 0 -m 5000
 * notifier -o 0 -g 1 -i 0 -n 2 -v 0 -t 1 -m 3 -v 1 -t 1 -m LAN1
 *
 * 
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>

#include <confd_lib.h>
#include <confd_dp.h>


#define OK(E) assert((E) == CONFD_OK)
#define ERR(E) assert((E) == CONFD_ERR)
#define XEOF(E) assert((E) == CONFD_EOF)

int debuglevel = CONFD_SILENT;
char *notify_name = "";

#if 0 //for reference
enum confd_snmp_var_type {
    CONFD_SNMP_VARIABLE = 1, // as now supported
    CONFD_SNMP_OID      = 2,
    CONFD_SNMP_COL_ROW  = 3
};
enum confd_debug_level {
    CONFD_SILENT = 0,
    CONFD_DEBUG  = 1,
    CONFD_TRACE  = 2,      /* trace callback calls */
    CONFD_PROTO_TRACE = 3  /* tailf internal protocol trace */
};
#endif

#define CONFD_CONNECT_PORT CONFD_PORT
#define CONFD_CONNECT_IPADDR "127.0.0.1"
#define MAX_VARABLE_BIND 4
#define MSG_TYPE_INT 0
#define MSG_TYPE_STR 1

enum confd_message_var_id_e{
    VAR_ID_0_POE_POWER_ON_OFF,
    VAR_ID_1_POE_IFACE,
    VAR_ID_2_POE_POWER_THRESOLD,
    VAR_ID_MAX
};

enum confd_message_notification_id_e{
    NOTE_ID_0_POE_POWER_ON_OFF,
    NOTE_ID_1_POE_POWER_THRESOLD,
    NOTE_ID_MAX
};

enum confd_mod_owner_id_e{
    MOD_ID_0_POE,
    MOD_ID_1_SNMPGLUE,
    MOD_ID_MAX
};


const char *confd_msg_var_str[VAR_ID_MAX]={
"pethPsePortDetectionStatus",
"ifName",
"pethMainPseConsumptionPower"
};


const char *confd_msg_notify_str[NOTE_ID_MAX]={
"pethPsePortOnOffNotification",
"pethMainPowerUsageOnNotification"
};

const char *confd_mod_name_str[MOD_ID_MAX]={
    "poemonitor",
    "snmpglue"
};

struct confd_daemon_ctx *dctx;
int ctlsock, workersock;

void print_usage(void)
{
  printf("usage:\n notifier -o ownerid -g msgtype(1-snmpvar) -i notifyid -n no.of.variable  -v variableid -t type(0-int/1-str) -m msg\n");
}

static int cnct(struct confd_daemon_ctx *dx,
                enum confd_sock_type type, int *sock)
{

    struct sockaddr_in addr;

    addr.sin_addr.s_addr = inet_addr(CONFD_CONNECT_IPADDR);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CONFD_CONNECT_PORT);

    if ((*sock = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        confd_fatal("Failed to open socket\n");

    if (confd_connect(dx, *sock, type, (struct sockaddr*)&addr,
                      sizeof (struct sockaddr_in)) < 0)
        confd_fatal("Failed to confd_connect() to confd \n");

    return CONFD_OK;
}

int main(int argc, char **argv)
{
    int c;
    //int num = -1;
    //int loops = 0;
    struct confd_notification_ctx *nctx;
    int imodule_id=0,iMsgType=0,inotify_id=0;
    struct confd_snmp_varbind vb[MAX_VARABLE_BIND];
    int ivar_index=0,ino_of_var=0,ivar_id=0,ivar_type=0;

    if(argc <= 1)
    {
        printf("ERROR: Unknown option !!\n");
        print_usage();
        exit(0);
    }
    
    while ((c = getopt(argc, argv, "o:g:i:n:v:t:m:D")) != -1) 
    {
        switch(c) 
        {
                    printf("char:%c\n",c);
            case 'o':
                imodule_id = atoi(optarg);
                if(imodule_id >= MOD_ID_MAX )
                {
                    printf("Invalid Module ID !!\n");
                    exit(0);
                }
                break;

            case 'g':
                iMsgType =  atoi(optarg);
                if(iMsgType != CONFD_SNMP_VARIABLE)
                {
                    printf(" Only the message type:%d is Handled \n",CONFD_SNMP_VARIABLE);
                    exit(0);
                }
                vb[ivar_index].type = CONFD_SNMP_VARIABLE;// 1
                break;

            case 'i':
                inotify_id = atoi(optarg);
                if(inotify_id >= NOTE_ID_MAX)
                {
                    printf(" Invalid notification Msg ID\n");
                    exit(0);
                }
                break;

            case 'n':
                ino_of_var =  atoi(optarg);
                if(ino_of_var > MAX_VARABLE_BIND)
                {
                    printf(" Supported Max variable value pair is:%d \n",MAX_VARABLE_BIND);
                    exit(0);
                }
                break;

            case 'v':
                ivar_id =  atoi(optarg);
                if(ivar_id >= VAR_ID_MAX)
                {
                    printf(" Invalid Variable Msg ID\n");
                    exit(0);
                }
                strcpy(vb[ivar_index].var.name,confd_msg_var_str[ivar_id]);
                break;

            case 't':
                ivar_type = atoi(optarg);
                if(ivar_type > MSG_TYPE_STR) //int (or) str
                {
                    printf(" Invalid Variable Type ID\n");
                    exit(0);
                }
                break;

            case 'm':
                if(ivar_type == MSG_TYPE_INT) 
                {
                    CONFD_SET_INT32(&vb[ivar_index].val, atoi(optarg));
                }
                else if(ivar_type == MSG_TYPE_STR)
                {
                    CONFD_SET_STR(&vb[ivar_index].val,optarg);
                }

                if((ino_of_var > ivar_index))
                {
                   ivar_index++; 
                }
                break;
     
            case 'D':
                debuglevel = CONFD_PROTO_TRACE;
                break;

            default:
                printf("ERROR: Unknown option !!\n");
                print_usage();
                exit(0);
                break;
        }
    }

    confd_init(confd_mod_name_str[imodule_id], stderr, debuglevel);
    dctx = confd_init_daemon(confd_mod_name_str[imodule_id]);
    OK(cnct(dctx, CONTROL_SOCKET, &ctlsock));
    OK(cnct(dctx, WORKER_SOCKET, &workersock));
    //TODO: other msg types
    OK(confd_register_snmp_notification(dctx, workersock,
                                        notify_name, "", &nctx));
    OK(confd_register_done(dctx));
    //send msg
    OK(confd_notification_send_snmp(nctx, confd_msg_notify_str[inotify_id], &vb[0],ino_of_var));
    exit(0);
}
