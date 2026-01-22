#include <stdint.h>
#include <time.h>
#include <stdio.h>

#include "lib/log.h"
#include "lib/lin.h"

#include "lin-this.h"
#include "lib/lin-transport.h"
#include "lin-this-upload.h"
#include "truma.h"

time_t timeOfLastStatus = 0;

uint16_t LinThisDownloadGetSecondsSinceLastStatus()
{
	time_t elapsedSeconds = time(0) - timeOfLastStatus;
	if (elapsedSeconds > 65535) elapsedSeconds = 65535;
	if (elapsedSeconds <     0) elapsedSeconds =     0;
	return (uint16_t)(elapsedSeconds);
}

static void checkByte(int index, char actual, char expected, char* format)
{
	if (actual != expected) Log('e', "LinThis handleStatusRequest index %d expected %02X but had %02X", index, expected, actual);
}

static void downloadStatus(char msgLen, char msgId, char* pRequest)
{
	/*
	uint16_t targetRoomTemp    0x00, 0x01                0,  1
	uint8_t  targetHeatingMode 0x02                      2
	uint8_t  actualRecvStatus  0x03       | 0x12         3,    | 18
	uint16_t targetPowerLevel  0x04, 0x05 | 0x08, 0x09   4,  5 |  8, 9
	uint16_t targetWaterTemp   0x06, 0x07                6,  7
	uint8_t  targetEnergyMix   0x0A       | 0x0B        10     | 11
	uint8_t  actualOpStatus    0x10                     16
	uint8_t  actualErrorCode   0x11                     17
	uint16_t actualWaterTemp   0x0C, 0x0D               12, 13
	uint16_t actualRoomTemp    0x0E, 0x0F               14, 15
	*/

	uint16_t rawTargetRoomTemp   = *(uint16_t*)(pRequest +  0);
	uint8_t  rawTargetFanMode    = *(uint8_t *)(pRequest +  2);
	uint8_t  rawActualRecvStatus = *(uint8_t *)(pRequest +  3);
	uint16_t rawTargetElecPower  = *(uint16_t*)(pRequest +  4);
	uint16_t rawTargetWaterTemp  = *(uint16_t*)(pRequest +  6);
	if (*(pRequest + 8) != *(pRequest + 4)) Log('e', "LinThis downloadStatus byte 8 not the same as byte 4");
	if (*(pRequest + 9) != *(pRequest + 5)) Log('e', "LinThis downloadStatus byte 9 not the same as byte 5");
	uint8_t  rawTargetEnergySel  = *(uint8_t *)(pRequest + 10);
	if (*(pRequest + 11) != *(uint8_t* )(pRequest + 10)) Log('e', "LinThis downloadStatus byte 11 not the same as byte 10");
	uint16_t rawActualWaterTemp  = *(uint16_t*)(pRequest + 12);
	uint16_t rawActualRoomTemp   = *(uint16_t*)(pRequest + 14);
	uint8_t  rawActualOpStatus   = *(uint8_t *)(pRequest + 16);
	uint8_t  rawActualErrorCode  = *(uint8_t *)(pRequest + 17);
	if (*(pRequest + 18) != *(pRequest + 3)) Log('e', "LinThis downloadStatus recvStatus-byte18 %02X is not the same as recvStatus-byte3 %02X", *(pRequest + 18), *(pRequest + 3));
	checkByte(19, *(pRequest + 19), 0x00, "LinThis downloadStatus index %d expected %02X but had %02X");
	
	TrumaTargetRoomTemp    = rawTargetRoomTemp ? (uint8_t)(rawTargetRoomTemp / 10 - 273) : 0;
	switch (rawTargetFanMode)
	{
		case 10: TrumaTargetFanMode = 'H'; break; //High
		case  1: TrumaTargetFanMode = 'E'; break; //Eco
		default: TrumaTargetFanMode = 'O'; break; //Off
	}
	TrumaActualRecvStatus  = rawActualRecvStatus;
	switch (rawTargetElecPower)
	{
		case 1800: TrumaTargetElecPower = '2'; break;
		case  900: TrumaTargetElecPower = '1'; break;
		default  : TrumaTargetElecPower = '0'; break;
	}
	switch (rawTargetWaterTemp)
	{
		case                0 : TrumaTargetWaterTemp = 'O'; break; //Off
		case ( 40 + 273) * 10 : TrumaTargetWaterTemp = 'E'; break; //Eco
		case ( 60 + 273) * 10 : TrumaTargetWaterTemp = 'H'; break; //High
		case (200 + 273) * 10 : TrumaTargetWaterTemp = 'B'; break; //Boost
		default               : TrumaTargetWaterTemp = 'U'; break; //Unknown
	}
	switch (rawTargetEnergySel)
	{
		case  1: TrumaTargetEnergySel = 'G'; break; //Gas
		case  2: TrumaTargetEnergySel = 'E'; break; //Elec
		case  3: TrumaTargetEnergySel = 'M'; break; //Mix
		default: TrumaTargetEnergySel = 'O'; break; //Off
	}
	TrumaActualWaterTemp   = rawActualWaterTemp;
	TrumaActualRoomTemp    = rawActualRoomTemp;
	TrumaActualOpStatus    = rawActualOpStatus;
	TrumaActualErrorCode   = rawActualErrorCode;
	
	
	time(&timeOfLastStatus);
	LinThisUploadHadStatus();
	
	if (LinTrace && TRACE_SID_BB_DOWNLOAD_STATUS)
	{
		Log ('d',"Download status message id 0x33");
		Log ('d',"Target room temp:   truma %04X, app %d"  , rawTargetRoomTemp,   TrumaTargetRoomTemp);
		Log ('d',"Target fan mode:    truma   %02X, app %c", rawTargetFanMode,    TrumaTargetFanMode );
		Log ('d',"Actual recv status: truma   %02X, app %d", rawActualRecvStatus, TrumaActualRecvStatus);
		Log ('d',"Target elec power:  truma %04X, app %c"  , rawTargetElecPower,  TrumaTargetElecPower);
		Log ('d',"Target water temp:  truma %04X, app %c"  , rawTargetWaterTemp,  TrumaTargetWaterTemp);
		Log ('d',"Target energy sel:  truma %04X, app %c"  , rawTargetEnergySel,  TrumaTargetEnergySel);
		Log ('d',"Actual water temp:  truma %04X, app %d"  , rawActualWaterTemp,  TrumaActualWaterTemp);
		Log ('d',"Actual room temp:   truma %04X, app %d"  , rawActualRoomTemp,   TrumaActualRoomTemp);
		Log ('d',"Actual op status:   truma   %02X, app %d", rawActualOpStatus,   TrumaActualOpStatus);
		Log ('d',"Actual error code:  truma   %02X, app %d", rawActualErrorCode,  TrumaActualErrorCode);
	
	}
	LinTransportResponse[0] = LinTransportSid + 0x40;
	LinTransportSetResponseLengthAndTrace(1, TRACE_SID_BB_DOWNLOAD_STATUS);  //No payload
}

