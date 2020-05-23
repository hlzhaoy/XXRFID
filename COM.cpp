#include "pub.h"
#include "COM.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "XXRFID.h"
#include "select.h"
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static pthread_t threadID;
static bool threadIsStop = true;

void* comThread(void* lpParam)  
{  
	XXRFIDCLient* client  = (XXRFIDCLient*)lpParam;

	while (threadIsStop == false) {
		int ret = readCom(client->handle, &client->data[client->index], BUF_LINE-client->index); 
		if(ret > 0) {
			client->index += ret;
			do 
			{
				if (client->index >= 6) {
					if (client->data[0] == 0x5A) {
						short tlen = (client->data[5]<<8) | client->data[6];
						if (client->index >= tlen+7+2) {
							if (CRCVerify((unsigned char*)(client->data), tlen + 9)) {
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

							usleep(5);
							continue;
						} 

						break;
					} else {
						memmove(client->data, &(client->data[1]), --client->index);
						usleep(5);
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

void setNewOptions(int m_fd, int baudrate)
{
   struct termios newtio;
   if (tcgetattr(m_fd, &newtio)!=0) {
      LOG_TICK("failed to tcgetattr");
   }

   speed_t _baud=0;
   switch (baudrate)
   {
#ifdef B0
   case      0: _baud=B0;     break;
#endif
   
#ifdef B50
   case     50: _baud=B50;    break;
#endif
#ifdef B75
   case     75: _baud=B75;    break;
#endif
#ifdef B110
   case    110: _baud=B110;   break;
#endif
#ifdef B134
   case    134: _baud=B134;   break;
#endif
#ifdef B150
   case    150: _baud=B150;   break;
#endif
#ifdef B200
   case    200: _baud=B200;   break;
#endif
#ifdef B300
   case    300: _baud=B300;   break;
#endif
#ifdef B600
   case    600: _baud=B600;   break;
#endif
#ifdef B1200
   case   1200: _baud=B1200;  break;
#endif
#ifdef B1800
   case   1800: _baud=B1800;  break;
#endif
#ifdef B2400
   case   2400: _baud=B2400;  break;
#endif
#ifdef B4800
   case   4800: _baud=B4800;  break;
#endif
#ifdef B7200
   case   7200: _baud=B7200;  break;
#endif
#ifdef B9600
   case   9600: _baud=B9600;  break;
#endif
#ifdef B14400
   case  14400: _baud=B14400; break;
#endif
#ifdef B19200
   case  19200: _baud=B19200; break;
#endif
#ifdef B28800
   case  28800: _baud=B28800; break;
#endif
#ifdef B38400
   case  38400: _baud=B38400; break;
#endif
#ifdef B57600
   case  57600: _baud=B57600; break;
#endif
#ifdef B76800
   case  76800: _baud=B76800; break;
#endif
#ifdef B115200
   case 115200: _baud=B115200; break;
#endif
#ifdef B128000
   case 128000: _baud=B128000; break;
#endif
#ifdef B230400
   case 230400: _baud=B230400; break;
#endif
#ifdef B460800
   case 460800: _baud=B460800; break;
#endif
#ifdef B576000
   case 576000: _baud=B576000; break;
#endif
#ifdef B921600
   case 921600: _baud=B921600; break;
#endif
   default:
      break;
   }
   cfsetospeed(&newtio, (speed_t)_baud);
   cfsetispeed(&newtio, (speed_t)_baud);

	int databits = 8;
	switch (databits) {
		case 5:
			newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS5;
			break;
		case 6:
			newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS6;
			break;
		case 7:
			newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS7;
			break;
		case 8:
		default:
			newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS8;
			break;
	}

	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~(PARENB | PARODD);
	newtio.c_cflag &= ~CRTSCTS;
	newtio.c_cflag &= ~CSTOPB;

	newtio.c_iflag = IGNBRK;
	newtio.c_iflag &= ~(IXON|IXOFF|IXANY);

	newtio.c_lflag = 0;
	newtio.c_oflag = 0;

	newtio.c_cc[VTIME] = 1;
	newtio.c_cc[VMIN] = 60;

	if (tcsetattr(m_fd, TCSANOW, &newtio) != 0) {
		LOG_TICK("failed to tcsetattr");
	}

	int mcs=0;
	ioctl(m_fd, TIOCMGET, &mcs);
	mcs |= TIOCM_RTS;
	ioctl(m_fd, TIOCMSET, &mcs);

	tcgetattr(m_fd, &newtio);
	newtio.c_cflag &= ~CRTSCTS;
	tcsetattr(m_fd, TCSANOW, &newtio);
}

int initCom(char* comNo, char* bandRate)
{
	int handle = open(comNo, O_RDWR);
	if (handle < 0) {
		LOG_TICK("failed to open");
		return -1;
	}

	int baud = atoi(bandRate);
	if (baud == 0) {
		LOG_TICK("the error baudrate");
		return -1;
	}

	setNewOptions(handle, baud);
	
	return handle;	
}

int StartCom(XXRFIDCLient* client)
{
	threadIsStop = false;
	int ret = pthread_create(&threadID, NULL, comThread, (void*)client);
	if (ret != 0) {
		LOG_TICK("failed to pthread_create");
	}

	return 0;
}

int readCom(int handCom, unsigned char* rbuf, int len)
{
	ssize_t ret = read(handCom, rbuf, len);
	if (ret == -1) {
		char buf[128] = {0};
		sprintf(buf, "failed to read : %d", errno);
		LOG_TICK(buf);
	}

	return ret;
}

int writeCom(int handCom, unsigned char* wbuf, int len)
{
	ssize_t ret = write(handCom, wbuf, len);
	if (ret == -1) {
		char buf[128] = {0};
		sprintf(buf, "failed to write : %d", errno);
		LOG_TICK(buf);
		return SYSTEMERR;
	}

	return SUCCESS;
}

int cleanCom(XXRFIDCLient* client)
{
	close((long)client->handle);
	client->handle = -1;

	LOG_TICK("in cleanCom");

	threadIsStop = true;
	pthread_join(threadID, NULL);

	return 0;
}

#ifdef __cplusplus
}
#endif
