#include "XXRFID.h"
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
#include "client.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
* accept线程
*/
void* ServerThread(void* lpParam) 
{
	XXRFIDCLient* client  = (XXRFIDCLient*)lpParam;

	while (client->threadIsStop == false) {
		int s = accept(client->handle, NULL, NULL);
		if (s == -1) {
			char tmp[128] = {0};
			sprintf(tmp, "failed to accept, errno = %d", errno);
			LOG_TICK(tmp);
			usleep(50);
			continue;
		}

		XXRFIDCLient* tmp = (XXRFIDCLient*)malloc(sizeof(*tmp));
		memset(tmp, 0, sizeof(tmp));
		
		tmp->sem = (sem_t*)malloc(sizeof(sem_t) * EMESS_Count);
		if (tmp->sem == NULL) {
			LOG_TICK("failed to malloc");
			exit(0);
		}

		memset(tmp->sem, 0, sizeof(sem_t) * EMESS_Count);
		for (int i = 0; i < EMESS_Count; i++) {
			sem_init(&tmp->sem[i], 0, 0);
		}

		tmp->result = (MessageResult*)malloc(sizeof(MessageResult) * EMESS_Count);
		if (tmp->result == NULL) {
			LOG_TICK("failed to malloc");
			exit(0);
		}
		memset(tmp->result, 0, sizeof(MessageResult) * EMESS_Count);

		tmp->data = (unsigned char*)malloc(1024);
		if (tmp->data == NULL) {
			LOG_TICK("failed to malloc");
			exit(0);
		}
		memset(tmp->data, 0, 1024);

		tmp->handle = s;
		tmp->type = CLIENT;

		if (client->call_GClientConnected != NULL) {
			client->call_GClientConnected(tmp);
		}

		StartClient(tmp);

		usleep(50);
	}
}

int StartServer(XXRFIDCLient* client)
{
	pthread_t g_acceptThreadId;
	client->threadIsStop = false;
	int ret = pthread_create(&g_acceptThreadId, NULL, ServerThread, (void*)client);
	if (ret != 0) {
		client->threadIsStop = true;
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

int CloseServer(XXRFIDCLient* client)
{
	client->threadIsStop = true;
    close(client->handle);
    client->handle;
}

#ifdef __cplusplus
}
#endif
