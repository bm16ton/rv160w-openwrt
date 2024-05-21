/************* Broadcom ROBO Switch client code *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define CLIENT_PORT 27876

int main(int argc, char* argv[])

{
	int clientSocket, nBytes, i, retval;
  	char buffer[BUFFER_SIZE] = "";
  	struct sockaddr_in serverAddr;
  	socklen_t addr_size;

#ifdef BCMSSDK_DEBUG
  printf("\nROBO CLIENT CALLED with %d arguments ==== \n", (argc-1));	
  for(i=1; i < argc; i++)
	printf("\n argv[%d] = %s", i, argv[i]);
#endif
	/*Create UDP socket*/
  	if ((clientSocket = socket(PF_INET, SOCK_DGRAM, 0)) == -1 ) 
	{
      	printf("\r\n socket() creation failed \r\n");
      	return -1;
  	}

	/*Configure settings in address struct*/
  	serverAddr.sin_family = AF_INET;
  	serverAddr.sin_port = htons(CLIENT_PORT);
  	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  	memset(serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));

  	/*Initialize size variable to be used later on*/
  	addr_size = sizeof(serverAddr);

  	for(i=0; i<argc;i++)
	{
    	strcat(buffer, argv[i]);
        strcat(buffer, " ");
	}
    nBytes = strlen(buffer) + 1;

#ifdef BCMSSDK_DEBUG
    printf("\nSending to ROBOSERVER [Socket = %d bytes = %d , DATA == %s ]\n", clientSocket, nBytes, buffer);
#endif
	/*Send message to server*/
	if ((retval = sendto(clientSocket,buffer,nBytes,0,(struct sockaddr *)&serverAddr,addr_size)) == -1 ) 
	{
    	printf("\r\n sendto() call failed \r\n");
      	return -1;
    }
    return 0;
}

