#include "XXRFID.h"
#include "select.h"
#include "pub.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

static int handle = -1;

int OpenServer(short port)
{
    handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

int CleanServer(XXRFIDCLient* client)
{
    close(handle);
    handle = -1;

    return ExitSelectThread(client);
}

#ifdef __cplusplus
}
#endif
