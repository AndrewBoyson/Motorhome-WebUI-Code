#include <stdint.h>
#include <string.h>

#include "lib/can.h"
#include "lib/can-reliable.h"
#include "lib/log.h"

#define CAN_ID_SERVER                    0x000
#define CAN_ID_TIME                       0x00

#define CAN_ID_BATTERY                   0x100
#define CAN_ID_COUNTED_AMP_SECONDS        0x00
#define CAN_ID_MA                         0x01
#define CAN_ID_OUTPUT_TARGET              0x02
#define CAN_ID_OUTPUT_STATE               0x03
#define CAN_ID_CHARGE_ENABLED             0x04
#define CAN_ID_DISCHARGE_ENABLED          0x05
#define CAN_ID_TEMPERATURE_8BFDP          0x06
#define CAN_ID_HEATER_TARGET              0x07
#define CAN_ID_HEATER_OUTPUT              0x08
#define CAN_ID_VOLTAGE                    0x09
#define CAN_ID_AGING_AS_PER_HOUR          0x0A
#define CAN_ID_HEATER_PROPORTIONAL        0x0B
#define CAN_ID_HEATER_INTEGRAL            0x0C

#define CAN_ID_TANK                0x200
#define CAN_ID_FRESH_TEMPERATURE    0x00
#define CAN_ID_FRESH_ROM            0x01
#define CAN_ID_FRESH_SUPPLY_MV      0x02
#define CAN_ID_FRESH_BASE_TEMP      0x03
#define CAN_ID_FRESH_BASE_MV        0x04
#define CAN_ID_FRESH_UV_PER_16TH    0x05
#define CAN_ID_FRESH_MV             0x06
#define CAN_ID_FRESH_DEPTH_MM       0x07
#define CAN_ID_FRESH_LITRES         0x08
#define CAN_ID_FRESH_SENSOR_FRONT   0x09
#define CAN_ID_FRESH_SENSOR_RIGHT   0x0A
#define CAN_ID_FRESH_TANK_WIDTH     0x0B
#define CAN_ID_FRESH_TANK_LENGTH    0x0C
#define CAN_ID_ACCELEROMETER_X_FLAT 0x0D
#define CAN_ID_ACCELEROMETER_Y_FLAT 0x0E
#define CAN_ID_ACCELEROMETER_Z_FLAT 0x0F
#define CAN_ID_ACCELEROMETER_X      0x10
#define CAN_ID_ACCELEROMETER_Y      0x11
#define CAN_ID_ACCELEROMETER_Z      0x12
#define CAN_ID_1WIRE_ROM_0          0x13
#define CAN_ID_1WIRE_ROM_1          0x14
#define CAN_ID_1WIRE_ROM_2          0x15
#define CAN_ID_1WIRE_ROM_3          0x16
#define CAN_ID_1WIRE_ROM_DATA_0     0x17
#define CAN_ID_1WIRE_ROM_DATA_1     0x18
#define CAN_ID_1WIRE_ROM_DATA_2     0x19
#define CAN_ID_1WIRE_ROM_DATA_3     0x1A
#define CAN_ID_LPG_MV               0x1B
#define CAN_ID_LPG_RESISTANCE       0x1C
#define CAN_ID_LPG_RESISTANCE_MIN   0x1D
#define CAN_ID_LPG_RESISTANCE_MAX   0x1E
#define CAN_ID_LPG_VOLUME_MIN       0x1F
#define CAN_ID_LPG_VOLUME_MAX       0x20
#define CAN_ID_LPG_VOLUME           0x21
#define CAN_ID_AMBIENT_OUTSIDE_ROM  0x22
#define CAN_ID_AMBIENT_HEATING_ROM  0x23
#define CAN_ID_AMBIENT_OUTSIDE_TEMP 0x24
#define CAN_ID_AMBIENT_HEATING_TEMP 0x25

