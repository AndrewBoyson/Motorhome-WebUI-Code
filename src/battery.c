#include <stdint.h>
#include <stdio.h>     //fopen printf
#include <errno.h>     //errno
#include <string.h>    //strerror
#include <time.h>      //struct tm
#include <unistd.h>    //usleep

#include "lib/settings.h"
#include "lib/log.h"
#include "lib/file.h"
#include "can-this.h"

#define MODE_MANUAL 0 //Manual control
#define MODE_AWAY   1 //Set charge target to the away target (typically 80)
#define MODE_HOME   2 //Decrement charge target and rest down to point of inflexion then calibrate then leave charge target (typically at 57)

#define DIR_NONE  0
#define DIR_UP    1
#define DIR_DOWN -1

#define DO_NOTHING        0
#define DO_AWAY           1
#define DO_HOME_TARGET    2
#define DO_HOME_CALIBRATE 3

#define RATE_AS_PER_HOUR_DIVISOR 1

static int8_t   _mode            = MODE_MANUAL;
static uint8_t  _awayPercent     = 0;
static uint8_t  _homePercent     = 0;
static int32_t  _restMa          = 0;
static time_t   _restStarted     = 0;
static uint32_t _secondsAtRest   = 0; //Calculated during poll routine from _restStarted
static uint32_t _plotRestSeconds = 0;
static int8_t   _plotDir         = DIR_NONE;
static uint8_t  _plotIncPercent  = 0;
static uint8_t  _plotMaxPercent  = 0;
static uint8_t  _plotMinPercent  = 0;
static uint32_t _calAs           = 0;
static int16_t  _calMv           = 0;
static uint32_t _calAsPerMv      = 0;
static uint32_t _calTime         = 0;
static uint32_t _calMinAs        = 0;

static int8_t   _do              = DO_NOTHING;
static char     _okToCalibrate   = 0; //Calculated during poll

int8_t   BatteryGetMode           () { return _mode;            }
uint8_t  BatteryGetAwayPercent    () { return _awayPercent;     }
uint8_t  BatteryGetHomePercent    () { return _homePercent;     }
int32_t  BatteryGetRestMa         () { return _restMa;          }
time_t   BatteryGetRestStarted    () { return _restStarted;     }
uint32_t BatteryGetPlotRestSeconds() { return _plotRestSeconds; }
int8_t   BatteryGetPlotDir        () { return _plotDir;         }
uint8_t  BatteryGetPlotIncPercent () { return _plotIncPercent;  }
uint8_t  BatteryGetPlotMaxPercent () { return _plotMaxPercent;  }
uint8_t  BatteryGetPlotMinPercent () { return _plotMinPercent;  }
uint32_t BatteryGetCalAs          () { return _calAs;           }
int16_t  BatteryGetCalMv          () { return _calMv;           }
uint32_t BatteryGetCalAsPerMv     () { return _calAsPerMv;      }
uint32_t BatteryGetCalTime        () { return _calTime;         }
uint32_t BatteryGetCalMinAs       () { return _calMinAs;        }

char     BatteryGetCalOkToStart   () { return _okToCalibrate || _do == DO_HOME_CALIBRATE; }
uint32_t BatteryGetRestedSeconds  () { return _secondsAtRest;   } //Calculated during poll routine from _restStarted

