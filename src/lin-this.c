#include <stdio.h>
#include <time.h>

#include "lib/lin.h"
#include "lib/lin-transport.h"
#include "lib/log.h"

#include "truma.h"

#define TRACE_FID_18_CHANGES_TO_SEND 0
#define TRACE_SID_B2_READ_BY_ID      1
#define TRACE_SID_BA_UPLOAD_COMMAND  1
#define TRACE_SID_BB_DOWNLOAD_STATUS 1
#define TRACE_SID_B9_HEARTBEAT       0


char _requestSid = 0;

static void unhandledBytesHandler()
{
	int count = LinGetUnhandledBytesCount();
	char* pBytes = LinGetUnhandledBytes();
	
	//Choose what to not report
	if (!LinTrace) return;
	if (count ==  2 && *(pBytes) == 0x55 && *(pBytes + 1) == 0x7D                                                                                  ) return; //This is a runt
	if (count ==  2 && *(pBytes) == 0x55 && *(pBytes + 1) == 0x97                                                                                  ) return; //This is a runt
	if (count == 11 && *(pBytes) == 0x55 && *(pBytes + 1) == 0xD8                                                   &&!TRACE_FID_18_CHANGES_TO_SEND) return; //Changes response
	if (count >=  3 && *(pBytes) == 0x55 && *(pBytes + 1) == 0x7D && *(pBytes + 2) == 0x01                                                         ) return; //Transport response by the Combi
	if (count == 11 && *(pBytes) == 0x55 && *(pBytes + 1) == 0x7D && *(pBytes + 2) == 0x03 && *(pBytes + 4) == 0xF2 &&!TRACE_SID_B2_READ_BY_ID     ) return; //Assign nad response
	if (count == 11 && *(pBytes) == 0x55 && *(pBytes + 1) == 0x7D && *(pBytes + 2) == 0x03 && *(pBytes + 4) == 0xFA &&!TRACE_SID_BA_UPLOAD_COMMAND ) return; //Upload command response
	if (count == 11 && *(pBytes) == 0x55 && *(pBytes + 1) == 0x7D && *(pBytes + 2) == 0x03 && *(pBytes + 4) == 0xFB &&!TRACE_SID_BB_DOWNLOAD_STATUS) return; //Download status response
	if (count == 11 && *(pBytes) == 0x55 && *(pBytes + 1) == 0x7D && *(pBytes + 2) == 0x03 && *(pBytes + 4) == 0xF9 &&!TRACE_SID_B9_HEARTBEAT      ) return; //Heartbeat response
	if (count == 11 && *(pBytes) == 0x55 && *(pBytes + 1) == 0x7D && *(pBytes + 2) == 0x03 && LinAllowBusWrites                                    ) return; //Our own responses
	if (count == 11 && *(pBytes) == 0x55 && *(pBytes + 1) == 0xD8                          && LinAllowBusWrites                                    ) return; //Our own responses

	//Report
	char text[100];
	char* p = text;
	p += sprintf(p, "%2d unhandled bytes: ", LinGetUnhandledBytesCount());
	for (int i = 0; i < LinGetUnhandledBytesCount(); i++) p += sprintf(p, " %02X", *(pBytes + i));
	Log('d', text);
}
static void respondChangesToSend()
{
	char sendWanted = TrumaGetSendWanted();
	if (LinTrace && TRACE_FID_18_CHANGES_TO_SEND)
	{
		Log('d', "<18> Changes to send? %s ", sendWanted ? "Yes" : "No");
	}
	char response[8];
	response[0] = sendWanted ? 0xFF : 0xFE;
	for (int i = 1; i < 8; i++) response[i] = 0xFF;
	LinSendFrameResponse(sizeof(response), response, LinTrace && TRACE_FID_18_CHANGES_TO_SEND);
}
static void idHandler()
{
	char frameId = LinGetFrameId();
	
	switch(frameId)
	{
		case 0x18: respondChangesToSend(); LinDataLength = -1; break;
		default:                           LinDataLength =  8; break;
	}
}
static char calculateCheckSum(int headerLen, char* pHeader, int bufferLen, char* pBuffer)
{
	unsigned int cs = 0;
	for (int i = 0; i < headerLen; i++)
	{
		cs += (unsigned char)*(pHeader + i);
		if (cs > 0xFF) cs -= 0xFF;
	}
	for (int i = 0; i < bufferLen; i++)
	{
		cs += (unsigned char)*(pBuffer + i);
		if (cs > 0xFF) cs -= 0xFF;
	}
	return (char)(~cs & 0xFF);
	
}

