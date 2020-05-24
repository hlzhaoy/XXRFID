#ifndef __SELECT_H__
#define __SELECT_H__

#include "XXRFID.h"

#define REGION(msg) 1

#ifdef __cplusplus
extern "C" {
#endif

void CreateSelectThread();
void InsertSelectList(XXRFIDCLient* client);
void SelectListInit();
int WriteSocket(unsigned char* buf, int len);
void cleanSelectList();

#ifdef __cplusplus
}
#endif

#endif
