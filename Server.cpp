#include "XXRFID.h"
#include "select.h"
#include "pub.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
* accepte线程
*/
static bool g_accepteThreadIsStop = true;
static pthread_t g_acceptThreadId = 0;
void* ServerThread(void* lpParam) 
{
	XXRFIDCLient* client  = (XXRFIDCLient*)lpParam;

	while (g_accepteThreadIsStop == false) {
		int s = accept(client->handle, NULL, NULL);
		if (s == -1) {
			char tmp[128] = {0};
			sprintf(tmp, "failed to accept, errno = %d", errno);
			LOG_TICK(tmp);
			usleep(50);
			continue;
		}
		LOG_TICK("accept");

		XXRFIDCLient* tmp = (XXRFIDCLient*)malloc(sizeof(*tmp));
		memset(tmp, 0, sizeof(tmp));
		memcpy(tmp, client, sizeof(*tmp));

		tmp->data = (unsigned char*)malloc(1024);
		memset(tmp->data, 0, 1024);
		tmp->handle = s;

		InsertSelectList(tmp);		
		CreateSelectThread();

		if (client->call_GClientConnected != NULL) {
			client->call_GClientConnected((char*)"a new connect");
		}

		usleep(50);
	}
}

int StartServer(XXRFIDCLient* client)
{
	g_accepteThreadIsStop = false;
	int ret = pthread_create(&g_acceptThreadId, NULL, ServerThread, (void*)client);
	if (ret != 0) {
		LOG_TICK("failed to pthread_create");
	} else {
		pthread_detach(g_acceptThreadId);
	}

	return ret;
}

int OpenServer(short port)
{
    int handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(handle == -1) {
		LOG_TICK("failed to socket");
		return -1;
	}

	struct sockaddr_in in;
	memset(&in, 0, sizeof(struct sockaddr_in));
	in.sin_addr.s_addr = INADDR_ANY;
	in.sin_family = AF_INET;
	in.sin_port = htons(port);

	if(bind(handle, (struct sockaddr*)&in, sizeof(in)) == -1) {
		LOG_TICK("failed to bind");
		return -1;
	}

	listen(handle, 5);

	return handle;
}

int WriteServer(int handle, unsigned char *buf, int len)
{
	return WriteSocket(buf, len);
}

int CloseServer(XXRFIDCLient* client)
{
	g_accepteThreadIsStop = true;
    close(client->handle);
    client->handle;
	
    cleanSelectList();
}

#ifdef __cplusplus
}
#endif
