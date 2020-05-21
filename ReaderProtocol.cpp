#include "protocol.h"
#include "pub.h"
#include "message.h"
#include "delegate.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "result.h"

#ifdef __cplusplus
	extern "C" {
#endif

int Reset(XXRFIDCLient* s, unsigned char* buf)
{
	unsigned short dataLen = 0, len = 0;

	buf[0] = FRAMEHEAD;
	len += 1;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_REBOOT);
	FillProtocol(&buf[len], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

int SetSerialParam(XXRFIDCLient* s, unsigned char* buf, MsgAppSetSerialParam* para)
{
	unsigned short dataLen = 0, len = 0;

	if (para == NULL) {
		return -1;
	}

	s->result[EMESS_SetSerialParam].rst = (void*)para;

	buf[0] = FRAMEHEAD;
	len += 1;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_SETCOM);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	switch(para->BaudrateIndex) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			buf[len] = para->BaudrateIndex;
			break;

		default:
			return -1;
	}
	len += 1;
	dataLen += 1;

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcSetSerialParam(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_SetSerialParam].rst != NULL) {

		((MsgAppSetSerialParam*)(s->result[EMESS_SetSerialParam].rst))->rst.RtCode = buf[index];

		if (buf[index] != 0) {
			strcpy(((MsgAppSetSerialParam*)(s->result[EMESS_SetSerialParam].rst))->rst.RtMsg, "不支持此波特率");
		}
	}

	sem_post(&s->sem[EMESS_SetSerialParam]);
}

int GetSerialParam(XXRFIDCLient* s, unsigned char* buf, MsgAppGetSerialParam* msg)
{
	unsigned short dataLen = 0, len = 0;

	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetSerialParam].rst = (void*)msg;

	buf[0] = FRAMEHEAD;
	len += 1;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_GETCOM);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcGetSerialParam(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_GetSerialParam].rst != NULL) {
		((MsgAppGetSerialParam*)(s->result[EMESS_GetSerialParam].rst))->BaudrateIndex = buf[index];
	}

	sem_post(&s->sem[EMESS_GetSerialParam]);
}

int FillGpoState(unsigned char* buf, MsgAppSetGpo* msg)
{
	buf[0] = 0x01;
	buf[1] = msg->Gpo1;

	buf[2] = 0x02;
	buf[3] = msg->Gpo2;

	buf[4] = 0x03;
	buf[5] = msg->Gpo3;

	buf[6] = 0x04;
	buf[7] = msg->Gpo4;

	return 8;
}

int SetGpo(XXRFIDCLient* s, unsigned char* buf, MsgAppSetGpo* msg)
{
	unsigned short dataLen = 0, len = 0;

	if(msg == NULL) {
		return -1;
	}

	s->result[EMESS_SetGpo].rst = (void*)msg;

	buf[0] = FRAMEHEAD;
	len += 1;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_SETGPO);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	int ret = FillGpoState(&buf[len], msg);
	len += ret;
	dataLen += ret;

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcSetGpo(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_SetGpo].rst != NULL) {
		((MsgAppSetGpo*)(s->result[EMESS_SetGpo].rst))->rst.RtCode = buf[index];

		if (buf[index] != 0) {
			strcpy(((MsgAppSetGpo*)(s->result[EMESS_SetGpo].rst))->rst.RtMsg, "端口参数读写器硬件不支持");
		}
	}

	sem_post(&s->sem[EMESS_SetGpo]);
}

int GetGpiState(XXRFIDCLient* s, unsigned char* buf, MsgAppGetGpiState* msg)
{
	unsigned short dataLen = 0, len = 0;

	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetGpiState].rst = (void*)msg;

	buf[0] = FRAMEHEAD;
	len += 1;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_GETGPI_STATE);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcGetGpiState(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_GetGpiState].rst != NULL) {
        MsgAppGetGpiState* msg = (MsgAppGetGpiState*)(s->result[EMESS_GetGpiState].rst);
		msg->Gpi1 = buf[3];
		msg->Gpi2 = buf[5];
		msg->Gpi3 = buf[7];
		msg->Gpi4 = buf[9];
	}

	sem_post(&s->sem[EMESS_GetGpiState]);
}