#define CAN_ID_CONTROL              0x300
#define CAN_ID_PUMP                 0x00
#define CAN_ID_FILL                 0x01
#define CAN_ID_DRAIN                0x02
#define CAN_ID_INVERTER             0x03
#define CAN_ID_D_PLUS               0x04
#define CAN_ID_EHU                  0x05
#define CAN_ID_PUMP_MIN_LITRES      0x06
#define CAN_ID_PUMP_DPLUS_LITRES    0x07
#define CAN_ID_DRAIN_MAX_LITRES     0x08

static uint32_t _batteryCountedCapacity         = 0;
static int32_t  _batteryCurrentMa               = 0;
static uint8_t  _batteryCapacityTargetPercent   = 0;
static char     _batteryOutputState             = ' ';
static char     _batteryChargeEnabled           = 0;
static char     _batteryDischargeEnabled        = 0;
static int16_t  _batteryTemperature8bfdp        = 0;
static int16_t  _batteryTemperatureTargetTenths = 0;
static uint8_t  _batteryHeaterOutput8bfdp       = 0;
static uint16_t _batteryHeaterProportional      = 0;
static uint16_t _batteryHeaterIntegral          = 0;
static int16_t  _batteryVoltageMv               = 0;
static int16_t  _batteryAgingAsPerHour          = 0;
static int16_t  _tankFreshTemperature           = 0;
static uint64_t _tankFreshRom                   = 0;
static int16_t  _tankFreshSupplyMv              = 0;
static int16_t  _tankFreshBaseTemp16ths         = 0;
static int16_t  _tankFreshBaseMv                = 0;
static int16_t  _tankFreshUvPer16th             = 0;
static int16_t  _tankFreshMv                    = 0;
static int16_t  _tankFreshMm                    = 0;
static int16_t  _tankFreshLitres                = 0;
static int16_t  _sensorPosnFront                = 0; //Front of CL is +ve; rear of CL is -ve
static int16_t  _sensorPosnRight                = 0; //Right of CL is +ve; left of CL is -ve seen as facing front
static uint16_t _tankWidth                      = 0;
static uint16_t _tankLength                     = 0;
static int16_t  _tankAccelerometerXFlat         = 0;
static int16_t  _tankAccelerometerYFlat         = 0;
static int16_t  _tankAccelerometerZFlat         = 0;
static int16_t  _tankAccelerometerX             = 0;
static int16_t  _tankAccelerometerY             = 0;
static int16_t  _tankAccelerometerZ             = 0;
static uint64_t _tankRom0                       = 0;
static uint64_t _tankRom1                       = 0;
static uint64_t _tankRom2                       = 0;
static uint64_t _tankRom3                       = 0;
static int16_t  _tankRomData0                   = 0;
static int16_t  _tankRomData1                   = 0;
static int16_t  _tankRomData2                   = 0;
static int16_t  _tankRomData3                   = 0;
static int16_t  _tankLpgMv                      = 0;
static int16_t  _tankLpgResistance16ths         = 0;
static int16_t  _tankLpgResistanceMin16ths      = 0;
static int16_t  _tankLpgResistanceMax16ths      = 0;
static int16_t  _tankLpgVolumeMinMl             = 0;
static int16_t  _tankLpgVolumeMaxMl             = 0;
static int16_t  _tankLpgVolumeMl                = 0;
static uint64_t _ambientOutsideRom              = 0;
static uint64_t _ambientHeatingRom              = 0;
static int16_t  _ambientOutsideTemp16ths        = 0;
static int16_t  _ambientHeatingTemp16ths        = 0;
static char     _controlPump                    = 0;
static char     _controlFill                    = 0;
static char     _controlDrain                   = 0;
static char     _controlInverter                = 0;
static char     _controlDPlus                   = 0;
static char     _controlEhu                     = 0;
static int16_t  _controlPumpMinLitres           = 0;
static int16_t  _controlPumpDplusLitres         = 0;
static int16_t  _controlDrainMaxLitres          = 0;

 
uint32_t CanThisGetBatteryCountedCapacityAs       (){ return _batteryCountedCapacity;         }
 int32_t CanThisGetBatteryCurrentMa               (){ return _batteryCurrentMa;               }
 uint8_t CanThisGetBatteryCapacityTargetPercent   (){ return _batteryCapacityTargetPercent;   }
 char    CanThisGetBatteryOutputState             (){ return _batteryOutputState;             }
 char    CanThisGetBatteryChargeEnabled           (){ return _batteryChargeEnabled;           }
 char    CanThisGetBatteryDischargeEnabled        (){ return _batteryDischargeEnabled;        }
 int16_t CanThisGetBatteryTemperature8bfdp        (){ return _batteryTemperature8bfdp ;       }
 int16_t CanThisGetBatteryTemperatureTargetTenths (){ return _batteryTemperatureTargetTenths; }
 uint8_t CanThisGetBatteryHeaterOutput8bfdp       (){ return _batteryHeaterOutput8bfdp;       }
