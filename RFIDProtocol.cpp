#include "RFIDProtocol.h"
#include "pub.h"
#include "XXRFID.h"
#include <string.h>
#include "message.h"
#include "delegate.h"
#include "result.h"
#include <stdio.h>
#include <errno.h>

#ifdef __cplusplus
    extern "C" {
#endif

void FillAnts(unsigned char* buf, unsigned int Ant)
{
    buf[3] = Ant & 0xFF;
    buf[2] = (Ant >> 8) & 0xFF;
    buf[1] = (Ant >> 16) & 0xFF;
    buf[0] = (Ant >> 24) & 0xFF;
}

int FillFilter(unsigned char* buf, EpcFilter filter)
{
	if(filter.bitLen == 0) {
		return -1;
	}

	buf[0] = 0x01;

	int len = filter.bitLen/8 + 4;
	buf[1] = (len>>8) & 0xFF;
	buf[2] = len & 0xFF;

	buf[3] = filter.Area;
	buf[4] = (filter.Start >> 8) & 0xFF;
	buf[5] = filter.Start & 0xFF;

	buf[6] = filter.bitLen;
	memcpy(&buf[7], filter.HexData, filter.bitLen / 8);

	return 7+filter.bitLen/8;
}

int FillReadTID(unsigned char* buf, EpcReadTid tid)
{
	if(tid.len == 0) {
		return -1;
	}

	buf[0] = 0x02;
	buf[1] = tid.Mode;
	buf[2] = tid.len;

	return 0;
}

int FillReadUserData(unsigned char* buf, EpcReadUserdata userData)
{
	if(userData.len == 0) {
		return -1;
	}

	buf[0] = 0x03;
	buf[1] = (userData.Start>>8) & 0xFF;
	buf[2] = userData.Start & 0xFF;
	buf[3] = userData.len;

	return 0;
}

int FillResverData(unsigned char* buf, EpcReadReserved reserved)
{
	if(reserved.len == 0) {
		return -1;
	}

	buf[0] = 0x04;
	buf[1] = (reserved.Start>>8) & 0xFF;
	buf[2] = reserved.Start & 0xFF;
	buf[3] = reserved.len;

	return 0;
}

int FillPasswd(unsigned char* buf, char* passwd, unsigned char PID)
{
	if(strlen(passwd)<8) {
		return -1;
	}

	buf[0] = PID;

   	char pa[4] = {0};
	int len;
	char* tpasswd = StringToHex(passwd, &len);
    memcpy(pa, tpasswd, len>4?4:len);
	delete []tpasswd;

	buf[1] = pa[0];
	buf[2] = pa[1];
	buf[3] = pa[2];
	buf[4] = pa[3];

	return 0;
}

void getConnectStateCmd(unsigned char* buf, int* len, unsigned char* rcv)
{
    *len = 0;
    unsigned char FrameHead = 0x5A;
    unsigned int Protocol = 0x00011112;
    unsigned short DataLen = 0;

    buf[0] = FrameHead;
    *len += sizeof(FrameHead);

    FillProtocol(&buf[1], Protocol);
    *len += sizeof(Protocol);

    *len += 2;  //占位数据长度

    memcpy(&buf[*len], &rcv[2], 4);
    *len += 4;
    DataLen += 4;

    FillDataLen(&buf[5], DataLen);

    FillCrC(&buf[1], *len-1);
    *len += 2;
}

int Stop(XXRFIDCLient* s, unsigned char* buf, MsgBaseStop* msg)
{
	unsigned char FrameHead = 0x5A;
    unsigned int Protocol = 0x000102FF;
	unsigned short dataLen = 0, len = 0;

	if (msg != NULL) {
		s->result[EMESS_Stop].rst = (void*)msg;
	}

	buf[0] = FrameHead;
    len += sizeof(FrameHead);

	FillProtocol(&buf[len], Protocol);
    len += sizeof(Protocol);

	len += 2;

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcStop(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_Stop].rst != NULL) {
		((MsgBaseStop*)(s->result[EMESS_Stop].rst))->rst.RtCode = buf[index];

		if (buf[index] == 1) {
			strcpy(((MsgBaseStop*)(s->result[EMESS_Stop].rst))->rst.RtMsg, "系统错误");
		}
	} else {
		char tmp[128] = {0};
		sprintf(tmp, "s->result[EMESS_Stop].rst == NULL");
		LOG_TICK(tmp);
	}

	int ret = sem_post(&s->sem[EMESS_Stop]);
	if (ret != 0) {
		char tmp[128] = {0};
		sprintf(tmp, "failed to set_post : %d", errno);
		LOG_TICK(tmp);
	}
}