int SetGpiTrigger(XXRFIDCLient* s, unsigned char* buf, MsgAppSetGpiTrigger* msg)
{
	unsigned short dataLen = 0, len = 0;

	if(msg == NULL) {
		return -1;
	}

	s->result[EMESS_SetGpiTrigger].rst = (void*)msg;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_SETGPI_TRIGGER);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	buf[len++] = msg->GpiPort;
	buf[len++] = msg->TriggerStart;
	dataLen += 2;

	if (strlen(msg->TriggerCommand) > 0) {
		int cmdLen = 0;
		char* cmd = StringToHex(msg->TriggerCommand , &cmdLen);
		buf[len++] = (len>>8) & 0xFF;
		buf[len++] = len & 0xFF;
		memcpy(&buf[len], cmd, cmdLen);
		len += cmdLen;
		dataLen += 2+cmdLen;

		delete []cmd;
	}

	buf[len++] = msg->TriggerOver;
	dataLen += 1;

	buf[len++] = 0x01;
	buf[len++] = (msg->OverDelayTime >> 8) & 0xFF;
	buf[len++] = msg->OverDelayTime & 0xFF;
	dataLen += 3;

	buf[len++] = 0x02;
	buf[len++] = msg->LevelUploadSwitch;
	dataLen += 2;

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcSetGpiTrigger(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_SetGpiTrigger].rst != NULL) {
		((MsgAppSetGpiTrigger*)(s->result[EMESS_SetGpiTrigger].rst))->rst.RtCode = buf[index];

		switch(buf[index]) {
			case 0:
				break;

			case 1:
				strcpy(((MsgAppSetGpiTrigger*)(s->result[EMESS_SetGpiTrigger].rst))->rst.RtMsg, "端口参数读写器硬件不支持");
				break;

			case 2:
				strcpy(((MsgAppSetGpiTrigger*)(s->result[EMESS_SetGpiTrigger].rst))->rst.RtMsg, "参数缺失");
				break;
		}
	}

	sem_post(&s->sem[EMESS_SetGpiTrigger]);
}

void ProcGpiTriggerStart(XXRFIDCLient* s, unsigned char* buf)
{
	if (s == NULL) {
		return;
	}

	LogBaseGpiStart tmp;
	memset(&tmp, 0, sizeof(tmp));
	tmp.GpiPort = buf[2];
	tmp.Level = buf[3];

	unsigned int second = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
	unsigned int msecond = (buf[8] << 24) | (buf[9] << 16) | (buf[10] << 8) | buf[10];
	msecond = msecond % 1000;

	struct tm* now = gmtime((time_t*)&second);
	if (now != NULL) {
		sprintf(tmp.TriggerTime, "%d-%d-%d %d:%d:%d %.3d", now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, msecond);
	}

	if (s->call_GpiTriggerStart != NULL) {
		s->call_GpiTriggerStart(tmp);
	}
}

void ProcGpiTriggerOver(XXRFIDCLient* s, unsigned char* buf)
{
	LogBaseGpiOver tmp;
	memset(&tmp, 0, sizeof(tmp));
	tmp.GpiPort = buf[2];
	tmp.Level = buf[3];

	unsigned int second = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | buf[7];
	unsigned int msecond = (buf[8] << 24) | (buf[9] << 16) | (buf[10] << 8) | buf[10];
	msecond = msecond % 1000;

	struct tm* now = gmtime((time_t*)&second);
	if (now != NULL) {
		sprintf(tmp.TriggerTime, "%d-%d-%d %d:%d:%d %.3d", now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, msecond);
	}

	if (s->call_GpiTriggerOver != NULL) {
		s->call_GpiTriggerOver(tmp);
	}
}

