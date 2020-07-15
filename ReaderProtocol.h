#ifndef __READERPROTOCOL_H__
#define __READERPROTOCOL_H__

#include "message.h"
#include "XXRFID.h"

#ifdef __cplusplus
	extern "C" {
#endif

int Reset(XXRFIDCLient* s, unsigned char* buf);

int SetSerialParam(XXRFIDCLient* s, unsigned char* buf, MsgAppSetSerialParam* msg);
void ProcSetSerialParam(XXRFIDCLient* s, unsigned char* buf);

int GetSerialParam(XXRFIDCLient* s, unsigned char* buf, MsgAppGetSerialParam* msg);
void ProcGetSerialParam(XXRFIDCLient* s, unsigned char* buf);

int SetGpo(XXRFIDCLient* s, unsigned char* buf, MsgAppSetGpo* msg);
void ProcSetGpo(XXRFIDCLient* s, unsigned char* buf);

int GetGpiState(XXRFIDCLient* s, unsigned char* buf, MsgAppGetGpiState* msg);
void ProcGetGpiState(XXRFIDCLient* s, unsigned char* buf);

int SetGpiTrigger(XXRFIDCLient* s, unsigned char* buf, MsgAppSetGpiTrigger* msg);
void ProcSetGpiTrigger(XXRFIDCLient* s, unsigned char* buf);
void ProcGpiTriggerStart(XXRFIDCLient* s, unsigned char* buf);
void ProcGpiTriggerOver(XXRFIDCLient* s, unsigned char* buf);

int GetGpiTrigger(XXRFIDCLient* s, unsigned char* buf, MsgAppGetGpiTrigger* msg);
void ProcGetGpiTrigger(XXRFIDCLient* s, unsigned char* buf);

int SetEthernetIp(XXRFIDCLient* s, unsigned char* buf, MsgAppSetEthernetIP* msg);
void ProcSetEthernetIp(XXRFIDCLient* s, unsigned char* buf);

int GetEthernetIp(XXRFIDCLient* s, unsigned char* buf, MsgAppGetEthernetIP* msg);
void ProcGetEthernetIp(XXRFIDCLient* s, unsigned char* buf);

int GetEtherneMac(XXRFIDCLient* s, unsigned char * buf, MsgAppGetEthernetMac*msg);
void ProcGetEthernetMac(XXRFIDCLient* s, unsigned char* buf);

int SetTcpMode(XXRFIDCLient* s, unsigned char* buf, MsgAppSetTcpMode* msg);
void ProcSetTcpMode(XXRFIDCLient* s, unsigned char* buf);

int GetTcpMode(XXRFIDCLient* s, unsigned char* buf, MsgAppGetTcpMode* msg);
void ProcGetTcpMode(XXRFIDCLient* s, unsigned char* buf);

int GetBaseVersion(XXRFIDCLient* s, unsigned char* buf, MsgAppGetBaseVersion* msg);
void ProcGetBaseVersion(XXRFIDCLient* s, unsigned char* buf);

int GetReaderInfo(XXRFIDCLient* s, unsigned char* buf, MsgAppGetReaderInfo*);
void ProcGetReaderInfo(XXRFIDCLient* s, unsigned char* buf);

int SetResume(XXRFIDCLient* s, unsigned char* buf, MsgAppSetResume*);
void ProcSetResume(XXRFIDCLient* s, unsigned char* buf);

int GetResume(XXRFIDCLient* s, unsigned char* buf, MsgAppGetResume*);
void ProcGetResume(XXRFIDCLient* s, unsigned char* buf);

int GetCache(XXRFIDCLient* s, unsigned char* buf, MsgAppGetCache*);
void ProcGetCache(XXRFIDCLient* s, unsigned char* buf);

int CleanCache(XXRFIDCLient* s, unsigned char* buf, MsgAppCleanCache*);
void ProcCleanCache(XXRFIDCLient* s, unsigned char* buf);

#ifdef __cplusplus
}
#endif

#endif
