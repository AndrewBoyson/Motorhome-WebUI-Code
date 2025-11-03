#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/serial.h> //defines serial_icounter_struct
#include <time.h>

#include "lin.h"
#include "log.h"

void (*LinUnhandledBytesHandler)(void) = 0; //Set by LinThisInit; called at the start of a new frame
void (*LinIdHandler            )(void) = 0; //Set by LinThisInit; called when a valid id has been received
void (*LinDataHandler          )(void) = 0; //Set by LinThisInit; called when LinDataLength bytes have been received

int    LinDataLength = 0;         //Set by LinIdHandler - must be -1 if id is not handled or zero if no bytes to read

static char   _recvBuffer[100];
static int    _recvBytesCount = 0;

static int  _serialDescriptor = 0;
static char _initialised = 0;
static struct serial_icounter_struct _lastCounters;

int LinGetUnhandledBytesCount()
{
	return _recvBytesCount;
}
char* LinGetUnhandledBytes()
{
	return _recvBuffer;
}
char LinGetProtectedId() //This is the frame id with parity
{
	if (!_recvBytesCount) return 0;
	return _recvBuffer[1];
}
char LinGetFrameId() //This is just the Pid with the parity stripped out
{
	if (!_recvBytesCount) return 0;
	return LinStripIdParity(_recvBuffer[1]);
}
char* LinGetDataPointer()
{
	if (!_recvBytesCount) return 0;
	return _recvBuffer + 2;
}
char LinGetCheckSum()
{
	if (!_recvBytesCount) return 0;
	return _recvBuffer[_recvBytesCount-1];
}