uint16_t CanThisGetBatteryHeaterProportional      (){ return _batteryHeaterProportional;      }
uint16_t CanThisGetBatteryHeaterIntegral          (){ return _batteryHeaterIntegral;          }
 int16_t CanThisGetBatteryVoltageMv               (){ return _batteryVoltageMv;               }
 int16_t CanThisGetBatteryAgingAsPerHour          (){ return _batteryAgingAsPerHour;          }
 
 int16_t CanThisGetTankFreshTemperature           (){ return _tankFreshTemperature;           }
uint64_t CanThisGetTankFreshRom                   (){ return _tankFreshRom;                   }
 int16_t CanThisGetTankFreshSupplyMv              (){ return _tankFreshSupplyMv;              }
 int16_t CanThisGetTankFreshBaseTemp16ths         (){ return _tankFreshBaseTemp16ths;         }
 int16_t CanThisGetTankFreshBaseMv                (){ return _tankFreshBaseMv;                }
 int16_t CanThisGetTankFreshUvPer16th             (){ return _tankFreshUvPer16th;             }
 int16_t CanThisGetTankFreshMv                    (){ return _tankFreshMv;                    }
 int16_t CanThisGetTankFreshDepthMm               (){ return _tankFreshMm;                    }
 int16_t CanThisGetTankFreshLitres                (){ return _tankFreshLitres;                }
 int16_t CanThisGetTankSensorPosnFront            (){ return _sensorPosnFront;                }
 int16_t CanThisGetTankSensorPosnRight            (){ return _sensorPosnRight;                }
uint16_t CanThisGetTankWidth                      (){ return _tankWidth      ;                }
uint16_t CanThisGetTankLength                     (){ return _tankLength     ;                }
 int16_t CanThisGetTankAccelerometerXFlat         (){ return _tankAccelerometerXFlat;         }
 int16_t CanThisGetTankAccelerometerYFlat         (){ return _tankAccelerometerYFlat;         }
 int16_t CanThisGetTankAccelerometerZFlat         (){ return _tankAccelerometerZFlat;         }
 int16_t CanThisGetTankAccelerometerX             (){ return _tankAccelerometerX;             }
 int16_t CanThisGetTankAccelerometerY             (){ return _tankAccelerometerY;             }
 int16_t CanThisGetTankAccelerometerZ             (){ return _tankAccelerometerZ;             }
uint64_t CanThisGetTankRom0                       (){ return _tankRom0    ;                   }
uint64_t CanThisGetTankRom1                       (){ return _tankRom1    ;                   }
uint64_t CanThisGetTankRom2                       (){ return _tankRom2    ;                   }
uint64_t CanThisGetTankRom3                       (){ return _tankRom3    ;                   }
 int16_t CanThisGetTankRomData0                   (){ return _tankRomData0;                   }
 int16_t CanThisGetTankRomData1                   (){ return _tankRomData1;                   }
 int16_t CanThisGetTankRomData2                   (){ return _tankRomData2;                   }
 int16_t CanThisGetTankRomData3                   (){ return _tankRomData3;                   }
 int16_t CanThisGetTankLpgMv                      (){ return _tankLpgMv;                      }
 int16_t CanThisGetTankLpgResistance16ths         (){ return _tankLpgResistance16ths;         }
 int16_t CanThisGetTankLpgResistanceMin16ths      (){ return _tankLpgResistanceMin16ths;      }
 int16_t CanThisGetTankLpgResistanceMax16ths      (){ return _tankLpgResistanceMax16ths;      }
 int16_t CanThisGetTankLpgVolumeMinMl             (){ return _tankLpgVolumeMinMl;             }
 int16_t CanThisGetTankLpgVolumeMaxMl             (){ return _tankLpgVolumeMaxMl;             }
 int16_t CanThisGetTankLpgVolumeMl                (){ return _tankLpgVolumeMl;                }