int InventoryEpc(XXRFIDCLient* s, unsigned char* buf, MsgBaseInventoryEpc* in)
{
    unsigned char FrameHead = 0x5A;
    unsigned int Protocol = 0x00010210;
    unsigned short dataLen = 0, len = 0;
    unsigned char readMode = in->InventoryMode; //0, danci;1,continue

	if (in == NULL || s == NULL) {
		return -1;
	}

	s->result[EMESS_InventoryEpc].rst = (void*)in;

    buf[0] = FrameHead;
    len += sizeof(FrameHead);

    FillProtocol(&buf[len], Protocol);
    len += sizeof(Protocol);

    len += 2; //占位数据长度

	FillAnts(&buf[len], in->AntennaEnable);
	len += sizeof(in->AntennaEnable);
	dataLen += sizeof(in->AntennaEnable);

	buf[len] = readMode;
	len += sizeof(readMode);
	dataLen += sizeof(readMode);

	int ret = FillFilter(&buf[len], in->Filter);
	if(ret != -1) {
		len += ret;
		dataLen += ret;
	}

	ret = FillReadTID(&buf[len], in->ReadTid);
	if(ret != -1) {
		len += 3;
		dataLen += 3;
	}

	ret = FillReadUserData(&buf[len], in->ReadUserdata);
	if(ret != -1) {
		len += 4;
		dataLen += 4;
	}

	ret = FillResverData(&buf[len], in->ReadReserved);
	if(ret != -1) {
		len += 4;
		dataLen += 4;
	}

	ret = FillPasswd(&buf[len], in->StrHexPassword, 0x05);
	if(ret != -1) {
		len += 5;
		dataLen += 5;
	}

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcInventoryEpc(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_InventoryEpc].rst != NULL) {

		((MsgBaseStop*)(s->result[EMESS_InventoryEpc].rst))->rst.RtCode = buf[index];

		switch(buf[index]) {
			case 0:
				break;

			case 1:
				strcpy(((MsgBaseStop*)(s->result[EMESS_InventoryEpc].rst))->rst.RtMsg, "天线端口参数错误");
				break;

			case 2:
				strcpy(((MsgBaseStop*)(s->result[EMESS_InventoryEpc].rst))->rst.RtMsg, "选择读取参数错误");
				break;

			case 3:
				strcpy(((MsgBaseStop*)(s->result[EMESS_InventoryEpc].rst))->rst.RtMsg, "TID读取参数错误");
				break;

			case 4:
				strcpy(((MsgBaseStop*)(s->result[EMESS_InventoryEpc].rst))->rst.RtMsg, "用户数据区读取参数错误");
				break;

			case 5:
				strcpy(((MsgBaseStop*)(s->result[EMESS_InventoryEpc].rst))->rst.RtMsg, "保留区读取参数错误");
				break;

			case 6:
				strcpy(((MsgBaseStop*)(s->result[EMESS_InventoryEpc].rst))->rst.RtMsg, "其他参数错误");
				break;
		}
	}

	sem_post(&s->sem[EMESS_InventoryEpc]);
}

int FillWriteEpcData(unsigned char* buf, char area, unsigned short start, char* data)
{
	buf[0] = (start >> 8) & 0xFF; //起始地址
	buf[1] = start & 0xFF;

	int index = 2;
	char hbuf[128] = {0};

    int len = 0;
    char* repc = StringToHex(data, &len);
    memcpy(hbuf, repc, len);
    delete []repc;

    if(len%2 != 0) {
        len += 1;
    }

	if(area == 0x01) { //写入EPC时需要前面两个字节的PC马
		unsigned short tlen = len+2; //2为PC的长度
		buf[index++] = (tlen>>8) & 0xFF;  //可变参数长度
		buf[index++] = tlen&0xFF;

		unsigned short pclen = 0;
		pclen = (len/2)<<11;
		buf[index++] = (pclen>>8) & 0xFF;  //PC值,前5bit代表写入数据字长度
		buf[index++] = 0x00;
	} else { //TID和USER区
		buf[index++] = (len>>8) & 0xFF;
		buf[index++] = len&0xFF;
	}

    memcpy(&buf[index], hbuf, len);

	if(area == 0x01) {
		len += 2;
	}

    return len+4;
}

int WriteEpc(XXRFIDCLient* s, unsigned char* buf, MsgBaseWriteEpc* in)
{
	unsigned char FrameHead = 0x5A;
    unsigned int Protocol = 0x00010211;
    unsigned short dataLen = 0, len = 0;
    unsigned int Ant = in->AntennaEnable;

	if (in == NULL) {
		return -1;
	}

	s->result[EMESS_WriteEpc].rst = (void*)in;

	buf[0] = FrameHead;
    len += sizeof(FrameHead);

    FillProtocol(&buf[len], Protocol);
    len += sizeof(Protocol);

	len += 2; //占位数据长度

	FillAnts(&buf[len], Ant);
	len += sizeof(Ant);
	dataLen += sizeof(Ant);

	buf[len] = in->Area;
	len += 1;
	dataLen += 1;

	int ret = FillWriteEpcData(&buf[len], in->Area, in->Start, in->HexStrWriteData);
	if (ret != -1) {
		len += ret;
		dataLen += ret;
	}

	ret = FillFilter(&buf[len], in->Filter);
	if (ret != -1) {
		len += ret;
		dataLen += ret;
	}

	ret = FillPasswd(&buf[len], in->StrHexPassword, 0x02);
	if (ret != -1) {
		len += 5;
		dataLen += 5;
	}

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcWriteEpc(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_WriteEpc].rst != NULL) {
		((MsgBaseWriteEpc*)(s->result[EMESS_WriteEpc].rst))->rst.RtCode = buf[index];

		switch(buf[index]) {
			case 0:
				break;

			case 1:
				strcpy(((MsgBaseWriteEpc*)(s->result[EMESS_WriteEpc].rst))->rst.RtMsg, "天线端口参数错误");
				break;

			case 2:
				strcpy(((MsgBaseWriteEpc*)(s->result[EMESS_WriteEpc].rst))->rst.RtMsg, "选择参数错误");
				break;

			case 3:
				strcpy(((MsgBaseWriteEpc*)(s->result[EMESS_WriteEpc].rst))->rst.RtMsg, "写入参数错误");
				break;

			case 4:
				strcpy(((MsgBaseWriteEpc*)(s->result[EMESS_WriteEpc].rst))->rst.RtMsg, "CRC校验错误");
				break;

			case 5:
				strcpy(((MsgBaseWriteEpc*)(s->result[EMESS_WriteEpc].rst))->rst.RtMsg, "功率不足");
				break;

			case 6:
				strcpy(((MsgBaseWriteEpc*)(s->result[EMESS_WriteEpc].rst))->rst.RtMsg, "数据区溢出");
				break;

			case 7:
				strcpy(((MsgBaseWriteEpc*)(s->result[EMESS_WriteEpc].rst))->rst.RtMsg, "数据区被锁定");
				break;

			case 8:
				strcpy(((MsgBaseWriteEpc*)(s->result[EMESS_WriteEpc].rst))->rst.RtMsg, "访问密码错误");
				break;

			case 9:
				strcpy(((MsgBaseWriteEpc*)(s->result[EMESS_WriteEpc].rst))->rst.RtMsg, "其他标签错误");
				break;

			case 10:
				strcpy(((MsgBaseWriteEpc*)(s->result[EMESS_WriteEpc].rst))->rst.RtMsg, "标签丢失");
				break;

			case 11:
				strcpy(((MsgBaseWriteEpc*)(s->result[EMESS_WriteEpc].rst))->rst.RtMsg, "读写器发送指令错误");
				break;

		}
	}

	bool ret = sem_post(&s->sem[EMESS_WriteEpc]);
	if (ret != true) {
		char tmp[128] = {0};
		sprintf(tmp, "failed to SetEvent : %d", errno);
		LOG_TICK(tmp);
	}

	LOG_TICK("");
}

