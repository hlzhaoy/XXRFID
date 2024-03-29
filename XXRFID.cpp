#include "XXRFID.h"
#include "pub.h"
#include "COM.h"
#include "socket.h"
#include "RFIDProtocol.h"
#include <string.h>
#include "delegate.h"
#include "message.h"
#include <errno.h>
#include <stdio.h>
#include "protocol.h"
#include "ReaderProtocol.h"
#include "result.h"
#include "XXRFID.h"
#include "Server.h"
#include <stdlib.h>
#include "USB.h"
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <unistd.h>
#include "client.h"

#ifdef __cplusplus
extern "C" {
#endif

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  g_MessageProcCreateMutex = PTHREAD_MUTEX_INITIALIZER;

static bool threadIsStop = true;

int WriteCmd(XXRFIDCLient* client, unsigned char* buf, int len)
{
    int ret = 0;
    switch(client->type)
    {
    case ETH:
        ret = writeSocket(client, buf, len);
        break;

    case COM:
        ret = writeCom(client->handle, buf, len);
        break;

	case USB:
		ret = writeUSB((void*)client->handle, buf, len);
		break;

	case CLIENT:
		ret = WriteClient(client, buf, len);
		break;

    default:
        break;
    }

    return ret;
}

void* messageProcThread(void* lpParameter)
{
    int ret = 0;
    while(threadIsStop == false) {
        serialData data;
        ret = QueueGetAndDel(&data);
        if(ret == -1) {
            PreciseSleepMillisecond(5);
            continue;
        }

        messageProc(data);

        delete data.data;
        data.data = NULL;

		usleep(5);
    }

    return 0;
}

void RegCallBack(XXRFIDCLient*s, Callback_Type type, void* call)
{
	if (s == NULL) {
		return ;
	}

    switch (type) {
        case ETagEpcLog:
            s->call_TagEpcLog = (delegateTagEpcLog)call;
            break;

        case ETagEpcOver:
            s->call_TagEpcOver = (delegateTagEpcOver)call;
            break;

		case EGpiTriggerStart:
			s->call_GpiTriggerStart = (delegateGpiTriggerStart)call;
			break;

		case EGpiTriggerOver:
			s->call_GpiTriggerOver = (delegateGpiTriggerOver)call;
			break;

		case ETcpDisconnected:
			s->call_TcpDisconnected = (delegateTcpDisconnected)call;
			break;
			
    	case EGClientConnected:
			s->call_GClientConnected = (delegateGClientConnected)call;
			break;

		case ETag6bLog:
			s->call_Tag6bLog = (delegateTag6bLog)call;
			break;

		case ETag6bOver:
			s->call_Tag6bOver = (delegateTag6bOver)call;
			break;

		case ETagGbLog:
			s->call_TagGbLog = (delegateTagGbLog)call;
			break;

		case ETagGbOver:
			break;

        default:
            break;
    }
}

void SendSynMsg(XXRFIDCLient* s, MESSAGE type, void* msg)
{
	if (s == NULL || s->isOpened == false) {
		if (msg != NULL) {
			((Result*)msg)->RtCode = -1;
			strcpy(((Result*)msg)->RtMsg, "closed");
		}

		return ;
	}

	pthread_mutex_lock(&g_mutex);
    unsigned char buf[512] = {0};
	int len;
    switch(type) {
        case EMESS_Stop:
			len = Stop(s, buf, (MsgBaseStop*)msg);
        	break;

        case EMESS_InventoryEpc:
			len = InventoryEpc(s, buf, (MsgBaseInventoryEpc*)msg);
        	break;

		case EMESS_WriteEpc:
			len = WriteEpc(s, buf, (MsgBaseWriteEpc*)msg);
			break;

		case EMESS_LockEpc:
			len = LockEpc(s, buf, (MsgBaseLockEpc*)msg);
			break;

		case EMESS_DestoryEpc:
			len = DestoryEpc(s, buf, (MsgBaseDestoryEpc*)msg);
			break;

		case EMESS_Reset:
			len = Reset(s, buf);
			break;

		case EMESS_SetSerialParam:
			len = SetSerialParam(s, buf, (MsgAppSetSerialParam*)msg);
			break;

		case EMESS_GetSerialParam:
			len = GetSerialParam(s, buf, (MsgAppGetSerialParam*)msg);
			break;

		case EMESS_SetGpo:
			len = SetGpo(s, buf, (MsgAppSetGpo*)msg);
			break;

		case EMESS_GetGpiState:
			len = GetGpiState(s, buf, (MsgAppGetGpiState*)msg);
			break;

		case EMESS_SetGpiTrigger:
			len = SetGpiTrigger(s, buf, (MsgAppSetGpiTrigger*)msg);
			break;

		case EMESS_GetGpiTrigger:
			len = GetGpiTrigger(s, buf, (MsgAppGetGpiTrigger*)msg);
			break;

		case EMESS_SetEthernetIp:
			len = SetEthernetIp(s, buf, (MsgAppSetEthernetIP*)msg);
			break;

		case EMESS_GetEthernetIp:
			len = GetEthernetIp(s, buf, (MsgAppGetEthernetIP*)msg);
			break;

		case EMESS_GetEthernetMac:
			len = GetEtherneMac(s, buf, (MsgAppGetEthernetMac*)msg);
			break;

		case EMESS_SetTcpMode:
			len = SetTcpMode(s, buf, (MsgAppSetTcpMode*)msg);
			break;

		case EMESS_GetTcpMode:
			len = GetTcpMode(s, buf, (MsgAppGetTcpMode*)msg);
			break;

		case EMESS_GetBaseVersion:
			len = GetBaseVersion(s, buf, (MsgAppGetBaseVersion*)msg);
			break;

		case EMESS_GetReaderInfo:
			len = GetReaderInfo(s, buf, (MsgAppGetReaderInfo*)msg);
			break;

		case EMESS_GetCapabilities:
			len = GetCapabilities(s, buf, (MsgBaseGetCapabilities*)msg);
			break;

		case EMESS_SetPower:
			len = SetPower(s, buf, (MsgBaseSetPower*)msg);
			break;

		case EMESS_GetPower:
			len = GetPower(s, buf, (MsgBaseGetPower*)msg);
			break;

		case EMESS_SetFreqRange:
			len = SetFreqRange(s, buf, (MsgBaseSetFreqRange*)msg);
			break;

		case EMESS_GetFreqRange:
			len = GetFreqRange(s, buf, (MsgBaseGetFreqRange*)msg);
			break;

		case EMESS_SetBaseband:
			len = SetBaseband(s, buf, (MsgBaseSetBaseband*)msg);
			break;

		case EMESS_GetBaseband:
			len = GetBaseband(s, buf, (MsgBaseGetBaseband*)msg);
			break;

		case EMESS_SetTagLog:
			len= SetTagLog(s, buf, (MsgBaseSetTagLog*)msg);
			break;

		case EMESS_GetTagLog:
			len = GetTagLog(s, buf, (MsgBaseGetTagLog*)msg);
			break;

		case EMESS_Inventory6b:
			len = Inventory6b(s, buf, (MsgBaseInventory6b*)msg);
			break;

		case EMESS_Write6b:
			len = Write6b(s, buf, (MsgBaseWrite6b*)msg);
			break;

		case EMESS_Lock6b:
			len = Lock6b(s, buf, (MsgBaseLock6b*)msg);
			break;

		case EMESS_Lock6bGet:
			len = Lock6bGet(s, buf, (MsgBaseLock6bGet*)msg);
			break;

		case EMESS_SetResume:
			len = SetResume(s, buf, (MsgAppSetResume*)msg);
			break;

		case EMESS_GetResume:
			len = GetResume(s, buf, (MsgAppGetResume*)msg);
			break;

		case EMESS_GetCache:
			len = GetCache(s, buf, (MsgAppGetCache*)msg);
			break;

		case EMESS_CleanCache:
			len = CleanCache(s, buf, (MsgAppCleanCache*)msg);
			break;

        default:
        	break;
    }

	do {
		if(len == -1) {
			LOG_TICK("获取写入命令出错");
			if (msg != NULL) {
				((Result*)msg)->RtCode = -1;
				strcpy(((Result*)msg)->RtMsg, "Failed to CreateCmd");
			}
			break;
		}

		int ret = WriteCmd(s, buf, len);
		if(ret != SUCCESS) {
	        LOG_TICK("failed to WriteCmd");
			if (msg != NULL) {
				((Result*)msg)->RtCode = -1;
				strcpy(((Result*)msg)->RtMsg, "Failed to WriteCmd");
			}
			break;
		}

		if (msg != NULL) {
			struct timespec to;
			memset(&to, 0, sizeof(to));
			to.tv_sec = time(NULL) + 5;
			to.tv_nsec = 0;
			int ret = sem_timedwait(&s->sem[type], &to);
			if (ret != 0) {
				if(errno == ETIMEDOUT) {
					((Result*)msg)->RtCode = -1;
					strcpy(((Result*)msg)->RtMsg, "Timeout");
				}
			}
		}
	}while(0);

	if (s != NULL && s->result != NULL) {
		s->result[type].rst = NULL;
	}
	pthread_mutex_unlock(&g_mutex);
}

int StartProcThread()
{
	if (threadIsStop == true) {
		pthread_mutex_lock(&g_MessageProcCreateMutex);
		if (threadIsStop == true) {
			QueueInit();
			threadIsStop = false;
			pthread_t threadID;
			int ret = pthread_create(&threadID, NULL, messageProcThread, NULL);
			if (ret != 0) {
				LOG_TICK("failed to pthread_create");
				pthread_mutex_unlock(&g_MessageProcCreateMutex);
				return -1;
			} else {
				pthread_detach(threadID);
			}
		}
		pthread_mutex_unlock(&g_MessageProcCreateMutex);
	}

	return 0;
}

/************************************************
 *  name:
 *      OpenSerial
 *  para:
 *      readerName: /dev/ttyS0:115200
 *      timeout:
 *  return:
 *
*************************************************/
XXRFIDCLient* OpenSerial(char* readerName, int timeout)
{
    int ret = SUCCESS;
	XXRFIDCLient* s = NULL;

    do {
		char com[128] = {0};
		strcpy(com, readerName);
		char *baudRate = strchr(com, ':');
		if ((baudRate == NULL) || (baudRate+1) == NULL) {
			LOG_TICK("错误的连接地址");
			return NULL;
		}

		com[baudRate-com] = '\0';
		baudRate += 1;

		s = (XXRFIDCLient*)malloc(sizeof(XXRFIDCLient));
		if (s == NULL) {
			ret = -1;
			LOG_TICK("failed to new");
			break;
		}

		memset(s, 0, sizeof(XXRFIDCLient));
		s->handle = initCom(com, baudRate);
		if (s->handle == -1) {
			ret = -1;
			LOG_TICK("failed to initCom");
			break;
		}

		if (StartProcThread() == -1) {
			ret = -1;
			LOG_TICK("failed to StartProcThread");
			break;
		}

		s->sem = (sem_t*)malloc(sizeof(sem_t) * EMESS_Count);
		if (s->sem == NULL) {
			ret = -1;
			LOG_TICK("failed to malloc");
			break;
		}
 
		memset(s->sem, 0, sizeof(sem_t) * EMESS_Count);
		for (int i = 0; i < EMESS_Count; i++) {
			int res = sem_init(&s->sem[i], 0, 0);
			if (res != 0) {
				LOG_TICK("filed to sem_init");
				ret = -1;
				break;
			}
		}

		s->result = (MessageResult*)malloc(sizeof(MessageResult) * EMESS_Count);
		if (s->result == NULL) {
			ret = -1;
			LOG_TICK("failed to malloc");
			break;
		}
		memset(s->result, 0, sizeof(MessageResult) * EMESS_Count);

		s->data = (unsigned char*)malloc(1024);
		if (s->data == NULL) {
			ret = -1;
			LOG_TICK("failed to malloc");
			break;
		}
		memset(s->data, 0, 1024);

		s->mutex = PTHREAD_MUTEX_INITIALIZER;
	} while (false) ;

	if (ret < 0) {
		if (s != NULL && s->result != NULL) {
			free(s->result);
			s->result = NULL;
		}

		if (s != NULL && s->sem != NULL) {
			free(s->sem);
			s->result = NULL;
		}

		if (s != NULL && s->data != NULL) {
			free(s->data);
			s->result = NULL;
		}

		if (s != NULL) {
			free(s);
		}

		return NULL;
	}

	s->type = COM;
	s->isOpened = true;

	StartCom(s);

    return s;
}

XXRFIDCLient* OpenUSB(int timeout)
{
	int ret = SUCCESS;
	XXRFIDCLient* s = NULL;

	do {
		s = (XXRFIDCLient*)malloc(sizeof(XXRFIDCLient));
		if (s == NULL) {
			ret = -1;
			LOG_TICK("failed to new");
			break;
		}

		memset(s, 0, sizeof(XXRFIDCLient));
		s->handle = (long)initUSB(timeout);
		if((s->handle) == NULL) {
			free(s);
			LOG_TICK("failed to initUSB");
			return NULL;
		}

		if (StartProcThread() == -1) {
			ret = -1;
			LOG_TICK("failed to StartProcThread");
			break;
		}

		s->sem = (sem_t*)malloc(sizeof(sem_t) * EMESS_Count);
		if (s->sem == NULL) {
			ret = -1;
			LOG_TICK("failed to malloc");
			break;
		}

		memset(s->sem, 0, sizeof(sem_t) * EMESS_Count);
		for (int i = 0; i < EMESS_Count; i++) {
			int res = sem_init(&s->sem[i], 0, 0);
			if (res != 0) {
				LOG_TICK("filed to sem_init");
				ret = -1;
				break;
			}
		}

		s->result = (MessageResult*)malloc(sizeof(MessageResult) * EMESS_Count);
		if (s->result == NULL) {
			ret = -1;
			LOG_TICK("failed to malloc");
			break;
		}
		memset(s->result, 0, sizeof(MessageResult) * EMESS_Count);

		s->data = (unsigned char*)malloc(1024);
		if (s->data == NULL) {
			ret = -1;
			LOG_TICK("failed to malloc");
			break;
		}
		memset(s->data, 0, 1024);

		s->mutex = PTHREAD_MUTEX_INITIALIZER;
	} while (false) ;

	if (ret < 0) {
		if (s != NULL && s->result != NULL) {
			free(s->result);
			s->result = NULL;
		}

		if (s != NULL && s->sem != NULL) {
			free(s->sem);
			s->sem = NULL;
		}

		if (s != NULL && s->data != NULL) {
			free(s->data);
			s->data = NULL;
		}

		if (s != NULL) {
			free(s);
		}

		return NULL;
	}

	s->type = USB;
	s->isOpened = true;

	StartUSB(s);

	return s;
}

XXRFIDCLient* OpenTcp(char* readerName, int timeout)
{
    int ret = SUCCESS;
	XXRFIDCLient* s;
	int handSocket = -1;

    do {
		char ip[128] = {0};
		strcpy(ip, readerName);
		char* port = strchr(ip, ':');
		if ((port == NULL) || ((port + 1) == NULL)) {
			LOG_TICK("错误的连接地址");
			return NULL;
		}

		ip[port - ip] = '\0';
		port += 1;

		s = (XXRFIDCLient*)malloc(sizeof(XXRFIDCLient));
		if (s == NULL) {
			LOG_TICK("failed to malloc");
			ret = -1;
			break;
		}
		memset(s, 0, sizeof(XXRFIDCLient));

		handSocket = initSocket(ip, port, timeout);
		if (handSocket == -1) {
			LOG_TICK("failed to initSocket");
			ret = -1;
			break;
		}

		s->handle = handSocket;

		if (StartProcThread() == -1) {
			ret = -1;
			LOG_TICK("failed to StartProcThread");
			break;
		}

		s->sem = (sem_t*)malloc(sizeof(sem_t) * EMESS_Count);
		if (s->sem == NULL) {
			ret = -1;
			LOG_TICK("failed to malloc");
			break;
		}

		memset(s->sem, 0, sizeof(sem_t) * EMESS_Count);
		for (int i = 0; i < EMESS_Count; i++) {
			int res = sem_init(&s->sem[i], 0, 0);
			if (res != 0) {
				LOG_TICK("filed to sem_init");
				ret = -1;
				break;
			}
		}

		s->result = (MessageResult*)malloc(sizeof(MessageResult) * EMESS_Count);
		if (s->result == NULL) {
			ret = -1;
			LOG_TICK("failed to malloc");
			break;
		}
		memset(s->result, 0, sizeof(MessageResult) * EMESS_Count);

		s->data = (unsigned char*)malloc(1024);
		if (s->data == NULL) {
			ret = -1;
			LOG_TICK("failed to malloc");
			break;
		}
		memset(s->data, 0, 1024);

		s->mutex = PTHREAD_MUTEX_INITIALIZER;
    } while(false);

	if (ret < 0) {
		if (s != NULL && s->result != NULL) {
			free(s->result);
			s->result = NULL;
		}

		if (s != NULL && s->sem != NULL) {
			free(s->sem);
			s->sem = NULL;
		}

		if (s != NULL && s->data != NULL) {
			free(s->data);
			s->data = NULL;
		}

		if (s != NULL) {
			free(s);
		}

		return NULL;
	}

	s->type = ETH;
	s->isOpened = true;
	StartSocket(s);

    return s;
}

XXRFIDCLient* Open(short port)
{
	int ret = 0;
	XXRFIDCLient* client = NULL;

	do {
		int handle = OpenServer(port);
		if(handle == -1) {
			LOG_TICK("failed to OpenServer");
			return NULL;
		}

		client = (XXRFIDCLient*)malloc(sizeof(*client));
		if(client == NULL) {
			close(handle);
			LOG_TICK("failed to malloc");
			return NULL;
		}

		client->handle = handle;
		client->type = SERVER;

		if (StartProcThread() == -1) {
			ret = -1;
			LOG_TICK("failed to StartProcThread");
			break;
		}
		client->mutex = PTHREAD_MUTEX_INITIALIZER;
	} while (false);

	if (ret < 0) {
		if (client != NULL) {
			free(client);
		}

		return NULL;
	}

	client->isOpened = true;
	StartServer(client);

	return client;
}

void Close(XXRFIDCLient* s)
{
    int ret = 0;

	switch(s->type) {
		case ETH:
			cleanSocket(s);
			break;

		case COM:
			cleanCom(s);
			break;

		case SERVER:
			CloseServer(s);
			break;

		case CLIENT:
			ClearClient(s);
			break;

		case USB:
			cleanUSB(s);
			break;

		default:
		break;
	}

    threadIsStop = true; 

    return;
}

#ifdef __cplusplus
}
#endif

