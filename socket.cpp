#include "pub.h"
#include "socket.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "select.h"
#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef __cplusplus
    extern "C" {
#endif

static int socketHandle = -1;
static bool threadIsStop = true;

int initSocket(char* ip, char* port, int timeout)
{
    socketHandle = socket(AF_INET, SOCK_STREAM, 0);
    if (socketHandle == -1) {
        return -1;
    }

    /*setsockopt(socketHandle, SOL_SOCKET, SO_SNDTIMEO,(char*)&timeout,sizeof(timeout));

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    // servAddr.sin_addr.s_addr = inet_addr(ip);
	inet_pton(AF_INET, ip, &servAddr.sin_addr);
    short int tmpPort = atoi(port);
    if(tmpPort == 0) {
        return -1;
    }
    servAddr.sin_port = htons(tmpPort);

    int ret = connect(socketHandle, (struct sockaddr*)&servAddr, sizeof(servAddr));
    if (ret == -1) {
        return -1;
    }*/

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr(ip);  //具体的IP地址
    serv_addr.sin_port = htons(atoi(port));  //端口
    int ret = connect(socketHandle, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret == -1) {
        return -1;
    }

    return socketHandle;
}

int readSocket(int handle, unsigned char* buf, int* len)
{
    int ret = recv(handle, (char*)buf, *len, 0);

    *len = ret;

    return ret;
}

int writeSocket(int handle, unsigned char* buf, int len)
{
    int ret = send(handle, (char*)buf, len, 0);
    if (ret == -1) {
        return SYSTEMERR;
    }

    return SUCCESS;
}

int cleanSocket(XXRFIDCLient* client)
{
    close((long)client->handle);

    return ExitSelectThread(client);
}

#ifdef __cplusplus
}
#endif // __cplusplus