int LockEpc(XXRFIDCLient* s, unsigned char* buf, MsgBaseLockEpc* in)
{
	unsigned char FrameHead = 0x5A;
    unsigned int Protocol = 0x00010212;
    unsigned short dataLen = 0, len = 0;
    unsigned int Ant = in->AntennaEnable;

	if (in == NULL) {
		return -1;
	}

	s->result[EMESS_LockEpc].rst = (void*)in;

	buf[0] = FrameHead;
    len += sizeof(FrameHead);

    FillProtocol(&buf[len], Protocol);
    len += sizeof(Protocol);

	len += 2; //占位数据长度

	FillAnts(&buf[len], Ant);
	len += sizeof(Ant);
	dataLen += sizeof(Ant);

	buf[len] = in->Area;
	len += 1;
	dataLen += 1;

	buf[len] = in->Mode;
	len += 1;
	dataLen += 1;

	int ret = FillFilter(&buf[len], in->Filter);
	if(ret != -1) {
		len += ret;
		dataLen += ret;
	}

	ret = FillPasswd(&buf[len], in->StrHexPassword, 0x02);
	if(ret != -1) {
		len += 5;
		dataLen += 5;
	}

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcLockEpc(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_LockEpc].rst != NULL) {
		((MsgBaseStop*)(s->result[EMESS_LockEpc].rst))->rst.RtCode = buf[index];

		switch(buf[index]) {
			case 0:
				break;

			case 1:
				strcpy(((MsgBaseStop*)(s->result[EMESS_LockEpc].rst))->rst.RtMsg, "天线端口参数错误");
				break;

			case 2:
				strcpy(((MsgBaseStop*)(s->result[EMESS_LockEpc].rst))->rst.RtMsg, "选择参数错误");
				break;

			case 3:
				strcpy(((MsgBaseStop*)(s->result[EMESS_LockEpc].rst))->rst.RtMsg, "写入参数错误");
				break;

			case 4:
				strcpy(((MsgBaseStop*)(s->result[EMESS_LockEpc].rst))->rst.RtMsg, "CRC校验错误");
				break;

			case 5:
				strcpy(((MsgBaseStop*)(s->result[EMESS_LockEpc].rst))->rst.RtMsg, "功率不足");
				break;

			case 6:
				strcpy(((MsgBaseStop*)(s->result[EMESS_LockEpc].rst))->rst.RtMsg, "数据区溢出");
				break;

			case 7:
				strcpy(((MsgBaseStop*)(s->result[EMESS_LockEpc].rst))->rst.RtMsg, "数据区被锁定");
				break;

			case 8:
				strcpy(((MsgBaseStop*)(s->result[EMESS_LockEpc].rst))->rst.RtMsg, "访问密码错误");
				break;

			case 9:
				strcpy(((MsgBaseStop*)(s->result[EMESS_LockEpc].rst))->rst.RtMsg, "其他标签错误");
				break;

			case 10:
				strcpy(((MsgBaseStop*)(s->result[EMESS_LockEpc].rst))->rst.RtMsg, "标签丢失");
				break;

			case 11:
				strcpy(((MsgBaseStop*)(s->result[EMESS_LockEpc].rst))->rst.RtMsg, "读写器发送指令错误");
				break;

		}
	}

	sem_post(&s->sem[EMESS_LockEpc]);
}

int FillDestoryEpcPasswd(unsigned char* buf, char* passwd)
{
	if(strlen(passwd)<8) {
		return -1;
	}

   	char pa[4] = {0};
	int len;
	char* tpasswd = StringToHex(passwd, &len);
    memcpy(pa, tpasswd, len>4?4:len);
	delete []tpasswd;

	buf[0] = tpasswd[3];
	buf[1] = tpasswd[2];
	buf[2] = tpasswd[1];
	buf[3] = tpasswd[0];

	return 0;
}

