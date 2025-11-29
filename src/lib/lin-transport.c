#include <stdio.h>

#include "lin.h"
#include "log.h"

void (*LinTransportHandler)(void) = 0; //Set by LinThisInit; called when a transport message is ready

char LinTransportNad = 0;
char LinTransportSid = 0;

char LinTransportRequest[1000];
int  LinTransportRequestLength = 0;

void LinTransportHandleRequestPacket()
{
	static int _requestIndex = 0;
	
	char text[1000];
	char* p = text;
	int i = 0;
	
	char nad = *(LinGetDataPointer() + i++);
	
	char pci = *(LinGetDataPointer() + i++);
	char pciType = pci >> 4;
	char pciLen  = pci & 0x0f;
	
	char lenLsb = 0;
	if (pciType == 0x0) //SF
	{
		LinTransportNad = nad;
		LinTransportRequestLength = pciLen;
		_requestIndex = 0;
	}
	if (pciType == 0x1) //FF
	{
		lenLsb = *(LinGetDataPointer() + i++);
		LinTransportNad = nad;
		LinTransportRequestLength = ((int)pciLen << 8) + lenLsb;
		_requestIndex = 0;
	}
	switch (pciType)
	{
		case 0x0:
			for (int j = 0; j < pciLen; j++)
			{
				LinTransportRequest[_requestIndex++] = *(LinGetDataPointer() + i);
				i++;
			}
			break;
		case 0x1:
			for (int j = 0; j < 5; j++)
			{
				LinTransportRequest[_requestIndex++] = *(LinGetDataPointer() + i);
				i++;
			}
			break;
		case 0x2:
			for (int j = 0; j < 6; j++)
			{
				LinTransportRequest[_requestIndex++] = *(LinGetDataPointer() + i);
				i++;
			}
			break;
		default:
			for (int j = 0; j < 7; j++) p += sprintf(p, ", %02X", *(LinGetDataPointer() + i++));
			return;
	}
	
	if (_requestIndex >= LinTransportRequestLength)
	{
		LinTransportSid = LinTransportRequest[0];
		LinTransportHandler();
	}
	
}
//Response section
char LinTransportResponse[1000];
static int _responseLength = 0;
static char _responseTrace = 0;
static int _responseFrameCounter = 0;

void LinTransportSetResponseLengthAndTrace(int length, char trace) //Length includes SID
{
	_responseLength       = length; //Length includes SID
	_responseTrace        = trace;
	_responseFrameCounter = 0;
}
void LinTransportHandleResponsePacket()
{
	static int _responseIndex = 0;
	if (!_responseLength) return;
	
	char packet[8];
	for (int i = 0; i < 8; i++) packet[i] = 0xFF;
	
	if (_responseLength <= 6) //SF
	{
		_responseIndex = 0;
		packet[0] = LinTransportNad; //NAD
		packet[1] = _responseLength; //PCI
		for (int i = 0; i < _responseLength; i++) packet[i + 2] = LinTransportResponse[_responseIndex++];
	}
	else
	{
		if (!_responseFrameCounter) //FF
		{
			_responseIndex = 0;
			packet[0] = LinTransportNad; //NAD
			packet[1] = 0x10 + (_responseLength >> 8); //PCI
			packet[2] = _responseLength & 0xFF;
			for (int i = 0; i < 5; i++) packet[i + 3] = LinTransportResponse[_responseIndex++];
		}
		else //CF
		{
			packet[0] = LinTransportNad; //NAD
			packet[1] = 0x20 + _responseFrameCounter; //PCI
			for (int i = 0; i < 6; i++) packet[i + 2] = LinTransportResponse[_responseIndex++];
		}
	}
	LinSendFrameResponse(8, packet, _responseTrace);
	_responseFrameCounter++;
	if (_responseIndex >= _responseLength) _responseLength = 0; //Were done, no more packets to send
}