int GetGpiTrigger(XXRFIDCLient* s, unsigned char* buf, MsgAppGetGpiTrigger* msg)
{
	unsigned short dataLen = 0, len = 0;

	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetGpiTrigger].rst = (void*)msg;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_GETGPI_TRIGGER);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	buf[len++] = msg->GpiPort;
	dataLen += 1;

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcGetGpiTrigger(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_GetGpiTrigger].rst != NULL) {
		MsgAppGetGpiTrigger* tmp = (MsgAppGetGpiTrigger*)(s->result[EMESS_GetGpiTrigger].rst);

		tmp->TriggerStart = buf[index++];

		int len = (buf[index++] << 8) | buf[index++];

		char* cmd = HexToString(&buf[5], len);
		strcpy(tmp->TriggerCommand, cmd);
		delete []cmd;

		index += len;

		tmp->TriggerOver = buf[index++];
		tmp->OverDelayTime = (buf[index++] << 8) | buf[index++];
		tmp->LevelUploadSwitch = buf[index++];
	}

	sem_post(&s->sem[EMESS_GetGpiTrigger]);
}

int StringIpToInt(char* ip)
{
	char* s = ip, *point = NULL;
	int result = 0;
	while ((point = strstr(s, ".")) != NULL) {
		char tmp[8] = {0};
		memcpy(tmp, s, point-s);
		result = (result << 8) | atoi(tmp);
		s = point + 1;
	}

    result = (result << 8) | atoi(s);
    s = point + 1;

	return result;
}

int SetEthernetIp(XXRFIDCLient* s, unsigned char* buf, MsgAppSetEthernetIP* msg)
{
	unsigned short dataLen = 0, len = 0;

	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_SetEthernetIp].rst = (void*)msg;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_SETIP);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	buf[len++] = msg->autoIp;
	dataLen += 1;

	if (strlen(msg->ip) > 0) {
		int ip = StringIpToInt(msg->ip);
		buf[len++] = 0x01;
		buf[len++] = (ip >> 24) & 0xFF;
		buf[len++] = (ip >> 16) & 0xFF;
		buf[len++] = (ip >> 8) & 0xFF;
		buf[len++] = ip & 0xFF;
		dataLen += 5;
	}

	if (strlen(msg->mask) > 0) {
		int mask = StringIpToInt(msg->mask);
		buf[len++] = 0x02;
		buf[len++] = (mask >> 24) & 0xFF;
		buf[len++] = (mask >> 16) & 0xFF;
		buf[len++] = (mask >> 8) & 0xFF;
		buf[len++] = mask & 0xFF;
		dataLen += 5;
	}

	if (strlen(msg->gateway) > 0) {
		int gateway = StringIpToInt(msg->gateway);
		buf[len++] = 0x03;
		buf[len++] = (gateway >> 24) & 0xFF;
		buf[len++] = (gateway >> 16) & 0xFF;
		buf[len++] = (gateway >> 8) & 0xFF;
		buf[len++] = gateway & 0xFF;
		dataLen += 5;
	}

	if (strlen(msg->dns1) > 0) {
		int dns1 = StringIpToInt(msg->dns1);
		buf[len++] = 0x04;
		buf[len++] = (dns1 >> 24) & 0xFF;
		buf[len++] = (dns1 >> 16) & 0xFF;
		buf[len++] = (dns1 >> 8) & 0xFF;
		buf[len++] = dns1 & 0xFF;
		dataLen += 5;
	}

	if (strlen(msg->dns2) > 0) {
		int dns2 = StringIpToInt(msg->dns2);
		buf[len++] = 0x05;
		buf[len++] = (dns2 >> 24) & 0xFF;
		buf[len++] = (dns2 >> 16) & 0xFF;
		buf[len++] = (dns2 >> 8) & 0xFF;
		buf[len++] = dns2 & 0xFF;
		dataLen += 5;
	}

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcSetEthernetIp(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_SetEthernetIp].rst != NULL) {
		MsgAppSetEthernetIP* tmp = (MsgAppSetEthernetIP*)(s->result[EMESS_SetEthernetIp].rst);

		tmp->rst.RtCode = buf[index];
		if(buf[index] != 0) {
			strcpy(tmp->rst.RtMsg, "读写器IP参数错误");
		}
	}

	sem_post(&s->sem[EMESS_SetEthernetIp]);
}

int GetEthernetIp(XXRFIDCLient* s, unsigned char* buf, MsgAppGetEthernetIP* msg)
{
	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetEthernetIp].rst = (void*)msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_GETIP);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void IntToStringIp(int hexIp, char* strIp)
{
	sprintf(strIp, "%u.%u.%u.%u", (hexIp>>24)&0xFF, (hexIp>>16)&0xFF, (hexIp>>8)&0xFF, hexIp&0xFF);
}