void BatterySetMode           (int8_t   v) { _mode            = v; SettingsSetS8  ("batteryMode"          , _mode           ); }
void BatterySetAwayPercent    (uint8_t  v) { _awayPercent     = v; SettingsSetU8  ("batteryAwayPercent"   , _awayPercent    ); }
void BatterySetHomePercent    (uint8_t  v) { _homePercent     = v; SettingsSetU8  ("batteryHomePercent"   , _homePercent    ); }
void BatterySetRestMa         (int32_t  v) { _restMa          = v; SettingsSetS32 ("batteryRestMa"        , _restMa         ); }
void BatterySetRestStarted    (time_t   v) { _restStarted     = v; SettingsSetTime("batteryRestStarted"   , _restStarted    ); }
void BatterySetPlotRestSeconds(uint32_t v) { _plotRestSeconds = v; SettingsSetU32 ("batteryRestSeconds"   , _plotRestSeconds); }
void BatterySetPlotDir        (int8_t   v) { _plotDir         = v; SettingsSetS8  ("batteryPlotDir"       , _plotDir        ); }
void BatterySetPlotIncPercent (uint8_t  v) { _plotIncPercent  = v; SettingsSetU8  ("batteryPlotIncPercent", _plotIncPercent ); }
void BatterySetPlotMaxPercent (uint8_t  v) { _plotMaxPercent  = v; SettingsSetU8  ("batteryPlotMaxPercent", _plotMaxPercent ); }
void BatterySetPlotMinPercent (uint8_t  v) { _plotMinPercent  = v; SettingsSetU8  ("batteryPlotMinPercent", _plotMinPercent ); }
void BatterySetCalAs          (uint32_t v) { _calAs           = v; SettingsSetU32 ("batteryCalAs"         , _calAs          ); }
void BatterySetCalMv          (int16_t  v) { _calMv           = v; SettingsSetS16 ("batteryCalMv"         , _calMv          ); }
void BatterySetCalAsPerMv     (uint32_t v) { _calAsPerMv      = v; SettingsSetU32 ("batteryCalAsPerMv"    , _calAsPerMv     ); }
void BatterySetCalTime        (uint32_t v) { _calTime         = v; SettingsSetU32 ("batteryCalTime"       , _calTime        ); }
void BatterySetCalMinAs       (uint32_t v) { _calMinAs        = v; SettingsSetU32 ("batteryCalMinAs"      , _calMinAs       ); }
static void setDo             (int8_t   v) { _do              = v; SettingsSetS8  ("batteryDo"            , _do             ); }

char*    BatteryGetModeAsString   ()
{
	switch(_mode)
	{
		case MODE_MANUAL: return "manual";
		case MODE_AWAY:   return "away";
		case MODE_HOME:   return "home";
		default:          return "?";
	}
}
char BatterySetModeAsString   (char*    p)
{
	if (strcasecmp(p, "Manual") == 0) { BatterySetMode(MODE_MANUAL); return 0; }
	if (strcasecmp(p, "Away"  ) == 0) { BatterySetMode(MODE_AWAY  ); return 0; }
	if (strcasecmp(p, "Home"  ) == 0) { BatterySetMode(MODE_HOME  ); return 0; }
	return -1;
}

void BatteryInit()
{
	int r = 0;
	r = SettingsGetS8  ("batteryMode"          , &_mode           ); if (r) _mode            = MODE_MANUAL ;
	r = SettingsGetU8  ("batteryAwayPercent"   , &_awayPercent    ); if (r) _awayPercent     = 80        ;
	r = SettingsGetU8  ("batteryHomePercent"   , &_homePercent    ); if (r) _homePercent     = 50        ;
	r = SettingsGetS32 ("batteryRestMa"        , &_restMa         ); if (r) _restMa          = 100       ;
	r = SettingsGetTime("batteryRestStarted"   , &_restStarted    ); if (r) _restStarted     = time(0)   ;
	r = SettingsGetU32 ("batteryRestSeconds"   , &_plotRestSeconds); if (r) _plotRestSeconds = 3600      ;
	r = SettingsGetS8  ("batteryPlotDir"       , &_plotDir        ); if (r) _plotDir         = DIR_NONE  ;
	r = SettingsGetU8  ("batteryPlotIncPercent", &_plotIncPercent ); if (r) _plotIncPercent  = 1         ;
	r = SettingsGetU8  ("batteryPlotMaxPercent", &_plotMaxPercent ); if (r) _plotMaxPercent  = 80        ;
	r = SettingsGetU8  ("batteryPlotMinPercent", &_plotMinPercent ); if (r) _plotMinPercent  = 50        ;
	r = SettingsGetU32 ("batteryCalAs"         , &_calAs          ); if (r) _calAs           = 0         ;
	r = SettingsGetS16 ("batteryCalMv"         , &_calMv          ); if (r) _calMv           = 0         ;
	r = SettingsGetU32 ("batteryCalAsPerMv"    , &_calAsPerMv     ); if (r) _calAsPerMv      = 0         ; 
	r = SettingsGetU32 ("batteryCalTime"       , &_calTime        ); if (r) _calTime         = time(0)   ;
	r = SettingsGetU32 ("batteryCalMinAs"      , &_calMinAs       ); if (r) _calMinAs        = 60        ;
	r = SettingsGetS8  ("batteryDo"            , &_do             ); if (r) _do              = DO_NOTHING;
}