int DestoryEpc(XXRFIDCLient* s, unsigned char* buf, MsgBaseDestoryEpc* in)
{
	unsigned char FrameHead = 0x5A;
    unsigned int Protocol = 0x00010213;
    unsigned short dataLen = 0, len = 0;
    unsigned int Ant = in->AntennaEnable;

	if (in == NULL) {
		return -1;
	}

	s->result[EMESS_DestoryEpc].rst = in;

	buf[0] = FrameHead;
    len += sizeof(FrameHead);

    FillProtocol(&buf[len], Protocol);
    len += sizeof(Protocol);

	len += 2; //占位数据长度

	FillAnts(&buf[len], Ant);
	len += sizeof(Ant);
	dataLen += sizeof(Ant);

	int ret = FillDestoryEpcPasswd(&buf[len], in->StrHexPassword);
	if(ret != -1) {
		len += 4;
		dataLen += 4;
	}

	ret = FillFilter(&buf[len], in->Filter);
	if(ret != -1) {
		len += ret;
		dataLen += ret;
	}

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcDestoryEpc(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_DestoryEpc].rst != NULL) {
		((MsgBaseStop*)(s->result[EMESS_DestoryEpc].rst))->rst.RtCode = buf[index];

		switch(buf[index]) {
			case 0:
				break;

			case 1:
				strcpy(((MsgBaseStop*)(s->result[EMESS_DestoryEpc].rst))->rst.RtMsg, "天线端口参数错误");
				break;

			case 2:
				strcpy(((MsgBaseStop*)(s->result[EMESS_DestoryEpc].rst))->rst.RtMsg, "选择参数错误");
				break;

			case 3:
				strcpy(((MsgBaseStop*)(s->result[EMESS_DestoryEpc].rst))->rst.RtMsg, "CRC校验错误");
				break;

			case 4:
				strcpy(((MsgBaseStop*)(s->result[EMESS_DestoryEpc].rst))->rst.RtMsg, "功率不足");
				break;

			case 5:
				strcpy(((MsgBaseStop*)(s->result[EMESS_DestoryEpc].rst))->rst.RtMsg, "灭活密码错误");
				break;

			case 6:
				strcpy(((MsgBaseStop*)(s->result[EMESS_DestoryEpc].rst))->rst.RtMsg, "其他标签错误");
				break;

			case 7:
				strcpy(((MsgBaseStop*)(s->result[EMESS_DestoryEpc].rst))->rst.RtMsg, "标签丢失");
				break;

			case 8:
				strcpy(((MsgBaseStop*)(s->result[EMESS_DestoryEpc].rst))->rst.RtMsg, "读写器发送指令错误");
				break;

		}
	}

	sem_post(&s->sem[EMESS_DestoryEpc]);
}

void ProcDataResultCmd(XXRFIDCLient* s, unsigned char* buf)
{
	try {
		LogBaseEpcInfo msg;
		int index = 0;
		int len = (buf[0]<<8) | buf[1];
		int epcLen = (buf[2]<<8) | buf[3];
		index += 4;

		memset(&msg, 0, sizeof(msg));

		char* tmp = HexToString(&buf[index], epcLen);
		memcpy(msg.Epc, tmp, strlen(tmp));
		delete [] tmp;
		index += epcLen;

		tmp = HexToString(&buf[index], 2);
		memcpy(msg.Pc, tmp, strlen(tmp));
		delete [] tmp;
		index += 2;

		msg.AntId = buf[index];
		index += 1;

		LOG_TICK("");

		while(len >= index) {
			switch(buf[index]) {
				case 0x01:
        			{
	             		msg.Rssi = buf[index+1];
						index += 1;
            		}
					break;

				case 0x02:
            		{
						msg.Result = buf[index+1];
						index += 1;
            		}
					break;

				case 0x03:
					{
						LOG_TICK("");
						int tidLen = (buf[index+1]<<8) | buf[index+2];
						tmp = HexToString(&buf[index+3], tidLen);
						memcpy(msg.Tid, tmp, strlen(tmp));
						delete [] tmp;
						index += tidLen+2;
						LOG_TICK("");
					}
					break;

				case 0x04:
					{
						LOG_TICK("");
						int userLen = (buf[index+1]<<8) | buf[index+2];
						tmp = HexToString(&buf[index+3], userLen);
						memcpy(msg.Userdata, tmp, strlen(tmp));
						delete [] tmp;
						index += userLen+2;
						LOG_TICK("");
					}
					break;

				case 0x05:
					{
						LOG_TICK("");
						int ReserveLen = (buf[index+1]<<8) | buf[index+2];
						tmp = HexToString(&buf[index+3], ReserveLen);
						memcpy(msg.Reserved, tmp, strlen(tmp));
						delete [] tmp;
						index += ReserveLen+2;
						LOG_TICK("");
					}

				case 0x06:
					index += 1;
					break;

				case 0x07:
					{
						index += 8;
					}
					break;

				case 0x08:
					{
						index += 4;
					}
					break;

				case 0x09:
					{
						index += 1;
					}
					break;

				default:
					break;

			}

			index += 1; //PID+1
		}

		if(s != NULL && s->call_TagEpcLog != NULL) {
			LOG_TICK("");
			s->call_TagEpcLog(msg);
			LOG_TICK("");
		}
	} catch (...) {
		LOG_TICK("ProcDataResultCmd exception");
	}
}

void ProcReadFinish(XXRFIDCLient* s)
{
    if(s->call_TagEpcOver != NULL) {
		s->call_TagEpcOver();
	}
}