static void downloadTimer(char msgLen, char msgId, char* pRequest)
{
	if (LinTransportRequestLength < 2)
	{
		Log('e', "LinThis downloadTimer length %d is less than 2", LinTransportRequestLength);
		return;
	}
	if (LinTrace && TRACE_SID_BB_DOWNLOAD_STATUS)
	{
		char text[1000];
		char* p = text;
		p += sprintf(p, "Timer data:");
		for (int i = 0; i < LinTransportRequestLength; i++) p += sprintf(p, " %02X", LinTransportRequest[i]);
		Log('d', text);
	}
	LinTransportResponse[0] = LinTransportSid + 0x40;
	LinTransportSetResponseLengthAndTrace(1, TRACE_SID_BB_DOWNLOAD_STATUS);  //No payload
}
static void downloadAck(char msgLen, char msgId, char* pRequest)
{
	uint8_t ack = *(uint8_t*)(pRequest)    ;      
	LinTransportResponse[0] = LinTransportSid + 0x40;
	
	if (LinTrace && (TRACE_SID_BB_DOWNLOAD_STATUS || TRACE_SID_BA_UPLOAD_COMMAND)) Log('d',"Acknowledged message id %02X with %02X", msgId, ack);
	switch (ack)
	{
		case  0: break;
		case  2: Log('e', "LinThis downloadCommandAck had 02 - Invalid message content"); break;
		case  3: Log('e', "LinThis downloadCommandAck had 03 - Invalid message type");    break;
		default: Log('e', "LinThis downloadCommandAck had %02X - Unknown error", ack); break;
	}
	LinThisUploadHadSendAcknowledgement(msgId, ack);
	
	LinTransportSetResponseLengthAndTrace(1, TRACE_SID_BB_DOWNLOAD_STATUS || TRACE_SID_BA_UPLOAD_COMMAND);  //No payload
}

