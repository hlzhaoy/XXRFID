#include "pub.h"
#include "USB.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "XXRFID.h"
#include <unistd.h>
#include <pthread.h>
#include <wchar.h>
#include "hidapi.h"
#include <unistd.h>

#define usVID 0x03eb
#define usPID 0x2421

#ifdef __cplusplus
extern "C" {
#endif

static pthread_t threadID;
static bool threadIsStop = true;

void* USBThread(void* lpParam)  
{  
	XXRFIDCLient* client  = (XXRFIDCLient*)lpParam;

	while (threadIsStop == false) {
		int ret = readUSB(client->handle, &client->data[client->index], BUF_LINE - client->index); 
		if(ret > 0) {
			client->index += ret;
			do 
			{
				if(client->index >= 6) {
					if (client->data[0] == 0x5A) {
						short tlen = (client->data[5]<<8) | client->data[6];
						if(client->index >= tlen+7+2) {
							if (CRCVerify((unsigned char*)(client->data), tlen + 9) == true) {
								serialData data;
								data.data = new unsigned char[9+tlen];
								int tmp = 9 + tlen;
								data.len = tmp;
								data.s = client;

								memcpy(data.data, client->data, data.len);

								QueueInsert(data);

								client->index -= 9+tlen;
								memmove(client->data, &client->data[tmp], client->index);								
							} else {
								memmove(client->data, &(client->data[1]), --client->index);
							}
							continue;
						}

						break;
					} else {
						memmove(client->data, &client->data[1], --client->index);
						continue;
					}
				} 

				break;
				
			} while (1);
		}

		usleep(5);
	}

	return 0;  
}

void* initUSB(int timeOut)
{
	int ret = hid_init();
	if (ret != 0) {
		LOG_TICK("failed to hid_init");
		return NULL;
	}

	hid_device *handle = hid_open(usVID, usPID, NULL);
	if (handle == NULL) {
		LOG_TICK("failed to hid_open");
		return NULL;
	}

	return handle;
}

int StartUSB(XXRFIDCLient* client)
{
	threadIsStop = false;
	int ret = pthread_create(&threadID, NULL, USBThread, (void*)client);
	if (ret != 0) {
		LOG_TICK("failed to pthread_create");
	}

	return ret;
}

int readUSB(void* handUSB, unsigned char* rbuf, int len)
{
	hid_device* handle = (hid_device*)handUSB;
	unsigned char buf[256] = {0};

	int ret = hid_read(handle, buf, sizeof(buf));
	if (ret > 0) {
		memcpy(rbuf, buf, ret);
		return ret;
	}

	return -1;
}

int writeUSB(void* handUSB, unsigned char* wbuf, int len)
{
	hid_device* handle = (hid_device*)handUSB;
	char buf[64] = {0};
	memcpy(&buf[1], wbuf, len);

	int ret = hid_write(handle, wbuf, sizeof(buf));
	if (ret != 64) {
		return -1;
	}

	return SUCCESS;
}

int cleanUSB(XXRFIDCLient* client)
{
	hid_close((hid_device*)client->handle);
	client->handle = NULL;

	threadIsStop = true;
	pthread_join(threadID, NULL);

	return 0;
}

#ifdef __cplusplus
}
#endif