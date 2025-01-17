
#include <stdint.h>
#include <string.h>
#include <stdlib.h> //system
#include <stdio.h>

#include "lib/log.h"
#include "lib/usbdrive.h"
#include "lib/http-get.h"
#include "lib/http-response.h"
#include "lib/http-credentials.h"
#include "http-this.h"
#include "global.h"
#include "can-this.h"
#include "battery.h"
#include "sms.h"
#include "alert.h"

//Utilities
int HttpThisMakeFullPath(char *filename, char *fullPath, int lengthFullPath) { //Converts a relative file name to a full path by prepending the www folder, returns -1 if no room

	char* p = fullPath;
	int room = lengthFullPath - 1; //-1 is for the 0 at the end
	if (filename[0] != '/')
	{
		room -= strlen(WWW_FOLDER);
		room -= 1;   //for the '/'
		if (strlen(filename) > room) return -1;
		p = stpcpy(p, WWW_FOLDER);
		*p++ = '/';
		p = stpncpy(p, filename, room);
	}
	else
	{
		if (strlen(filename) > room) return -1;
		p = stpncpy(p, filename, room);
	}
	*p = 0;
	return 0;
}

int HttpThisNameValue(unsigned rid, char* name, char* value) { //returns -1 if unhandled error, 1 if not handled, 0 if handled ok

	static unsigned _smsRid = 0; //Use the request id to match two name value pairs for handling together
	static char     _smsNumber[20];
	static char     _smsText[1000];
	
	//From Teltonika
	if (strcmp(name, "sms-number") == 0) { strncpy(_smsNumber, value, sizeof(_smsNumber)); if (rid == _smsRid) SmsHandleRequest(_smsNumber, _smsText); _smsRid = rid; return 0; }
	if (strcmp(name, "sms-text"  ) == 0) { strncpy(_smsText,   value, sizeof(_smsText  )); if (rid == _smsRid) SmsHandleRequest(_smsNumber, _smsText); _smsRid = rid; return 0; }
	
	//From Server
	if (strcmp(name, "sms-send-test-alert"                 ) == 0) { AlertSend             ( value ); return 0; }    
	if (strcmp(name, "sms-alert-number"                    ) == 0) { AlertSetNumber        ( value ); return 0; }
	if (strcmp(name, "sms-username"                        ) == 0) { SmsSetUserName        ( value ); return 0; }
	if (strcmp(name, "sms-password"                        ) == 0) { SmsSetPassword        ( value ); return 0; }
	if (strcmp(name, "sms-hostname"                        ) == 0) { SmsSetHostname        ( value ); return 0; }
	
	if (strcmp(name, "log-level"                           ) == 0) { LogSetLevel           (*value ); return 0; } //Take the first character from the value string
	if (strcmp(name, "credentials-password"                ) == 0) { CredentialsSetPassword( value ); return 0; }
	
	if (strcmp(name, "battery-counted-capacity-amp-seconds") == 0) { uint32_t v; if (HttpGetParseU32  (value, &v)) return -1; CanThisSetBatteryCountedCapacityAs      (v); return 0; }
	if (strcmp(name, "battery-capacity-setpoint-percent"   ) == 0) { uint8_t  v; if (HttpGetParseU8   (value, &v)) return -1; CanThisSetBatteryCapacityTargetPercent  (v); return 0; }
	if (strcmp(name, "battery-temperature-target-tenths"   ) == 0) { int16_t  v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetBatteryTemperatureTargetTenths(v); return 0; }
	if (strcmp(name, "battery-heater-proportional"         ) == 0) { uint16_t v; if (HttpGetParseU16  (value, &v)) return -1; CanThisSetBatteryHeaterProportional     (v); return 0; }
	if (strcmp(name, "battery-heater-integral"             ) == 0) { uint16_t v; if (HttpGetParseU16  (value, &v)) return -1; CanThisSetBatteryHeaterIntegral         (v); return 0; }
	if (strcmp(name, "battery-charge-enabled"              ) == 0) {  int8_t  v; if (HttpGetParseS8   (value, &v)) return -1; CanThisSetBatteryChargeEnabled          (v); return 0; }
	if (strcmp(name, "battery-discharge-enabled"           ) == 0) {  int8_t  v; if (HttpGetParseS8   (value, &v)) return -1; CanThisSetBatteryDischargeEnabled       (v); return 0; }
	if (strcmp(name, "battery-aging-as-per-hour"           ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetBatteryAgingAsPerHour         (v); return 0; }
	if (strcmp(name, "battery-mode"                        ) == 0) {  int8_t  v; if (HttpGetParseS8   (value, &v)) return -1; BatterySetMode                          (v); return 0; }
	if (strcmp(name, "battery-away-percent"                ) == 0) { uint8_t  v; if (HttpGetParseU8   (value, &v)) return -1; BatterySetAwayPercent                   (v); return 0; }
	if (strcmp(name, "battery-home-percent"                ) == 0) { uint8_t  v; if (HttpGetParseU8   (value, &v)) return -1; BatterySetHomePercent                   (v); return 0; }
	if (strcmp(name, "battery-rest-ma"                     ) == 0) { int32_t  v; if (HttpGetParseS32  (value, &v)) return -1; BatterySetRestMa                        (v); return 0; }
	if (strcmp(name, "plot-rest-seconds"                   ) == 0) { uint32_t v; if (HttpGetParseU32  (value, &v)) return -1; BatterySetPlotRestSeconds               (v); return 0; }
	if (strcmp(name, "plot-direction"                      ) == 0) {  int8_t  v; if (HttpGetParseS8   (value, &v)) return -1; BatterySetPlotDir                       (v); return 0; }
	if (strcmp(name, "plot-inc-percent"                    ) == 0) { uint8_t  v; if (HttpGetParseU8   (value, &v)) return -1; BatterySetPlotIncPercent                (v); return 0; }
	if (strcmp(name, "plot-max-percent"                    ) == 0) { uint8_t  v; if (HttpGetParseU8   (value, &v)) return -1; BatterySetPlotMaxPercent                (v); return 0; }
	if (strcmp(name, "plot-min-percent"                    ) == 0) { uint8_t  v; if (HttpGetParseU8   (value, &v)) return -1; BatterySetPlotMinPercent                (v); return 0; }
	if (strcmp(name, "battery-cal-as"                      ) == 0) { uint32_t v; if (HttpGetParseU32  (value, &v)) return -1; BatterySetCalAs                         (v); return 0; }
	if (strcmp(name, "battery-cal-mv"                      ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; BatterySetCalMv                         (v); return 0; }
	if (strcmp(name, "battery-cal-as-per-mv"               ) == 0) { uint32_t v; if (HttpGetParseU32  (value, &v)) return -1; BatterySetCalAsPerMv                    (v); return 0; }
	if (strcmp(name, "battery-cal-time"                    ) == 0) { uint32_t v; if (HttpGetParseU32  (value, &v)) return -1; BatterySetCalTime                       (v); return 0; }
	if (strcmp(name, "battery-cal-min-as"                  ) == 0) { uint32_t v; if (HttpGetParseU32  (value, &v)) return -1; BatterySetCalMinAs                      (v); return 0; }
	
	if (strcmp(name, "tank-fresh-base-temp-16ths"          ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetTankFreshBaseTemp16ths        (v); return 0; }
	if (strcmp(name, "tank-fresh-base-mv"                  ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetTankFreshBaseMv               (v); return 0; }
	if (strcmp(name, "tank-fresh-uv-per-16th"              ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetTankFreshUvPer16th            (v); return 0; }
	if (strcmp(name, "tank-fresh-sensor-front"             ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetTankSensorPosnFront           (v); return 0; }
	if (strcmp(name, "tank-fresh-sensor-right"             ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetTankSensorPosnRight           (v); return 0; }
	if (strcmp(name, "tank-fresh-width"                    ) == 0) { uint16_t v; if (HttpGetParseU16  (value, &v)) return -1; CanThisSetTankWidth                     (v); return 0; }
	if (strcmp(name, "tank-fresh-length"                   ) == 0) { uint16_t v; if (HttpGetParseU16  (value, &v)) return -1; CanThisSetTankLength                    (v); return 0; }
	if (strcmp(name, "tank-accelerometer-x-flat"           ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetTankAccelerometerXFlat        (v); return 0; }
	if (strcmp(name, "tank-accelerometer-y-flat"           ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetTankAccelerometerYFlat        (v); return 0; }
	if (strcmp(name, "tank-accelerometer-z-flat"           ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetTankAccelerometerZFlat        (v); return 0; }
	if (strcmp(name, "tank-fresh-rom"                      ) == 0) { uint64_t v; if (HttpGetParseX64  (value, &v)) return -1; CanThisSetTankFreshRom                  (v); return 0; }
	
	if (strcmp(name, "tank-lpg-resistance-min-16ths"       ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetTankLpgResistanceMin16ths     (v); return 0; }
	if (strcmp(name, "tank-lpg-resistance-max-16ths"       ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetTankLpgResistanceMax16ths     (v); return 0; }
	if (strcmp(name, "tank-lpg-volume-min-ml"              ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetTankLpgVolumeMinMl            (v); return 0; }
	if (strcmp(name, "tank-lpg-volume-max-ml"              ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetTankLpgVolumeMaxMl            (v); return 0; }
	
	if (strcmp(name, "ambient-outside-rom"                 ) == 0) { uint64_t v; if (HttpGetParseX64  (value, &v)) return -1; CanThisSetAmbientOutsideRom             (v); return 0; }
	if (strcmp(name, "ambient-heating-rom"                 ) == 0) { uint64_t v; if (HttpGetParseX64  (value, &v)) return -1; CanThisSetAmbientHeatingRom             (v); return 0; }
	
	if (strcmp(name, "control-water-pump"                  ) == 0) {  int8_t  v; if (HttpGetParseS8   (value, &v)) return -1; CanThisSetControlWaterPump              (v); return 0; }
	if (strcmp(name, "control-water-fill"                  ) == 0) {  int8_t  v; if (HttpGetParseS8   (value, &v)) return -1; CanThisSetControlWaterFill              (v); return 0; }
	if (strcmp(name, "control-water-drain"                 ) == 0) {  int8_t  v; if (HttpGetParseS8   (value, &v)) return -1; CanThisSetControlWaterDrain             (v); return 0; }
	if (strcmp(name, "control-inverter"                    ) == 0) {  int8_t  v; if (HttpGetParseS8   (value, &v)) return -1; CanThisSetControlInverter               (v); return 0; }
	if (strcmp(name, "control-lpg-heater"                  ) == 0) {  int8_t  v; if (HttpGetParseS8   (value, &v)) return -1; CanThisSetControlLpgHeater              (v); return 0; }
	if (strcmp(name, "control-pump-min-litres"             ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetControlPumpMinLitres          (v); return 0; }
	if (strcmp(name, "control-pump-dplus-litres"           ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetControlPumpDplusLitres        (v); return 0; }
	if (strcmp(name, "control-drain-max-litres"            ) == 0) {  int16_t v; if (HttpGetParseS16  (value, &v)) return -1; CanThisSetControlDrainMaxLitres         (v); return 0; }
	
	if (strcmp(name, "backup-to-usb"                       ) == 0) { system("cp -rT /home/pi/server /media/usb/server-`date -I`");                                         return 0; }

	return 1;
}

int HttpThisInclude(char* name, char* format) { // Returns 0 if handled, 1 if not handled

	//Battery
	if (strcmp (name, "BatteryCountedCapacityAs"      ) == 0) { HttpResponseAddU32 (        CanThisGetBatteryCountedCapacityAs       ()); return 0; }
	if (strcmp (name, "BatteryCurrent"                ) == 0) { HttpResponseAddS32 (        CanThisGetBatteryCurrentMa               ()); return 0; }
	if (strcmp (name, "BatteryCapacitySetpoint"       ) == 0) { HttpResponseAddU8  (        CanThisGetBatteryCapacityTargetPercent   ()); return 0; }
	if (strcmp (name, "BatteryOutputState"            ) == 0) { HttpResponseAddChar(        CanThisGetBatteryOutputState             ()); return 0; }
	if (strcmp (name, "BatteryChargeEnabled"          ) == 0) { HttpResponseAddBool(format, CanThisGetBatteryChargeEnabled           ()); return 0; }
	if (strcmp (name, "BatteryDischargeEnabled"       ) == 0) { HttpResponseAddBool(format, CanThisGetBatteryDischargeEnabled        ()); return 0; }
	if (strcmp (name, "BatteryTemperature8bfdp"       ) == 0) { HttpResponseAddS16 (        CanThisGetBatteryTemperature8bfdp        ()); return 0; }
	if (strcmp (name, "BatteryTemperatureTargetTenths") == 0) { HttpResponseAddS16 (        CanThisGetBatteryTemperatureTargetTenths ()); return 0; }
	if (strcmp (name, "BatteryHeater8bfdp"            ) == 0) { HttpResponseAddU8  (        CanThisGetBatteryHeaterOutput8bfdp       ()); return 0; }
	if (strcmp (name, "BatteryHeaterProportional"     ) == 0) { HttpResponseAddU16 (        CanThisGetBatteryHeaterProportional      ()); return 0; }
	if (strcmp (name, "BatteryHeaterIntegral"         ) == 0) { HttpResponseAddU16 (        CanThisGetBatteryHeaterIntegral          ()); return 0; }
	if (strcmp (name, "BatteryVoltageMv"              ) == 0) { HttpResponseAddS16 (        CanThisGetBatteryVoltageMv               ()); return 0; }
	if (strcmp (name, "BatteryAgingAsPerHour"         ) == 0) { HttpResponseAddS16 (        CanThisGetBatteryAgingAsPerHour          ()); return 0; }
	
	//Plot
	if (strcmp (name, "BatteryMode"                  ) == 0) {HttpResponseAddS8    (        BatteryGetMode                           ()); return 0; }
	if (strcmp (name, "BatteryAwayPercent"           ) == 0) {HttpResponseAddU8    (        BatteryGetAwayPercent                    ()); return 0; }
	if (strcmp (name, "BatteryHomePercent"           ) == 0) {HttpResponseAddU8    (        BatteryGetHomePercent                    ()); return 0; }
	if (strcmp (name, "BatteryRestMa"                ) == 0) {HttpResponseAddS32   (        BatteryGetRestMa                         ()); return 0; }
	if (strcmp (name, "BatteryRestedSeconds"         ) == 0) {HttpResponseAddU32   (        BatteryGetRestedSeconds                  ()); return 0; }
	if (strcmp (name, "PlotRestSeconds"              ) == 0) {HttpResponseAddU32   (        BatteryGetPlotRestSeconds                ()); return 0; }
	if (strcmp (name, "PlotDirection"                ) == 0) {HttpResponseAddS8    (        BatteryGetPlotDir                        ()); return 0; }
	if (strcmp (name, "PlotIncPercent"               ) == 0) {HttpResponseAddU8    (        BatteryGetPlotIncPercent                 ()); return 0; }
	if (strcmp (name, "PlotMaxPercent"               ) == 0) {HttpResponseAddU8    (        BatteryGetPlotMaxPercent                 ()); return 0; }
	if (strcmp (name, "PlotMinPercent"               ) == 0) {HttpResponseAddU8    (        BatteryGetPlotMinPercent                 ()); return 0; }
	if (strcmp (name, "BatteryCalAs"                 ) == 0) {HttpResponseAddU32   (        BatteryGetCalAs                          ()); return 0; }
	if (strcmp (name, "BatteryCalMv"                 ) == 0) {HttpResponseAddS16   (        BatteryGetCalMv                          ()); return 0; }
	if (strcmp (name, "BatteryCalAsPerMv"            ) == 0) {HttpResponseAddU32   (        BatteryGetCalAsPerMv                     ()); return 0; }
	if (strcmp (name, "BatteryCalTime"               ) == 0) {HttpResponseAddU32   (        BatteryGetCalTime                        ()); return 0; }
	if (strcmp (name, "BatteryCalMinAs"              ) == 0) {HttpResponseAddU32   (        BatteryGetCalMinAs                       ()); return 0; }
	if (strcmp (name, "BatteryCalOkToStart"          ) == 0) {HttpResponseAddS8    (        BatteryGetCalOkToStart                   ()); return 0; }
	
	//Tank
	if (strcmp (name, "TankFreshTemperature"         ) == 0) { HttpResponseAddS16  (        CanThisGetTankFreshTemperature           ()); return 0; }
	if (strcmp (name, "TankFreshRom"                 ) == 0) { HttpResponseAddX64  (        CanThisGetTankFreshRom                   ()); return 0; }
	if (strcmp (name, "TankFreshSupplyMv"            ) == 0) { HttpResponseAddS16  (        CanThisGetTankFreshSupplyMv              ()); return 0; }
	if (strcmp (name, "TankFreshBaseTemp16ths"       ) == 0) { HttpResponseAddS16  (        CanThisGetTankFreshBaseTemp16ths         ()); return 0; }
	if (strcmp (name, "TankFreshBaseMv"              ) == 0) { HttpResponseAddS16  (        CanThisGetTankFreshBaseMv                ()); return 0; }
	if (strcmp (name, "TankFreshUvPer16th"           ) == 0) { HttpResponseAddS16  (        CanThisGetTankFreshUvPer16th             ()); return 0; }
	if (strcmp (name, "TankFreshMv"                  ) == 0) { HttpResponseAddS16  (        CanThisGetTankFreshMv                    ()); return 0; }
	if (strcmp (name, "TankFreshDepthMm"             ) == 0) { HttpResponseAddS16  (        CanThisGetTankFreshDepthMm               ()); return 0; }
	if (strcmp (name, "TankFreshLitres"              ) == 0) { HttpResponseAddS16  (        CanThisGetTankFreshLitres                ()); return 0; }
	if (strcmp (name, "TankFreshSensorFront"         ) == 0) { HttpResponseAddS16  (        CanThisGetTankSensorPosnFront            ()); return 0; }
	if (strcmp (name, "TankFreshSensorRight"         ) == 0) { HttpResponseAddS16  (        CanThisGetTankSensorPosnRight            ()); return 0; }
	if (strcmp (name, "TankFreshWidth"               ) == 0) { HttpResponseAddS16  (        CanThisGetTankWidth                      ()); return 0; }
	if (strcmp (name, "TankFreshLength"              ) == 0) { HttpResponseAddS16  (        CanThisGetTankLength                     ()); return 0; }
	if (strcmp (name, "TankAccelerometerXFlat"       ) == 0) { HttpResponseAddS16  (        CanThisGetTankAccelerometerXFlat         ()); return 0; }
	if (strcmp (name, "TankAccelerometerYFlat"       ) == 0) { HttpResponseAddS16  (        CanThisGetTankAccelerometerYFlat         ()); return 0; }
	if (strcmp (name, "TankAccelerometerZFlat"       ) == 0) { HttpResponseAddS16  (        CanThisGetTankAccelerometerZFlat         ()); return 0; }
	if (strcmp (name, "TankAccelerometerX"           ) == 0) { HttpResponseAddS16  (        CanThisGetTankAccelerometerX             ()); return 0; }
	if (strcmp (name, "TankAccelerometerY"           ) == 0) { HttpResponseAddS16  (        CanThisGetTankAccelerometerY             ()); return 0; }
	if (strcmp (name, "TankAccelerometerZ"           ) == 0) { HttpResponseAddS16  (        CanThisGetTankAccelerometerZ             ()); return 0; }
	if (strcmp (name, "TankRom0"                     ) == 0) { HttpResponseAddX64  (        CanThisGetTankRom0                       ()); return 0; }
	if (strcmp (name, "TankRom1"                     ) == 0) { HttpResponseAddX64  (        CanThisGetTankRom1                       ()); return 0; }
	if (strcmp (name, "TankRom2"                     ) == 0) { HttpResponseAddX64  (        CanThisGetTankRom2                       ()); return 0; }
	if (strcmp (name, "TankRom3"                     ) == 0) { HttpResponseAddX64  (        CanThisGetTankRom3                       ()); return 0; }
	if (strcmp (name, "TankRomData0"                 ) == 0) { HttpResponseAddS16  (        CanThisGetTankRomData0                   ()); return 0; }
	if (strcmp (name, "TankRomData1"                 ) == 0) { HttpResponseAddS16  (        CanThisGetTankRomData1                   ()); return 0; }
	if (strcmp (name, "TankRomData2"                 ) == 0) { HttpResponseAddS16  (        CanThisGetTankRomData2                   ()); return 0; }
	if (strcmp (name, "TankRomData3"                 ) == 0) { HttpResponseAddS16  (        CanThisGetTankRomData3                   ()); return 0; }
	if (strcmp (name, "TankLpgMv"                    ) == 0) { HttpResponseAddS16  (        CanThisGetTankLpgMv                      ()); return 0; }
	if (strcmp (name, "TankLpgResistance16ths"       ) == 0) { HttpResponseAddS16  (        CanThisGetTankLpgResistance16ths         ()); return 0; }
	if (strcmp (name, "TankLpgResistanceMin16ths"    ) == 0) { HttpResponseAddS16  (        CanThisGetTankLpgResistanceMin16ths      ()); return 0; }
	if (strcmp (name, "TankLpgResistanceMax16ths"    ) == 0) { HttpResponseAddS16  (        CanThisGetTankLpgResistanceMax16ths      ()); return 0; }
	if (strcmp (name, "TankLpgVolumeMinMl"           ) == 0) { HttpResponseAddS16  (        CanThisGetTankLpgVolumeMinMl             ()); return 0; }
	if (strcmp (name, "TankLpgVolumeMaxMl"           ) == 0) { HttpResponseAddS16  (        CanThisGetTankLpgVolumeMaxMl             ()); return 0; }
	if (strcmp (name, "TankLpgVolumeMl"              ) == 0) { HttpResponseAddS16  (        CanThisGetTankLpgVolumeMl                ()); return 0; }
	
	//Ambient
	if (strcmp (name, "AmbientOutsideRom"            ) == 0) { HttpResponseAddX64  (        CanThisGetAmbientOutsideRom              ()); return 0; }
	if (strcmp (name, "AmbientHeatingRom"            ) == 0) { HttpResponseAddX64  (        CanThisGetAmbientHeatingRom              ()); return 0; }
	if (strcmp (name, "AmbientOutsideTemp16ths"      ) == 0) { HttpResponseAddS16  (        CanThisGetAmbientOutsideTemp16ths        ()); return 0; }
	if (strcmp (name, "AmbientHeatingTemp16ths"      ) == 0) { HttpResponseAddS16  (        CanThisGetAmbientHeatingTemp16ths        ()); return 0; }
	
	//Control
	if (strcmp (name, "ControlWaterPump"             ) == 0) {HttpResponseAddS8    (        CanThisGetControlWaterPump               ()); return 0; }
	if (strcmp (name, "ControlWaterFill"             ) == 0) {HttpResponseAddS8    (        CanThisGetControlWaterFill               ()); return 0; }
	if (strcmp (name, "ControlWaterDrain"            ) == 0) {HttpResponseAddS8    (        CanThisGetControlWaterDrain              ()); return 0; }
	if (strcmp (name, "ControlInverter"              ) == 0) {HttpResponseAddS8    (        CanThisGetControlInverter                ()); return 0; }
	if (strcmp (name, "ControlLpgHeater"             ) == 0) {HttpResponseAddS8    (        CanThisGetControlLpgHeater               ()); return 0; }
	if (strcmp (name, "ControlDPlus"                 ) == 0) {HttpResponseAddS8    (        CanThisGetControlDPlus                   ()); return 0; }
	if (strcmp (name, "ControlEhu"                   ) == 0) {HttpResponseAddS8    (        CanThisGetControlEhu                     ()); return 0; }
	if (strcmp (name, "ControlPumpMinLitres"         ) == 0) {HttpResponseAddS16   (        CanThisGetControlPumpMinLitres           ()); return 0; }
	if (strcmp (name, "ControlPumpDplusLitres"       ) == 0) {HttpResponseAddS16   (        CanThisGetControlPumpDplusLitres         ()); return 0; }
	if (strcmp (name, "ControlDrainMaxLitres"        ) == 0) {HttpResponseAddS16   (        CanThisGetControlDrainMaxLitres          ()); return 0; }
	
	//Usb
	if (strcmp (name, "UsbDriveIsPluggedIn"          ) == 0) {HttpResponseAddBool  (format, UsbDriveGetIsPluggedIn                   ()); return 0; }
	if (strcmp (name, "UsbDriveIsMounted"            ) == 0) {HttpResponseAddBool  (format, UsbDriveGetIsMounted                     ()); return 0; }
	if (strcmp (name, "UsbDriveSizeBytes"            ) == 0) {HttpResponseAddU32   (        UsbDriveGetSizeBytes                     ()); return 0; }
	if (strcmp (name, "UsbDriveFreeBytes"            ) == 0) {HttpResponseAddU32   (        UsbDriveGetFreeBytes                     ()); return 0; }
	if (strcmp (name, "UsbDriveLabel"                ) == 0) {HttpResponseAddString(        UsbDriveGetLabel                         ()); return 0; }
	
	//Alert
	if (strcmp (name, "SmsAlertNumber"               ) == 0) {HttpResponseAddString(        AlertGetNumber                           ()); return 0; }
	if (strcmp (name, "SmsUserName"                  ) == 0) {HttpResponseAddString(        SmsGetUserName                           ()); return 0; }
	if (strcmp (name, "SmsPassword"                  ) == 0) {HttpResponseAddString(        SmsGetPassword                           ()); return 0; }
	if (strcmp (name, "SmsHostName"                  ) == 0) {HttpResponseAddString(        SmsGetHostname                           ()); return 0; }
	
	//Credentials
	if (strcmp (name, "CredentialsPassword"          ) == 0) {HttpResponseAddString(        CredentialsGetPassword                   ()); return 0; }
	if (strcmp (name, "CredentialsId"                ) == 0) {HttpResponseAddString(        CredentialsGetId                         ()); return 0; }
	
	//Log
	if (strcmp (name, "LogLevel"                     ) == 0) {HttpResponseAddChar  (        LogGetLevel                              ()); return 0; }
	
	//Count
	if (strcmp (name, "CanCounts"                    ) == 0)
	{
		char text[1000000];
		for (int i = 0; i < 2048; i++)
		{
			if (CanCounts[i])
			{
				snprintf(text, sizeof(text), "%x %u\n", i, CanCounts[i]);
				HttpResponseAddString(text);
			}
		}
		return 0;
	}
	
	return 1;
}