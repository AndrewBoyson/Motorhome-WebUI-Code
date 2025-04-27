#include <stdlib.h> //system
#include <string.h> //strcat
#include <stdio.h>  //sprintf
#include <unistd.h> //sleep
#include <stdarg.h> //variable arguments

#include "lib/log.h"
#include "lib/socket.h"
#include "lib/settings.h"
#include "can-this.h"
#include "battery.h"
#include "alert.h"

#define BUFFER_SIZE 1000
#define DEBUG_LOG_LEVEL 'i'

static char _username[50];
static char _password[50];
static char _hostname[50];

void SmsSetUserName(char* text)
{
	strncpy(_username, text, sizeof(_username));
	SettingsSetString("SmsUsername", _username);
}
char* SmsGetUserName(void)
{
	return _username;
}
void SmsSetPassword(char* text)
{
	strncpy(_password, text, sizeof(_password));
	SettingsSetString("SmsPassword", _password);
}
char* SmsGetPassword()
{
	return _password;
}
void SmsSetHostname(char* text)
{
	strncpy(_hostname, text, sizeof(_hostname));
	SettingsSetString("SmsHostname", _hostname);
}
char* SmsGetHostname()
{
	return _hostname;
}
void SmsInit()
{
	SettingsGetString("SmsUsername", _username, sizeof(_username));
	SettingsGetString("SmsPassword", _password, sizeof(_password));
	SettingsGetString("SmsHostname", _hostname, sizeof(_hostname));
}

int urlQueryEncode(char* to, char* from) //Returns the number of characters in the 'to' buffer (not including the end NUL
{
	char* pFrom = from;
	char* pTo = to;
	while (1)
	{
		switch (*pFrom)
		{
			case  0 : *pTo = 0;                                 return pTo - to;
			case ' ': *pTo++ = '+';   		                    break;
			case '%': *pTo++ = '%'; *pTo++ = '2'; *pTo++ = '5'; break;
			case '&': *pTo++ = '%'; *pTo++ = '2'; *pTo++ = '6'; break;
			case '+': *pTo++ = '%'; *pTo++ = '2'; *pTo++ = 'B'; break;
			case '=': *pTo++ = '%'; *pTo++ = '3'; *pTo++ = 'D'; break;
			default : *pTo++ = *pFrom;
		}
		pFrom++;
	}
}
static char isValidMobileNumber(char* number)
{
/*
Can be 07xxx xxxxxx or +447
*/
	if (!number) return 0;
	
	int i = 0;
	switch (number[i])
	{
		case '0':
			break;
		case '+':
			i++; if (number[i] != '4') return 0; //UK code
			i++; if (number[i] != '4') return 0; //UK code
			break;
		default:
			return 0;
	}
	i++; if (number[i] != '7'                  ) return 0;
	i++; if (number[i] < '0' || number[i] > '9') return 0; //A short code will have a NUL char
	i++; if (number[i] < '0' || number[i] > '9') return 0;
	i++; if (number[i] < '0' || number[i] > '9') return 0;
	
	i++; if (number[i] < '0' || number[i] > '9') return 0;
	i++; if (number[i] < '0' || number[i] > '9') return 0;
	i++; if (number[i] < '0' || number[i] > '9') return 0;
	i++; if (number[i] < '0' || number[i] > '9') return 0;
	i++; if (number[i] < '0' || number[i] > '9') return 0;
	i++; if (number[i] < '0' || number[i] > '9') return 0;
	i++; if (number[i] != 0                    ) return 0; //NUL char at end of string
	
	return 1;
}

