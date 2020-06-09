#include "pub.h"
#include "ReaderProtocol.h"
#include "RFIDProtocol.h"
#include "delegate.h"
#include <stdio.h>
#include <sys/time.h>

#ifdef __cplusplus
	extern "C" {
#endif

void ProcError(unsigned char* buf)
{
    LOG_TICK("Error");
}

void ProcConnectState(XXRFIDCLient* client, unsigned char* rcv)
{
    unsigned char buf[128] = {0};
    int len = 0;
    getConnectStateCmd(buf, &len, rcv);
    WriteCmd(client, buf, len);
}

void messageProc(serialData data)
{
	struct timeval tv;
	int ret = gettimeofday(&tv, NULL);
	if (ret != 0) {
		LOG_TICK("failed to gettimeofday");
		return ;
	}
	data.s->tick = tv.tv_sec;

	unsigned char MType = data.data[3];
	switch((MType>>4)&0x0F)
	{
	case ReportFlag_Respons:   //下发命令的响应
		{
			switch(MType&0xF)
			{
			case MsgType_ReaderConfig:
				{
					switch(data.data[4])
					{
					case READER_MID_SETCOM:
						ProcSetSerialParam(data.s, &data.data[5]);
						break;

					case READER_MID_GETCOM:
						ProcGetSerialParam(data.s, &data.data[5]);
						break;

					case READER_MID_SETGPO:
						ProcSetGpo(data.s, &data.data[5]);
						break;

					case READER_MID_GETGPI_STATE:
						ProcGetGpiState(data.s, &data.data[5]);
						break;

					case READER_MID_SETGPI_TRIGGER:
						ProcSetGpiTrigger(data.s, &data.data[5]);
						break;

					case READER_MID_GETGPI_TRIGGER:
						ProcGetGpiTrigger(data.s, &data.data[5]);
						break;

					case READER_MID_SETIP:
						ProcSetEthernetIp(data.s, &data.data[5]);
						break;

					case READER_MID_GETIP:
						ProcGetEthernetIp(data.s, &data.data[5]);
						break;

					case READER_MID_GETMAC:
						ProcGetEthernetMac(data.s, &data.data[5]);
						break;

					case READER_MID_SETCS:
						ProcSetTcpMode(data.s, &data.data[5]);
						break;

					case READER_MID_GETCS:
						ProcGetTcpMode(data.s, &data.data[5]);
						break;

					case READER_MID_GETBASEV:
						ProcGetBaseVersion(data.s, &data.data[5]);
						break;

					case READER_MID_GETREADER:
						ProcGetReaderInfo(data.s, &data.data[5]);
						break;

					default:
						break;
					}
				}
				break;

			case MsgType_RFIDConfig:
				{
					switch((unsigned char)(data.data[4]))
					{
					case RFID_MID_READEPC:
						ProcInventoryEpc(data.s, &data.data[5]);
						break;

						case RFID_MID_WRITEEPC:
						ProcWriteEpc(data.s, &data.data[5]);
						break;

					case RFID_MID_LOCKEPC:
						ProcLockEpc(data.s, &data.data[5]);
						break;

					case RFID_MID_DESTORYECP:
						ProcDestoryEpc(data.s, &data.data[5]);
						break;

					case RFID_MID_GETRFIDCAPATICY:
						ProcGetCapabilities(data.s, &data.data[5]);
						break;

					case RFID_MID_GETPOWER:
						ProcGetPower(data.s, &data.data[5]);
						break;

					case RFID_MID_SETRFBAUD:
						ProcSetFreqRange(data.s, &data.data[5]);
						break;

					case RFID_MID_GETRFBAUD:
						ProcGetFreqRange(data.s, &data.data[5]);
						break;

					case RFID_MID_SETEPCBASEPARA:
						ProcSetBaseband(data.s, &data.data[5]);
						break;

					case RFID_MID_GETEPCBASEPARA:
						ProcGetBaseband(data.s, &data.data[5]);
						break;

					case RFID_MID_SETTAGREPORTPARA:
						ProcSetTagLog(data.s, &data.data[5]);
						break;

					case RFID_MID_GETTAGREPORTPARA:
						ProcGetTagLog(data.s, &data.data[5]);
						break;

					case RFID_MID_STOP:
						ProcStop(data.s, &data.data[5]);
						break;

					case RFID_MID_SETPOWER:
						ProcSetPower(data.s, &data.data[5]);
						break;

					case RFID_MID_READ6B:
						break;

					default:
						break;
					}
				}
				break;

			default:
				break;
			}
		}
		break;

	case ReportFlag_Auto:  //下位机的主动上报
		{
			switch(MType&0xF)
			{
			case MsgType_Err:
				ProcError(&data.data[5]);
				break;

			case MsgType_ReaderConfig:
				{
					switch(data.data[4])
					{
					case READER_MID_ACKCONNSTATUS:
						ProcConnectState(data.s, &data.data[5]);
						break;

					case READER_MID_TRIGGERSTART:
						ProcGpiTriggerStart(data.s, &data.data[5]);
						break;

					case READER_MID_TRIGGEROVER:
						ProcGpiTriggerOver(data.s, &data.data[5]);
						break;

					default:
						break;
					}
				}
				break;

			case MsgType_RFIDConfig:
				{
					switch(data.data[4])
					{
					case RFID_MID_EPCREPORT:
						{
							ProcDataResultCmd(data.s, (unsigned char*)&data.data[5]);
						}
						break;

					case RFID_MID_EPCOVER:
						{
							ProcReadFinish(data.s);
						}
						break;

					case RFID_MID_6BREPORT:
						{
							Proc6bDataResultCmd(data.s, (unsigned char*)&data.data[5]);
						}
						break;

					case RFID_MID_6BOVER:
						{
							Proc6BReadFinish(data.s, (unsigned char*)&data.data[5]);
						}
						break;

					case RFID_MID_GBREPORT:
						{

						}
						break;

					case RFID_MID_GBOVER:
						{

						}
						break;

					case RFID_MID_CURANT:
						break;

					default:
						break;
					}
				}
				break;
			}
		}
		break;

	default:
		break;
	}
}

#ifdef __cplusplus
}
#endif