int Char2Int(unsigned char* str, int len)
{
    int ret = 0;
    for (int i = 0; i < len; i++) {
        ret |= (unsigned int)(str[i] << (len-1-i)*8);
    }

    return ret;
}

void ProcGetEthernetIp(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_GetEthernetIp].rst != NULL) {
		MsgAppGetEthernetIP* msg = (MsgAppGetEthernetIP*)(s->result[EMESS_GetEthernetIp].rst);

		msg->autoIp = buf[index++];

		int ip = Char2Int((unsigned char*)&buf[index], 4);
		IntToStringIp(ip, msg->ip);
		index += 4;

		int mask = Char2Int((unsigned char*)&buf[index], 4);
		IntToStringIp(mask, msg->mask);
		index += 4;

		int gateway = Char2Int((unsigned char*)&buf[index], 4);
		IntToStringIp(gateway, msg->gateway);
		index += 4;

		int dns1 = Char2Int((unsigned char*)&buf[index], 4);
		IntToStringIp(dns1, msg->dns1);
		index += 4;

		int dns2 = Char2Int((unsigned char*)&buf[index], 4);
		IntToStringIp(dns2, msg->dns2);
		index += 4;
	}

	sem_post(&s->sem[EMESS_GetEthernetIp]);
}

int GetEtherneMac(XXRFIDCLient* s, unsigned char * buf, MsgAppGetEthernetMac* msg)
{
	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetEthernetMac].rst = (void*)msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_GETMAC);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcGetEthernetMac(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_GetEthernetMac].rst != NULL) {
		MsgAppGetEthernetMac* msg = (MsgAppGetEthernetMac*)(s->result[EMESS_GetEthernetMac].rst);

        sprintf(msg->mac, "%.2X", buf[index++]);
		for (int i = 1; i < 6; i++) {
			sprintf(msg->mac, "%s:%.2X", msg->mac, (unsigned char)buf[index++]);
		}
	}

	sem_post(&s->sem[EMESS_GetEthernetMac]);
}

int SetTcpMode(XXRFIDCLient* s, unsigned char* buf, MsgAppSetTcpMode* msg)
{
	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_SetTcpMode].rst = msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_SETCS);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	buf[len++] = msg->tcpMode;
	dataLen += 1;

	if (msg->tcpMode == 0) { // 服务器模式
        unsigned short serverPort = 8160;
        if (msg->serverPort != 0) {
            serverPort = msg->serverPort;
        }
        buf[len++] = 0x01;
        buf[len++] = (serverPort >> 8) & 0xFF;
        buf[len++] = serverPort & 0xFF;
        dataLen += 3;
	} else { // 客户端模式
        if (msg->clientIp != NULL) {
		int ip = StringIpToInt(msg->clientIp);
		buf[len++] = 0x02;
		buf[len++] = (ip >> 24) & 0xFF;
		buf[len++] = (ip >> 16) & 0xFF;
		buf[len++] = (ip >> 8) & 0xFF;
		buf[len++] = ip & 0xFF;
		dataLen += 5;

		unsigned short clientPort = 8160;
        if (msg->clientPort != 0) {
            clientPort = msg->clientPort;
        }
        buf[len++] = 0x03;
        buf[len++] = (clientPort >> 8) & 0xFF;
        buf[len++] = clientPort & 0xFF;
        dataLen += 3;
        }
	}

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcSetTcpMode(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_SetTcpMode].rst != NULL) {
		MsgAppSetTcpMode* msg = (MsgAppSetTcpMode*)(s->result[EMESS_SetTcpMode].rst);

		msg->rst.RtCode = buf[index];
		if (buf[index] != 0) {
			strcpy(msg->rst.RtMsg, "服务器IP参数错误");
		}
	}

	sem_post(&s->sem[EMESS_SetTcpMode]);
}

