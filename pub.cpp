#include "pub.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "list.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

list g_serialData = NULL;
static pthread_mutex_t g_queueMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_serialDataCreateMutex = PTHREAD_MUTEX_INITIALIZER;

void QueueInit()
{
    pthread_mutex_lock(&g_serialDataCreateMutex);
    if (g_serialData == NULL) {
        g_serialData = InitList();
    }
    pthread_mutex_unlock(&g_serialDataCreateMutex);
}

void QueueInsert(serialData sdata)
{
	pthread_mutex_lock(&g_queueMutex);
	serialData* tdata = (serialData*)malloc(sizeof(*tdata));
	memcpy(tdata, &sdata, sizeof(*tdata));

	list t = (list)malloc(sizeof(*t));
	memset(t, 0, sizeof(*t));
	t->data = tdata;

	InsertHead(g_serialData, t);
	pthread_mutex_unlock(&g_queueMutex);
}

int QueueGetAndDel(serialData* sdata)
{
	pthread_mutex_lock(&g_queueMutex);
	if (IsEmpty(g_serialData) == true) {
		pthread_mutex_unlock(&g_queueMutex);
		return -1;
	}

	serialData* tmp = (serialData*)GetListTailData(g_serialData);
	if (tmp == NULL) {
		pthread_mutex_unlock(&g_queueMutex);
		return -1;
	}

	memcpy(sdata, tmp, sizeof(serialData));
	free(tmp);
	pthread_mutex_unlock(&g_queueMutex);

    return 0;
}

unsigned short CRC16_CCITT(unsigned char* pchMsg, unsigned short wDataLen)
{
    unsigned char i, chChar;
    unsigned short wCRC = 0;
    while (wDataLen--) {
        chChar = *pchMsg++;
        wCRC ^= (((unsigned short)chChar) << 8);
        for (i = 0; i < 8; i++) {
			if (wCRC & 0x8000) {
				wCRC = (wCRC << 1) ^ 0x1021;
			} else {
                wCRC <<= 1;
			}
        }
    }

    return wCRC;
}

bool CRCVerify(unsigned char* buf, int len)
{
	unsigned short ret = CRC16_CCITT(&buf[1], len - 3);
	if (((ret & 0xFF) == buf[len - 1]) && ((ret >> 8 & 0xFF) == buf[len - 2])) {
		return true;
	}

	return false;
}

char* HexToString(unsigned char* buf, int len)  //调用者释放内存
{
    char* result = new char[len*2+1];
    memset(result, 0, len*2+1);
    char* pAscii = result;

    unsigned char Nibble[2];
    for (int i = 0; i < len; i++){
        Nibble[0] = (buf[i] & 0xF0) >> 4;
        Nibble[1] = buf[i] & 0x0F;
        for (int j = 0; j < 2; j++){
            if (Nibble[j] < 10){
				Nibble[j] += 0x30;
            }
            else{
				if (Nibble[j] < 16) {
					Nibble[j] = Nibble[j] - 10 + 'A';
				}
            }
            *pAscii++ = Nibble[j];
        }
    }

    return result;
}

char* StringToHex(char* buf, int* len)  //调用者释放内存
{
    char* out = new char[strlen(buf)/2+1];
    char *p = buf;
    char high = 0, low = 0;
    int tmplen = strlen(p), cnt = 0;
    tmplen = strlen(p);
    while(cnt < (tmplen / 2))
    {
        high = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
        low = (*(++ p) > '9' && ((*p <= 'F') || (*p <= 'f'))) ? *(p) - 48 - 7 : *(p) - 48;
        out[cnt] = ((high & 0x0f) << 4 | (low & 0x0f));
        p ++;
        cnt ++;
    }

    if(tmplen % 2 != 0) {
        char tc = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
        out[cnt] = tc<<4;
    }

    if(len != NULL) *len = tmplen / 2 + tmplen % 2;
    return out;
}


void Log(const char* func, int lineNo, const char*msg)
{
	struct tm *tempTm;
    struct timeval time;
        
    gettimeofday(&time,NULL);
    tempTm = localtime(&time.tv_sec);

	char buf[8192] = {0};
	sprintf(buf, "[%d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d][%s : %d] %s \n",
			tempTm->tm_year + 1900, tempTm->tm_mon + 1, tempTm->tm_mday, tempTm->tm_hour, tempTm->tm_min, tempTm->tm_sec,
			time.tv_usec / 1000, func, lineNo, msg);

    FILE* pf = fopen("Log.log", "a");
    if(pf != NULL) {
        fwrite(buf, strlen(buf), 1, pf);
        fclose(pf);
    } else {
		printf("failed to fopen: %s \n", strerror(errno));
    }

    printf("%s \n", buf);
}

void PreciseSleepMillisecond(int ms)
{
	usleep(ms);
}

#if REGION("协议填充")
void FillProtocol(unsigned char* buf, unsigned int proto)
{
    buf[3] = proto & 0xFF;
    buf[2] = (proto >> 8) & 0xFF;
    buf[1] = (proto >> 16) & 0xFF;
    buf[0] = (proto >> 24) & 0xFF;
}

void FillDataLen(unsigned char* buf, unsigned short len)
{
    buf[1] = len&0xFF;
    buf[0] = (len>>8) & 0xFF;
}

void FillCrC(unsigned char* buf, unsigned int len)
{
    unsigned short crc = CRC16_CCITT((unsigned char *)buf, len);
    buf[len] = (crc>>8) & 0xFF;
    buf[len+1] = crc & 0xFF;
}

unsigned int GetMsgHead(MsgType type, MsgId mid)
{
	unsigned int head = 0x0001<<16 | type <<8 | mid;
	return head;
}
#endif

#ifdef __cplusplus
}
#endif
