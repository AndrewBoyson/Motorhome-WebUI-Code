#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#include "lib/settings.h"

/*
Wanted - new value from the gui for sending to the CP
Target - setting read from the CP
Actual - actual reading read from the CP
*/
static char    _roomOn    =   0; //On or off        == 1, 0
static uint8_t _roomTemp  =   0; //Degrees C        == < 5 is zero
static char    _fanMode   = 'E'; //Eco, High        == 1, 10

static char    _waterOn   =   0; //On or off        == 1, 0
static char    _waterTemp = 'H'; //Eco, High, Boost == 40K, 60K, 200K

static char    _energy    = 'E'; //Gas, Elec, Both  == 1, 2, 3 little, capital 900, 1800

char    TrumaGetWantedRoomOn   () { return _roomOn;    }
uint8_t TrumaGetWantedRoomTemp () { return _roomTemp;  }
char    TrumaGetWantedFanMode  () { return _fanMode;   }

char    TrumaGetWantedWaterOn  () { return _waterOn;   }
char    TrumaGetWantedWaterTemp() { return _waterTemp; }

char    TrumaGetWantedEnergy   () { return _energy;    }

void TrumaSetWantedRoomOn   (char    v) {                                        _roomOn    = v; SettingsSetChar("heatingRoomOn"   , v);} //Boolean so no need to sanitise
void TrumaSetWantedRoomTemp (uint8_t v) { if (v < 5) v = 5; if (v > 25) v =  25; _roomTemp  = v; SettingsSetU8  ("heatingRoomTemp" , v);} //Sanitise to be between 5 and 25
void TrumaSetWantedFanMode  (char    v) { if (!strchr("EH",    v)     ) v = 'E'; _fanMode   = v; SettingsSetChar("heatingFanMode"  , v);} //Sanitise to 'E' or 'H'

void TrumaSetWantedWaterOn  (char    v) {                                        _waterOn   = v; SettingsSetChar("heatingWaterOn"  , v);} //Boolean so no need to sanitise
void TrumaSetWantedWaterTemp(char    v) { if (!strchr("EHB",   v)     ) v = 'H'; _waterTemp = v; SettingsSetChar("heatingWaterTemp", v);} //Sanitise to 'E', 'H' or 'B'

void TrumaSetWantedEnergy   (char    v) { if (!strchr("GEeMm", v)     ) v = 'E'; _energy    = v; SettingsSetChar("heatingEnergy"   , v);} //Sanitise to 'G', 'E', 'e', 'M' or 'm'

uint8_t TrumaTargetRoomTemp  = 0;
char    TrumaTargetFanMode   = 'E';

char    TrumaTargetWaterTemp = 'O';

char    TrumaTargetEnergy    = 'E';

uint16_t TrumaActualRoomTemp    = 2730; //0C
uint16_t TrumaActualWaterTemp   = 2730; //0C
uint8_t  TrumaActualRecvStatus  = 0;
uint8_t  TrumaActualOpStatus    = 0;
uint8_t  TrumaActualErrorCode   = 0;

void TrumaHadSendAcknowledgement() //This means we know the CP has received the new target values so update them without waiting for a status
{
	TrumaTargetRoomTemp  = _roomOn ? _roomTemp   :  0;
	TrumaTargetFanMode   = _roomOn ? _fanMode    : 'O';

	TrumaTargetWaterTemp = _waterOn ? _waterTemp : 'O';

	TrumaTargetEnergy    = _energy;
}
char TrumaHasSameActualAsTarget()
{
	return TrumaTargetRoomTemp  == (_roomOn  ? _roomTemp  :  0 ) &&
	    TrumaTargetFanMode      == (_roomOn  ? _fanMode   : 'O') &&
		TrumaTargetWaterTemp    == (_waterOn ? _waterTemp : 'O') &&
		TrumaTargetEnergy       ==             _energy;
}
void TrumaInit()
{
	int r = 0;
	r = SettingsGetChar("heatingRoomOn"   , &_roomOn   ); if (r) _roomOn    =  0 ;
	r = SettingsGetU8  ("heatingRoomTemp" , &_roomTemp ); if (r) _roomTemp  =  0 ;
	r = SettingsGetChar("heatingFanMode"  , &_fanMode  ); if (r) _fanMode   = 'E';
	
	r = SettingsGetChar("heatingWaterOn"  , &_waterOn  ); if (r) _waterOn   =  0 ;
	r = SettingsGetChar("heatingWaterTemp", &_waterTemp); if (r) _waterTemp = 'O';
	
	r = SettingsGetChar("heatingEnergy"   , &_energy   ); if (r) _energy    = 'E';
}
