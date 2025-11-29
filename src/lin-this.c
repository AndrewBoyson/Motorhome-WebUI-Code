#include <stdio.h>
#include <time.h>

#include "lib/lin.h"
#include "lib/lin-transport.h"
#include "lib/log.h"

#include "truma.h"
#include "lin-this.h"
#include "lin-this-upload.h"
#include "lin-this-download.h"

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
static void pollChangesToSend()
{
	char sendWanted = LinThisUploadPollSendWanted();
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
		case 0x18: pollChangesToSend(); LinDataLength = -1; break;
		default:                        LinDataLength =  8; break;
	}
}
char LinThisCalculateCheckSum(int headerLen, char* pHeader, int bufferLen, char* pBuffer)
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
			LinTransportResponse[0] = LinTransportSid + 0x40;
			LinTransportResponse[1] = 0x17; //Device id 4617
			LinTransportResponse[2] = 0x46;
			LinTransportResponse[3] = 0x00; //Hardware revision
			LinTransportResponse[4] = 0x1F;
			LinTransportResponse[5] = 0x00; //Variant
			LinTransportNad = 0x03; //Set NAD to me - it might have been 0x7F
			LinTransportSetResponseLengthAndTrace(6, TRACE_SID_B2_READ_BY_ID);  //6 bytes
			break;
		case 0x20:
			LinTransportResponse[0] = LinTransportSid + 0x40;
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
	LinTransportResponse[0] = LinTransportSid + 0x40;
	LinTransportResponse[1] = 0x00;
	LinTransportSetResponseLengthAndTrace(2, TRACE_SID_B9_HEARTBEAT);  //Just one byte of zero
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

	char doTrace = 0;
	switch(LinTransportSid)
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

	switch(LinTransportSid)
	{
		case 0xB2: readById();        break;
		case 0xB9: heartbeat();       break;
		case 0xBA: LinThisUpload();   break;
		case 0xBB: LinThisDownload(); break;
		default: Log('e', "Unhandled request sid %02X", LinTransportSid); break;
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
	LinInit();
	LinThisUploadInit();
	TrumaInit();
}
void LinThisReceive()
{
	LinReadOrWait();
}