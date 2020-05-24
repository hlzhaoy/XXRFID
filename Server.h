#ifndef __SERVER_H__
#define __SERVER_H__

#include "XXRFID.h"

#ifdef __cplusplus
extern "C" {
#endif

int OpenServer(short port);
int StartServer(XXRFIDCLient* client);
int CloseServer(XXRFIDCLient* client);

#ifdef __cplusplus
}
#endif

#endif