static int recordPlot(time_t t, uint8_t capacity, int16_t mV, int16_t temp8bfdp)
{
	FILE* fp = fopen("plot", "a");
	if (fp                                                                == NULL) { LogErrno("Record plot - fopen: "   );             return 1; }
	
	struct tm tm;
	if (gmtime_r(&t, &tm)                                                 == 0   ) { LogErrno("Record plot - gmtime-r: "); fclose(fp); return 1; }
	char sTime[20];
	if (strftime(sTime, sizeof(sTime), "%Y-%m-%d %H:%M:%S", &tm)           < 0   ) { LogErrno("Record plot - strftime: "); fclose(fp); return 1; }
	
	if (fprintf(fp, "%s, %u, %d, %d\r\n", sTime, capacity, mV, temp8bfdp) < 0   ) { LogErrno("Record plot - fprintf: " ); fclose(fp); return 1; }
	
	if (fclose(fp)                                                               ) { LogErrno("Record plot - fclose: "  );             return 1; }
	return 0;
}
static int recordRest(time_t t, uint32_t secondsAtRest, int16_t mV, uint8_t capacity, int16_t temp8bfdp)
{
	FILE* fp = fopen("rest", "a");
	if (fp                                                                                   == NULL) { LogErrno("Record rest - fopen: "   );             return 1; }
	
	struct tm tm;
	if (gmtime_r(&t, &tm)                                                                    == 0   ) { LogErrno("Record rest - gmtime-r: "); fclose(fp); return 1; }
	char sTime[20];
	if (strftime(sTime, sizeof(sTime), "%Y-%m-%d %H:%M:%S", &tm)                              < 0   ) { LogErrno("Record rest - strftime: "); fclose(fp); return 1; }
	
	if (fprintf(fp, "%s, %u, %d, %u, %d\r\n", sTime, secondsAtRest, mV, capacity, temp8bfdp) < 0   ) { LogErrno("Record rest - fprintf: " ); fclose(fp); return 1; }
	
	if (fclose(fp)                                                                                  ) { LogErrno("Record rest - fclose: "  );             return 1; }
	return 0;
}
static int recordAging(time_t t, int16_t agingAsPerHour)
{
	FILE* fp = fopen("aging", "a");
	if (fp                                                         == NULL          ) { LogErrno("Record aging - fopen: "   );             return 1; }
	
	struct tm tm;
	if (gmtime_r(&t, &tm)                                          == 0             ) { LogErrno("Record aging - gmtime-r: "); fclose(fp); return 1; }
	char sTime[20];
	if (strftime(sTime, sizeof(sTime), "%Y-%m-%d %H:%M:%S", &tm)    < 0             ) { LogErrno("Record aging - strftime: "); fclose(fp); return 1; }
	
	if (fprintf(fp, "%s, %d\r\n", sTime, agingAsPerHour)            < 0             ) { LogErrno("Record aging - fprintf: " ); fclose(fp); return 1; }
	
	if (fclose(fp)                                                                  ) { LogErrno("Record aging - fclose: "  );             return 1; }
	return 0;
}

static int recordHeating(time_t t, int16_t tempTenths, uint8_t heaterPercent)
{
	FILE* fp = fopen("heater", "a");
	if (fp                                                             == NULL      ) { LogErrno("Record heater - fopen: "   );             return 1; }
	
	struct tm tm;
	if (gmtime_r(&t, &tm)                                              == 0         ) { LogErrno("Record heater - gmtime-r: "); fclose(fp); return 1; }
	char sTime[20];
	if (strftime(sTime, sizeof(sTime), "%Y-%m-%d %H:%M:%S", &tm)        < 0         ) { LogErrno("Record heater - strftime: "); fclose(fp); return 1; }
	
	if (fprintf(fp, "%s, %d, %u\r\n", sTime, tempTenths, heaterPercent) < 0         ) { LogErrno("Record heater - fprintf: " ); fclose(fp); return 1; }
	
	if (fclose(fp)                                                                  ) { LogErrno("Record heater - fclose: "  );             return 1; }
	return 0;
}
static int recordCharge(time_t t, int16_t mv, uint32_t as)
{
	FILE* fp = fopen("charge", "a");
	if (fp                                                             == NULL      ) { LogErrno("Record charge - fopen: "   );             return 1; }
	
	struct tm tm;
	if (gmtime_r(&t, &tm)                                              == 0         ) { LogErrno("Record charge - gmtime-r: "); fclose(fp); return 1; }
	char sTime[20];
	if (strftime(sTime, sizeof(sTime), "%Y-%m-%d %H:%M:%S", &tm)        < 0         ) { LogErrno("Record charge - strftime: "); fclose(fp); return 1; }
	
	if (fprintf(fp, "%s, %d, %u\r\n", sTime, mv, as)                    < 0         ) { LogErrno("Record charge - fprintf: " ); fclose(fp); return 1; }
	
	if (fclose(fp)                                                                  ) { LogErrno("Record charge - fclose: "  );             return 1; }
	return 0;
}