uint64_t CanThisGetAmbientOutsideRom              (){ return _ambientOutsideRom;              }
uint64_t CanThisGetAmbientHeatingRom              (){ return _ambientHeatingRom;              }
 int16_t CanThisGetAmbientOutsideTemp16ths        (){ return _ambientOutsideTemp16ths;        }
 int16_t CanThisGetAmbientHeatingTemp16ths        (){ return _ambientHeatingTemp16ths;        }
 
 char    CanThisGetControlWaterPump               (){ return _controlPump;                    }
 char    CanThisGetControlWaterFill               (){ return _controlFill;                    }
 char    CanThisGetControlWaterDrain              (){ return _controlDrain;                   }
 char    CanThisGetControlInverter                (){ return _controlInverter;                }
 char    CanThisGetControlDPlus                   (){ return _controlDPlus;                   }
 char    CanThisGetControlEhu                     (){ return _controlEhu  ;                   }
 int16_t CanThisGetControlPumpMinLitres           (){ return _controlPumpMinLitres;           }
 int16_t CanThisGetControlPumpDplusLitres         (){ return _controlPumpDplusLitres;         }
 int16_t CanThisGetControlDrainMaxLitres          (){ return _controlDrainMaxLitres;          }

static void set(void* pSetting, int32_t id, int len, void* pValue)
{
	if (memcmp(pSetting, pValue, len) != 0)
	{
		memcpy(pSetting, pValue, len);
		CanReliableSend(id, len, pValue);
	}
}	

