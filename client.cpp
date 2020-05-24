/*
* 动态库启动服务器模式，当有新的连接到达时，需要为其创建一个线程用作读取数据的接口
*/

#include "XXRFID.h"
#include "pub.h"
#include <string.h>
#include <unistd.h>
#include "client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

void *ClientThread(void *para)
{
    XXRFIDCLient* client  = (XXRFIDCLient*)para;

	while (client->threadIsStop == false) {
		int ret = ReadClient(client, &client->data[client->index], BUF_LINE-client->index); 
		if(ret > 0) {
			client->index += ret;
			do 
			{
				if (client->index >= 6) {
					if (client->data[0] == 0x5A) {
						short tlen = (client->data[5]<<8) | client->data[6];
						if (client->index >= tlen+7+2) {
							if (CRCVerify((unsigned char*)(client->data), tlen + 9)) {
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

							usleep(5);
							continue;
						} 

						break;
					} else {
						memmove(client->data, &(client->data[1]), --client->index);
						usleep(5);
						continue;
					}
				}

				break;
			} while (1);
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

void *ClientTimerThread(void *para)
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
                close(s->handle);
                FreeResource(s);
                s->threadIsStop = true;
                s->timerThreadIsStop = true;
            }
        }
    };
}

int ReadClient(XXRFIDCLient *s, unsigned char* rbuf, int len)
{
    int ret = recv(s->handle, rbuf, len, 0);
    if (ret == 0) {
        if (s->call_TcpDisconnected != NULL) {
            s->call_TcpDisconnected((char*)"connection has been closed by peer");
            FreeResource(s);
            s->threadIsStop = true;
        }
    }

    return ret;
}

int StartClient(XXRFIDCLient *s)
{
    s->threadIsStop = false;
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, ClientThread, (void*)s);
    if (ret != 0) {
        s->threadIsStop = true;
        LOG_TICK("failed to pthread_create");
    } else {
        pthread_detach(tid);
    }

    pthread_t ttid;
    s->timerThreadIsStop = false;
    ret = pthread_create(&ttid, NULL, ClientTimerThread, (void*)s);
    if (ret != 0) {
        s->timerThreadIsStop = true;
        LOG_TICK("failed to pthread_create");
    } else {
        pthread_detach(ttid);
    }
}

int WriteClient(XXRFIDCLient *s, unsigned char* rbuf, int len)
{
    int ret = send(s->handle, rbuf, len, 0);
    if (ret == -1) {
        if (errno == ECONNRESET) {
            if (s->call_TcpDisconnected != NULL) {
                s->call_TcpDisconnected((char*)"connection has been closed by peer");
                FreeResource(s);
                s->threadIsStop = true;
            }    
        }
    }

    if (ret > 0) {
        return SUCCESS;
    }

    return ret;
}

int ClearClient(XXRFIDCLient *s)
{
    s->threadIsStop = true;
    close(s->handle);

    if (s->sem != NULL) {
        free(s->sem);
        s->sem = NULL;
    }

    if (s->result != NULL) {
        free(s->result);
        s->result = NULL;
    }
}

#ifdef __cplusplus
}
#endif