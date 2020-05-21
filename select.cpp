#include "list.h"
#include <sys/types.h>
#include "XXRFID.h"
#include <errno.h>
#include "pub.h"
#include "COM.h"
#include "socket.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

list g_SelectList = NULL;   // 连接列表

static pthread_t threadID;
static bool g_IsRuning = false;
static pthread_mutex_t g_SelectCreateMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_SelectListMutex = PTHREAD_MUTEX_INITIALIZER;

void InsertSelectList(XXRFIDCLient* client);

void* ProcSelect(void* lpParameter)
{
    fd_set readset;
    int maxfd = 0;

    while (g_IsRuning == true) {
        pthread_mutex_lock(&g_SelectListMutex);

        FD_ZERO(&readset);
        maxfd = 0;

		for (list t = g_SelectList->next; t != NULL; t = t->next) {
			FD_SET((int)(long)(((XXRFIDCLient*)(t->data))->handle), &readset);
		}

        struct timeval to;
        to.tv_sec = 0;
        to.tv_usec = 1000*10;

        int ret = select(maxfd + 1, &readset, NULL, NULL, &to);
        if (ret == -1) {
            pthread_mutex_unlock(&g_SelectListMutex);
            LOG_TICK("failed to select");
			int err = errno;
			char tmp[128] = {0};
			sprintf(tmp, "errno = %d \n", err);
			LOG_TICK(tmp);

			if (err == EBADF) {
				LOG_TICK("EBADF");
			} else if (err == EINTR) {
				LOG_TICK("EINTR");
                continue;
			} else if (err == EINVAL) {
				LOG_TICK("EINVAL");
			} else if (err == EINVAL) {
				LOG_TICK("EINVAL");
			} else if (err == ENOMEM) {
				LOG_TICK("ENOMEM");
			} else {
                LOG_TICK("other error of select");
            }

            return 0;
        } else if(ret == 0) {
            LOG_TICK("ret = 0");
            pthread_mutex_unlock(&g_SelectListMutex);
            PreciseSleepMillisecond(50);
            continue;
        }

        LOG_TICK("hello");
		for (list t = g_SelectList->next; t != NULL; t = t->next) {
            if (FD_ISSET((long)((XXRFIDCLient*)(t->data))->handle, &readset)) {
                if (((XXRFIDCLient*)(t->data))->type == ETH || ((XXRFIDCLient*)(t->data))->type == ACCEPT) {
                    ret = recv((long)((XXRFIDCLient*)(t->data))->handle, (char*)&(((XXRFIDCLient*)(t->data))->data[((XXRFIDCLient*)(t->data))->index]), 
                        BUF_LINE - ((XXRFIDCLient*)(t->data))->index, 0);  //socket使用recv，文件句柄使用read，后面再考虑合并问题
                    if (ret == 0) {
                        if (((XXRFIDCLient*)(t->data))->call_TcpDisconnected != NULL) {
                            ((XXRFIDCLient*)(t->data))->call_TcpDisconnected((char*)"peer close the socket");
                        }

						DeleteList(g_SelectList, t);
						if (IsEmpty(g_SelectList) == true) {
							break;
						}
                        continue;
                    } else if(ret < 0) {

                    }
                } else {
                    int s = accept((long)((XXRFIDCLient*)(t->data))->handle, NULL, NULL);
                    if(s == -1) {
                        LOG_TICK("failed to accept");
                        continue;
                    }

                    XXRFIDCLient* client = (XXRFIDCLient*)malloc(sizeof(*client));
                    if(client == NULL) {
                        LOG_TICK("failed to malloc");
                        continue;
                    }
                    memset(client, 0, sizeof(*client));

                    memcpy(client, t->data, sizeof(*client));
                    client->handle = (void*)s;
                    client->sem = (sem_t*)malloc(sizeof(sem_t) * EMESS_Count);
                    if (client->sem == NULL) {
                        ret = -1;
                        LOG_TICK("failed to malloc");
                        break;
                    }

                    memset(client->sem, 0, sizeof(sem_t) * EMESS_Count);
                    for (int i = 0; i < EMESS_Count; i++) {
                        int ret = sem_init(&client->sem[i], 0, 0);
                        if (ret != 0) {
                             LOG_TICK("filed to sem_init");
                            ret = -1;
                            break;
                        }
                    }

                    client->result = (MessageResult*)malloc(sizeof(MessageResult) * EMESS_Count);
                    if (client->result == NULL) {
                        ret = -1;
                        LOG_TICK("failed to malloc");
                        break;
                    }
                    memset(client->result, 0, sizeof(MessageResult) * EMESS_Count);

                    client->data = (unsigned char*)malloc(1024);
                    if (client->data == NULL) {
                        ret = -1;
                        LOG_TICK("failed to malloc");
                        break;
                    }

                    client->type = ACCEPT;
                    InsertSelectList(client);

                    if(client->call_GClientConnected != NULL) {
                        client->call_GClientConnected((char*)"accept a reader");
                    }

                    PreciseSleepMillisecond(50);
                    continue;
                }

                LOG_TICK("hello");
				char* tmp = HexToString(((XXRFIDCLient*)(t->data))->data, ret);
				LOG_TICK(tmp);
				delete []tmp;

                ((XXRFIDCLient*)(t->data))->index += ret;
                do
                {
                    if (((XXRFIDCLient*)(t->data))->index >= 6) {
						if (((XXRFIDCLient*)(t->data))->data[0] == 0x5A) {
							short tlen = (((XXRFIDCLient*)(t->data))->data[5]<<8) | ((XXRFIDCLient*)(t->data))->data[6];
							if (((XXRFIDCLient*)(t->data))->index >= tlen+7+2) { // 数据长度加上crc(两个字节)
								if (CRCVerify((unsigned char*)(((XXRFIDCLient*)(t->data))->data), tlen + 9) == true) {
									serialData data;
									data.data = new unsigned char[9+tlen];
									int tmp = 9 + tlen;
									data.len = tmp;
									memcpy(data.data, ((XXRFIDCLient*)(t->data))->data, data.len);
									data.s = (XXRFIDCLient*)t->data;  // ?

									QueueInsert(data);

									((XXRFIDCLient*)(t->data))->index -= 9+tlen;
									memmove(((XXRFIDCLient*)(t->data))->data, &(((XXRFIDCLient*)(t->data))->data[tmp]), ((XXRFIDCLient*)(t->data))->index);
									
								} else {
									memmove(((XXRFIDCLient*)(t->data))->data, &(((XXRFIDCLient*)(t->data))->data[1]), --((XXRFIDCLient*)(t->data))->index);
								}
								continue;
							} 
						} else {
							memmove(((XXRFIDCLient*)(t->data))->data, &(((XXRFIDCLient*)(t->data))->data[1]), --((XXRFIDCLient*)(t->data))->index);
							continue;
						}
					}

					break;
                } while (1);
            }
        }

        FD_ZERO(&readset);
        
        for (list t = g_SelectList->next; t != NULL; t = t->next) {
            FD_SET((long)((XXRFIDCLient*)(t->data))->handle, &readset);  //将列表中的所有fd重新SET
        }
        pthread_mutex_unlock(&g_SelectListMutex);

        PreciseSleepMillisecond(50);
    }

	return 0;
}

