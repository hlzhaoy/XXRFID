#ifndef __DELEGATE_H__
#define __DELEGATE_H__

typedef struct
{
	unsigned char Epc[512]; //16 进制EPC 字符串
	unsigned char Pc[5]; //PC 值, 16进制字符串
	unsigned char AntId; //天线编号
	unsigned char Rssi; //信号强度
	unsigned char Result; //标签读取结果，0 为读取成功，非0 为失败	1，标签无响应	2，CRC 错误	3，数据区被锁定	4，数据区溢出	5，访问密码错误	6，其他标签错误	7，其他读写器错误
	unsigned char Tid[512]; //16 进制TID 字符串
	unsigned char Userdata[512]; //16 进制Userdata 字符串
	unsigned char Reserved[512]; //16 进制保留区字符串
}LogBaseEpcInfo;
typedef void (*delegateTagEpcLog)(LogBaseEpcInfo msg);
typedef void (*delegateTagEpcOver)(void);

typedef struct {
	unsigned char GpiPort;
	unsigned char Level;
	char TriggerTime[32];
}LogBaseGpiStart;
typedef void (*delegateGpiTriggerStart)(LogBaseGpiStart msg);

typedef struct {
	unsigned char GpiPort;
	unsigned char Level;
	char TriggerTime[32];
}LogBaseGpiOver;
typedef void (*delegateGpiTriggerOver)(LogBaseGpiOver msg);

typedef struct {
	char tid[128];
	char antId;
	char rssi;
	char result;
	char userData[128];
}LogBase6bInfo;
typedef void (*delegateTag6bLog)(LogBase6bInfo msg);

typedef struct {
	char rst;
	char msg[256];
}LogBase6bOver;
typedef void (*delegateTag6bOver)(LogBase6bOver msg);

typedef struct {
	char epc[128];
	char tid[128];
	unsigned short pc;
	char andId;
	char rssi;
	char userData[128];
}LogBaseGbInfo;
typedef void (*delegateTagGbLog)(LogBaseGbInfo msg);

typedef void (*delegateTcpDisconnected)(char* msg);

/* 声明结构体XXRFIDCLient */
struct XXRFIDCLient;
typedef void (*delegateGClientConnected)(XXRFIDCLient *client);

#endif // DELEGATE_H