static void downloadDeviceList(char msgLen, char msgId, char* pRequest)
{
	LinTransportResponse[0] = LinTransportSid + 0x40;
	LinTransportSetResponseLengthAndTrace(1, TRACE_SID_BB_DOWNLOAD_STATUS);  //No payload
	
	if (LinTrace && TRACE_SID_BB_DOWNLOAD_STATUS) Log('d',"LinThis downloadDeviceList");
}
static void downloadConfig(char msgLen, char msgId, char* pRequest)
{
	LinTransportResponse[0] = LinTransportSid + 0x40;
	LinTransportSetResponseLengthAndTrace(1, TRACE_SID_BB_DOWNLOAD_STATUS);  //No payload
	
	if (LinTrace && TRACE_SID_BB_DOWNLOAD_STATUS) Log('d',"LinThis downloadConfig");
}


void LinThisDownload()
{
	/*
	00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40
	                                                   00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23
	BB 00 1F 00 1E 00 00 22 FF FF FF 54 01 14 33 00 09 00 00 00 00 08 07 00 00 08 07 02 02 0E 0B 0D 0B 07 00 00 00 00 00 00 00
	^        ^                       ^     ^  ^  ^  ^  ^
	|        |						 |     |  |  |	|  Message start
	|        |						 |     |  |  |  Message checksum
	|        |						 |     |  |	 Message id
	|        |						 |     |  Message type
	|        |						 |     Message length
	|        |                       Header start
	|        Preamble
	Sid
	*/
	int i = 0;
	char* checkFormat = "LinThisDownload index %d expected %02X but had %02X";
	checkByte(i, LinTransportRequest[i], 0xBB, checkFormat); i++; // 0 request sid
	checkByte(i, LinTransportRequest[i], 0x00, checkFormat); i++; // 1
	checkByte(i, LinTransportRequest[i], 0x1F, checkFormat); i++; // 2
	checkByte(i, LinTransportRequest[i], 0x00, checkFormat); i++; // 3
	checkByte(i, LinTransportRequest[i], 0x1E, checkFormat); i++; // 4
	checkByte(i, LinTransportRequest[i], 0x00, checkFormat); i++; // 5
	checkByte(i, LinTransportRequest[i], 0x00, checkFormat); i++; // 6
	checkByte(i, LinTransportRequest[i], 0x22, checkFormat); i++; // 7
	checkByte(i, LinTransportRequest[i], 0xFF, checkFormat); i++; // 8
	checkByte(i, LinTransportRequest[i], 0xFF, checkFormat); i++; // 9
	checkByte(i, LinTransportRequest[i], 0xFF, checkFormat); i++; //10
	checkByte(i, LinTransportRequest[i], 0x54, checkFormat); i++; //11 message start
	checkByte(i, LinTransportRequest[i], 0x01, checkFormat); i++; //12
	char msgLength   = LinTransportRequest[13];                   //13 message length
	char msgType     = LinTransportRequest[14];                   //14 message type
	char msgId       = LinTransportRequest[15];                   //15 message id
	char msgCheckSum = LinTransportRequest[16];                   //16 message checksum
	char* pMsg       = LinTransportRequest + 17;                  //17 message
	char calCheckSum = LinThisCalculateCheckSum(5, LinTransportRequest + 11, msgLength, pMsg); //Calculate over 5 bytes of message header then the message itself
	if (calCheckSum != msgCheckSum)
	{
		Log('e', "LinThis downloadBB checksum expected %02X but had %02X", calCheckSum, msgCheckSum);
		return;
	}
	switch (msgType)
	{
		case 0x3D: downloadTimer     (msgLength, msgId, pMsg); return;
		case 0x33: downloadStatus    (msgLength, msgId, pMsg); return;
		case 0x0D: downloadAck       (msgLength, msgId, pMsg); return;
		case 0x0B: downloadDeviceList(msgLength, msgId, pMsg); return;
		case 0x17: downloadConfig    (msgLength, msgId, pMsg); return;
		default:
			Log('e', "LinThisDownload unknown message type %02X, length %d", msgType, msgLength);
			return;			
	}
}
