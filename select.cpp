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
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

list g_SelectList = NULL;   // 连接列表

static pthread_t threadID;
static bool g_IsRuning = false;
static pthread_mutex_t g_SelectCreateMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_SelectListMutex = PTHREAD_MUTEX_INITIALIZER;

void* ProcSelect(void* lpParameter)
{
    fd_set readset;
    int maxfd = 0;

    while (g_IsRuning == true) {
        pthread_mutex_lock(&g_SelectListMutex);

        if (IsEmpty(g_SelectList) == true) {
            break;
        }

        FD_ZERO(&readset);
        maxfd = 0;

		for (list t = g_SelectList->next; t != NULL; t = t->next) {
			FD_SET(((XXRFIDCLient*)(t->data))->handle, &readset);
            if (((XXRFIDCLient*)(t->data))->handle > maxfd) {
                maxfd = ((XXRFIDCLient*)(t->data))->handle;
            }
		}

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

            break;
        } else if(ret == 0) {
            pthread_mutex_unlock(&g_SelectListMutex);
            usleep(50);
            continue;
        }

		for (list t = g_SelectList->next; t != NULL;) {
            if (FD_ISSET(((XXRFIDCLient*)(t->data))->handle, &readset)) {
                ret = recv(((XXRFIDCLient*)(t->data))->handle, (char*)&(((XXRFIDCLient*)(t->data))->data[((XXRFIDCLient*)(t->data))->index]), 
                    BUF_LINE - ((XXRFIDCLient*)(t->data))->index, 0);
                if (ret == 0) {
                    LOG_TICK("recv = 1");
                    if (((XXRFIDCLient*)(t->data))->call_TcpDisconnected != NULL) {
                        ((XXRFIDCLient*)(t->data))->call_TcpDisconnected((char*)"peer close the socket");
                    }

                    list tmp = t;
                    t = t->next;
                    DeleteList(g_SelectList, tmp);
                    if (IsEmpty(g_SelectList) == true) {
                        pthread_mutex_unlock(&g_SelectListMutex);
                        break;
                    }
                    pthread_mutex_unlock(&g_SelectListMutex);
                    continue;
                } else if(ret < 0) {

                }

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

            t = t->next;
        }

        pthread_mutex_unlock(&g_SelectListMutex);
        PreciseSleepMillisecond(50);
    }
    pthread_mutex_unlock(&g_SelectListMutex);
    g_IsRuning = false;

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
            } else {
                pthread_detach(threadID);
            }
        }
        pthread_mutex_unlock(&g_SelectCreateMutex);
    }
}

void InsertSelectList(XXRFIDCLient* client)
{
    LOG_TICK("");
    pthread_mutex_lock(&g_SelectListMutex);
    LOG_TICK("");
	list t = (list)malloc(sizeof(*t));
	memset(t, 0, sizeof(*t));
	t->data = client;

	InsertHead(g_SelectList, t);
    LOG_TICK("");
    pthread_mutex_unlock(&g_SelectListMutex);
    LOG_TICK("");
}

void DelFromSelectList(XXRFIDCLient* client)
{
    LOG_TICK("");
    pthread_mutex_lock(&g_SelectListMutex);
    LOG_TICK("");
    list head = g_SelectList;
    while (head->next != NULL) {
        if (head->next->data == client) {
            break;
        }
    }

    if (head->next != NULL) {
        list t = head->next;
        head->next = head->next->next;

        free(((XXRFIDCLient*)t->data)->sem);
        free(((XXRFIDCLient*)t->data)->result);
        free(t);
    }
    LOG_TICK("");
    pthread_mutex_unlock(&g_SelectListMutex);
    LOG_TICK("");
}

int WriteSocket(unsigned char* buf, int len)
{
    bool errFlag = false;
    list head = g_SelectList;
    if (head == NULL) {
        LOG_TICK("head == NULL");
        return -1;
    }

    while (head->next != NULL) {
        int ret = send(((XXRFIDCLient*)(head->next->data))->handle, buf, len, 0);
        if (ret == -1) {
            errFlag = true;
        }
        
        head = head->next;
    }

    if (errFlag == true) {
        LOG_TICK("head == NULL");
        return -1;
    }

    return SUCCESS;
}

void cleanSelectList()
{
    pthread_mutex_lock(&g_SelectListMutex);

    list head = g_SelectList;
    if (head == NULL) {
        return;
    }

    while (head->next != NULL) {
        list t = head->next;

        XXRFIDCLient* tmp = (XXRFIDCLient*)(t->data);
        free(tmp->data);
        free(t);

        head = head->next;
    }

    g_SelectList->next = NULL;

    pthread_mutex_unlock(&g_SelectListMutex);
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