void CanThisSendServerTime                   (uint32_t value){ CanSend                              (CAN_ID_SERVER  + CAN_ID_TIME                 , 4, &value);}
void CanThisSetBatteryCountedCapacityAs      (uint32_t value){ set(&_batteryCountedCapacity        , CAN_ID_BATTERY + CAN_ID_COUNTED_AMP_SECONDS  , 4, &value);}
void CanThisSetBatteryCapacityTargetPercent  (uint8_t  value){ set(&_batteryCapacityTargetPercent  , CAN_ID_BATTERY + CAN_ID_OUTPUT_TARGET        , 1, &value);}
void CanThisSetBatteryChargeEnabled          (char     value){ set(&_batteryChargeEnabled          , CAN_ID_BATTERY + CAN_ID_CHARGE_ENABLED       , 1, &value);}
void CanThisSetBatteryDischargeEnabled       (char     value){ set(&_batteryDischargeEnabled       , CAN_ID_BATTERY + CAN_ID_DISCHARGE_ENABLED    , 1, &value);}
void CanThisSetBatteryTemperatureTargetTenths(int16_t  value){ set(&_batteryTemperatureTargetTenths, CAN_ID_BATTERY + CAN_ID_HEATER_TARGET        , 2, &value);}
void CanThisSetBatteryHeaterProportional     (uint16_t value){ set(&_batteryHeaterProportional     , CAN_ID_BATTERY + CAN_ID_HEATER_PROPORTIONAL  , 2, &value);}
void CanThisSetBatteryHeaterIntegral         (uint16_t value){ set(&_batteryHeaterIntegral         , CAN_ID_BATTERY + CAN_ID_HEATER_INTEGRAL      , 2, &value);}
void CanThisSetBatteryAgingAsPerHour         (int16_t  value){ set(&_batteryAgingAsPerHour         , CAN_ID_BATTERY + CAN_ID_AGING_AS_PER_HOUR    , 2, &value);}
void CanThisSetTankFreshBaseTemp16ths        (int16_t  value){ set(&_tankFreshBaseTemp16ths        , CAN_ID_TANK    + CAN_ID_FRESH_BASE_TEMP      , 2, &value);}
void CanThisSetTankFreshBaseMv               (int16_t  value){ set(&_tankFreshBaseMv               , CAN_ID_TANK    + CAN_ID_FRESH_BASE_MV        , 2, &value);}
void CanThisSetTankFreshUvPer16th            (int16_t  value){ set(&_tankFreshUvPer16th            , CAN_ID_TANK    + CAN_ID_FRESH_UV_PER_16TH    , 2, &value);}
void CanThisSetTankSensorPosnFront           (int16_t  value){ set(&_sensorPosnFront               , CAN_ID_TANK    + CAN_ID_FRESH_SENSOR_FRONT   , 2, &value);}
void CanThisSetTankSensorPosnRight           (int16_t  value){ set(&_sensorPosnRight               , CAN_ID_TANK    + CAN_ID_FRESH_SENSOR_RIGHT   , 2, &value);}
void CanThisSetTankWidth                     (uint16_t value){ set(&_tankWidth                     , CAN_ID_TANK    + CAN_ID_FRESH_TANK_WIDTH     , 2, &value);}
void CanThisSetTankLength                    (uint16_t value){ set(&_tankLength                    , CAN_ID_TANK    + CAN_ID_FRESH_TANK_LENGTH    , 2, &value);}
void CanThisSetTankAccelerometerXFlat        (int16_t  value){ set(&_tankAccelerometerXFlat        , CAN_ID_TANK    + CAN_ID_ACCELEROMETER_X_FLAT , 2, &value);}
void CanThisSetTankAccelerometerYFlat        (int16_t  value){ set(&_tankAccelerometerYFlat        , CAN_ID_TANK    + CAN_ID_ACCELEROMETER_Y_FLAT , 2, &value);}
void CanThisSetTankAccelerometerZFlat        (int16_t  value){ set(&_tankAccelerometerZFlat        , CAN_ID_TANK    + CAN_ID_ACCELEROMETER_Z_FLAT , 2, &value);}
void CanThisSetTankFreshRom                  (uint64_t value){ set(&_tankFreshRom                  , CAN_ID_TANK    + CAN_ID_FRESH_ROM            , 8, &value);}
void CanThisSetTankLpgResistanceMin16ths     (int16_t  value){ set(&_tankLpgResistanceMin16ths     , CAN_ID_TANK    + CAN_ID_LPG_RESISTANCE_MIN   , 2, &value);}
void CanThisSetTankLpgResistanceMax16ths     (int16_t  value){ set(&_tankLpgResistanceMax16ths     , CAN_ID_TANK    + CAN_ID_LPG_RESISTANCE_MAX   , 2, &value);}
void CanThisSetTankLpgVolumeMinMl            (int16_t  value){ set(&_tankLpgVolumeMinMl            , CAN_ID_TANK    + CAN_ID_LPG_VOLUME_MIN       , 2, &value);}
void CanThisSetTankLpgVolumeMaxMl            (int16_t  value){ set(&_tankLpgVolumeMaxMl            , CAN_ID_TANK    + CAN_ID_LPG_VOLUME_MAX       , 2, &value);}
void CanThisSetAmbientOutsideRom             (uint64_t value){ set(&_ambientOutsideRom             , CAN_ID_TANK    + CAN_ID_AMBIENT_OUTSIDE_ROM  , 8, &value);}
void CanThisSetAmbientHeatingRom             (uint64_t value){ set(&_ambientHeatingRom             , CAN_ID_TANK    + CAN_ID_AMBIENT_HEATING_ROM  , 8, &value);}
void CanThisSetControlWaterPump              (char     value){ set(&_controlPump                   , CAN_ID_CONTROL + CAN_ID_PUMP                 , 1, &value);}
void CanThisSetControlWaterFill              (char     value){ set(&_controlFill                   , CAN_ID_CONTROL + CAN_ID_FILL                 , 1, &value);}
void CanThisSetControlWaterDrain             (char     value){ set(&_controlDrain                  , CAN_ID_CONTROL + CAN_ID_DRAIN                , 1, &value);}
void CanThisSetControlInverter               (char     value){ set(&_controlInverter               , CAN_ID_CONTROL + CAN_ID_INVERTER             , 1, &value);}
void CanThisSetControlPumpMinLitres          (int16_t  value){ set(&_controlPumpMinLitres          , CAN_ID_CONTROL + CAN_ID_PUMP_MIN_LITRES      , 2, &value);}
void CanThisSetControlPumpDplusLitres        (int16_t  value){ set(&_controlPumpDplusLitres        , CAN_ID_CONTROL + CAN_ID_PUMP_DPLUS_LITRES    , 2, &value);}
void CanThisSetControlDrainMaxLitres         (int16_t  value){ set(&_controlDrainMaxLitres         , CAN_ID_CONTROL + CAN_ID_DRAIN_MAX_LITRES     , 2, &value);}

