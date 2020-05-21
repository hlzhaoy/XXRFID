#ifndef __COM_H__
#define __COM_H__

#ifdef __cplusplus
extern "C" {
#endif

int initCom(char* comNo, char* bandRate);

int StartCom(XXRFIDCLient* client);

int readCom(int handCom, unsigned char* rbuf, int len);

int writeCom(int handCom, unsigned char* wbuf, int len);

int cleanCom(XXRFIDCLient* client);

#ifdef __cplusplus
}
#endif

#endif
