#include <stdint.h>
#include <string.h>

#include "lib/settings.h"
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

void AlertPoll()
{
	static uint8_t lastChargePercent   = 50;
	static int16_t lastTemperatureDeg  = 20;
	
	uint8_t thisChargePercent   = (uint8_t)(CanThisGetBatteryCountedCapacityAs() / BATTERY_CAPACITY_AH / 36);
	int16_t thisTemperatureDeg  =           CanThisGetBatteryTemperature8bfdp() / 256;
		   
	     if (lastChargePercent   <= 95 && thisChargePercent   > 95) AlertSend("Capacity above 95");
	else if (lastChargePercent   <= 90 && thisChargePercent   > 90) AlertSend("Capacity above 90");
	else if (lastChargePercent   >= 10 && thisChargePercent   < 10) AlertSend("Capacity below 10");
	else if (lastChargePercent   >= 20 && thisChargePercent   < 20) AlertSend("Capacity below 20");
	else if (lastChargePercent   >= 30 && thisChargePercent   < 30) AlertSend("Capacity below 30");
	else if (lastChargePercent   >= 40 && thisChargePercent   < 40) AlertSend("Capacity below 40");
	
	     if (lastTemperatureDeg  <= 30 && thisTemperatureDeg  > 30) AlertSend("Temperature above 30");
	else if (lastTemperatureDeg  <= 25 && thisTemperatureDeg  > 25) AlertSend("Temperature above 25");
	else if (lastTemperatureDeg  >=  0 && thisTemperatureDeg  <  0) AlertSend("Temperature below 0");
	else if (lastTemperatureDeg  >=  5 && thisTemperatureDeg  <  5) AlertSend("Temperature below 5");
	else if (lastTemperatureDeg  >= 10 && thisTemperatureDeg  < 10) AlertSend("Temperature below 10");
	
	lastChargePercent   = thisChargePercent;
	lastTemperatureDeg  = thisTemperatureDeg;
}
