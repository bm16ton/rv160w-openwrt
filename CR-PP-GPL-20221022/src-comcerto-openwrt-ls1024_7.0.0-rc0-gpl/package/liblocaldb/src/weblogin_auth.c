#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
int my_conv(int num_msg, const struct pam_message** msg,
        struct pam_response** resp, void* appdata_ptr);

/*Base64 support for username and password for radisu/AD/LDAP user authentication */
#define WEBLOGIN_BASE64_SUPPORT

#ifdef WEBLOGIN_BASE64_SUPPORT
char* base64Decoder(char encoded[], int len_str);
#endif


static struct pam_conv conv = {
    my_conv,
    NULL
};


char group[256]={'\0'};
int main(int argc,char*argv[])
{
    pam_handle_t *pamh=NULL;
    int retval, i;
    char buffer[1024]={'\0'};
    char *user=NULL;
    char *password=NULL;
    char *context=NULL;
    FILE *fp;
#ifdef WEBLOGIN_BASE64_SUPPORT
    char decode_password[255]={'\0'};
    char decode_user[512]={'\0'};
#endif

    fp = popen("wc -c /tmp/etc/config/ldap /tmp/etc/config/radius /tmp/etc/config/ad | tail -n 1 | grep -Eo [0-9]*", "r");
    fscanf(fp, "%d", &retval);
    pclose(fp);
    if(retval<=3)
               exit(1);

       memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strlen(buffer)-1]='\0';

     /* trying to find and validate '[' and ']' chars in the request buffer at first and last positions */
     /* sample buffer format: [uname;pwd;ip;arg1;context;arg2] */
     char *position_ptr = strchr(buffer, '[');
     char *r_position_ptr = strrchr(buffer, ']');

     int position = (position_ptr == NULL ? -1 : position_ptr - buffer);
     int r_position = (r_position_ptr == NULL ? -1 : r_position_ptr - buffer);

     if (position != 0 ) {
         /* Request is wrong, so exit from here with out processing further*/
         syslog(LOG_ERR,"Login request format begining is in-appropriate");
         exit(1);
     }

     if (r_position != strlen(buffer)-1) {
         /* Request is wrong, so exit from here with out processing further*/
         syslog(LOG_ERR,"Login request format ending is in-appropriate");
         exit(1);
     }

     /* Removing first '[' and last ']' chars from the request for tokenizing  */
     for(i=1; i<strlen(buffer)-1; i++) {
         buffer[i-1] = buffer[i];
     }
     buffer[i-1] = '\0';

     /*Extract remaining parameters from the request  */
     user=strtok(buffer,";");

    password = strtok(NULL,";");
    for(i=0;i<=2;i++)
    {
	context=strtok(NULL,";");
    }
     /* If context is null, exit from here to ruleout weblogin crash */
     if (!context) {
         syslog(LOG_ERR,"invalid context (NULLL)");
         exit(1);
     }

    if((!strcmp(context,"webui")) && (!strcmp(context,"netconf")) && (!strcmp(context,"rest")))
    	exit(1);

#ifdef WEBLOGIN_BASE64_SUPPORT
    strncpy(decode_password, password, strlen(password));
    strncpy(decode_user, user, strlen(user));
    int len_pwd_str = strlen(decode_password);
    int len_user_str = strlen(decode_password);

    // Do not count last NULL character.
    len_pwd_str -= 1;
    len_user_str -= 1;

    password = base64Decoder(decode_password, len_pwd_str);
    user = base64Decoder(decode_user, len_user_str);
#endif

    conv.appdata_ptr = (void*) password;

    retval = pam_start("weblogin", user, &conv,  &pamh);

    if (retval == PAM_SUCCESS)
    {
        retval = pam_authenticate(pamh, 0);    
        if (retval == PAM_SUCCESS)
        {
            retval = pam_acct_mgmt(pamh, 0);    
        }
	else if ( retval == PAM_PERM_DENIED)
        {
            fputs("reject Bad password\n",stdout);
	    if (pam_end(pamh,retval) != PAM_SUCCESS) {
	           pamh = NULL;
#ifdef WEBLOGIN_BASE64_SUPPORT
                  free(password);
                  free(user);
#endif
	           exit(1);
	    }
            return ( retval == PAM_SUCCESS ? 0:1 );
        }
        else
	{
		fputs("abort External auth rejected the login\n",stdout);
	}
    }
    
    if (pam_end(pamh,retval) != PAM_SUCCESS) {     /* close Linux-PAM */
        pamh = NULL;
#ifdef WEBLOGIN_BASE64_SUPPORT
        free(password);
        free(user);
#endif
        exit(1);
    }
    sprintf(buffer,"accept %s 1000 500 /usr/bin\n",group);
    fputs(buffer,stdout);
