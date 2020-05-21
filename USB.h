#ifndef __USB_H__
#define __USB_H__

#ifdef __cplusplus
extern "C" {
#endif

void* initUSB(int timeOut);

int StartUSB(XXRFIDCLient* client);

int readUSB(void* handUSB, unsigned char* rbuf, int len);

int writeUSB(void* handUSB, unsigned char* wbuf, int len);

int cleanUSB(XXRFIDCLient* client);

#ifdef __cplusplus
}
#endif

#endif