static void plotHeating()
{
	#define SIGNIFICANT_TEMP_CHANGE 16
	#define SIGNIFICANT_HEATER_CHANGE 4
	int16_t temp = CanThisGetBatteryTemperature8bfdp();
	uint8_t heater = CanThisGetBatteryHeaterOutput8bfdp();
	time_t now = time(0);
	static int16_t lastTemp = 0;
	static uint8_t lastHeater = 0;
	static time_t lastUpdate = 0;
	char significantChange = lastTemp   >= temp   + SIGNIFICANT_TEMP_CHANGE   || lastTemp   <= temp   - SIGNIFICANT_TEMP_CHANGE   ||
							 lastHeater >= heater + SIGNIFICANT_HEATER_CHANGE || lastHeater <= heater - SIGNIFICANT_HEATER_CHANGE;
	if (significantChange || now > lastUpdate + 1200)
	{
		recordHeating(now, temp, heater);
		lastTemp   = temp;
		lastHeater = heater;
		lastUpdate = now;
	}
}
static void plotCharge()
{
	#define SIGNIFICANT_MV_CHANGE 10
	#define SIGNIFICANT_AS_CHANGE 1000 //1% = 2.8Ah * 3600 = 10080
	 int16_t thisMv = CanThisGetBatteryVoltageMv();
	uint32_t thisAs = CanThisGetBatteryCountedCapacityAs();
	time_t now = time(0);
	static  int16_t lastMv = 0;
	static uint32_t lastAs = 0;
	static time_t lastUpdate = 0;
	char significantChange = lastMv >= thisMv + SIGNIFICANT_MV_CHANGE || lastMv <= thisMv - SIGNIFICANT_MV_CHANGE   ||
							 lastAs >= thisAs + SIGNIFICANT_AS_CHANGE || lastAs <= thisAs - SIGNIFICANT_AS_CHANGE;
	if (significantChange || now > lastUpdate + 1200)
	{
		recordCharge(now, thisMv, thisAs);
		lastMv = thisMv;
		lastAs = thisAs;
		lastUpdate = now;
	}
}
void BatteryPoll()
{
	plotHeating();
	plotCharge();
	
	char state = CanThisGetBatteryOutputState();
	
	if (state == 0) return; //Not yet received any information
	
	time_t now = time(0);
	int32_t mA = CanThisGetBatteryCurrentMa();
	int16_t cellMv = CanThisGetBatteryVoltageMv() / 4;
	int16_t batteryTemp8bfdp = CanThisGetBatteryTemperature8bfdp();
	
    _okToCalibrate = cellMv > _calMv && CanThisGetBatteryCountedCapacityAs() >= _calMinAs;
	
	switch (_mode)
	{
		case MODE_MANUAL: //Manual - don't set anything
			setDo(DO_NOTHING);
			break;
		case MODE_AWAY:
			setDo(DO_AWAY);
			break;
		case MODE_HOME:
			if (_do == DO_NOTHING || _do == DO_AWAY)
			{
				if (_okToCalibrate)
				{
					setDo(DO_HOME_CALIBRATE);
					CanThisSetBatteryCapacityTargetPercent((uint8_t)(CanThisGetBatteryCountedCapacityAs() / 280 / 36)); //Make first target the next lower integer percent
				}
				else
				{
					setDo(DO_HOME_TARGET);
				}
			}
			break;
	}
	switch (_do)
	{
		case DO_NOTHING:
			break;
		case DO_AWAY:
			BatterySetPlotDir(DIR_NONE);                          //Don't plot
			CanThisSetBatteryCapacityTargetPercent(_awayPercent); //Charge to away target
			break;
		case DO_HOME_CALIBRATE:
			BatterySetPlotDir(DIR_DOWN);
			break;
		case DO_HOME_TARGET:
			BatterySetPlotDir(DIR_NONE);
			CanThisSetBatteryCapacityTargetPercent(_homePercent); //Discharge to home target
			break;
	}
	
	//Rest
	char isAtRest = state == 'N' && mA < _restMa && mA > -_restMa;
	if (isAtRest)
	{
		if (!_restStarted) BatterySetRestStarted(now);
		_secondsAtRest = now - _restStarted;
	}
	else
	{
		if (_restStarted) BatterySetRestStarted(0);
		_secondsAtRest = 0;
	}
	
	//Record rest
	char doRecordRest = 0;
	if (isAtRest && _secondsAtRest == 0) doRecordRest = 1;                        //Record on reaching rest state (actually about 5 mins after neutral)
	for (int i = 8; i < 32; i++) if (_secondsAtRest == 1 << i) doRecordRest = 1;  //Record every multiple of 2 after 256 seconds
	if (doRecordRest) recordRest(now, _secondsAtRest, cellMv, CanThisGetBatteryCapacityTargetPercent(), batteryTemp8bfdp);
		
	//Increment plot - only run every ten seconds to give plenty of time for the battery to have updated
	static int _secondsCount = 0;
	_secondsCount++;
	if (_secondsCount >= 10) _secondsCount = 0;
	if (_secondsCount != 0) return;
	if ((_secondsAtRest >= _plotRestSeconds) && _plotDir)
	{
		recordPlot(now, CanThisGetBatteryCapacityTargetPercent(), cellMv, batteryTemp8bfdp);
		int16_t halfIncrementMv = 280 * 36 * _plotIncPercent / _calAsPerMv / 2;
		if (_do == DO_HOME_CALIBRATE && cellMv <= (_calMv + halfIncrementMv))
		{
			 int32_t elapsedSeconds = now - _calTime;
			 int32_t elapsedHours = elapsedSeconds / 3600;
			uint32_t measuredCapacityAs = _calAs - (_calMv - cellMv) * _calAsPerMv;
			uint32_t countedCapacityAs = CanThisGetBatteryCountedCapacityAs();
			 int32_t capacityToAddToCountAs = measuredCapacityAs - countedCapacityAs;
			   float mAToAdd = (float)capacityToAddToCountAs * 1000 / elapsedSeconds;
			 int32_t asPerHourToAdd = capacityToAddToCountAs / elapsedHours;
			 
			Log('e', "Calibrate at cell voltage %dmV -> measured capacity %uAs (%2.1f%%) and counted capacity %uAs (%2.1f%%) after %d hours", cellMv,
			                                                                                                                    measuredCapacityAs, (float)measuredCapacityAs / 280 / 36,
																																 countedCapacityAs, (float) countedCapacityAs / 280 / 36,
																																elapsedHours);
			Log('e', "Capacity to add to count %dAs (%2.1f%%) and correction to add %dAs/h (%2.1fmA)", capacityToAddToCountAs, (float)capacityToAddToCountAs / 280 / 36,
																							     asPerHourToAdd, mAToAdd);
			Log('e', "Capacity was %uAs (%2.1f%%), correction was %dAs/h (%2.1fmA)", CanThisGetBatteryCountedCapacityAs(), (float)CanThisGetBatteryCountedCapacityAs() / 280 / 36, 
																					 CanThisGetBatteryAgingAsPerHour(),    (float)CanThisGetBatteryAgingAsPerHour() / 3.6);
			CanThisSetBatteryCountedCapacityAs(CanThisGetBatteryCountedCapacityAs() + capacityToAddToCountAs);
			CanThisSetBatteryAgingAsPerHour   (CanThisGetBatteryAgingAsPerHour()    + asPerHourToAdd / RATE_AS_PER_HOUR_DIVISOR);
			BatterySetCalTime(now);
			Log('e', "Capacity now %uAs (%2.1f%%), correction now %dAs/h (%2.1fmA)", CanThisGetBatteryCountedCapacityAs(), (float)CanThisGetBatteryCountedCapacityAs() / 280 / 36, 
																					 CanThisGetBatteryAgingAsPerHour(),    (float)CanThisGetBatteryAgingAsPerHour() / 3.6);
																								 
			recordAging(now, CanThisGetBatteryAgingAsPerHour());
			setDo(DO_HOME_TARGET);
			BatterySetPlotDir(DIR_NONE); //This is needed to prevent an increment from taking place for one scan
		}
		if (CanThisGetBatteryCapacityTargetPercent() >= _plotMaxPercent) BatterySetPlotDir(DIR_NONE);
		if (CanThisGetBatteryCapacityTargetPercent() <= _plotMinPercent) BatterySetPlotDir(DIR_NONE);
		if (_plotDir == DIR_UP  ) CanThisSetBatteryCapacityTargetPercent(CanThisGetBatteryCapacityTargetPercent() + _plotIncPercent);
		if (_plotDir == DIR_DOWN) CanThisSetBatteryCapacityTargetPercent(CanThisGetBatteryCapacityTargetPercent() - _plotIncPercent);
	}
}

