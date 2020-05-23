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
#include "select.h"

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

        // LOG_TICK("before for");
		for (list t = g_SelectList->next; t != NULL; t = t->next) {
			FD_SET(((XXRFIDCLient*)(t->data))->handle, &readset);
            if (((XXRFIDCLient*)(t->data))->handle > maxfd) {
                maxfd = ((XXRFIDCLient*)(t->data))->handle;
            }
		}

        // LOG_TICK("after for");

        struct timeval to;
        to.tv_sec = 0;
        to.tv_usec = 1000*10;

        int ret = select(maxfd + 1, &readset, NULL, NULL, &to);
        if (ret == -1) {
            pthread_mutex_unlock(&g_SelectListMutex);
			int err = errno;
			char tmp[128] = {0};
			sprintf(tmp, "faield to select, errno = %d \n", err);
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
            pthread_mutex_unlock(&g_SelectListMutex);
            PreciseSleepMillisecond(50);
            continue;
        }

		for (list t = g_SelectList->next; t != NULL; t = t->next) {
            if (FD_ISSET(((XXRFIDCLient*)(t->data))->handle, &readset)) {
                if (((XXRFIDCLient*)(t->data))->type == ETH || ((XXRFIDCLient*)(t->data))->type == ACCEPT) {
                    ret = recv(((XXRFIDCLient*)(t->data))->handle, (char*)&(((XXRFIDCLient*)(t->data))->data[((XXRFIDCLient*)(t->data))->index]), 
                        BUF_LINE - ((XXRFIDCLient*)(t->data))->index, 0);  // socket使用recv，文件句柄使用read，后面再考虑合并问题
                    if (ret == 0) {
                        if (((XXRFIDCLient*)(t->data))->call_TcpDisconnected != NULL) {
                            ((XXRFIDCLient*)(t->data))->call_TcpDisconnected((char*)"peer close the socket");
                        }

                        DelFromSocketList(((XXRFIDCLient*)(t->data))->handle);
						DeleteList(g_SelectList, t);
						if (IsEmpty(g_SelectList) == true) {
                            pthread_mutex_unlock(&g_SelectListMutex);
							break;
						}
                        pthread_mutex_unlock(&g_SelectListMutex);
                        continue;
                    } else if(ret < 0) {

                    }
                } else {
                    int s = accept(((XXRFIDCLient*)(t->data))->handle, NULL, NULL);
                    if(s == -1) {
                        LOG_TICK("failed to accept");
                        continue;
                    }
                    LOG_TICK("accept");

                    XXRFIDCLient* client = (XXRFIDCLient*)malloc(sizeof(*client));
                    if(client == NULL) {
                        LOG_TICK("failed to malloc");
                        pthread_mutex_unlock(&g_SelectListMutex);
                        continue;
                    }

                    memset(client, 0, sizeof(*client));
                    memcpy(client, t->data, sizeof(*client));
                    client->handle = s;
                    client->type = ACCEPT;

                    LOG_TICK("2");
                    pthread_mutex_unlock(&g_SelectListMutex);
                    InsertSelectList(client);
                    pthread_mutex_lock(&g_SelectListMutex);
                    LOG_TICK("3");
                    if(client->call_GClientConnected != NULL) {
                        client->call_GClientConnected((char*)"accept a reader");
                    }
                    LOG_TICK("4");
                    InsertSocketList(s);
                    LOG_TICK("5");
                    PreciseSleepMillisecond(50);
                    pthread_mutex_unlock(&g_SelectListMutex);
                    LOG_TICK("pthread_mutex_unlock");
                    continue;
                }

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

#if REGION("接收到的socket连接")
list g_socketList = NULL;
static pthread_mutex_t g_SocketListCreateMutex = PTHREAD_MUTEX_INITIALIZER;
void SocketListInit()
{
    if (g_socketList == NULL) {
        pthread_mutex_lock(&g_SocketListCreateMutex);
        if (g_socketList == NULL) {
            g_socketList = InitList();
        }
        pthread_mutex_unlock(&g_SocketListCreateMutex);
    }
}

void InsertSocketList(int s)
{
    struct node* t = (struct node*)malloc(sizeof (*t));
    memset(t, 0, sizeof(*t));
    t->data = (int*)malloc(sizeof(s));
    memcpy(t->data, &s, sizeof(s));

    InsertHead(g_socketList, t);
}

void DelFromSocketList(int s)
{
    list head = g_socketList;
    while (head->next != NULL) {
        if (*(int*)(head->next->data) == s) {
            break;
        }
        head = head->next;
    }

    list t = NULL;
    if (head->next != NULL) {
        t = head->next;
        head->next = head->next->next;

        free(t->data);
        free(t);
    }
}

int WriteServerSocket(unsigned char* wbuf, int len)
{
    list head = g_socketList;
    while (head->next != NULL) {
       int ret = send(*(int*)head->next->data, wbuf, len, 0);
       if (ret == -1) {
           return ret;
       }
    }
}

#endif

#ifdef __cplusplus
}
#endif
