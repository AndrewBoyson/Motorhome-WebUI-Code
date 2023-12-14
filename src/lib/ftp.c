#include <stdio.h>   //fopen printf
#include <errno.h>   //errno
#include <string.h>  //strerror
#include <stdlib.h>  //free strdupa
#include <pthread.h> //rwlock
#include <stdint.h>  //uint64_t

#include <sys/socket.h> //connect read write
#include <unistd.h>     //close
#include <netdb.h>      //addrinfo
#include <arpa/inet.h>  //inet_ntop
#include <stdarg.h>     //va_list, va_start, va_end, ...
#include <sys/stat.h>   //File date
#include <ctype.h>      //isdigit

#include "log.h"
#include "socket.h"
#include "ftp.h"
#include "file.h"

#define RECEIVE_BUF_SIZE     1000
#define MAX_COMMAND_LENGTH    256
#define MAX_IP_LENGTH          64
#define SOCKET_TIMEOUT_SECS    10

static char receivebuf[RECEIVE_BUF_SIZE];

static void   parsePASV(char ** ip, unsigned short * port) {
//227 Entering Passive Mode (128,138,140,44,161,238)
//                              1   2   3  4   5
//strtol reads up to first non valid character
	int commaCount = 0;
	for (char *p = receivebuf; *p != 0; p++)
	{
		if (*p == '(') *ip = ++p; //Record first character after the '(' as the start of the ip address

		if (*p == ',')
		{
			switch (++commaCount)
			{
				case 1:
				case 2:
				case 3: 
					*p = '.'; //Change commas to dots for the ip address
					break;
				case 4:
					*p =  0 ; //Terminate ip address
					*port = strtol(++p, NULL, 10) << 8; //Load the high byte of the port
					break;
				case 5:
					*port += strtol(++p, NULL, 10); //Load the low byte of the port
					return;
			}
		}
	}
}
static char*  parseEPSV() {
//229 Entering Extended Passive Mode (|||6446|)
//                                    123^   4
//                                       ^
//                                       ^port
	int delimCount = 0;
	char * port;
	for (char *p = receivebuf; *p != 0; p++)
	{
		if (*p == '|' )
		{
			switch (++delimCount)
			{
				case 3:
					port = p + 1; //Point the port field to the 3rd delimiter plus one
					break;
				case 4:
					*p = 0;       //Terminate the port field by replacing the 4th delimeter with a zero
					return port;
			}
		}
	}
	Log('e', "parseEPSV - Did not find port");
	return 0;
}
static time_t parseMDTM(void) {
	//213 20120807154341
	//012345678901234567
	//0         1
	char start[5];
	struct tm t;
	if (sscanf(receivebuf,
		"%4c%4d%2d%2d%2d%2d%2d",
		&start[0],
		&t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec) == EOF)
	{
		LogErrno("Error sscanf");
		return 0;
	} 
	t.tm_mon  -= 1;    // month is 0 to 11 while day is 1 to 31!!!
	t.tm_year -= 1900; // struct tm year is from 1900
	return mktime(&t);
}

static int   readFEAT(int sck, int *pSupportsPASV, int *pSupportsEPSV, int *pSupportsMDTM) {
/*
-->FEAT
<--211-Features:
<-- EPRT
<-- EPSV
<-- MDTM
<-- PASV
<-- REST STREAM
<-- SIZE
<-- TVFS
<--211 End
*/
	char* pN = receivebuf;            //Pointer to the next unused location in the buffer
	char* pE = pN + RECEIVE_BUF_SIZE; //Pointer to just beyond the end of the buffer.
	char* pL = pN;                    //Pointer to the start of the current line

	*pSupportsPASV = 0;
	*pSupportsEPSV = 0;
	*pSupportsMDTM = 0;


	while (1)
	{
		char* pS = pN;                //Pointer to the start of the unread data in the buffer
		int bytes = TcpRecv(sck, pS, pE - pS);
		if (bytes == 0)
		{
			Log('e', "readFEAT-TcpRecv - Socket closed unexpectedly by remote server");
		}
		if (bytes < 0) return -1;
		pN = pS + bytes;

		for (char*p = pS; p < pN; p++)
		{
			if (*p == '\r') *p = 0;
			if (*p == '\n')
			{
				Log('i', "<-- %s", pL);
				if (isdigit(*(pL + 0))) //First character is a digit
				{
					if (isdigit(*(pL + 1)) && isdigit(*(pL + 2)) && *(pL + 3) == ' ') //Finish if have space after a three digit status
					{
						if (*pL == '2') return 0;
						Log('e', "readFEAT - Wrong response");
						return -1;
					}
				}
				else //This is a continuation line
				{
					if (*(pL + 0) == ' ')
					{
						if (memcmp(pL + 1, "PASV", 4) == 0) *pSupportsPASV = 1;
						if (memcmp(pL + 1, "EPSV", 4) == 0) *pSupportsEPSV = 1;
						if (memcmp(pL + 1, "MDTM", 4) == 0) *pSupportsMDTM = 1;
					}
				}
				pL = p + 1; //Set start of current line
			}
		}
	}
}