static void checkByte(int index, char actual, char expected, char* format)
{
	if (actual != expected) Log('e', "LinThis handleStatusRequest index %d expected %02X but had %02X", index, expected, actual);
}
static void readById()
{
	if (LinTransportRequestLength < 2)
	{
		Log('e', "LinThis readById length %d is less than 2", LinTransportRequestLength);
		return;
	}
	char rawId = LinTransportRequest[1];
	if ( LinTransportRequest[2] != 0x17) return; //Just ignore any message which is not for me
	if ( LinTransportRequest[3] != 0x46) return; 
	if ( LinTransportRequest[4] != 0x00) return; 
	if ( LinTransportRequest[5] != 0x1F) return; 
	if (LinTrace && TRACE_SID_B2_READ_BY_ID)
	{
		char text[1000];
		char* p = text;
		p += sprintf(p, "Read by id:");
		for (int i = 0; i < LinTransportRequestLength; i++) p += sprintf(p, " %02X", LinTransportRequest[i]);
		p += sprintf(p, " Id is %02X", rawId);
		Log('d', text);
	}
	switch (rawId)
	{
		case 0x00:
			LinTransportResponse[0] = _requestSid + 0x40;
			LinTransportResponse[1] = 0x17; //Device id 4617
			LinTransportResponse[2] = 0x46;
			LinTransportResponse[3] = 0x00; //Hardware revision
			LinTransportResponse[4] = 0x1F;
			LinTransportResponse[5] = 0x00; //Variant
			LinTransportNad = 0x03; //Set NAD to me - it might have been 0x7F
			LinTransportSetResponseLengthAndTrace(6, TRACE_SID_B2_READ_BY_ID);  //6 bytes
			break;
		case 0x20:
			LinTransportResponse[0] = _requestSid + 0x40;
			LinTransportResponse[1] = 0x03;
			LinTransportResponse[2] = 0x01;
			LinTransportResponse[3] = 0x05;
			LinTransportNad = 0x03; //Set NAD to me - it might have been 0x7F
			LinTransportSetResponseLengthAndTrace(4, TRACE_SID_B2_READ_BY_ID);  //4 bytes
			break;
		default:
			Log('e', "LinThis readById unknown id %02X", rawId);
			return;
	}
}
static void timerData()
{
	if (LinTransportRequestLength < 2)
	{
		Log('e', "LinThis timerData length %d is less than 2", LinTransportRequestLength);
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
	LinTransportResponse[0] = _requestSid + 0x40;
	LinTransportSetResponseLengthAndTrace(1, TRACE_SID_BB_DOWNLOAD_STATUS);  //No payload
}
static void heartbeat()
{
	if (LinTrace && TRACE_SID_B9_HEARTBEAT)
	{
		char text[1000];
		char* p = text;
		p += sprintf(p, "Heartbeat:");
		for (int i = 0; i < LinTransportRequestLength; i++) p += sprintf(p, " %02X", LinTransportRequest[i]);
		Log('d', text);
	}
	LinTransportResponse[0] = _requestSid + 0x40;
	LinTransportResponse[1] = 0x00;
	LinTransportSetResponseLengthAndTrace(2, TRACE_SID_B9_HEARTBEAT);  //Just one byte of zero
}
static uint8_t _commandId = 0;
static void uploadCommand()
{	
	uint16_t rawWantedRoomTemp = 0;
	uint8_t  rawWantedFanMode = 0;
	if (TrumaGetWantedRoomOn())
	{
		rawWantedRoomTemp = ((uint16_t)TrumaGetWantedRoomTemp() + 273) * 10;
		switch (TrumaGetWantedFanMode())
		{
			case 'H': rawWantedFanMode = 10; break;
			case 'E': rawWantedFanMode =  1; break;
			default : rawWantedFanMode =  0; break;
		}
	}
	uint16_t rawWantedWaterTemp = 0;
	if (TrumaGetWantedWaterOn())
	{
		switch (TrumaGetWantedWaterTemp())
		{
			case 'E': rawWantedWaterTemp = ( 40 + 273) * 10; break;
			case 'H': rawWantedWaterTemp = ( 60 + 273) * 10; break;
			case 'B': rawWantedWaterTemp = (200 + 273) * 10; break;
			default:  rawWantedWaterTemp =    0;
		}
	}
	uint8_t rawWantedEnergySel = 0;
	switch (TrumaGetWantedEnergySel())
	{
		case 'G': rawWantedEnergySel = 1; break;
		case 'E': rawWantedEnergySel = 2; break;
		case 'B':
		case 'M': rawWantedEnergySel = 3; break;
		default : rawWantedEnergySel = 0; break;
	}
	uint16_t rawWantedElecPower = 0;
	switch (TrumaGetWantedElecPower())
	{
		case '2': rawWantedElecPower = 1800; break;
		case '1': rawWantedElecPower =  900; break;
		default : rawWantedElecPower =    0; break;
	}
	
	if (LinTrace && TRACE_SID_BA_UPLOAD_COMMAND)
	{
		Log ('d',"Upload command message id 0x32");
		Log ('d',"Target room temp:   truma %04X, app %d"  , rawWantedRoomTemp,   TrumaGetWantedRoomTemp ());
		Log ('d',"Target fan mode:    truma   %02X, app %c", rawWantedFanMode,    TrumaGetWantedFanMode  ());
		Log ('d',"Target water temp:  truma %04X, app %c"  , rawWantedWaterTemp,  TrumaGetWantedWaterTemp());
		Log ('d',"Target elec power:  truma %04X, app %c"  , rawWantedElecPower,  TrumaGetWantedElecPower());
		Log ('d',"Target energy sel:  truma %04X, app %c"  , rawWantedEnergySel,  TrumaGetWantedEnergySel());
	}
	
	LinTransportResponse[ 0] = 0xFA; //
	LinTransportResponse[ 1] = 0x00; //
	LinTransportResponse[ 2] = 0x1F; //
	LinTransportResponse[ 3] = 0x00; //
	LinTransportResponse[ 4] = 0x1E; //
	LinTransportResponse[ 5] = 0x00; //
	LinTransportResponse[ 6] = 0x00; //
	LinTransportResponse[ 7] = 0x22; //
	LinTransportResponse[ 8] = 0xFF; //
	LinTransportResponse[ 9] = 0xFF; //
	LinTransportResponse[10] = 0xFF; //
	LinTransportResponse[11] = 0x54; //message start
	LinTransportResponse[12] = 0x01; //
	LinTransportResponse[13] = 0x0C; //message length 12
	LinTransportResponse[14] = 0x32; //message type of request -1
	LinTransportResponse[15] = _commandId++; //message id
	                                 //16 message checksum
	char* pResp = LinTransportResponse + 17;
	*(uint16_t*)(pResp +  0) = rawWantedRoomTemp;        //17
	           *(pResp +  2) = rawWantedFanMode;         //19
	           *(pResp +  3) = 0;                        //20 Recv status
	*(uint16_t*)(pResp +  4) = rawWantedElecPower;       //21
	*(uint16_t*)(pResp +  6) = rawWantedWaterTemp;       //23
	*(uint8_t* )(pResp +  8) = *(uint8_t* )(pResp +  4); //25
	*(uint8_t* )(pResp +  9) = *(uint8_t* )(pResp +  5); //26
	*(uint8_t* )(pResp + 10) = rawWantedEnergySel;       //27
	*(uint8_t* )(pResp + 11) = *(uint8_t* )(pResp + 10); //28
	
	LinTransportResponse[16] = calculateCheckSum(5, LinTransportResponse + 11, 12, LinTransportResponse + 17);
	
	LinTransportSetResponseLengthAndTrace(41, TRACE_SID_BA_UPLOAD_COMMAND);  //41 bytes
	
	TrumaSetSendWanted(0); 
}
static void downloadAck(char msgLen, char* pRequest)
{
	uint8_t rawId  = *(uint8_t*)(pRequest - 2); 
	uint8_t rawAck = *(uint8_t*)(pRequest)    ;      
	LinTransportResponse[0] = _requestSid + 0x40;
	LinTransportSetResponseLengthAndTrace(1, TRACE_SID_BB_DOWNLOAD_STATUS);  //Empty response
	
	if (LinTrace && TRACE_SID_BB_DOWNLOAD_STATUS) Log('d',"Acknowledged command id %02X with %02X", rawId, rawAck);
	switch (rawAck)
	{
		case  0: TrumaHadSendAcknowledgement(); break; //Update the status information without waiting for the next status
		case  2: Log('e', "LinThis downloadCommandAck had 02 - Invalid message content"); break;
		case  3: Log('e', "LinThis downloadCommandAck had 03 - Invalid message type");    break;
		default: Log('e', "LinThis downloadCommandAck had %02X - Unknown error", rawAck); break;
	}
}

static void downloadDeviceList(char msgLen, char* pRequest)
{
	LinTransportResponse[0] = _requestSid + 0x40;
	LinTransportSetResponseLengthAndTrace(1, TRACE_SID_BB_DOWNLOAD_STATUS);  //Empty response
	
	if (LinTrace && TRACE_SID_BB_DOWNLOAD_STATUS) Log('d',"LinThis downloadDeviceList");
}

static void downloadStatus(char msgLen, char* pRequest)
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
	if (*(pRequest + 18) != *(pRequest + 3)) Log('e', "LinThis downloadStatus byte 18 not the same as byte 3");
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
	
	TrumaHadStatus();
	
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
	LinTransportResponse[0] = _requestSid + 0x40;
	LinTransportSetResponseLengthAndTrace(1, TRACE_SID_BB_DOWNLOAD_STATUS);  //Empty response
}

