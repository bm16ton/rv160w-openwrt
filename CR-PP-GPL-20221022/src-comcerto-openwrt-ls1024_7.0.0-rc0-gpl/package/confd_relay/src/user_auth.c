

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>

#include <confd.h>
#include <confd_maapi.h>
#include <confd_lib.h>
#include <confd_cdb.h>
#include "tacm.h"


#define FALSE 0
#define TRUE 1

void user_authentication(char *user, char *password);
void service_authorization(char *service_name, char *groupname, char *connection_name);


int main(int argc, char*argv[])
{

    char user[50];
    char password[50];
    char buffer[256]={'\0'};

   memset(buffer, 0, sizeof(buffer));

    if (argc >=3)
    {
      strcpy(user, argv[2]);
      strcpy(password, argv[3]);
    } else {
      sprintf(buffer,"PAM_AUTH_ERR");
      fputs(buffer,stdout);
      return FALSE;
    }

    confd_init("local_db",stderr,CONFD_SILENT);

    if (strcmp(argv[1], "authenticate") == 0)
    {
      user_authentication(user, password);
      return TRUE;
    }
    else if (strcmp(argv[1],"authorize") == 0)
    {
      service_authorization(user, password, argv[4]);
      return TRUE;
    }
    else
    {
      sprintf(buffer,"PAM_AUTH_ERR");
      fputs(buffer,stdout);
      return FALSE;
    }

   return FALSE;
}


void user_authentication(char *user, char *password)
{

    int i, ret = -1;
    int maapi_socket;
    struct sockaddr_in addr;
    int n=2;
    char *groups[2];
    struct confd_ip ip;
    char buffer[256]={'\0'};

    memset(buffer, 0, sizeof(buffer));

    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4565);

    maapi_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (maapi_socket < 0 )
    {
        sprintf(buffer,"PAM_AUTH_ERR");
        fputs(buffer,stdout);
        return;
    }

    if (maapi_connect(maapi_socket, (struct sockaddr*)&addr,
                sizeof (struct sockaddr_in)) < 0)
    {
        sprintf(buffer,"PAM_AUTH_ERR");
        fputs(buffer,stdout);
        return;
    }

    ip.af = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &ip.ip.v4);
    ret = maapi_authenticate2(maapi_socket, user, password, &ip,0,"pamlocal",CONFD_PROTO_TCP,&groups[0], n);
    if (ret == 1)
    {
        if(groups[0] != NULL)
        {
           sprintf(buffer,"%s",groups[0]);
           fputs(buffer,stdout);
        }
        else
        {
           sprintf(buffer,"default");
           fputs(buffer,stdout);
        }
        maapi_close(maapi_socket);
        syslog(LOG_INFO, "userauth:user:%s authentication sucess",user);

        for(i=0; groups[i]; i++)
        {
            free(groups[i]);
        }

        return;
    }
    else
    {
        maapi_close(maapi_socket);
        syslog(LOG_ERR, "Localdb:user:%s authentication failed",user);
        sprintf(buffer,"PAM_AUTH_ERR");
        fputs(buffer,stdout);
        return;
    }

   sprintf(buffer,"PAM_AUTH_ERR");
   fputs(buffer,stdout);
   return;
}



