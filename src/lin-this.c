#include <stdio.h>

#include "lib/lin.h"
#include "lib/log.h"

char LinThisTrace = 0;

//static char _buffer[8];

static void unhandledBytesHandler()
{
	if (LinThisTrace && LinGetUnhandledBytesCount())
	{
		char text[100];
		char* p = text;
		p += sprintf(p, "%d unhandled bytes: ", LinGetUnhandledBytesCount());
		for (int i = 0; i < LinGetUnhandledBytesCount(); i++) p += sprintf(p, " %02X", *(LinGetUnhandledBytes() + i));
		Log('d', text);
	}
}

static void idHandler()
{
	char frameId = LinGetFrameId();
	if (LinThisTrace) Log('d', "Handling id %02X", frameId);
	
	if (frameId == 0x0A || frameId == 0x3d) LinDataLength =  3;
	else                 LinDataLength = -1;
}
static void dataHandler()
{
	if (LinThisTrace)
	{
		char text[100];
		char* p = text;
		p += sprintf(p, "Received %d bytes of data for id %02X:", LinDataLength, LinGetFrameId());
		for (int i = 0; i < LinDataLength; i++) p += sprintf(p, " %02X", *(LinGetDataPointer() + i));
		p += sprintf(p, ": checksum %s", LinGetCheckSum() == LinGetCheckSumWithId(LinGetProtectedId(), LinDataLength, LinGetDataPointer()) ? "ok" : "not ok");
		Log('d', text);
	}
}

void LinThisInit()
{
	LinUnhandledBytesHandler = unhandledBytesHandler;
	LinIdHandler             =             idHandler;
	LinDataHandler           =           dataHandler;
	LinInit();
}
void LinThisReceive()
{
	LinReadOrWait();
}
void LinThisPoll()
{
	char data[3] = { 0x55, 0x93, 0xe5 };
	char id      = LinAddIdParity(0x0a);
	char sync    = 0x55;
	char cs      = LinGetCheckSumWithId(id, sizeof(data), data);
	LinSendBreak();
	LinSend(1, &sync);
	LinSend(1, &id);
	LinSend(sizeof(data), &data);
	LinSend(1, &cs);
}