#include <stdlib.h> //system
#include <string.h> //strcat
#include <stdio.h>  //sprintf
#include <unistd.h> //sleep

#include "lib/log.h"
#include "lib/socket.h"
#include "can-this.h"
#include "battery.h"
#include "alert.h"

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
	char buffer[1000];
	char* p = buffer;
	p = stpcpy(p, "GET /cgi-bin/sms_send?username=pi&password=pi&number=");
	p += urlQueryEncode(p, number);
	p = stpcpy(p, "&text=");
	p += urlQueryEncode(p, text);
	p = stpcpy(p, " HTTP/1.0\r\n\r\n");
	int sfd = TcpMakeTalkingSocket("192.168.1.1", "80", 100);
	if (sfd == -1)
	{
		Log('e', "SmsSend - Could not connect to sms server");
		return;
	}
	Log('i', "Opened socket %d for sms and sent '%s'\r\n", sfd, buffer);
	TcpSendString(sfd, buffer);
	buffer[0] = 0;
	int length = TcpRecvAll(sfd, buffer, sizeof(buffer));
	Log('i', "Received %d characters from sms '%.*s'\r\n", length, length, buffer);
	TcpClose(sfd);
}

static void sendStatus(char* number)
{
	char buffer[1000];
	sprintf(buffer,
				"Status:\n"
				"Battery %3.1f%%\n"
				"Water %d litres\n"
				"Lpg %d litres\n"
				"Water pump %s\n"
				"Water fill %s\n"
				"Water drain %s\n"
				"Inverter %s\n"
				"Mode %s",
				(float)CanThisGetBatteryCountedCapacityAs() / (BATTERY_CAPACITY_AH * 36),
				CanThisGetTankFreshLitres(),
				CanThisGetTankLpgVolumeMl() / 1024,
				CanThisGetControlWaterPump() ? "on" : "off",
				CanThisGetControlWaterFill() ? "on" : "off",
				CanThisGetControlWaterDrain() ? "on" : "off",
				CanThisGetControlInverter() ? "on" : "off",
				BatteryGetModeAsString()
				);
				
	SmsSend(number, buffer);
}
static void sendBattery(char* number)
{
	char buffer[1000];
	sprintf(buffer,
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
	SmsSend(number, buffer);
}
static char parseBool(char* sValue)
{
	char iValue = 0;
	if (strncasecmp(sValue, "on", 2) == 0) iValue = 1;
	return iValue;
}
static void setTarget(char* number, char* sValue)
{
	int iValue = atoi(sValue);
	if (iValue > 90 || iValue < 20)
	{
		SmsSend(number, "Target %% outside 20 to 90");
		return;
	}
	CanThisSetBatteryCapacityTargetPercent((uint8_t)iValue);
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendBattery(number);
}
static void setPump(char* number, char* sValue)
{
	CanThisSetControlWaterPump(parseBool(sValue));
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendStatus(number);
}
static void setFill(char* number, char* sValue)
{
	CanThisSetControlWaterFill(parseBool(sValue));
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendStatus(number);
}
static void setDrain(char* number, char* sValue)
{
	Log('w', "Value is '%s' -> %d", sValue, parseBool(sValue));
	CanThisSetControlWaterDrain(parseBool(sValue));
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendStatus(number);
}
static void setInverter(char* number, char* sValue)
{
	CanThisSetControlInverter(parseBool(sValue));
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendStatus(number);
}
static void setMode(char* number, char* sValue)
{
	BatterySetModeAsString(sValue);
	sleep(3); //Need time for the values to be updated in their thread so sleep this one
	sendStatus(number);
}
void SmsHandleRequest(char* number, char* request)
{
	if (strncasecmp(request, "status"  , 6) == 0) { sendStatus (number);              return; }
	if (strncasecmp(request, "battery" , 7) == 0) { sendBattery(number);              return; }
	if (strncasecmp(request, "target"  , 6) == 0) { setTarget  (number, request + 6); return; }
	if (strncasecmp(request, "pump"    , 4) == 0) { setPump    (number, request + 5); return; }
	if (strncasecmp(request, "fill"    , 4) == 0) { setFill    (number, request + 5); return; }
	if (strncasecmp(request, "drain"   , 5) == 0) { setDrain   (number, request + 6); return; }
	if (strncasecmp(request, "inverter", 8) == 0) { setInverter(number, request + 8); return; }
	if (strncasecmp(request, "mode"    , 4) == 0) { setMode    (number, request + 5); return; }
	if (strncasecmp(request, "help"    , 4) == 0)
	{
		SmsSend(number, "Help:\n"
						"help\n"
						"status\n"
						"battery\n"
						"target 20-90\n"
						"pump on|off\n"
						"fill on|off\n"
						"drain on|off\n"
						"inverter on|off\n"
						"mode home|away|manual\n"
						);
		return;
	}
	Log('w', "SMS '%s' sent '%s'", number, request);
	char buffer[2000];
	sprintf(buffer, "SMS received from %s\n\n%s", number, request);
	AlertSend(buffer);
}