#ifdef WEBLOGIN_BASE64_SUPPORT
    free(password);
    free(user);
#endif
    return ( retval == PAM_SUCCESS ? 0:1 );       /* indicate success */
}




#define PAM_AUTH_RESP_ATTRIBUTES  0x1000
int my_conv(int num_msg, const struct pam_message** msg,
        struct pam_response** resp, void* appdata_ptr)
{
    int i;
    char* password = NULL;
    struct pam_response* myresp = NULL;

    for(i = 0; i<num_msg; i++)
    {
        switch(msg[i]->msg_style)
        {
            case PAM_PROMPT_ECHO_OFF:
            case PAM_PROMPT_ECHO_ON:
                if(!strncmp(msg[0]->msg, "Password", strlen("Password")))
                {
                    myresp = (struct pam_response*) calloc(1,sizeof(struct pam_response));
                    if(!myresp)
                        return PAM_CRED_ERR;
                    password = (char*) calloc(1, strlen((char*)appdata_ptr)+1);
                    if(!password)
                    {
                        free(myresp);
                        return PAM_CRED_ERR;
                    }
                    strcpy(password, (char*)appdata_ptr);
                    myresp->resp = password;
                    myresp->resp_retcode = 0;
                    *resp = myresp;
                }
                break;
            case PAM_AUTH_RESP_ATTRIBUTES :
                strcpy(group,msg[i]->msg);
                break;
            case PAM_ERROR_MSG:
            case PAM_TEXT_INFO:
                /* We will add more here */
            default:
                return PAM_CRED_ERR;
        }
    }
    return PAM_SUCCESS;
}

#ifdef WEBLOGIN_BASE64_SUPPORT
#define SIZE 512
char* base64Decoder(char encoded[], int len_str)
{
    char* decoded_string;

    decoded_string = (char*)malloc(sizeof(char) * SIZE+1);

    int i, j, k = 0;

    // stores the bitstream.
    int num = 0;

    // count_bits stores current
    // number of bits in num.
    int count_bits = 0;

    // selects 4 characters from
    // encoded string at a time.
    // find the position of each encoded
    // character in char_set and stores in num.
    for (i = 0; i < len_str; i += 4) {
        num = 0, count_bits = 0;
        for (j = 0; j < 4; j++) {
            // make space for 6 bits.
            if (encoded[i + j] != '=') {
                num = num << 6;
                count_bits += 6;
            }

            /* Finding the position of each encoded
            character in char_set
            and storing in "num", use OR
            '|' operator to store bits.*/

            // encoded[i + j] = 'E', 'E' - 'A' = 5
            // 'E' has 5th position in char_set.
            if (encoded[i + j] >= 'A' && encoded[i + j] <= 'Z')
                num = num | (encoded[i + j] - 'A');

            // encoded[i + j] = 'e', 'e' - 'a' = 5,
            // 5 + 26 = 31, 'e' has 31st position in char_set.
            else if (encoded[i + j] >= 'a' && encoded[i + j] <= 'z')
                num = num | (encoded[i + j] - 'a' + 26);

            // encoded[i + j] = '8', '8' - '0' = 8
            // 8 + 52 = 60, '8' has 60th position in char_set.
            else if (encoded[i + j] >= '0' && encoded[i + j] <= '9')
                num = num | (encoded[i + j] - '0' + 52);

            // '+' occurs in 62nd position in char_set.
            else if (encoded[i + j] == '+')
                num = num | 62;

            // '/' occurs in 63rd position in char_set.
            else if (encoded[i + j] == '/')
                num = num | 63;

            // ( str[i + j] == '=' ) remove 2 bits
            // to delete appended bits during encoding.
            else {
                num = num >> 2;
                count_bits -= 2;
            }
        }

        while (count_bits != 0) {
            count_bits -= 8;

            // 255 in binary is 11111111
            decoded_string[k++] = (num >> count_bits) & 255;
        }
    }

    // place NULL character to mark end of string.
    decoded_string[k] = '\0';

    return decoded_string;
}
#endif

