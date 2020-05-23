#include <stdio.h>
#include <string.h>
#include "delegate.h"
#include "message.h"
#include "XXRFID.h"
#include <stdlib.h>

void TagEpcLog(LogBaseEpcInfo msg)
{
	printf("EPC: %s\n", msg.Epc);
}

void TagEpcOver(void)
{
	printf("TagEpcOver \n");
}

void TcpDisconnected(char* msg)
{
	printf("%s : %d \n", __FUNCTION__, __LINE__);
}

void GClientConnected(char* msg)
{
	printf("%s : %d \n", __FUNCTION__, __LINE__);
}

int main(int argc, char* argv[])
{
    // XXRFIDCLient* s = OpenSerial("/dev/ttyUSB0:115200", 5);
	// XXRFIDCLient* s = OpenUSB(5);
	// XXRFIDCLient *s = OpenTcp("192.168.1.168:8168", 5);
	XXRFIDCLient *s = Open(8168);
	if(s == NULL)
	{
		printf("failed to OpenSerial \n");
		return 0;
	}

    RegCallBack(s, ETagEpcLog, (void*)TagEpcLog);
    RegCallBack(s, ETagEpcOver, (void*)TagEpcOver);
	RegCallBack(s, ETcpDisconnected, (void*)TcpDisconnected);
	RegCallBack(s, EGClientConnected, (void*)GClientConnected);

	printf("请选择命令：\n"
		"0，停止。\n"
		"1，盘存。\n"
		"2，关闭。\n"
		"4, 查询串口参数\n"
		"19, 配置串口参数 \n"
		"5, 获取GPI状态\n"
		"6, 获取Gpi触发参数\n"
		"21, 配置Gpi触发参数 \n"
		"7, 读取以太网IP\n"
		"8, 查询以太网MAC\n"
		"9, 查询服务器客户端模式\n"
		"10, 设置服务器客户端模式 \n"
		"11, 查询读写器功率 \n"
		"12, 查询读写器工作频段 \n"
		"24, 设置读写器工作频段 \n"
		"13, 查询EPC基带参数 \n"
		"25, 设置EPC基带参数 \n"
		"14, 查询标签上传参数 \n"
		"26, 配置标签上传参数 \n"
		"15, 查询基带软件版本 \n"
		"16, 查询读写器信息 \n"
		"17, 查询读写器RFID能力 \n"
		"18, 重启读写器 \n"
		"20, 配置Gpo状态 \n"
		"22, 查询读写器功率 \n"
		"23, 配置读写器功率 \n");

	int select = 0;
	while (scanf("%d%*c", &select) != EOF) {

        printf("select : %d \n", select);

        switch(select) {
        case 0:
		{
			MsgBaseStop stop;
			memset(&stop, 0, sizeof(stop));
        	SendSynMsg(s, EMESS_Stop, &stop);
			if (stop.rst.RtCode != 0) {
				printf("failed to stop: %s \n", stop.rst.RtMsg);
			}
		}
        break;

        case 1:
        {
            MsgBaseInventoryEpc epc;
            memset(&epc, 0, sizeof(MsgBaseInventoryEpc));
            epc.AntennaEnable = 0x1;
            epc.InventoryMode = 1;

            SendSynMsg(s, EMESS_InventoryEpc, &epc);
			if (epc.rst.RtCode != 0) {
				printf("failed to EPC: %s \n", epc.rst.RtMsg);
			}
        }
        break;

		case 2:
		{
			Close(s);
		}
		break;

		case 4:
		{
			MsgAppGetSerialParam msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetSerialParam, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_GetSerialParam : %s \n", msg.rst.RtMsg);
				continue;
			}
			printf("GetSerialParam : %d \n", msg.BaudrateIndex);
		}
		break;

		case 5:
		{
			MsgAppGetGpiState msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetGpiState, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_GetGpiState : %s \n", msg.rst.RtMsg);
				continue;
			}
			printf("Gpi1: %d, Gpi2: %d, Gpi3:%d, Gpi4:%d \n", msg.Gpi1, msg.Gpi2, msg.Gpi3, msg.Gpi4);
		}
		break;

		case 6:
		{
			MsgAppGetGpiTrigger msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetGpiTrigger, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_GetGpiTrigger : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("Gpi: %d, TriggerStart: %d  ...\n", msg.GpiPort, msg.TriggerStart);
		}
		break;

		case 7:
		{
			MsgAppGetEthernetIP msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetEthernetIp, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_GetEthernetIp : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("autoIp: %d, ip: %s, mask: %s, gateway: %s, dns1: %s, dns2: %s \n",
					msg.autoIp, msg.ip, msg.mask, msg.gateway, msg.dns1, msg.dns2);
		}
		break;

		case 8:
		{
			MsgAppGetEthernetMac msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetEthernetMac, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_GetEthernetMac : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("Ethernet Mac : %s \n", msg.mac);
		}
		break;

		case 9:
		{
			MsgAppGetTcpMode msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetTcpMode, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_SetTcpMode : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("tcpMode: %d, serverPort: %d, clientIp: %s, clientPort: %d \n",
					msg.tcpMode, msg.serverPort, msg.tcpMode == 0 ? "" : msg.clientIp,
					msg.clientPort);
		}
		break;

		case 10:
        {
            MsgAppSetTcpMode msg;
            memset(&msg, 0, sizeof(msg));

            msg.tcpMode = 1;
            msg.serverPort = 1000;
            msg.clientPort = 1000;
            strcpy(msg.clientIp, "192.168.1.200");
            SendSynMsg(s, EMESS_SetTcpMode, &msg);
            if (msg.rst.RtCode != 0) {
                printf("faild to EMESS_SetTcpMode : %s \n", msg.rst.RtMsg);
                continue;
            }
        }
		break;

		case 11:
		{
			MsgBaseGetPower msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetPower, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_GetPower : %s \n", msg.rst.RtMsg);
				continue;
			}

			for (int i = 0; i < msg.dicCount; i++) {
				printf("Ant%d : power: %d \n", msg.DicPower[i].AntennaNo, msg.DicPower[i].power);
			}
		}
		break;

		case 12:
		{
			MsgBaseGetFreqRange msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetFreqRange, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_GetFreqRange : %s \n", msg.rst.RtCode);
				continue;
			}

			printf("GetFreqRange : %d \n", msg.FreqRangeIndex);
		}
		break;

		case 13:
		{
			MsgBaseGetBaseband msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetBaseband, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to GetBaseBand : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("BaseSpeed: %d, QValue: %d, Session: %d, InventoryFlag: %d \n",
					msg.BaseSpeed, msg.QValue, msg.Session, msg.InventoryFlag);
		}
		break;

		case 14:
		{
			MsgBaseGetTagLog msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetTagLog, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_GetTagLog : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("RepeatedTime: %d, RssiTV: %d \n", msg.RepeatedTime, msg.RssiTV);
		}
		break;

		case 15:
		{
			MsgAppGetBaseVersion msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetBaseVersion, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_GetBaseVersion : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("BaseVersion : %s \n", msg.version);
		}
		break;

		case 16:
		{
			MsgAppGetReaderInfo msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetReaderInfo, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_GetReaderInfo : %s \n", msg.rst.RtMsg);
				continue ;
			}

			printf("Imei: %s, PowerOnTime: %s, BaseBuildDate: %s, AppVersion: %s, AppBuildDate: %s, SystemVersion: %s\n",
					msg.Imei, msg.PowerOnTime, msg.BaseBuildDate, msg.AppVersion, msg.AppBuildDate, msg.SystemVersion);
		}
		break;

		case 17:
		{
			MsgBaseGetCapabilities msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetCapabilities, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_GetCapabilities : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("MaxPower: %d, MinPower: %d, AntennaCount: %d \n", msg.MaxPower, msg.MinPower, msg.AntennaCount);
			for (int i = 0; i < msg.FrequencyArraySize; i++) {
				printf("Frequency : %d \n", msg.FrequencyArray[i]);
			}

			for (int i = 0; i < msg.ProtocolArraySize; i++) {
				printf("Protocol : %d \n", msg.ProtocolArray[i]);
			}
		}
		break;

		case 18:
		{
			MsgAppReset msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_Reset, &msg);
		}
		break;

		case 19:
		{
			MsgAppSetSerialParam msg;
			memset(&msg, 0, sizeof(msg));
			msg.BaudrateIndex = 2; // 115200

			SendSynMsg(s, EMESS_SetSerialParam, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_SetSerialParam : %s \n", msg.rst.RtMsg);
				continue;
			}
			printf("success \n");
		}
		break;

		case 20:
		{
			MsgAppSetGpo msg;
			memset(&msg, 0, sizeof(msg));
			msg.Gpo1 = 1;

			SendSynMsg(s, EMESS_SetGpo, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_SetGpo : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("success \n");
		}
		break;

		case 21:
		{
			MsgAppSetGpiTrigger msg;
			memset(&msg, 0, sizeof(msg));

			msg.GpiPort = 0;
			msg.LevelUploadSwitch = 0;
			msg.TriggerStart = 4;

			SendSynMsg(s, EMESS_SetGpiTrigger, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_SetGpiTrigger : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("success \n");
		}
		break;

		case 22:
		{
			MsgBaseGetPower msg;
			memset(&msg, 0, sizeof(msg));

			SendSynMsg(s, EMESS_GetPower, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_GetPower : %s \n", msg.rst.RtMsg);
				continue;
			}

			for (int i = 0; i < msg.dicCount; i++) {
				printf("Ant%d : %ddB \n", msg.DicPower[i].AntennaNo, msg.DicPower[i].power);
			}
		}
		break;

		case 23:
		{
			MsgBaseSetPower msg;
			memset(&msg, 0, sizeof(msg));
			msg.DicPower[0].AntennaNo = 1;
			msg.DicPower[0].power = 20;
			msg.dicCount = 1;

			SendSynMsg(s, EMESS_SetPower, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_SetPower : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("success \n");
		}
		break;

		case 24:
		{
			MsgBaseSetFreqRange msg;
			memset(&msg, 0, sizeof(msg));
			msg.FreqRangeIndex = 3;

			SendSynMsg(s, EMESS_SetFreqRange, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_SetFreqRange : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("success \n");
		}
		break;

		case 25:
		{
			MsgBaseSetBaseband msg;
			memset(&msg, 0, sizeof(msg));
			msg.QValue = 4;
			msg.setQValueFlag = 1;

			SendSynMsg(s, EMESS_SetBaseband, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_SetBaseband : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("success \n");
		}
		break;

		case 26:
		{
			MsgBaseSetTagLog msg;
			memset(&msg, 0, sizeof(msg));
			msg.setRepeatedTimeFlag = 1;
			msg.RepeatedTime = 0;

			msg.setRssiTVFlag = 1;
			msg.RssiTV = 0;

			SendSynMsg(s, EMESS_SetTagLog, &msg);
			if (msg.rst.RtCode != 0) {
				printf("failed to EMESS_SetTagLog : %s \n", msg.rst.RtMsg);
				continue;
			}

			printf("success \n");
		}

        default:
            break;
        }
	}


    return 0;
}