int GetTcpMode(XXRFIDCLient* s, unsigned char* buf, MsgAppGetTcpMode* msg)
{
	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetTcpMode].rst = (void*)msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_GETCS);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcGetTcpMode(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_GetTcpMode].rst != NULL) {
		MsgAppGetTcpMode* msg = (MsgAppGetTcpMode*)(s->result[EMESS_GetTcpMode].rst);

		msg->tcpMode = buf[index++];
		msg->serverPort = (short)Char2Int((unsigned char*)&buf[index], 2);//(buf[index++] << 8) | buf[index++];
		index += 2;

		int clientIp = Char2Int((unsigned char*)&buf[index], 4);//(buf[index++] << 24) | (buf[index++] << 16) | (buf[index++] << 8) | buf[index++];
		IntToStringIp(clientIp, msg->clientIp);
		index += 4;

		msg->clientPort = (short)Char2Int((unsigned char*)&buf[index], 2);//(buf[index++] << 8) | buf[index++];
		index += 2;
	}

	sem_post(&s->sem[EMESS_GetTcpMode]);
}

int GetBaseVersion(XXRFIDCLient* s, unsigned char* buf, MsgAppGetBaseVersion* msg)
{
	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetBaseVersion].rst = (void*)msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_GETBASEV);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcGetBaseVersion(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_GetBaseVersion].rst != NULL) {
		MsgAppGetBaseVersion* msg = (MsgAppGetBaseVersion*)(s->result[EMESS_GetBaseVersion].rst);

		sprintf(msg->version, "%d.%d.%d.%d", buf[index++], buf[index++], buf[index++], buf[index++]);
	}

	sem_post(&s->sem[EMESS_GetBaseVersion]);
}

int GetReaderInfo(XXRFIDCLient* s, unsigned char* buf, MsgAppGetReaderInfo* msg)
{
	if (msg == NULL) {
		return -1;
	}

	s->result[EMESS_GetReaderInfo].rst = (void*)msg;

	unsigned short dataLen = 0, len = 0;

	buf[len++] = FRAMEHEAD;

	unsigned int head = GetMsgHead(MsgType_ReaderConfig, READER_MID_GETREADER);
	FillProtocol(&buf[1], head);
	len += sizeof(head);

	len += 2; //长度字节占位

	FillDataLen(&buf[5], dataLen);

	FillCrC(&buf[1], len-1);
    len += 2;

	return len;
}

void ProcGetReaderInfo(XXRFIDCLient* s, unsigned char* buf)
{
	int index = 2;

	if (s->result[EMESS_GetReaderInfo].rst != NULL) {
		MsgAppGetReaderInfo* msg = (MsgAppGetReaderInfo*)(s->result[EMESS_GetReaderInfo].rst);

		int len = (buf[0] << 8) | buf[1];

		short serialLen = (buf[index++] << 8) | buf[index++];
		memcpy(msg->Imei, &buf[index], serialLen);
		index += serialLen;

		unsigned int uptime = (buf[index++] << 24) | (buf[index++] << 16) | (buf[index++] << 8) | buf[index++];
		time_t now;
		time(&now);
		now = now - uptime;
		tm* t = gmtime(&now);
		sprintf(msg->PowerOnTime, "%d-%d-%d %.2d:%.2d:%.2d", t->tm_year+1900, t->tm_mon+1,
			t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

		int baseBuildTimeLen = (buf[index++] << 8) | buf[index++];
		memcpy(msg->BaseBuildDate, &buf[index], baseBuildTimeLen);
		index += baseBuildTimeLen;

		while (len >= index) {
			switch(buf[index++]) {
				case 0x01:
				{
					sprintf(msg->AppVersion, "%d", buf[index++]);
					for (int i = 1; i < 4; i ++) {
						sprintf(msg->AppVersion, "%s.%d", msg->AppVersion, buf[index++]);
					}
				}
					break;

				case 0x02:
				{
					int systemVerLen = (buf[index++] << 8) | buf[index++];
					memcpy(msg->SystemVersion, &buf[index], systemVerLen);
					index += systemVerLen;
				}
					break;

				case 0x03:
				{
					int appBuildTimeLen = (buf[index++] << 8) | buf[index++];
					memcpy(msg->AppBuildDate, &buf[index], appBuildTimeLen);
					index += appBuildTimeLen;
				}
					break;

				default:
					break;

			}
		}
	}

	sem_post(&s->sem[EMESS_GetReaderInfo]);
}

#ifdef __cplusplus
}
#endif
