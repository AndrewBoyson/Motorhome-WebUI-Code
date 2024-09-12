#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "lib/settings.h"
#include "lib/log.h"
#include "sms.h"
#include "can-this.h"
#include "battery.h"

static char _alertNumber[20];

void AlertSetNumber(char* number)
{
	strncpy(_alertNumber, number, sizeof(_alertNumber));
	SettingsSetString("smsAlertNumber", _alertNumber);
}
char* AlertGetNumber()
{
	return _alertNumber;
}

void AlertInit()
{
	SettingsGetString("smsAlertNumber", _alertNumber);
}

void AlertSend(char* text)
{
	SmsSend(_alertNumber, text);
}
void AlertSendF(const char *format, ...) {

	va_list args;
	va_start (args, format);
	SmsSendV(_alertNumber, format, args);
	va_end (args);
}
static void checkforInternalTemperatureBelow()
{
	const int16_t NO_ALERT = 20 * 16;
	const int16_t HYS      =       8;
	
	static int16_t _existingAlert = NO_ALERT;
	
	int16_t value = CanThisGetAmbientHeatingTemp16ths();
	int16_t newAlert = NO_ALERT;
	if (value <  5 * 16) newAlert =  5 * 16;
	if (value <  4 * 16) newAlert =  4 * 16;
	if (value <  3 * 16) newAlert =  3 * 16;
	if (value <  2 * 16) newAlert =  2 * 16;
	if (value <  1 * 16) newAlert =  1 * 16;
	if (value <  0 * 16) newAlert =  0 * 16;
	if (value < -1 * 16) newAlert = -1 * 16;
	if (value < -2 * 16) newAlert = -2 * 16;
	
	char gettingWorse  = newAlert <  _existingAlert;
	char gettingBetter =    value >= _existingAlert + HYS;
	
	if (gettingWorse || gettingBetter) _existingAlert = newAlert;
	
	if (gettingWorse) AlertSendF("Internal temperature (%04hX hex) is below %d deg", value, newAlert/16);
}
static char checkForAlertAbove(int16_t value, int16_t limit, int16_t hysteresis, char* haveAlert, char* hadAlert)
{
	if (value > limit             ) *haveAlert = 1;
	if (value < limit - hysteresis) *haveAlert = 0;
	
	char oneShot = *haveAlert && !*hadAlert;
	*hadAlert = *haveAlert;
	
	return oneShot;
}
static char checkForAlertBelow(int16_t value, int16_t limit, int16_t hysteresis, char* haveAlert, char* hadAlert)
{
	if (value < limit             ) *haveAlert = 1;
	if (value > limit + hysteresis) *haveAlert = 0;
	
	char oneShot = *haveAlert && !*hadAlert;
	*hadAlert = *haveAlert;
	
	return oneShot;
}
static void pollInternalTemperature()
{
	static char _have30 = 0;	static char _had30 = 0;
	static char _have00 = 0;	static char _had00 = 0;
	
	int16_t temperature16ths = CanThisGetAmbientHeatingTemp16ths();
	
	if (checkForAlertAbove(temperature16ths, 30 * 16, 5, &_have30, &_had30)) AlertSendF("Internal temperature (%04hX hex) is above 30 deg", temperature16ths);
	if (checkForAlertBelow(temperature16ths,  0 * 16, 1, &_have00, &_had00)) AlertSendF("Internal temperature (%04hX hex) is below 0 deg" , temperature16ths);
}
void AlertPoll()
{
	pollInternalTemperature();
	//checkforInternalTemperatureBelow();
	static uint8_t lastChargePercent   = 50;
	static int16_t lastTemperatureDeg  = 20;
	
	uint8_t thisChargePercent   = (uint8_t)(CanThisGetBatteryCountedCapacityAs() / BATTERY_CAPACITY_AH / 36);
	int16_t thisTemperatureDeg  =           CanThisGetBatteryTemperature8bfdp() / 256;
		   
	     if (lastChargePercent   <= 95 && thisChargePercent   > 95) AlertSend("Battery capacity above 95");
	else if (lastChargePercent   <= 90 && thisChargePercent   > 90) AlertSend("Battery capacity above 90");
	else if (lastChargePercent   >= 10 && thisChargePercent   < 10) AlertSend("Battery capacity below 10");
	else if (lastChargePercent   >= 20 && thisChargePercent   < 20) AlertSend("Battery capacity below 20");
	else if (lastChargePercent   >= 30 && thisChargePercent   < 30) AlertSend("Battery capacity below 30");
	else if (lastChargePercent   >= 40 && thisChargePercent   < 40) AlertSend("Battery capacity below 40");
	
	     if (lastTemperatureDeg  <= 30 && thisTemperatureDeg  > 30) AlertSend("Battery temperature above 30");
	else if (lastTemperatureDeg  <= 25 && thisTemperatureDeg  > 25) AlertSend("Battery temperature above 25");
	else if (lastTemperatureDeg  >=  0 && thisTemperatureDeg  <  0) AlertSend("Battery temperature below 0");
	else if (lastTemperatureDeg  >=  5 && thisTemperatureDeg  <  5) AlertSend("Battery temperature below 5");
	else if (lastTemperatureDeg  >= 10 && thisTemperatureDeg  < 10) AlertSend("Battery temperature below 10");
	
	lastChargePercent   = thisChargePercent;
	lastTemperatureDeg  = thisTemperatureDeg;
}