static void downloadBB()
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
	char* checkFormat = "LinThis downloadBB index %d expected %02X but had %02X";
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
	                                                              //15 message id
	char msgCheckSum = LinTransportRequest[16];                   //16 message checksum
	char* pMsg       = LinTransportRequest + 17;                  //17 message
	char calCheckSum = calculateCheckSum(5, LinTransportRequest + 11, msgLength, pMsg); //Calculate over 5 bytes of message header then the message itself
	if (calCheckSum != msgCheckSum)
	{
		Log('e', "LinThis downloadBB checksum expected %02X but had %02X", calCheckSum, msgCheckSum);
		return;
	}
	switch (msgType)
	{
		case 0x33: downloadStatus    (msgLength, pMsg); return;
		case 0x0D: downloadAck       (msgLength, pMsg); return;
		case 0x0B: downloadDeviceList(msgLength, pMsg); return;
		default:
			Log('e', "LinThis downloadBB unknown message type %02X", msgType);
			return;			
	}
}

static void transportHandler() //Called by lin-transport.c
{
	if (LinTransportRequestLength <= 0) return; //Only handle actual messages
	switch (LinTransportNad)
	{
		case 0x01: return; //Combi - just ignore
		case 0x03: break;  //Me
		case 0x7F: break;  //Everyone
		default:
		{
			Log('e', "LinThis transportHandler unknown NAD %02X", LinTransportNad);
			char text[1000];
			char* p = text;
			p += sprintf(p, "<%02X> Transport len %2d      <<<<<", LinGetFrameId(), LinTransportRequestLength);
			for (int i = 0; i < LinTransportRequestLength; i++) p += sprintf(p, " %02X", LinTransportRequest[i]);
			Log('d', text);
			return;
		}
	}
	_requestSid = LinTransportRequest[0];

	char doTrace = 0;
	switch(_requestSid)
	{
		case 0xB2: doTrace = TRACE_SID_B2_READ_BY_ID;      break;
		case 0xB9: doTrace = TRACE_SID_B9_HEARTBEAT;       break;
		case 0xBA: doTrace = TRACE_SID_BA_UPLOAD_COMMAND;  break;
		case 0xBB: doTrace = TRACE_SID_BB_DOWNLOAD_STATUS; break;
		default:   doTrace = 1;                            break;
	}
	if (LinTrace && doTrace)
	{
		char text[1000];
		char* p = text;
		p += sprintf(p, "<%02X> Transport len %2d      <<<<<", LinGetFrameId(), LinTransportRequestLength);
		for (int i = 0; i < LinTransportRequestLength; i++) p += sprintf(p, " %02X", LinTransportRequest[i]);
		Log('d', text);
	}

	switch(_requestSid)
	{
		case 0xB2: readById();      break;
		case 0xB9: heartbeat();     break;
		case 0xBA: uploadCommand(); break;
		case 0xBB: downloadBB();    break;
		default: Log('e', "Unhandled request sid %02X", _requestSid); break;
	}
}
static void dataHandler() //Called by lin.c though no frames are sent to the iNet box
{
	char text[1000];
	char* p = text;
	char frameId = LinGetFrameId();
	switch (frameId)
	{
		case 0x03: break; //Contains AA 0A FF FF FF FF FF FF Combi to CP setting Heating : AA=off; 
		case 0x04: break; //Contains AA 0A FF FF FF FF FF FF Combi to CP setting Hot water: AA=off
		case 0x05: break; //Contains 02 FF FF FF FF FF FF FF Combi to CP setting Energy mix: 01=Gas, 02=EL1/EL2, 03=Mix1/Mix2
		case 0x06: break; //Contains 08 07 FF FF FF FF FF FF
		case 0x07: break; //Contains 00 00 FF FF FF FF FF FF
		case 0x08: break; //Sent from CP to Combi when temperature setting is changed, contains set temperature + 273 in 1/10s degree
		case 0x09: break; //Contains FF FF FF FF FF FF FF FF
		case 0x16: break; //Combi to CP status 00 07 34 0B 44 0B FF 7F air 13.7C water 15.3C
		default:
			p += sprintf(p, "Unhandled id %02x: ", frameId);
			for (int i = 0; i < LinDataLength; i++) p += sprintf(p, " %02X", *(LinGetDataPointer() + i));
			Log('d', text);
			break;
	}
}

void LinThisInit()
{
	LinUnhandledBytesHandler = unhandledBytesHandler;
	LinIdHandler             =             idHandler;
	LinDataHandler           =           dataHandler;
	LinTransportHandler      =      transportHandler;
	_commandId = (uint8_t)time(0); //Put a random starting id
	LinInit();
	TrumaInit();
}
void LinThisReceive()
{
	LinReadOrWait();
}