#ifndef __RFIDPROTOCOL_H__
#define __RFIDPROTOCOL_H__

#include "XXRFID.h"
#include "message.h"

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

void ProcDataResultCmd(XXRFIDCLient* s, unsigned char* buf);
void ProcReadFinish(XXRFIDCLient* s);

void getConnectStateCmd(unsigned char* buf, int* len, unsigned char* serial);

int Stop(XXRFIDCLient* s, unsigned char* buf, MsgBaseStop* msg);
void ProcStop(XXRFIDCLient* s, unsigned char* buf);

int InventoryEpc(XXRFIDCLient* s, unsigned char* buf, MsgBaseInventoryEpc* in);
void ProcInventoryEpc(XXRFIDCLient* s, unsigned char* buf);

int WriteEpc(XXRFIDCLient* s, unsigned char* buf, MsgBaseWriteEpc* in);
void ProcWriteEpc(XXRFIDCLient* s, unsigned char* buf);

int LockEpc(XXRFIDCLient* s, unsigned char* buf, MsgBaseLockEpc* in);
void ProcLockEpc(XXRFIDCLient* s, unsigned char* buf);

int DestoryEpc(XXRFIDCLient* s, unsigned char* buf, MsgBaseDestoryEpc* in);
void ProcDestoryEpc(XXRFIDCLient* s, unsigned char* buf);

int GetCapabilities(XXRFIDCLient* s, unsigned char* buf, MsgBaseGetCapabilities* msg);
void ProcGetCapabilities(XXRFIDCLient* s, unsigned char* buf);

int SetPower(XXRFIDCLient* s, unsigned char* buf, MsgBaseSetPower* msg);
void ProcSetPower(XXRFIDCLient* s, unsigned char* buf);

int GetPower(XXRFIDCLient* s, unsigned char* buf, MsgBaseGetPower* msg);
void ProcGetPower(XXRFIDCLient* s, unsigned char* buf);

int SetFreqRange(XXRFIDCLient* s, unsigned char* buf, MsgBaseSetFreqRange* msg);
void ProcSetFreqRange(XXRFIDCLient* s, unsigned char* buf);

int GetFreqRange(XXRFIDCLient* s, unsigned char* buf, MsgBaseGetFreqRange* msg);
void ProcGetFreqRange(XXRFIDCLient* s, unsigned char* buf);

int SetBaseband(XXRFIDCLient* s, unsigned char* buf, MsgBaseSetBaseband* msg);
void ProcSetBaseband(XXRFIDCLient* s, unsigned char* buf);

int GetBaseband(XXRFIDCLient* s, unsigned char* buf, MsgBaseGetBaseband* msg);
void ProcGetBaseband(XXRFIDCLient* s, unsigned char* buf);

int SetTagLog(XXRFIDCLient* s, unsigned char* buf, MsgBaseSetTagLog* msg);
void ProcSetTagLog(XXRFIDCLient* s, unsigned char *buf);

int GetTagLog(XXRFIDCLient* s, unsigned char *buf, MsgBaseGetTagLog* msg);
void ProcGetTagLog(XXRFIDCLient* s, unsigned char* buf);

int Inventory6b(XXRFIDCLient* s, unsigned char *buf, MsgBaseInventory6b *msg);
void ProcInventory6b(XXRFIDCLient* s, unsigned char* buf);

void Proc6bDataResultCmd(XXRFIDCLient* s, unsigned char* buf);
void Proc6BReadFinish(XXRFIDCLient* s, unsigned char* buf);

int Write6b(XXRFIDCLient* s, unsigned char *buf, MsgBaseWrite6b *msg);
void ProcWrite6b(XXRFIDCLient* s, unsigned char* buf);

int Lock6b(XXRFIDCLient* s, unsigned char *buf, MsgBaseLock6b *msg);
void ProcLock6b(XXRFIDCLient* s, unsigned char* buf);

int Lock6bGet(XXRFIDCLient* s, unsigned char *buf, MsgBaseLock6bGet *msg);
void ProcLock6bGet(XXRFIDCLient* s, unsigned char* buf);

int InventoryGB(XXRFIDCLient* s, unsigned char *buf, MsgBaseInventoryGb *msg);
void ProcInventoryGB(XXRFIDCLient* s, unsigned char* buf);

void ProcGBDataResultCmd(XXRFIDCLient* s, unsigned char* buf);
void ProcGBReadFinish(XXRFIDCLient* s, unsigned char* buf);

#ifdef __cplusplus
}
#endif

#endif