uint32_t CanCounts[2048];

void CanThisReceive()
{
	int32_t id      = 0;
	int     length  = 0;
	int64_t data    = 0;

	int r = CanReadOrWait(&id, &length, &data);
	if (r) return;
	CanCounts[id]++;
	if (CanCounts[id] > 1000000000) for (int i = 0; i < 2048; i++) CanCounts[i] = 0; //Auto reset counts after a billion
	
	switch(id)
	{
		case CAN_ID_BATTERY + CAN_ID_COUNTED_AMP_SECONDS:   _batteryCountedCapacity         = (uint32_t)data; break;
		case CAN_ID_BATTERY + CAN_ID_MA:                    _batteryCurrentMa               = ( int32_t)data; break;
		case CAN_ID_BATTERY + CAN_ID_OUTPUT_TARGET:         _batteryCapacityTargetPercent   = ( uint8_t)data; break;
		case CAN_ID_BATTERY + CAN_ID_OUTPUT_STATE:          _batteryOutputState             = (    char)data; break;
		case CAN_ID_BATTERY + CAN_ID_CHARGE_ENABLED:        _batteryChargeEnabled           = (    char)data; break;
		case CAN_ID_BATTERY + CAN_ID_DISCHARGE_ENABLED:     _batteryDischargeEnabled        = (    char)data; break;
		case CAN_ID_BATTERY + CAN_ID_TEMPERATURE_8BFDP:     _batteryTemperature8bfdp        = ( int16_t)data; break;
		case CAN_ID_BATTERY + CAN_ID_HEATER_TARGET:         _batteryTemperatureTargetTenths = ( int16_t)data; break;
		case CAN_ID_BATTERY + CAN_ID_HEATER_OUTPUT:         _batteryHeaterOutput8bfdp       = ( uint8_t)data; break;
		case CAN_ID_BATTERY + CAN_ID_HEATER_PROPORTIONAL:   _batteryHeaterProportional      = (uint16_t)data; break;
		case CAN_ID_BATTERY + CAN_ID_HEATER_INTEGRAL:       _batteryHeaterIntegral          = (uint16_t)data; break;
		case CAN_ID_BATTERY + CAN_ID_VOLTAGE:               _batteryVoltageMv               = ( int16_t)data; break;
		case CAN_ID_BATTERY + CAN_ID_AGING_AS_PER_HOUR:     _batteryAgingAsPerHour          = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_TEMPERATURE:     _tankFreshTemperature           = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_ROM:             _tankFreshRom                   = (uint64_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_SUPPLY_MV:       _tankFreshSupplyMv              = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_BASE_TEMP:       _tankFreshBaseTemp16ths         = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_BASE_MV:         _tankFreshBaseMv                = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_UV_PER_16TH:     _tankFreshUvPer16th             = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_MV:              _tankFreshMv                    = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_DEPTH_MM:        _tankFreshMm                    = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_LITRES:          _tankFreshLitres                = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_SENSOR_FRONT:    _sensorPosnFront                = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_SENSOR_RIGHT:    _sensorPosnRight                = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_TANK_WIDTH:      _tankWidth                      = (uint16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_FRESH_TANK_LENGTH:     _tankLength                     = (uint16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_ACCELEROMETER_X:       _tankAccelerometerX             = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_ACCELEROMETER_Y:       _tankAccelerometerY             = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_ACCELEROMETER_Z:       _tankAccelerometerZ             = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_ACCELEROMETER_X_FLAT:  _tankAccelerometerXFlat         = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_ACCELEROMETER_Y_FLAT:  _tankAccelerometerYFlat         = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_ACCELEROMETER_Z_FLAT:  _tankAccelerometerZFlat         = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_1WIRE_ROM_0:           _tankRom0                       = (uint64_t)data; break;
		case CAN_ID_TANK    + CAN_ID_1WIRE_ROM_1:           _tankRom1                       = (uint64_t)data; break;
		case CAN_ID_TANK    + CAN_ID_1WIRE_ROM_2:           _tankRom2                       = (uint64_t)data; break;
		case CAN_ID_TANK    + CAN_ID_1WIRE_ROM_3:           _tankRom3                       = (uint64_t)data; break;
		case CAN_ID_TANK    + CAN_ID_1WIRE_ROM_DATA_0:      _tankRomData0                   = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_1WIRE_ROM_DATA_1:      _tankRomData1                   = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_1WIRE_ROM_DATA_2:      _tankRomData2                   = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_1WIRE_ROM_DATA_3:      _tankRomData3                   = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_LPG_MV:                _tankLpgMv                      = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_LPG_RESISTANCE:        _tankLpgResistance16ths         = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_LPG_RESISTANCE_MIN:    _tankLpgResistanceMin16ths      = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_LPG_RESISTANCE_MAX:    _tankLpgResistanceMax16ths      = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_LPG_VOLUME_MIN:        _tankLpgVolumeMinMl             = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_LPG_VOLUME_MAX:        _tankLpgVolumeMaxMl             = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_LPG_VOLUME:            _tankLpgVolumeMl                = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_AMBIENT_OUTSIDE_ROM:   _ambientOutsideRom              = (uint64_t)data; break;
		case CAN_ID_TANK    + CAN_ID_AMBIENT_HEATING_ROM:   _ambientHeatingRom              = (uint64_t)data; break;
		case CAN_ID_TANK    + CAN_ID_AMBIENT_OUTSIDE_TEMP:  _ambientOutsideTemp16ths        = ( int16_t)data; break;
		case CAN_ID_TANK    + CAN_ID_AMBIENT_HEATING_TEMP:  _ambientHeatingTemp16ths        = ( int16_t)data; break;
		case CAN_ID_CONTROL + CAN_ID_PUMP:                  _controlPump                    = (    char)data; break;
		case CAN_ID_CONTROL + CAN_ID_FILL:                  _controlFill                    = (    char)data; break;
		case CAN_ID_CONTROL + CAN_ID_DRAIN:                 _controlDrain                   = (    char)data; break;
		case CAN_ID_CONTROL + CAN_ID_INVERTER:              _controlInverter                = (    char)data; break;
		case CAN_ID_CONTROL + CAN_ID_D_PLUS:                _controlDPlus                   = (    char)data; break;
		case CAN_ID_CONTROL + CAN_ID_EHU:                   _controlEhu                     = (    char)data; break;
		case CAN_ID_CONTROL + CAN_ID_PUMP_MIN_LITRES:       _controlPumpMinLitres           = ( int16_t)data; break;
		case CAN_ID_CONTROL + CAN_ID_PUMP_DPLUS_LITRES:     _controlPumpDplusLitres         = ( int16_t)data; break;
		case CAN_ID_CONTROL + CAN_ID_DRAIN_MAX_LITRES:      _controlDrainMaxLitres          = ( int16_t)data; break;
		default:
				Log('w', "Unknown CAN message id %03X received containing %d bytes: %0*llX", id, length, length*2, data);
				break;
    }
}
void CanThisInit()
{
	CanInit();
}