void LinInit      (void)                                 //Safe to be called multiple times
{
	_serialDescriptor = open("/dev/serial0", O_RDWR | O_NOCTTY); //RDWR opens for read and write, NOCTTY specifies the serial port not kill the process if ^C is received
	if (_serialDescriptor == -1)
	{
		LogErrno("LinInit descriptor open error");
		return;
	}
	struct termios options;
	bzero(&options, sizeof(options));

	options.c_cflag     =  B9600;   //9600 baud
	options.c_cflag    &= ~CSTOPB;  //One stop bit - CSTOPB sets two stop bits rather than one (superfluous but aids understanding)
	options.c_cflag    |=  CS8;     //8 bit word
	options.c_cflag    |=  CLOCAL;  //Ignore modem control lines
	options.c_cflag    |=  CREAD;   //Enable receiver
	options.c_oflag    &= ~OPOST;   //Raw output - OPOST enables implementation defined output processing (superfluous but aids understanding)
	options.c_iflag     =  IGNBRK;  //Ignore break
	options.c_cc[VTIME] =  0;       //Timeout in deciseconds for a raw read, 0 means no timeout.
	options.c_cc[VMIN ] =  1;       //Minimum number of characters for a raw read (MIN).
	tcflush(_serialDescriptor, TCIFLUSH);
	int r = tcsetattr(_serialDescriptor, TCSANOW, &options); //TCSANOW applies the attributes now
	if ( r < 0)
	{
		LogErrno("LinInit tcsetattr error");
		return;
	}

	int ret = ioctl(_serialDescriptor, TIOCGICOUNT, &_lastCounters);
	if (ret == -1)
	{
		LogErrno("LinInit ioctl TIOCGICOUNT error");
		return;
	}
	Log('e', "Lin is initialised on socket %d", _serialDescriptor);
	_initialised = 1;
}
static void getNextByte(char* pByte, char* hadBreak)
{
	int len = read(_serialDescriptor, pByte, 1);
	
	if (len == 0)
	{
		Log('e', "LinReadOrWait getNextByte no bytes read");
		return;
	}
	if (len < 0)
	{
		LogErrno("LinReadOrWait getNextByte read error");
		return;
	}

	struct serial_icounter_struct thisCounters;
	int ret = ioctl(_serialDescriptor, TIOCGICOUNT, &thisCounters);
	if (ret == -1)
	{
		LogErrno("LinReadOrWait getNextByte ioctl TIOCGICOUNT error");
		return;
	}
	*hadBreak = 0;
	if (thisCounters.brk != _lastCounters.brk) *hadBreak = 1;
	memcpy(&_lastCounters, &thisCounters, sizeof(thisCounters));
}
void  LinReadOrWait()  //Only called by the LIN thread and only after being initialised.
{
	#define AWAITING_BREAK    0
	#define AWAITING_ID       1
	#define AWAITING_DATA     2
	#define AWAITING_CHECKSUM 3
	static char _state = AWAITING_BREAK;
	static int  _dataCount = 0;
	char hadBreak = 0;
	char data = 0;
	getNextByte(&data, &hadBreak);
	if (hadBreak)
	{
		if (!LinUnhandledBytesHandler)
		{
			Log('e', "LinReadOrWait LinUnhandledBytesHandler not set");
			return;
		}
		LinUnhandledBytesHandler();
		_recvBytesCount = 0;
	}
	if (_recvBytesCount < sizeof(_recvBuffer)) _recvBuffer[_recvBytesCount++] = data;
	if (hadBreak)
	{
		if (data == 0x55) _state = AWAITING_ID;
		else              _state = AWAITING_BREAK;
		return;
	}
	if (_state == AWAITING_ID)
	{
		char parityOk = LinCheckIdParityIsOk(data);
		if (!parityOk)
		{
			_state = AWAITING_BREAK;
			return;
		}
			
		//LinPid = data;
		if (!LinIdHandler)
		{
			Log('e', "LinReadOrWait LinIdHandler not set");
			_state = AWAITING_BREAK;
			return;
		}
		LinIdHandler();
		_dataCount = 0;
		if (LinDataLength > 0) _state = AWAITING_DATA;
		else                   _state = AWAITING_BREAK;
		return;
	}
	
	if (_state == AWAITING_DATA)
	{
		_dataCount++;
		if (_dataCount >= LinDataLength)
		{
			_state = AWAITING_CHECKSUM;
			return;
		}
	}
	
	if (_state == AWAITING_CHECKSUM)
	{
		if (!LinDataHandler)
		{
			Log('e', "LinReadOrWait LinDataHandler not set");
			_state = AWAITING_BREAK;
			return;
		}
		LinDataHandler();
		_recvBytesCount = 0;
		_state = AWAITING_BREAK;
		return;
	}
	
	return;
}
int  LinSend      (int len, void* pData) //Returns 0 on success
{
	if (!_initialised)
	{
		Log('e', "LinSend - not initialised");
		return -1;
	}
	
	int bytesSent = write(_serialDescriptor, pData, len);
	if (bytesSent < 0)
	{
		LogErrno("LinSend write error");
		return -1;
	}
	
	return 0;
}
int LinSendBreak()
{
	struct timespec duration;

	duration.tv_sec  =       0 ;
	duration.tv_nsec = 1354167 ; //13 bits at 9600 baud is 1.3ms
	int ret = 0;
	ret = ioctl(_serialDescriptor, TIOCSBRK, 0);
	if (ret == -1)
	{
		LogErrno("LinSendBreak ioctl TIOCSBRK error");
		return -1;
	}
	
	nanosleep(&duration, 0);
	
	ret = ioctl(_serialDescriptor, TIOCCBRK, 0);
	if (ret == -1)
	{
		LogErrno("LinSendBreak ioctl TIOCCBRK error");
		return -1;
	}
	return 0;
}
/*
	Lower six bits (ID0-ID5) are data, upper two bits are parity
	The parity bits are calculated as follows:
	P0 =    ID0 xor ID1 xor ID2 xor ID4
	P1 = ! (ID1 xor ID3 xor ID4 xor ID5)
*/
static char calP0(char id) { return   !!(id & 0x01) ^ !!(id & 0x02) ^ !!(id & 0x04) ^ !!(id & 0x10) ; }
static char calP1(char id) { return !(!!(id & 0x02) ^ !!(id & 0x08) ^ !!(id & 0x10) ^ !!(id & 0x20)); }
static char actP0(char id) { return   !!(id & 0x40); }
static char actP1(char id) { return   !!(id & 0x80); }
char LinCheckIdParityIsOk(char id) //Returns 1 if ok, 0 if not
{
	return calP0(id) == actP0(id) && calP1(id) == actP1(id);
}
char LinStripIdParity(char id)
{
	return id & 0x3F;
}
char LinAddIdParity(char id)
{
	id &= 0x3F;
	char p0 = calP0(id);
	char p1 = calP1(id);
	if (p0) id |= 0x40;
	if (p1) id |= 0x80;
	return id;
}
char LinGetCheckSumWithId(char pid, int bufferLen, char* pBuffer)
{
	unsigned int cs = (unsigned char)pid;
	for (int i = 0; i < bufferLen; i++)
	{
		cs += (unsigned char)*(pBuffer + i);
		if (cs > 0xFF) cs -= 0xFF;
	}
	return (char)(~cs & 0xFF);
	
}
char LinGetCheckSumWithoutId(int bufferLen, char* pBuffer)
{
	return 0;
}
