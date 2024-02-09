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
static char checkForAlertAbove(int16_t value, int16_t limit, int16_t hysteresis, char* thisScan, char* lastScan)
{
	if (value > limit             ) *thisScan = 1;
	if (value < limit - hysteresis) *thisScan = 0;
	
	char oneShot = *thisScan && !*lastScan;
	*lastScan = *thisScan;
	
	return oneShot;
}
static char checkForAlertBelow(int16_t value, int16_t limit, int16_t hysteresis, char* thisScan, char* lastScan)
{
	if (value < limit             ) *thisScan = 1;
	if (value > limit + hysteresis) *thisScan = 0;
	
	char oneShot = *thisScan && !*lastScan;
	*lastScan = *thisScan;
	
	return oneShot;
}
static void pollInternalTemperature()
{
	static char _this40 = 0;	static char _last40 = 0;
	static char _this30 = 0;	static char _last30 = 0;
	static char _this10 = 0;	static char _last10 = 0;
	static char _this08 = 0;	static char _last08 = 0;
	static char _this06 = 0;	static char _last06 = 0;
	static char _this04 = 0;	static char _last04 = 0;
	static char _this02 = 0;	static char _last02 = 0;
	static char _this00 = 0;	static char _last00 = 0;
	
	int16_t temperature = CanThisGetAmbientHeatingTemp16ths() / 16;
	
	if (checkForAlertAbove(temperature, 40, 1, &_this40, &_last40)) AlertSend("Internal temperature is above 40");
	if (checkForAlertAbove(temperature, 30, 1, &_this30, &_last30)) AlertSend("Internal temperature is above 30");
	if (checkForAlertBelow(temperature, 10, 1, &_this10, &_last10)) AlertSend("Internal temperature is below 10");
	if (checkForAlertBelow(temperature,  8, 1, &_this08, &_last08)) AlertSend("Internal temperature is below 8");
	if (checkForAlertBelow(temperature,  6, 1, &_this06, &_last06)) AlertSend("Internal temperature is below 6");
	if (checkForAlertBelow(temperature,  4, 1, &_this04, &_last04)) AlertSend("Internal temperature is below 4");
	if (checkForAlertBelow(temperature,  2, 1, &_this02, &_last02)) AlertSend("Internal temperature is below 2");
	if (checkForAlertBelow(temperature,  0, 1, &_this00, &_last00)) AlertSend("Internal temperature is below 0");
}
void AlertPoll()
{
	//pollInternalTemperature();
	checkforInternalTemperatureBelow();
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