int GetCapabilities(XXRFIDCLient* s, unsigned char* buf, MsgBaseGetCapabilities* msg)
{
	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetCapabilities].rst = (void*)msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_GETRFIDCAPATICY);
	FillProtocol(&buf[len], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcGetCapabilities(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_GetCapabilities].rst != NULL) {
		MsgBaseGetCapabilities* msg = ((MsgBaseGetCapabilities*)(s->result[EMESS_GetCapabilities].rst));

		msg->MinPower = buf[index++];
		msg->MaxPower = buf[index++];
		msg->AntennaCount = buf[index++];

		int frequencyLen = (buf[index++] << 8) | buf[index++];
		msg->FrequencyArraySize = frequencyLen;
		for (int i = 0; i < frequencyLen; i++) {
			msg->FrequencyArray[i] = buf[index++];
		}

		int protocolLen = (buf[index++] << 8) | buf[index++];
		msg->ProtocolArraySize = protocolLen;
		for (int i = 0; i < protocolLen; i++) {
			msg->ProtocolArray[i] = buf[index++];
		}
	}

	sem_post(&s->sem[EMESS_GetCapabilities]);
}

int SetPower(XXRFIDCLient* s, unsigned char* buf, MsgBaseSetPower* msg)
{
	if (msg == NULL) {
		LOG_TICK("msg == NULL");
		return -1;
	}

	if (msg->dicCount == 0 || msg->DicPower == NULL) {
		LOG_TICK("dicCount == 0 || msg->DicPower == NULL");
		return -1;
	}

	s->result[EMESS_SetPower].rst = (void*)msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_SETPOWER);
	FillProtocol(&buf[len], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	for (int i = 0; i < msg->dicCount; i++) {
		buf[len++] = msg->DicPower[i].AntennaNo;
		buf[len++] = msg->DicPower[i].power;

		dataLen += 2;
	}

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcSetPower(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_SetPower].rst != NULL) {
		MsgBaseSetPower* msg = ((MsgBaseSetPower*)(s->result[EMESS_SetPower].rst));

		msg->rst.RtCode = buf[index];

		switch(buf[index]) {
			case 0:
				break;

			case 1:
				strcpy(msg->rst.RtMsg, "端口参数读写器硬件不支持");
				break;

			case 2:
				strcpy(msg->rst.RtMsg, "功率参数读写器硬件不支持");
				break;

			case 3:
				strcpy(msg->rst.RtMsg, "保存失败");
				break;
		}
	}

	sem_post(&s->sem[EMESS_SetPower]);
}

int GetPower(XXRFIDCLient* s, unsigned char* buf, MsgBaseGetPower* msg)
{
	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetPower].rst = (void*)msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_GETPOWER);
	FillProtocol(&buf[len], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcGetPower(XXRFIDCLient* s, unsigned char* buf)
{
	if (s->result[EMESS_GetPower].rst == NULL) {
		return ;
	}

	MsgBaseGetPower* msg = ((MsgBaseGetPower*)(s->result[EMESS_GetPower].rst));

	int index = 2;
	int len = (buf[0] << 8) | buf[1];

	while (len > index-2) {
		msg->DicPower[msg->dicCount].AntennaNo = buf[index++];
		msg->DicPower[msg->dicCount].power = buf[index++];
		msg->dicCount++;
	}

	sem_post(&s->sem[EMESS_GetPower]);
}

int SetFreqRange(XXRFIDCLient* s, unsigned char* buf, MsgBaseSetFreqRange* msg)
{
	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_SetFreqRange].rst = (void*)msg;

	unsigned short dataLen = 0, len = 0;
	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_SETRFBAUD);
	FillProtocol(&buf[len], head);
	len += sizeof(head);
	len += 2; //长度字节占位

	buf[len++] = msg->FreqRangeIndex;
	dataLen += 1;

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcSetFreqRange(XXRFIDCLient* s, unsigned char* buf)
{
	if (s->result[EMESS_SetFreqRange].rst == NULL) {
		return ;
	}

	int index = 2;
	MsgBaseSetFreqRange* msg = (MsgBaseSetFreqRange*)(s->result[EMESS_SetFreqRange].rst);

	msg->rst.RtCode = buf[index];

	switch(buf[index]) {
		case 1:
			strcpy(msg->rst.RtMsg, "频段参数读写器硬件不支持");
			break;

		case 2:
			strcpy(msg->rst.RtMsg, "保存失败");
			break;
	}

	sem_post(&s->sem[EMESS_SetFreqRange]);
}