void SmsSend(char* number, char* text)
{
	
	if (!number)
	{
		Log('e', "SmsSend - Sms number is not set");
		return;
	}
	if (!text)
	{
		Log('e', "SmsSend - Sms text is not set");
		return;
	}
	if (!isValidMobileNumber(number))
	{
		Log('e', "SmsSend - Sms number '%s' is not valid", number);
		return;
	}
	Log(DEBUG_LOG_LEVEL, "SmsSend number '%s', text '%s'\r\n", number, text);
	
	char buffer[5000];
	int  length = 0;
	char data[4000];
	char dataLength[10];
	char token[100];
	char *p ;
	int sfd = 0;
	
	//Get token
	sfd = TcpMakeTalkingSocket(_hostname, "80", 100);
	if (sfd == -1)
	{
		Log('e', "SmsSend - Could not connect to sms server '%s'", _hostname);
		return;
	}
	Log(DEBUG_LOG_LEVEL, "Opened socket %d\r\n", sfd);
	p = data;
	*p = 0;
	p = stpcpy(p, "{\"username\":\"");
	p = stpcpy(p, _username);
	p = stpcpy(p, "\",\"password\":\"");
	p = stpcpy(p, _password);
	p = stpcpy(p, "\"}");
	sprintf(dataLength, "%d", strlen(data));
	
	p = buffer;
	*p = 0;
	p = stpcpy(p, "POST /api/login HTTP/1.0\r\n");
	p = stpcpy(p, "Content-Type: application/json\r\n");
	p = stpcpy(p, "Content-Length: ");
	p = stpcpy(p, dataLength);
	p = stpcpy(p, "\r\n");
	p = stpcpy(p, "\r\n");
	p = stpcpy(p, data);
	Log(DEBUG_LOG_LEVEL, "Sent '%s'\r\n", buffer);
	TcpSendString(sfd, buffer);
	buffer[0] = 0;
	length = TcpRecvAll(sfd, buffer, sizeof(buffer));
	Log(DEBUG_LOG_LEVEL, "Received %d characters from sms '%.*s'\r\n", length, length, buffer);
	char *pToken = strstr(buffer, "\"token\":\"") + 9;
	p = pToken;
	while (p)
	{
		if (!*p) break;
		if (*p == '"')
		{
			*p = 0;
			break;
		}
		p++;
	}
	strcpy(token, pToken);
	
	Log(DEBUG_LOG_LEVEL, "Token is '%s'\r\n", token);
	
	//Close socket
	TcpClose(sfd);
	
	//Send sms
	sfd = TcpMakeTalkingSocket(_hostname, "80", 100);
	if (sfd == -1)
	{
		Log('e', "SmsSend - Could not connect to sms server '%s'", _hostname);
		return;
	}
	Log(DEBUG_LOG_LEVEL, "Opened socket %d\r\n", sfd);
	p = data;
	*p = 0;
	p = stpcpy(p, "{\"data\":{\"number\":\"");
    p = stpcpy(p, number);
	p = stpcpy(p, "\",\"message\":\"");
    p = stpcpy(p, text);
	p = stpcpy(p, "\",\"modem\":\"2-1\"}}");
	sprintf(dataLength, "%d", strlen(data));
	
	p = buffer;
	*p = 0;
	p = stpcpy(p, "POST /api/messages/actions/send HTTP/1.0\r\n");
	p = stpcpy(p, "Content-Type: application/json\r\n");
	p = stpcpy(p, "Authorization: Bearer ");
	p = stpcpy(p, token);
	p = stpcpy(p, "\r\n");
	p = stpcpy(p, "Content-Length: ");
	p = stpcpy(p, dataLength);
	p = stpcpy(p, "\r\n");
	p = stpcpy(p, "\r\n");
	p = stpcpy(p, data);
	Log(DEBUG_LOG_LEVEL, "Sent '%s'\r\n", buffer);
	TcpSendString(sfd, buffer);
	buffer[0] = 0;
	length = TcpRecvAll(sfd, buffer, sizeof(buffer));
	Log(DEBUG_LOG_LEVEL, "Received %d characters from sms '%.*s'\r\n", length, length, buffer);
	
	//Close socket
	TcpClose(sfd);
}