void service_authorization(char *service_name, char *groupname, 
                            char *connection_name)
{

    struct sockaddr_in addr;  
    int rsock;
    confd_value_t *service = NULL;
    char *ptmp = NULL;
    int n_service;
    int i;
    int action = FALSE;
    char buffer[256]={'\0'};

    memset(buffer, 0, sizeof(buffer));
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4565);

    if ((rsock = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
    {
        syslog(LOG_ERR, "Localdb:authorization failed as socket is NULL group:%s service:%s",
                         groupname,service_name);
        return;
    }
    if (cdb_connect(rsock, CDB_READ_SOCKET, (struct sockaddr *)&addr,
                sizeof (struct sockaddr_in)) < 0)
    {
        syslog(LOG_ERR, "Localdb:authorization cd connect failed grooup:%s service:%s",
                         groupname,service_name);
        sprintf(buffer,"PAM_AUTH_FAILED");
        fputs(buffer,stdout);
        return;
    }
    if (cdb_start_session(rsock, CDB_RUNNING) != CONFD_OK)
    {
        syslog(LOG_ERR, "Localdb:authorization cd session failed grooup:%s service:%s",
                         groupname,service_name);
        return;
    }

    if(strcmp(service_name,"anyconnect-vpn") == 0)
    {
        cdb_set_namespace(rsock, tacm__ns);
        if(cdb_get_list(rsock, &service,&n_service,
                "/nacm:nacm/nacm:groups/group{%s}/feature-rule{%s}/names",
               groupname,service_name) == CONFD_OK)
        {

            memset(buffer, 0, sizeof(buffer));
          for (i = 0; i < n_service; i++) {
              strcpy(buffer, CONFD_GET_CBUFPTR(&service[i]));
            }
            fputs(buffer,stdout);

            cdb_close(rsock);
             if (service != NULL) {
               for (i = 0; i < n_service; i++) {
                     confd_free_value(&service[i]);
                 }
             free(service);
            }

          return;
        }
        else
        {
          syslog(LOG_INFO, "Localdb: authorization not enabled on group:%s,service:%s",groupname,service_name);
          return; 
        }

    }
    else if((strcmp(service_name,"captive-portal") == 0) ||
			(strcmp(service_name, "ssid") == 0))
	{
		char tmp_group[50]={'\0'};
		int length=0;

		memset(tmp_group, 0, sizeof(tmp_group));

		cdb_set_namespace(rsock, tacm__ns);
		if (cdb_get_list(rsock, &service,&n_service,
						 "/nacm:nacm/nacm:groups/group{%s}/feature-rule{%s}/option",
						 groupname,service_name) == CONFD_OK)
		{

			action = FALSE;
			for (i=0;i<n_service;i++) {
				ptmp = CONFD_GET_CBUFPTR(&service[i]);
				if (NULL != ptmp) {
					length=strlen(connection_name);
					strncpy(tmp_group,ptmp,length);
					if (strcmp(tmp_group, connection_name) == 0) {
						action = TRUE;
						break;
					}
				}
			}

			if (service != NULL) {
				for (i = 0; i < n_service; i++) {
					confd_free_value(&service[i]);
				}
				free(service);
			}
		}
		else
		{
			cdb_close(rsock);
			syslog(LOG_INFO, "Localdb: authorization not enabled on group:%s,service:%s",groupname,service_name);
			return;
		}

		cdb_close(rsock);
		if (action == TRUE) {
			syslog(LOG_INFO,"Localdb authorization Sucess\n");
			sprintf(buffer,"PAM_SUCCESS");
		} else{
			syslog(LOG_INFO, "Localdb:authorization failed for group:%s service:%s", 
				   groupname,service_name);
			sprintf(buffer,"PAM_AUTH_ERR");
		}

		fputs(buffer,stdout);
		return;
	}
    else if((strcmp(service_name,"s2s-vpn") == 0) || (strcmp(service_name,"ezvpn") == 0) || 
                         (strcmp(service_name,"3rdparty") == 0))
    {

#if 0
        ptmpmsg[0] = &tmpmsg[0];
        tmpmsg[0].msg_style = PAM_AUTH_CONNECTION_NAME;
        tmpmsg[0].msg = "Connection: ";

        sprintf(cmd,"uci get strongswan.%s.xauth_group",connection_name);
        fp=popen(cmd,"r");
        if(fgets(var, sizeof(var), fp) == NULL)
          {
            printf("fgets failed\n");
            pclose(fp);
            return T_PERM_DENIED;
           }
        var[strlen(var)-1]='\0';
        pclose(fp);
        if (strcmp(var,groupname) == 0)
             retval = PAM_SUCCESS;
        else
             retval = T_PERM_DENIED;
#else
	    char *tmpservice_name=NULL, *tmpconnection_name=NULL, *tmp_connection_name;

	tmp_connection_name=connection_name;
        tmpservice_name=strsep(&tmp_connection_name,"_");
	    if(strcmp(tmpservice_name,"c2s")  == 0)
		    strcpy(service_name,"ezvpn");
	    else
		    strcpy(service_name,"s2s-vpn");
	
	tmpconnection_name=strsep(&tmp_connection_name,"\0");
        cdb_set_namespace(rsock, tacm__ns);

        if (cdb_get_list(rsock, &service,&n_service,
             "/nacm:nacm/nacm:groups/group{%s}/feature-rule{%s}/option",
             groupname,service_name) == CONFD_OK)
        {

           action = FALSE;
           for (i=0;i<n_service;i++) {
             ptmp = CONFD_GET_CBUFPTR(&service[i]);
             if (strcmp(ptmp,tmpconnection_name) == 0) {
                action = TRUE;
                break;
               }
            }

          if (service != NULL) {
             for (i = 0; i < n_service; i++) {
                 confd_free_value(&service[i]);
             }
             free(service);
          }
        }
        else
        {
          syslog(LOG_INFO, "Localdb: authorization not enabled on group:%s,service:%s",groupname,service_name);
          cdb_close(rsock);
          return;
        }

#endif
        cdb_close(rsock);
        if (action == TRUE)
        {
            syslog(LOG_INFO,"Localdb authorization Sucess\n");
        }

       sprintf(buffer,"PAM_SUCCESS");
       fputs(buffer,stdout);
      return;
    }
    else
    {
       
       	 cdb_set_namespace(rsock, tacm__ns);
       	 if (cdb_get_enum_value(rsock, &action,
                "/nacm:nacm/nacm:groups/group{%s}/feature-rule{%s}/action",
                  groupname,service_name) == CONFD_OK)
         {
           if(action == 0)
           {
               syslog(LOG_INFO, "Localdb:authorization sucess group:%s service:%s", 
                         groupname,service_name);
               cdb_close(rsock);
               sprintf(buffer,"PAM_SUCCESS");
               fputs(buffer,stdout);
               return;
           }
           else
           {
               syslog(LOG_INFO, "Localdb:authorization failed for group:%s service:%s", 
                         groupname,service_name);
               sprintf(buffer,"PAM_AUTH_ERR");
               fputs(buffer,stdout);
               return;
           }
         }
         else
         {
          syslog(LOG_INFO, "Localdb: authorization not enabled on group:%s,service:%s",groupname,service_name);
         }

    }
   return;
}

