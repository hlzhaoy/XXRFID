#ifndef __SELECT_H__
#define __SELECT_H__

#include "XXRFID.h"

#ifdef __cplusplus
extern "C" {
#endif

void CreateSelectThread();
void InsertSelectList(XXRFIDCLient* client);
int ExitSelectThread(XXRFIDCLient* client);
void SelectListInit();

#ifdef __cplusplus
}
#endif

#endif