void SmsSendV(char* number, const char *format, va_list args)
{
 	char buffer[BUFFER_SIZE];
	vsnprintf(buffer, sizeof(buffer), format, args);
	SmsSend(number, buffer);
}
void SmsSendF(char* number, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	SmsSendV(number, format, args);
	va_end(args);
}
static void sendStatus(char* number)
{
	SmsSendF(
		number,
		"Status:\n"
		"Battery %3.1f%%\n"
		"Water %d litres\n"
		"Lpg %d litres\n"
		"Water pump %s\n"
		"Water fill %s\n"
		"Water drain %s\n"
		"Inverter %s\n"
		"Lpg heater %s\n"
		"EHU %s\n"
		"Mode %s",
		(float)CanThisGetBatteryCountedCapacityAs() / (BATTERY_CAPACITY_AH * 36),
		CanThisGetTankFreshLitres(),
		CanThisGetTankLpgVolumeMl() / 1024,
		CanThisGetControlWaterPump()  ? "on" : "off",
		CanThisGetControlWaterFill()  ? "on" : "off",
		CanThisGetControlWaterDrain() ? "on" : "off",
		CanThisGetControlInverter()   ? "on" : "off",
		CanThisGetControlLpgHeater()  ? "on" : "off",
		CanThisGetControlEhu()        ? "on" : "off",
		BatteryGetModeAsString()
	);
}
static void sendBattery(char* number)
{
	SmsSendF(number,
		"Battery:\n"
		"Counted capacity %3.1f%%\n"
		"Target capacity %d%%\n"
		"Current %dmA\n"
		"Temperature %2.1f°\n"
		"State %c\n",
		(float)CanThisGetBatteryCountedCapacityAs() / (BATTERY_CAPACITY_AH * 36),
		CanThisGetBatteryCapacityTargetPercent(),
		CanThisGetBatteryCurrentMa(),
		(float)CanThisGetBatteryTemperature8bfdp() / 256,
		CanThisGetBatteryOutputState()
	);
}
static void sendHelp(char* number)
{
	SmsSend(number,
		"Help:\n"
		"help\n"
		"status\n"
		"battery\n"
		"target 20-90\n"
		"pump on|off\n"
		"fill on|off\n"
		"drain on|off\n"
		"inverter on|off\n"
		"lpgheater on|off\n"
		"mode home|away|manual\n"
	);
}
static int parseBool(char* sValue) //Returns -1 if not understood, 1 if on and 0 if off
{
	if (strcasecmp(sValue, "on"   ) == 0) return 1;
	if (strcasecmp(sValue, "off"  ) == 0) return 0;
	if (strcasecmp(sValue, "1"    ) == 0) return 1;
	if (strcasecmp(sValue, "0"    ) == 0) return 0;
	if (strcasecmp(sValue, "true" ) == 0) return 1;
	if (strcasecmp(sValue, "false") == 0) return 0;
	return -1;
}
static void setTarget(char* number, char* sValue)
{
	int iValue = atoi(sValue);
	if (iValue > 90 || iValue < 20)
	{
		SmsSendF(number, "Target %d%% outside 20 to 90", iValue);
		return;
	}
	CanThisSetBatteryCapacityTargetPercent((uint8_t)iValue);
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendBattery(number);
}
static char setPump(char* number, char* sValue)
{
	int iValue = parseBool(sValue);
	if (iValue < 0) return -1;
	CanThisSetControlWaterPump((char)iValue);
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendStatus(number);
	return 0;
}
static char setFill(char* number, char* sValue)
{
	int iValue = parseBool(sValue);
	if (iValue < 0) return -1;
	CanThisSetControlWaterFill((char)iValue);
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendStatus(number);
	return 0;
}
static char setDrain(char* number, char* sValue)
{
	int iValue = parseBool(sValue);
	if (iValue < 0) return -1;
	CanThisSetControlWaterDrain((char)iValue);
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendStatus(number);
	return 0;
}
static char setInverter(char* number, char* sValue)
{
	int iValue = parseBool(sValue);
	if (iValue < 0) return -1;
	CanThisSetControlInverter((char)iValue);
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendStatus(number);
	return 0;
}
static char setLpgHeater(char* number, char* sValue)
{
	int iValue = parseBool(sValue);
	if (iValue < 0) return -1;
	CanThisSetControlLpgHeater((char)iValue);
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendStatus(number);
	return 0;
}
static char setMode(char* number, char* sValue)
{
	if (BatterySetModeAsString(sValue)) return -1;
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendStatus(number);
	return 0;
}
static void splitRequest(char* request, int commandSize, char* command, int paramSize, char* param) //Returns 0 on success
{
	if (!request)     return;
	if (!command)     return;
	if (!param)       return;
	if (!commandSize) return;
	if (!paramSize  ) return;
	
	char* pCommand = command;
	char* pParam   = param;
	
	*pCommand = 0;
	*pParam = 0;
	
	//Trim leading white space
	while(1)
	{
		char c = *request;
		if (c ==  0 ) return;  //Normal end so finish
		if (c != ' ') break;   //Found non white space so move on
		request++;
	}
	
	//Read the command
	while(1)
	{
		char c = *request;
		if (c ==  0 ) return; //Normal end so finish
		if (c == ' ') break;  //Found white space so move on
		
		if (pCommand < command + commandSize - 1)
		{
			*pCommand = c;
			 pCommand++;
			*pCommand = 0;
		}
		request++;
	}
	
	//Trim white space
	while(1)
	{
		char c = *request;
		if (c ==  0 ) return; //Normal end so finish
		if (c != ' ') break;  //Found non white space so move on
		request++;
	}
	
	//Read the parameter
	while(1)
	{
		char c = *request;
		if (c ==  0 ) return; //Normal end so finish
		if (c == ' ') break;  //Found white space so move on
		
		if (pParam < param + paramSize - 1)
		{
			*pParam = c;
			 pParam++;
			*pParam = 0;
		}
		request++;
	}
}
void SmsHandleRequest(char* number, char* request)
{	
	char command[20];
	char param[20];
	splitRequest(request, sizeof(command), command, sizeof(param), param);
	
	     if (strcasecmp(command, "status"   ) == 0) {      sendStatus  (number       ); return; } //Handles its own error response
	else if (strcasecmp(command, "battery"  ) == 0) {      sendBattery (number       ); return; } //Handles its own error response
	else if (strcasecmp(command, "help"     ) == 0) {      sendHelp    (number       ); return; } //Handles its own error response
	else if (strcasecmp(command, "target"   ) == 0) {      setTarget   (number, param); return; } //Handles its own error response
	else if (strcasecmp(command, "pump"     ) == 0) { if (!setPump     (number, param)) return; }
	else if (strcasecmp(command, "fill"     ) == 0) { if (!setFill     (number, param)) return; }
	else if (strcasecmp(command, "drain"    ) == 0) { if (!setDrain    (number, param)) return; }
	else if (strcasecmp(command, "inverter" ) == 0) { if (!setInverter (number, param)) return; }
	else if (strcasecmp(command, "lpgheater") == 0) { if (!setLpgHeater(number, param)) return; }
	else if (strcasecmp(command, "mode"     ) == 0) { if (!setMode     (number, param)) return; }
	Log('w', "SMS '%s' sent '%s' command '%s' param '%s'", number, request, command, param);
	SmsSendF(number, "Sorry, I don't understand '%s'", request);
	
}