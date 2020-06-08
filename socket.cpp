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
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef __cplusplus
    extern "C" {
#endif

void* SocketThread(void* lpParam)  
{  
	XXRFIDCLient* client  = (XXRFIDCLient*)lpParam;

	while (client->threadIsStop == false) {
		int ret = readSocket(client, &client->data[client->index], BUF_LINE - client->index); 
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

    if (client->data != NULL) {
        free(client->data);
        client->data = NULL;
    }

	return 0;  
}

static void FreeResource(XXRFIDCLient *s) 
{
    if (s->sem != NULL) {
        free(s->sem);
        s->sem = NULL;
    }

    if (s->result != NULL) {
        free(s->result);
        s->result = NULL;
    }
}

void *SocketTimerThread(void *para)
{
    XXRFIDCLient *s = (XXRFIDCLient*)para;

    while (s->timerThreadIsStop == false) {
        sleep(2);

        struct timeval tv;
        int ret = gettimeofday(&tv, NULL);
        if (ret != 0) {
            LOG_TICK("failed to gettimeofday");
        }

        if (s->tick == 0) {
            continue;
        }

        if (tv.tv_sec - s->tick > 10) {  // 10s超时，则认为连接断开
            if (s->call_TcpDisconnected != NULL) {
                s->call_TcpDisconnected((char*)"time out to disconnect");
                s->threadIsStop = true;
                s->timerThreadIsStop = true;
                close(s->handle);                
                FreeResource(s);                
            }
        }
    };
}

int initSocket(char* ip, char* port, int timeout)
{
    int socketHandle = socket(AF_INET, SOCK_STREAM, 0);
    if (socketHandle == -1) {
        return -1;
    }

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &servAddr.sin_addr);
    short int tmpPort = atoi(port);
    if(tmpPort == 0) {
        return -1;
    }
    servAddr.sin_port = htons(tmpPort);

    fcntl(socketHandle, F_SETFL, fcntl(socketHandle, F_GETFL) | O_NONBLOCK); // 非阻塞

    int ret = connect(socketHandle, (struct sockaddr*)&servAddr, sizeof(servAddr));
    if (ret == -1) {
        if (errno != EINPROGRESS) {
            return -1;
        }

        fd_set wset;
        FD_ZERO(&wset);
        FD_SET(socketHandle, &wset);

        struct timeval to;
        memset(&to, 0, sizeof(struct timeval));
        to.tv_sec = timeout;
        to.tv_usec = 0;

        LOG_TICK("");
        ret = select(socketHandle + 1, NULL, &wset, NULL, &to);
        LOG_TICK("");
        switch(ret)
        {
            case -1:
                perror("select");
                return -1;
            case 0:
                printf("timeout\n");
                return -1;
            default:
                if(FD_ISSET(socketHandle, &wset))
                {
                    int error;
                    int len;
                    ret = getsockopt(socketHandle, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
                    if (ret < 0) {
                        return -1;
                    }
                    
                    if (error != 0) {
                        LOG_TICK(strerror(error));
                        return -1;
                    }
                }
                break;
        }
    }

    fcntl(socketHandle, F_SETFL, fcntl(socketHandle, F_GETFL) & O_NONBLOCK); // 阻塞

    return socketHandle;
}

int readSocket(XXRFIDCLient *client, unsigned char* buf, int len)
{
    if (client->threadIsStop == true) {
        LOG_TICK("client->threadIsStop == true");
        return -1;
    }

    int ret = recv(client->handle, (char*)buf, len, 0);
    if (ret == 0) { // reset been peer
        if (client->call_TcpDisconnected != NULL) {
            client->call_TcpDisconnected((char*)"connection has been closed by peer");
            FreeResource(client);
            client->threadIsStop = true;
        }
    }

    return ret;
}

int writeSocket(XXRFIDCLient *client, unsigned char* buf, int len)
{
    if (client->threadIsStop == true) {
        LOG_TICK("client->threadIsStop == true");
        return -1;
    }

    int ret = send(client->handle, (char*)buf, len, 0);
    if (ret == -1) {
        if (errno == ECONNRESET) {
            if (client->call_TcpDisconnected != NULL) {
                client->call_TcpDisconnected((char*)"connection has been closed by peer");
                FreeResource(client);
                client->threadIsStop = true;
            }    
        }
    }

    if (ret > 0) {
        return SUCCESS;
    }

    return -1;
}

void StartSocket(XXRFIDCLient *client)
{
    pthread_t threadID;
    client->threadIsStop = false;
    int ret = pthread_create(&threadID, NULL, SocketThread, (XXRFIDCLient*)client);
    if (ret != 0) {
        client->threadIsStop = true;
        LOG_TICK("failed to pthread_create");
    } else {
        pthread_detach(threadID);
    }

    client->timerThreadIsStop = false;
    pthread_t ttid;
    ret = pthread_create(&ttid, NULL, SocketTimerThread, (void*)client);
    if (ret != 0) {
        client->timerThreadIsStop = true;
        LOG_TICK("failed to pthread_create");
    } else {
        pthread_detach(ttid);
    }
}

int cleanSocket(XXRFIDCLient* client)
{
    client->threadIsStop = true;
    client->timerThreadIsStop = true;
    close(client->handle);

    if (client->data != NULL) {
        free(client->data);
        client->data = NULL;
    }

    if (client->sem != NULL) {
        free(client->sem);
        client->sem = NULL;
    }

    if (client->result != NULL) {
        free(client->result);
        client->result = NULL;
    }

    return SUCCESS;
}

#ifdef __cplusplus
}
#endif // __cplusplus
