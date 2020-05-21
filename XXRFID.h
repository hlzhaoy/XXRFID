#ifndef __XXRFID_H__
#define __XXRFID_H__

#include "delegate.h"
#include <semaphore.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EMESS_Reset,
    EMESS_SetSerialParam,
    EMESS_GetSerialParam,
    EMESS_SetGpo,
    EMESS_GetGpiState,
    EMESS_SetGpiTrigger,
    EMESS_GetGpiTrigger,
    EMESS_GetBaseVersion,
    EMESS_GetReaderInfo,
    EMESS_GetCapabilities,
    EMESS_Stop,
    EMESS_SetPower,
    EMESS_GetPower,
    EMESS_SetFreqRange,
    EMESS_GetFreqRange,
    EMESS_SetBaseband,
    EMESS_GetBaseband,
    EMESS_SetTagLog,
    EMESS_GetTagLog,
    EMESS_InventoryEpc,
    EMESS_WriteEpc,
    EMESS_LockEpc,
    EMESS_DestoryEpc,
    EMESS_Inventory6b,
    EMESS_Write6b,
    EMESS_Lock6b,
    EMESS_Lock6bGet,
    EMESS_InventoryGb,
    EMESS_WriteGb,
    EMESS_LockGb,
    EMESS_DestoryGb,
    EMESS_SetEthernetIp,
    EMESS_GetEthernetIp,
    EMESS_GetEthernetMac,
    EMESS_SetTcpMode,
    EMESS_GetTcpMode,
    EMESS_Count
}MESSAGE;

typedef enum
{
    ETagEpcLog,
    ETagEpcOver,
    ETag6bLog,
    ETag6bOver,
    ETagGbLog,
    ETagGbOver,
    EGpiStart,
    EGpiOver,
    ETcpDisconnected,
    EGClientConnected,
    EGpiTriggerStart,
    EGpiTriggerOver,
}Callback_Type;

typedef enum {
    ETH = 1,
    COM = 2,
    SERVER = 3,
    ACCEPT = 4,
	USB = 5,
    OTHER = 1000
}ConnType;

#define BUF_LINE 1024

typedef struct {
	void* rst;
}MessageResult;

typedef struct {
    void* handle;
    delegateTagEpcLog call_TagEpcLog;
    delegateTagEpcOver call_TagEpcOver;
    delegateGpiTriggerStart call_GpiTriggerStart;
    delegateGpiTriggerOver call_GpiTriggerOver;
    delegateTag6bLog call_Tag6bLog;
    delegateTag6bOver call_Tag6bOver;
    delegateTagGbLog call_TagGbLog;
    delegateTcpDisconnected call_TcpDisconnected;
    delegateGClientConnected call_GClientConnected;
    sem_t* sem;
    MessageResult* result;
    pthread_mutex_t mutex;
    unsigned char* data;
    int index;
    ConnType type;
    bool isOpened;
}XXRFIDCLient;

XXRFIDCLient* OpenSerial(char* readerName, int timeout);
XXRFIDCLient* OpenUSB(int timeout);
XXRFIDCLient* OpenTcp(char* readerName, int timeout);
XXRFIDCLient* Open(short port);
void RegCallBack(XXRFIDCLient*s, Callback_Type type, void* call);
void SendSynMsg(XXRFIDCLient*s, MESSAGE type, void* msg);
void Close(XXRFIDCLient* s);

#ifdef __cplusplus
}
#endif

#endif