int GetFreqRange(XXRFIDCLient* s, unsigned char* buf, MsgBaseGetFreqRange* msg)
{
	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetFreqRange].rst = (void*)msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_GETRFBAUD);
	FillProtocol(&buf[len], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcGetFreqRange(XXRFIDCLient* s, unsigned char* buf)
{
	if (s->result[EMESS_GetFreqRange].rst == NULL) {
		return ;
	}

	int index = 2;
	MsgBaseGetFreqRange* msg = (MsgBaseGetFreqRange*)(s->result[EMESS_GetFreqRange].rst);

	msg->FreqRangeIndex = buf[index];

	sem_post(&s->sem[EMESS_GetFreqRange]);
}

int SetBaseband(XXRFIDCLient* s, unsigned char* buf, MsgBaseSetBaseband* msg)
{
	if (msg == NULL) {
		LOG_TICK("msg == NULL");
		return -1;
	}

	s->result[EMESS_SetBaseband].rst = (void*)msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_SETEPCBASEPARA);
	FillProtocol(&buf[len], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	if (msg->setBaseSpeedFlag != 0) {
		buf[len++] = 0x01;
		buf[len++] = msg->BaseSpeed;
		dataLen += 2;
	}

	if (msg->setQValueFlag != 0) {
		buf[len++] = 0x02;
		buf[len++] = msg->QValue;
		dataLen += 2;
	}

	if (msg->setSessionFlag != 0) {
		buf[len++] = 0x03;
		buf[len++] = msg->Session;
		dataLen += 2;
	}

	if (msg->setInventoryFlagFlag != 0) {
		buf[len++] = 0x04;
		buf[len++] = msg->InventoryFlag;
		dataLen += 2;
	}

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcSetBaseband(XXRFIDCLient* s, unsigned char* buf)
{
	if (s->result[EMESS_SetBaseband].rst == NULL) {
		return ;
	}

	int index = 2;
	MsgBaseSetBaseband* msg = (MsgBaseSetBaseband*)(s->result[EMESS_SetBaseband].rst);

	msg->rst.RtCode = buf[index];

	switch(buf[index]) {
		case 1:
			strcpy(msg->rst.RtMsg, "读写器不支持的基带速率");
			break;

		case 2:
			strcpy(msg->rst.RtMsg, "Q值参数错误");
			break;

		case 3:
			strcpy(msg->rst.RtMsg, "Session参数错误");
			break;

		case 4:
			strcpy(msg->rst.RtMsg, "盘存参数错误");
			break;

		case 5:
			strcpy(msg->rst.RtMsg, "其他参数错误");
			break;

		case 6:
			strcpy(msg->rst.RtMsg, "保存失败");
			break;
	}

	sem_post(&s->sem[EMESS_SetBaseband]);
}

int GetBaseband(XXRFIDCLient* s, unsigned char* buf, MsgBaseGetBaseband* msg)
{
	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetBaseband].rst = (void*)msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_GETEPCBASEPARA);
	FillProtocol(&buf[len], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcGetBaseband(XXRFIDCLient* s, unsigned char* buf)
{
	if (s->result[EMESS_GetBaseband].rst == NULL) {
		return ;
	}

	int index = 2;
	MsgBaseGetBaseband* msg = (MsgBaseGetBaseband*)(s->result[EMESS_GetBaseband].rst);

	msg->BaseSpeed = buf[index++];
	msg->QValue = buf[index++];
	msg->Session = buf[index++];
	msg->InventoryFlag = buf[index++];

	sem_post(&s->sem[EMESS_GetBaseband]);
}

int SetTagLog(XXRFIDCLient* s, unsigned char* buf, MsgBaseSetTagLog* msg)
{
	if (msg == NULL) {
		LOG_TICK("msg == NULL");
		return -1;
	}

	s->result[EMESS_SetTagLog].rst = (MsgBaseSetTagLog*)msg;

	unsigned short dataLen = 0, len = 0;
	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_SETTAGREPORTPARA);
	FillProtocol(&buf[len], head);
	len += sizeof(head);
	len += 2; //长度字节占位

	if (msg->setRepeatedTimeFlag != 0) {
		buf[len++] = 0x01;
		buf[len++] = (msg->RepeatedTime >> 8) & 0xFF;
		buf[len++] = msg->RepeatedTime & 0xFF;
		dataLen += 3;
	}

	if (msg->setRssiTVFlag != 0) {
		buf[len++] = 0x2;
		buf[len++] = msg->RssiTV;
		dataLen += 2;
	}

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcSetTagLog(XXRFIDCLient* s, unsigned char *buf)
{
	if (s->result[EMESS_SetTagLog].rst == NULL) {
		return ;
	}

	int index = 2;
	MsgBaseSetTagLog* msg = (MsgBaseSetTagLog*)(s->result[EMESS_SetTagLog].rst);

	msg->rst.RtCode = buf[index];

	switch(buf[index]) {
		case 1:
			strcpy(msg->rst.RtMsg, "参数错误");
			break;

		case 2:
			strcpy(msg->rst.RtMsg, "保存失败");
			break;
	}

	sem_post(&s->sem[EMESS_SetTagLog]);
}

int GetTagLog(XXRFIDCLient* s, unsigned char *buf, MsgBaseGetTagLog* msg)
{
	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetTagLog].rst = (MsgBaseGetTagLog*)msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_GETTAGREPORTPARA);
	FillProtocol(&buf[len], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcGetTagLog(XXRFIDCLient* s, unsigned char* buf)
{
	if (s->result[EMESS_GetTagLog].rst == NULL) {
		return ;
	}

	int index = 2;
	MsgBaseGetTagLog* msg = (MsgBaseGetTagLog*)(s->result[EMESS_GetTagLog].rst);

	msg->RepeatedTime = (buf[index++] << 8) | buf[index++];
	msg->RssiTV = buf[index++];

	sem_post(&s->sem[EMESS_GetTagLog]);
}

#if REGION("6b")
int Inventory6b(XXRFIDCLient* s, unsigned char *buf, MsgBaseInventory6b *msg)
{
	if (s == NULL) {
		return -1;
	}

	s->result[EMESS_Inventory6b].rst = (void*)msg;
	
	unsigned short dataLen = 0, len = 0;
	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_READ6B);
	FillProtocol(&buf[len], head);
	len += sizeof(head);
	len += 2; //长度字节占位
	
	FillAnts(&buf[len], msg->antennaEnable);
	len += sizeof(msg->antennaEnable);
	dataLen += sizeof(msg->antennaEnable);

	buf[len] = msg->inventoryMode;
	len += sizeof(msg->inventoryMode);
	dataLen += sizeof(msg->inventoryMode);

	buf[len] = msg->area;
	len += sizeof(msg->area);
	dataLen += sizeof(msg->area);

	if (msg->readUserdata.len > 0) {
		buf[len++] = 0x01;
		buf[len++] = msg->readUserdata.Start;
		buf[len++] = msg->readUserdata.len;
		dataLen += 3;
	}

	if (strlen(msg->hexMatchTid) > 0) {
		buf[len++] = 0x02;
		int tlen = 0;
		char *tmp = StringToHex(msg->hexMatchTid, &tlen);
		memcpy(&buf[len], tmp, tlen);
		delete []tmp;
		len += 8;
		dataLen += 9;
	}

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcInventory6b(XXRFIDCLient* s, unsigned char* buf)
{
	if (s == NULL) {
		return;
	}

	int index = 2;

	if (s->result[EMESS_Inventory6b].rst != NULL) {

		((MsgBaseStop*)(s->result[EMESS_Inventory6b].rst))->rst.RtCode = buf[index];

		switch(buf[index]) {
			case 0:
				break;

			case 1:
				strcpy(((MsgBaseStop*)(s->result[EMESS_Inventory6b].rst))->rst.RtMsg, "天线端口参数错误");
				break;

			case 2:
				strcpy(((MsgBaseStop*)(s->result[EMESS_Inventory6b].rst))->rst.RtMsg, "读取内容参数错误");
				break;

			case 3:
				strcpy(((MsgBaseStop*)(s->result[EMESS_Inventory6b].rst))->rst.RtMsg, "用户数据区读取参数错误");
				break;

			case 4:
				strcpy(((MsgBaseStop*)(s->result[EMESS_Inventory6b].rst))->rst.RtMsg, "其他参数错误");
				break;

			default:
				break;
		}
	}

	sem_post(&s->sem[EMESS_Inventory6b]);
}

void Proc6bDataResultCmd(XXRFIDCLient* s, unsigned char* buf)
{
	if (s == NULL) {
		return;
	}

	LogBase6bInfo msg;
	memset(&msg, 0, sizeof(msg));

	int index = 0;
	char *tmp = HexToString(&buf[index], 8);
	strcpy(msg.tid, tmp);
	index += 8;

	if (buf[index] == 0x01) {
		msg.rssi = buf[index + 1];
		index += 2;
	}

	if (buf[index] == 0x02) {
		msg.result = buf[index + 1];
		index += 2;
	}

	if (buf[index] == 0x03) {
		int ulen = buf[index + 1] << 8 | buf[index + 2];
		index += 3;
		char *tmp = HexToString(&buf[index], ulen);
		strcpy(msg.userData, tmp);
		delete []tmp;
	}

	if (s->call_Tag6bLog != NULL) {
		s->call_Tag6bLog(msg);
	}
}

void Proc6BReadFinish(XXRFIDCLient* s, unsigned char* buf)
{
	if (s == NULL) {
		return;
	}
	LogBase6bOver msg;
	memset(&msg, 0, sizeof(msg));

	msg.rst = buf[2];

	switch(msg.rst) {
		case 0:
			strcpy(msg.msg, "Single operation complete.");
			break;

		case 1:
			strcpy(msg.msg, "Receive stop instruction.");
			break;

		case 3:
			strcpy(msg.msg, "A hardware failure causes an interrupt.");
			break;
	}

	if (s->call_Tag6bOver != NULL) {
		s->call_Tag6bOver(msg);
	}
}

int Write6b(XXRFIDCLient* s, unsigned char *buf, MsgBaseWrite6b *msg)
{
	if (s == NULL) {
		return -1;
	}

	s->result[EMESS_Write6b].rst = (void*)msg;
	
	unsigned short dataLen = 0, len = 0;
	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_WRITE6B);
	FillProtocol(&buf[len], head);
	len += sizeof(head);
	len += 2; //长度字节占位
	
	FillAnts(&buf[len], msg->antennaEnable);
	len += sizeof(msg->antennaEnable);
	dataLen += sizeof(msg->antennaEnable);

	if (strlen(msg->hexMatchTid) > 0) {
		int tlen = 0;
		char *tid = StringToHex(msg->hexMatchTid, &tlen);
		memcpy(&buf[len], tid, tlen);
		len += 8;
		dataLen += 8;
	}

	buf[len] = msg->start;
	len += 1;
	dataLen += 1;

	if (strlen(msg->HexWriteData) > 0) {
		int dlen = 0;
		char *data = StringToHex(msg->HexWriteData, &dlen);
		memcpy(&buf[len], data, dlen);
		len += dlen;
		dataLen += dlen;
	}

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcWrite6b(XXRFIDCLient* s, unsigned char* buf)
{
	if (s == NULL) {
		return;
	}

	int index = 2;
	if (s->result[EMESS_Write6b].rst != NULL) {
		((MsgBaseWrite6b*)(s->result[EMESS_Write6b].rst))->rst.RtCode = buf[index];

		switch(buf[index]) {
			case 0:
				break;

			case 1:
				strcpy(((MsgBaseWrite6b*)(s->result[EMESS_Write6b].rst))->rst.RtMsg, "天线端口参数错误");
				break;

			case 2:
				strcpy(((MsgBaseWrite6b*)(s->result[EMESS_Write6b].rst))->rst.RtMsg, "写入参数错误");
				break;

			case 3:
				strcpy(((MsgBaseWrite6b*)(s->result[EMESS_Write6b].rst))->rst.RtMsg, "其他错误");
				break;
		}
	}

	sem_post(&s->sem[EMESS_Write6b]);
}

int Lock6b(XXRFIDCLient* s, unsigned char *buf, MsgBaseLock6b *msg)
{
	if (s == NULL) {
		return -1;
	}

	s->result[EMESS_Lock6b].rst = (void*)msg;
	
	unsigned short dataLen = 0, len = 0;
	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_LOCK6B);
	FillProtocol(&buf[len], head);
	len += sizeof(head);
	len += 2; //长度字节占位
	
	FillAnts(&buf[len], msg->antennaEnable);
	len += sizeof(msg->antennaEnable);
	dataLen += sizeof(msg->antennaEnable);

	if (strlen(msg->hexMatchTid) > 0) {
		int tlen = 0;
		char *tid = StringToHex(msg->hexMatchTid, &tlen);
		memcpy(&buf[len], tid, tlen);
		len += 8;
		dataLen += 8;
	}

	buf[len] = msg->lockIndex;
	len += 1;
	dataLen += 1;

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcLock6b(XXRFIDCLient* s, unsigned char* buf)
{
	if (s == NULL) {
		return;
	}

	int index = 2;
	if (s->result[EMESS_Lock6b].rst != NULL) {
		((MsgBaseWrite6b*)(s->result[EMESS_Lock6b].rst))->rst.RtCode = buf[index];

		switch(buf[index]) {
			case 0:
				break;

			case 1:
				strcpy(((MsgBaseWrite6b*)(s->result[EMESS_Lock6b].rst))->rst.RtMsg, "其他错误");
				break;
		}
	}

	sem_post(&s->sem[EMESS_Lock6b]);
}

int Lock6bGet(XXRFIDCLient* s, unsigned char *buf, MsgBaseLock6bGet *msg)
{
	if (s == NULL) {
		return -1;
	}

	s->result[EMESS_Lock6b].rst = (void*)msg;
	
	unsigned short dataLen = 0, len = 0;
	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_LOCKQUERY6B);
	FillProtocol(&buf[len], head);
	len += sizeof(head);
	len += 2; //长度字节占位
	
	FillAnts(&buf[len], msg->antennaEnable);
	len += sizeof(msg->antennaEnable);
	dataLen += sizeof(msg->antennaEnable);

	if (strlen(msg->hexMatchTid) > 0) {
		int tlen = 0;
		char *tid = StringToHex(msg->hexMatchTid, &tlen);
		memcpy(&buf[len], tid, tlen);
		len += 8;
		dataLen += 8;
	}

	buf[len] = msg->lockIndex;
	len += 1;
	dataLen += 1;

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcLock6bGet(XXRFIDCLient* s, unsigned char* buf)
{
	if (s == NULL) {
		return;
	}

	int index = 2;
	if (s->result[EMESS_Lock6bGet].rst != NULL) {
		((MsgBaseWrite6b*)(s->result[EMESS_Lock6bGet].rst))->rst.RtCode = buf[index];

		switch(buf[index]) {
			case 0:
				break;

			case 1:
				strcpy(((MsgBaseWrite6b*)(s->result[EMESS_Lock6bGet].rst))->rst.RtMsg, "其他错误");
				break;
		}
	}

	sem_post(&s->sem[EMESS_Lock6bGet]);
}

int InventoryGB(XXRFIDCLient* s, unsigned char *buf, MsgBaseInventoryGb *msg)
{
	if (s == NULL) {
		return -1;
	}

	unsigned char FrameHead = 0x5A;
    unsigned int Protocol = 0x00010210;
    unsigned short dataLen = 0, len = 0;
    unsigned char readMode = msg->inventoryMode; //0, danci;1,continue

	s->result[EMESS_InventoryEpc].rst = (void*)msg;

    buf[0] = FrameHead;
    len += sizeof(FrameHead);

   	unsigned int head = GetMsgHead(MsgType_RFIDConfig, RFID_MID_READGB);
	FillProtocol(&buf[len], head);
	len += sizeof(head);
	len += 2; //长度字节占位

	FillAnts(&buf[len], msg->antennaEnable);
	len += sizeof(msg->antennaEnable);
	dataLen += sizeof(msg->antennaEnable);

	buf[len] = readMode;
	len += sizeof(readMode);
	dataLen += sizeof(readMode);

	int ret = FillFilter(&buf[len], msg->filter);
	if(ret != -1) {
		len += ret;
		dataLen += ret;
	}

	ret = FillReadTID(&buf[len], msg->readTid);
	if(ret != -1) {
		len += 3;
		dataLen += 3;
	}

	ret = FillReadUserData(&buf[len], msg->readUserdata);
	if(ret != -1) {
		len += 4;
		dataLen += 4;
	}

	ret = FillPasswd(&buf[len], msg->strHexPassword, 0x05);
	if(ret != -1) {
		len += 5;
		dataLen += 5;
	}

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcInventoryGB(XXRFIDCLient* s, unsigned char* buf)
{
	if (s == NULL) {
		return;
	}

	int index = 2;

	if (s->result[EMESS_InventoryGb].rst != NULL) {

		((MsgBaseStop*)(s->result[EMESS_InventoryGb].rst))->rst.RtCode = buf[index];

		switch(buf[index]) {
			case 0:
				break;

			case 1:
				strcpy(((MsgBaseStop*)(s->result[EMESS_InventoryGb].rst))->rst.RtMsg, "天线端口参数错误");
				break;

			case 2:
				strcpy(((MsgBaseStop*)(s->result[EMESS_InventoryGb].rst))->rst.RtMsg, "选择读取参数错误");
				break;

			case 3:
				strcpy(((MsgBaseStop*)(s->result[EMESS_InventoryGb].rst))->rst.RtMsg, "TID读取参数错误");
				break;

			case 4:
				strcpy(((MsgBaseStop*)(s->result[EMESS_InventoryGb].rst))->rst.RtMsg, "用户数据区读取参数错误");
				break;

			case 5:
				strcpy(((MsgBaseStop*)(s->result[EMESS_InventoryGb].rst))->rst.RtMsg, "其他参数错误");
				break;
		}
	}

	sem_post(&s->sem[EMESS_InventoryGb]);
}

void ProcGBDataResultCmd(XXRFIDCLient* s, unsigned char* buf)
{

}

void ProcGBReadFinish(XXRFIDCLient* s, unsigned char* buf)
{
	
}

#endif

#ifdef __cplusplus
}
#endif
