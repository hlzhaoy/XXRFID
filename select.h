#ifndef __SELECT_H__
#define __SELECT_H__

#include "XXRFID.h"

#define REGION(msg) 1

#ifdef __cplusplus
extern "C" {
#endif

void CreateSelectThread();
void InsertSelectList(XXRFIDCLient* client);
int ExitSelectThread(XXRFIDCLient* client);
void SelectListInit();

#if REGION("接收到的socket连接")
void SocketListInit();
void InsertSocketList(int s);
void DelFromSocketList(int s);
int WriteServerSocket(unsigned char* wbuf, int len);
#endif

#ifdef __cplusplus
}
#endif

#endif
