#ifndef __SOCKET_H__
#define __SOCKET_H__

#ifdef __cplusplus
    extern "C" {
#endif

int initSocket(char* ip, char* port, int timeout);

int writeSocket(XXRFIDCLient *client, unsigned char* buf, int len);

int readSocket(XXRFIDCLient *client, unsigned char* buf, int len);

void StartSocket(XXRFIDCLient *client);

int cleanSocket(XXRFIDCLient* client);

#ifdef __cplusplus
}
#endif

#endif
