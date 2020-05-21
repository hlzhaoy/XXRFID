#ifndef MESSAGE_H
#define MESSAGE_H

#define REGION(msg) 1

#ifdef __cplusplus
    extern "C" {
#endif

typedef enum {
    AntennaNo_1 = 0x1,
    AntennaNo_2 = 0x2,
    AntennaNo_3 = 0x4,
    AntennaNo_4 = 0x8,
    AntennaNo_5 = 0x10,
    AntennaNo_6 = 0x20,
    AntennaNo_7 = 0x40,
    AntennaNo_8 = 0x80,
    AntennaNo_9 = 0x100,
    AntennaNo_10 = 0x200,
    AntennaNo_11 = 0x400,
    AntennaNo_12 = 0x800,
    AntennaNo_13 = 0x1000,
    AntennaNo_14 = 0x2000,
    AntennaNo_15 = 0x4000,
    AntennaNo_16 = 0x8000,
    AntennaNo_17 = 0x10000,
    AntennaNo_18 = 0x20000,
    AntennaNo_19 = 0x40000,
    AntennaNo_20 = 0x80000,
    AntennaNo_21 = 0x100000,
    AntennaNo_22 = 0x200000,
    AntennaNo_23 = 0x400000,
    AntennaNo_24 = 0x800000,
    AntennaNo_25 = 0x1000000,
    AntennaNo_26 = 0x2000000,
    AntennaNo_27 = 0x4000000,
    AntennaNo_28 = 0x8000000,
    AntennaNo_29 = 0x10000000,
    AntennaNo_30 = 0x20000000,
    AntennaNo_31 = 0x40000000,
    AntennaNo_32 = 0x80000000,
}AntennaNo;

typedef struct {
	int RtCode;
	char RtMsg[128];
}Result;

#if REGION("停止")
typedef struct {
	Result rst;
}MsgBaseStop;
#endif

#if REGION("重启读写器")
typedef struct {
	Result rst;
}MsgAppReset;
#endif

#if REGION("读6c")
typedef struct {
	Result rst;
    char Area;
    unsigned short Start;
    char* HexData; //需要匹配的数据内容,十六进展
    int bitLen;   //位长度
}EpcFilter;

typedef struct {
	Result rst;
    char Mode;
    unsigned char len;
}EpcReadTid;

typedef struct {
	Result rst;
    unsigned short Start;
    unsigned char len;
}EpcReadUserdata;

typedef struct {
	Result rst;
	unsigned short Start;
    unsigned char len;
}EpcReadReserved;

typedef struct {
	Result rst;
    int AntennaEnable;
    char InventoryMode;
    EpcFilter Filter;
    EpcReadTid ReadTid;
    EpcReadUserdata ReadUserdata;
    EpcReadReserved ReadReserved;
    char StrHexPassword[9];
} MsgBaseInventoryEpc;
#endif

#if REGION("写EPC")
typedef struct {
	Result rst;
	int AntennaEnable;// 天线端口（使用天线枚举，详见AntennaNo,多个天线使用或）
	char Area; //待写入的标签数据区(0，保留区；1，EPC 区；2，TID 区；3，用户数据区)
	unsigned short Start; //待写入标签数据区的字起始地址
	char* HexStrWriteData; //待写入的数据内容(16进制字符串)
	EpcFilter Filter; //选择读取参数（详见参数说明）
	char StrHexPassword[9]; //访问密码, 16进制字符串
}MsgBaseWriteEpc;
#endif

#if REGION("锁EPC")
typedef struct {
	Result rst;
	int AntennaEnable; //天线端口（使用天线枚举，详见AntennaNo,多个天线使用或）
	char Area; //待锁定的标签数据区(0，灭活密码区；1，访问密码区；2，EPC 区；3，TID 区；4，用户数据区)
	char Mode; //锁操作类型(0，解锁；1，锁定；2，永久解锁；3，永久锁定)
	EpcFilter Filter; //选择读取参数（详见参数说明）
	char StrHexPassword[9]; //访问密码,, 16进制字符串
}MsgBaseLockEpc;
#endif

#if REGION("灭活标签")
typedef struct {
	Result rst;
	int AntennaEnable; //天线端口（使用天线枚举，详见AntennaNo,多个天线使用或）
	char StrHexPassword[9]; //密码,, 16进制字符串
	EpcFilter Filter; //选择读取参数（详见参数说明）
}MsgBaseDestoryEpc;
#endif

#if REGION("串口相关")
typedef struct {
	Result rst;
	char BaudrateIndex;
}MsgAppSetSerialParam;

typedef struct {
	Result rst;
	char BaudrateIndex;
}MsgAppGetSerialParam;
#endif

#if REGION("设置GPO")
typedef struct {
	Result rst;
	unsigned char Gpo1;
	unsigned char Gpo2;
	unsigned char Gpo3;
	unsigned char Gpo4;
}MsgAppSetGpo; //Gpo1-4:0, 继电器断开；1，继电器闭合
#endif

#if REGION("获取GPI状态")
typedef struct {
	Result rst;
	unsigned char Gpi1;
	unsigned char Gpi2;
	unsigned char Gpi3;
	unsigned char Gpi4;
}MsgAppGetGpiState;

#endif 

#if REGION("GPI触发参数")
typedef struct {
	Result rst;
	char GpiPort; //GPI 端口号，索引从0 开始
	char TriggerStart; //触发开始（0 触发关闭，1 低电平触发，2 高电平触发，3 上升沿触发，4 下降沿触发，5 任意边沿触发）
	char TriggerCommand[128]; //触发绑定命令（Hex字符串）,由调用者释放
	char TriggerOver; //触发停止（0 不停止，1 低电平触发，2 高电平触发，3 上升沿触发，4 下降沿触发，5 任意边沿触发，6 延时停止）
	unsigned short OverDelayTime; //延时停止时间（仅当停止条件为“延时停止”生效）, 以10ms为单位
	char LevelUploadSwitch; //触发不停止时IO 电平变化上传开关（0 不上传，1 上传）
}MsgAppSetGpiTrigger;

typedef struct {
	Result rst;
	char GpiPort; //GPI 端口号，索引从0 开始
	char TriggerStart; //触发开始（0 触发关闭，1 低电平触发，2 高电平触发，3 上升沿触发，4 下降沿触发，5 任意边沿触发）
	char TriggerCommand[128]; //触发绑定命令（Hex字符串）
	char TriggerOver; //触发停止（0 不停止，1 低电平触发，2 高电平触发，3 上升沿触发，4 下降沿触发，5 任意边沿触发，6 延时停止）
	unsigned short OverDelayTime; //延时停止时间（仅当停止条件为“延时停止”生效）, 以10ms为单位
	char LevelUploadSwitch; //触发不停止时IO 电平变化上传开关（0 不上传，1 上传）
}MsgAppGetGpiTrigger;
#endif

#if REGION("基带版本号")
typedef struct {
	Result rst;
	char version[16];
}MsgAppGetBaseVersion;
#endif

#if REGION("查询读写器信息")
typedef struct {
	Result rst;
	char Imei[64];// 读写器流水号
	char PowerOnTime[32]; //：上电时间
	char BaseBuildDate[32];//基带编译时间
	char AppVersion[16]; //应用软件版本（如：“0.1.0.0”）
	char AppBuildDate[32]; //应用编译时间
	char SystemVersion[64]; //操作系统版本
}MsgAppGetReaderInfo;
#endif

#if REGION("查询读写器能力")
typedef struct {
	Result rst;
	unsigned char MaxPower; // 最大支持功率
	unsigned char MinPower; //最小支持功率
	unsigned char AntennaCount; //天线数量
	int FrequencyArraySize;
	unsigned char FrequencyArray[8]; //支持的频段列表，
	int ProtocolArraySize;
	unsigned char ProtocolArray[8]; //支持的协议列表，
}MsgBaseGetCapabilities;
#endif

#if REGION("配置网络参数")
typedef struct {
	Result rst;
	unsigned char autoIp;
	char ip[32];
	char mask[32];
	char gateway[32];
	char dns1[32];
	char dns2[32];
}MsgAppSetEthernetIP;

typedef struct {
	Result rst;
	unsigned char autoIp;
	char ip[32];
	char mask[32];
	char gateway[32];
	char dns1[32];
	char dns2[32];
}MsgAppGetEthernetIP;
#endif

#if REGION("查询读写器以太网MAC")
typedef struct {
	Result rst;
	char mac[32];
}MsgAppGetEthernetMac;
#endif

#if REGION("配置服务器/客户端模式参数")
typedef struct {
	Result rst;
    unsigned char tcpMode;
    unsigned short serverPort;
    char clientIp[32];
    unsigned short clientPort;
}MsgAppSetTcpMode;

typedef struct {
	Result rst;
    unsigned char tcpMode;
    unsigned short serverPort;
    char clientIp[32];
    unsigned short clientPort;
}MsgAppGetTcpMode;
#endif

#if REGION("配置与查询读写器功率")
typedef struct {
	unsigned char AntennaNo;
	unsigned char power;
}Dictionary;

typedef struct {
	Result rst;
	Dictionary DicPower[128];
	unsigned char dicCount;
}MsgBaseSetPower;

typedef struct {
	Result rst;
	Dictionary DicPower[128];
	unsigned char dicCount;
}MsgBaseGetPower;

#endif

#if REGION("配置读写器RF频段")
typedef struct {
	Result rst;
	unsigned char FreqRangeIndex;
}MsgBaseSetFreqRange;

typedef struct {
	Result rst;
	unsigned char FreqRangeIndex;
}MsgBaseGetFreqRange;
#endif

#if REGION("配置和查询EPC基带参数")
typedef struct {
	Result rst;

	char setBaseSpeedFlag;
	unsigned char BaseSpeed; // EPC 基带速率（可选）。

	char setQValueFlag;
	unsigned char QValue; // 默认Q 值（可选）(0~15)。

	char setSessionFlag;
	unsigned char Session; // （可选）(0,Session0; 1,Session1; 2,Session2; 3,Session3)。

	char setInventoryFlagFlag;
	unsigned char InventoryFlag; // 盘存标志参数（可选）(0,仅用Flag A 盘存;1,仅用Flag B 盘存;2,轮流使用Flag	A 和Flag B)。 
}MsgBaseSetBaseband;

typedef struct {
	Result rst;

	char setBaseSpeedFlag;
	unsigned char BaseSpeed; // EPC 基带速率（可选）。

	char setQValueFlag;
	unsigned char QValue; // 默认Q 值（可选）(0~15)。

	char setSessionFlag;
	unsigned char Session; // （可选）(0,Session0; 1,Session1; 2,Session2; 3,Session3)。

	char setInventoryFlagFlag;
	unsigned char InventoryFlag; // 盘存标志参数（可选）(0,仅用Flag A 盘存;1,仅用Flag B 盘存;2,轮流使用Flag	A 和Flag B)。 
}MsgBaseGetBaseband;

#endif

#if REGION("设置标签上传参数")
typedef struct {
	Result rst;
	
	char setRepeatedTimeFlag;
	unsigned short RepeatedTime; //  重复标签过滤时间（可选）（表示在一个读卡指令执行周期内，在指定的重复过滤时间内相同的标签内容只上传一次，0~65535，时间单位：10ms）。

	char setRssiTVFlag;
	unsigned char RssiTV; //RSSI 阈值（可选）（标签RSSI 值低于阈值时标签数据将不上传并丢弃）。
}MsgBaseSetTagLog;

typedef struct {
	Result rst;
	
	char setRepeatedTimeFlag;
	unsigned short RepeatedTime; //  重复标签过滤时间（可选）（表示在一个读卡指令执行周期内，在指定的重复过滤时间内相同的标签内容只上传一次，0~65535，时间单位：10ms）。

	char setRssiTVFlag;
	unsigned char RssiTV; //RSSI 阈值（可选）（标签RSSI 值低于阈值时标签数据将不上传并丢弃）。
}MsgBaseGetTagLog;
#endif 

#if REGION("6B操作")
typedef struct {
	Result rst;
	unsigned short Start;
    unsigned char len;
}Param6bReadUserdata;

typedef struct {
	Result rst;
	int antennaEnable;
	char inventoryMode;
	char area;
	Param6bReadUserdata readUserdata;
	char hexMatchTid[128];
}MsgBaseInventory6b;

typedef struct {
	Result rst;
	int antennaEnable;
	char hexMatchTid[128]; //十六进制字符串
	int start;
	char HexWriteData[128]; // 十六进制字符串
}MsgBaseWrite6b;

typedef struct {
	Result rst;
	int antennaEnable;
	char hexMatchTid[128]; //十六进制字符串
	int lockIndex;
}MsgBaseLock6b;

typedef struct {
	Result rst;
	int antennaEnable;
	char hexMatchTid[128]; //十六进制字符串
	int lockIndex;
}MsgBaseLock6bGet;
#endif 

#if REGION("国标标签")

typedef struct {
	Result rst;
	int antennaEnable;
	char inventoryMode;
	EpcFilter filter;
	EpcReadTid readTid;
	EpcReadUserdata readUserdata;
    char strHexPassword[9];
}MsgBaseInventoryGb;

#endif 

#ifdef __cplusplus
}
#endif

#endif // MESSAGE_H