void CreateSelectThread()
{
    if (g_IsRuning == false) {  //单例，确保ProcSelect只有一个线程
        pthread_mutex_lock(&g_SelectCreateMutex);
        if (g_IsRuning == false) {
            g_IsRuning = true;
            int ret = pthread_create(&threadID, NULL, ProcSelect, NULL);
            if (ret != 0) {
                LOG_TICK("failed to pthread_create");
                exit(0);
            }
        }
        pthread_mutex_unlock(&g_SelectCreateMutex);
    }
}

void InsertSelectList(XXRFIDCLient* client)
{
    pthread_mutex_lock(&g_SelectListMutex);
	list t = (list)malloc(sizeof(*t));
	memset(t, 0, sizeof(*t));
	t->data = client;

	InsertHead(g_SelectList, t);
    LOG_TICK("InsertHead");
    pthread_mutex_unlock(&g_SelectListMutex);
}

int ExitSelectThread(XXRFIDCLient* client)
{
    pthread_mutex_lock(&g_SelectListMutex);

	DeleteListByData(g_SelectList, client);

	int listSize = GetListCount(g_SelectList);

    if(listSize > 0) {
        pthread_mutex_unlock(&g_SelectListMutex);
        return listSize;
    }

    if(listSize == 0) {
        g_IsRuning = false;
        pthread_join(threadID, NULL);
    }

    pthread_mutex_unlock(&g_SelectListMutex);

    return 0;
}

/**************************************
退出一个种类的客户端
***************************************/
int ExitSelectThreadType(ConnType type)
{
    pthread_mutex_lock(&g_SelectListMutex);
	
	while(IsEmpty(g_SelectList) == true) {
		list tmp = g_SelectList->next;
		if(((XXRFIDCLient*)(tmp->data))->type = ACCEPT) {
			free(((XXRFIDCLient*)(tmp->data))->data);
			((XXRFIDCLient*)(tmp->data))->data = NULL;

			free(((XXRFIDCLient*)(tmp->data))->sem);
			((XXRFIDCLient*)(tmp->data))->sem = NULL;

			free(((XXRFIDCLient*)(tmp->data))->result);
			((XXRFIDCLient*)(tmp->data))->result = NULL;

			DeleteList(g_SelectList, tmp);

			continue;
		}
	}

    pthread_mutex_lock(&g_SelectListMutex);

	return 0;
}

static pthread_mutex_t g_SelectListCreateMutex = PTHREAD_MUTEX_INITIALIZER;
void SelectListInit()
{
    if (g_SelectList == NULL) {
        pthread_mutex_lock(&g_SelectListCreateMutex);
        if (g_SelectList == NULL) {
            g_SelectList = InitList();
        }
        pthread_mutex_unlock(&g_SelectListCreateMutex);
    }
}

#ifdef __cplusplus
}
#endif
