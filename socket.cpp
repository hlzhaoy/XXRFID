#include "pub.h"
#include "socket.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "select.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#ifdef __cplusplus
    extern "C" {
#endif

static pthread_t threadID;
static bool threadIsStop = true;

void* SocketThread(void* lpParam)  
{  
	XXRFIDCLient* client  = (XXRFIDCLient*)lpParam;

	while (threadIsStop == false) {
		int ret = readSocket(client->handle, &client->data[client->index], BUF_LINE - client->index); 
		if(ret > 0) {
			client->index += ret;
			do 
			{
				if(client->index >= 6) {
					if (client->data[0] == 0x5A) {
						short tlen = (client->data[5]<<8) | client->data[6];
						if(client->index >= tlen+7+2) {
							if (CRCVerify((unsigned char*)(client->data), tlen + 9) == true) {
								serialData data;
								data.data = new unsigned char[9+tlen];
								int tmp = 9 + tlen;
								data.len = tmp;
								data.s = client;

								memcpy(data.data, client->data, data.len);

								QueueInsert(data);

								client->index -= 9+tlen;
								memmove(client->data, &client->data[tmp], client->index);								
							} else {
								memmove(client->data, &(client->data[1]), --client->index);
							}
							continue;
						}

						break;
					} else {
						memmove(client->data, &client->data[1], --client->index);
						continue;
					}
				} 

				break;
				
			} while (1);
		} else if (ret == 0) { //远端断开
            if (client->call_TcpDisconnected != NULL) {
                client->call_TcpDisconnected((char*)"peer close the socket");
                break;
            }
        }

		usleep(5);
	}

	return 0;  
}

int initSocket(char* ip, char* port, int timeout)
{
    int socketHandle = socket(AF_INET, SOCK_STREAM, 0);
    if (socketHandle == -1) {
        return -1;
    }

    setsockopt(socketHandle, SOL_SOCKET, SO_SNDTIMEO,(char*)&timeout,sizeof(timeout));

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &servAddr.sin_addr);
    short int tmpPort = atoi(port);
    if(tmpPort == 0) {
        return -1;
    }
    servAddr.sin_port = htons(tmpPort);

    int ret = connect(socketHandle, (struct sockaddr*)&servAddr, sizeof(servAddr));
    if (ret == -1) {
        return -1;
    }

    return socketHandle;
}

int readSocket(int handle, unsigned char* buf, int len)
{
    int ret = recv(handle, (char*)buf, len, 0);
    if (ret == -1) {
        return -1;
    }

    return ret;
}

int writeSocket(int handle, unsigned char* buf, int len)
{
    int ret = send(handle, (char*)buf, len, 0);
    if (ret == -1) {
        return SYSTEMERR;
    }

    return SUCCESS;
}

void StartSocket(XXRFIDCLient *client)
{
    threadIsStop = false;
    int ret = pthread_create(&threadID, NULL, SocketThread, (XXRFIDCLient*)client);
    if (ret != 0) {
        threadIsStop = true;
        LOG_TICK("failed to pthread_create");
    } else {
        pthread_detach(threadID);
    }
}

int cleanSocket(XXRFIDCLient* client)
{
    threadIsStop = true;
    close(client->handle);

    free(client->data);
    free(client->sem);
    free(client->result);
    free(client);

    return SUCCESS;
}

#ifdef __cplusplus
}
#endif // __cplusplus
