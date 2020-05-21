#ifndef __PUB_H__
#define __PUB_H__

#include "protocol.h"
#include "XXRFID.h"

#ifdef __cplusplus
    extern "C" {
#endif

#if 1
#define LOG_TICK(msg) Log(__FUNCTION__, __LINE__, msg);
#else
#define LOG_TICK
#endif

typedef enum {
    SUCCESS = 0,   //函数执行成功
    ADDRERR,       //连接地址参数错误
    TIMEOUTERR,    //超时时间参数错误
    ANTIDERR,      //启用的天线编号参数错误
    ANTPOWERERR,   //天线功率参数错误
    RWTIMEOUT,     //读写超时
    PASSWDERR,     //访问密码错误
    LOWPOWER,      //功率不足
    DATAOVERFLOW,  //数据区溢出
    DATALOSE,      //标签数据丢失
    UNMATCH,       //写标签时未匹配到标签数据
    SENDCMDERR,    //读写器发送指令错误
    SYSTEMERR,
}ReturnNum;

typedef struct
{
    unsigned char* data;
    int len;
    XXRFIDCLient* s;
}serialData;

void QueueInit();
void QueueInsert(serialData sdata);

int QueueGetAndDel(serialData* sdata);

unsigned short CRC16_CCITT(unsigned char* pchMsg, unsigned short wDataLen);
bool CRCVerify(unsigned char* buf, int len);

char* HexToString(unsigned char* buf, int len);

char* StringToHex(char* buf, int* len);

void Log(const char* func, int lineNo, const char*msg);
void PreciseSleepMillisecond(int ms);

void messageProc(serialData data);
int WriteCmd(XXRFIDCLient* client, unsigned char* buf, int len);

#if REGION("协议填充")
void FillProtocol(unsigned char* buf, unsigned int proto);
void FillDataLen(unsigned char* buf, unsigned short len);
void FillCrC(unsigned char* buf, unsigned int len);
unsigned int GetMsgHead(MsgType type, MsgId mid);
#endif

#ifdef __cplusplus
}
#endif

#endif