static int  readControl(int sck, char expected) { //returns 0 on success or -1 if not

	char* pN = receivebuf;            //Pointer to the next unused location in the buffer
	char* pE = pN + RECEIVE_BUF_SIZE; //Pointer to just beyond the end of the buffer.
	char* pL = pN;                    //Pointer to the start of the current line
	

	while (1)
	{
		char* pS = pN;                //Pointer to the start of the unread data in the buffer
		int bytes = TcpRecv(sck, pS, pE - pS);
		if (bytes == 0)
		{
			Log('e', "readControl-TcpRecv - Socket closed unexpectedly by remote server");
		}
		if (bytes < 0) return -1;
		pN = pS + bytes;

		for (char*p = pS; p < pN; p++)
		{
			if (*p == '\r') *p = 0;
			if (*p == '\n')
			{
				Log('i', "<-- %s", pL);
				if (isdigit(*(pL + 0)) && isdigit(*(pL + 1)) && isdigit(*(pL + 2)) && *(pL + 3) == ' ') //Finish if have space after a three digit status
				{
					if (*pL == expected) return 0;
					Log('e', "readControl - Wrong response");
					return -1;
				}
				pL = p + 1; //Set start of current line
			}
		}
	}
}
static int vsendControl(int sck, char *format, va_list args) {
	char text[MAX_COMMAND_LENGTH];
	vsprintf(text, format, args); //Make the command from the arguments
	
	if (TcpSendString(sck, text  )) return -1;
	if (TcpSendString(sck, "\r\n")) return -1;
	Log('i', "--> %s", text);
	return 0;
}
static int  sendControl(int sck, char *format, ...) {
	va_list args;           //Create an empty argument list
	va_start(args, format); //Initiate the argument list from the last fixed argument

	if (vsendControl(sck, format, args)) { va_end(args); return -1; }
	
	va_end(args);
	return 0;
}
static int  xchgControl(int sck, char expected, char *format, ...) {
	va_list args;           //Create an empty argument list
	va_start(args, format); //Initiate the argument list from the last fixed argument
	
	if (vsendControl(sck, format, args)) { va_end(args); return -1; }
	if ( readControl(sck, expected    )) { va_end(args); return -1; }
	
	va_end(args);	
	return 0;
}

int    FtpLogin      (char *host, int *pSck) {
	int supportsPASV;
	int supportsEPSV;
	int supportsMDTM;

	*pSck = TcpMakeTalkingSocket(host, "21", SOCKET_TIMEOUT_SECS);
	if (pSck < 0)                                                        return FTP_NOT_CONNECTED;
	if (readControl(*pSck, '2'                                        )) return FTP_CONNECTED;
	if (xchgControl(*pSck, '3', "USER anonymous"                      )) return FTP_CONNECTED;
	if (xchgControl(*pSck, '2', "PASS guest"                          )) return FTP_CONNECTED;
	
	if (sendControl(*pSck,      "FEAT"                                )) return FTP_CONNECTED;
	if (readFEAT   (*pSck, &supportsPASV, &supportsEPSV, &supportsMDTM)) return FTP_CONNECTED;
	Log('i', "--- PASV = %d", supportsPASV);
	Log('i', "--- EPSV = %d", supportsEPSV);
	Log('i', "--- MDTM = %d", supportsMDTM);
	
	return FTP_LOGGED_IN;
}
void   FtpLogout     (int sck, int state) {
	if (state >= FTP_LOGGED_IN) xchgControl(sck, '2', "QUIT");
	if (state >= FTP_CONNECTED) TcpClose(sck);
}
int    FtpCwd        (int sck, char *path) {
	if (xchgControl(sck, '2', "CWD /%s", path )) return -1;
	return 0;
}
time_t FtpGetFileTime(int sck, char *file) {
	if (xchgControl(sck, '2', "MDTM %s", file     )) return -1;
	return parseMDTM();
}
int    FtpDownload   (int sck, char *file, char *localfilename) {
	if (xchgControl(sck, '2', "TYPE A"       )) return -1;
	if (xchgControl(sck, '2', "EPSV"         )) return -1;
	
	struct sockaddr_storage sockaddr;
	if (SocketRemoteAddress(sck, &sockaddr)) return -1;
	
	char ip[MAX_IP_LENGTH];
	if (SocketIpAsString(sockaddr, ip, MAX_IP_LENGTH)) return -1;
	char* port = parseEPSV();
	if (port == 0) return -1;
	
	Log('i', "--- Connecting to remote server %s on port %s", ip, port);
	int sckData = TcpMakeTalkingSocket(ip, port, SOCKET_TIMEOUT_SECS);
	if (sckData < 0) return -1;
	

	if (xchgControl   (sck, '1', "RETR %s", file)) { TcpClose(sckData); return -1; }
	
	int bytes = TcpDownload(sckData, receivebuf, RECEIVE_BUF_SIZE, localfilename);
	if (bytes < 0) { TcpClose(sckData); return -1; }
	Log('i', "<++ %d bytes read from data port into file %s", bytes, localfilename);

	if (readControl(sck, '2'                 )) return -1;
	if (TcpClose   (sckData                  )) return -1;

	return 0;
}
