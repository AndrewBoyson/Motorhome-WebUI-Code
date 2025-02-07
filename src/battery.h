#include <stdint.h>

#define BATTERY_CAPACITY_AH  280
#define BATTERY_AS_PER_PERCENT  (BATTERY_CAPACITY_AH*36)

extern void BatteryInit(void);
extern void BatteryPoll(void);

extern void BatterySetMode           (int8_t   v);
extern char BatterySetModeAsString   (char*    v);
extern void BatterySetAwayPercent    (uint8_t  v);
extern void BatterySetHomePercent    (uint8_t  v);
extern void BatterySetRestMa         (int32_t  v);
extern void BatterySetPlotRestSeconds(uint32_t v);
extern void BatterySetPlotDir        (int8_t   v);
extern void BatterySetPlotIncPercent (uint8_t  v);
extern void BatterySetPlotMaxPercent (uint8_t  v);
extern void BatterySetPlotMinPercent (uint8_t  v);
extern void BatterySetCalAs          (uint32_t v);
extern void BatterySetCalMv          (int16_t  v);
extern void BatterySetCalAsPerMv     (uint32_t v);
extern void BatterySetCalTime        (uint32_t v);
extern void BatterySetCalMinAs       (uint32_t v);

extern int8_t   BatteryGetMode           (void);
extern char*    BatteryGetModeAsString   (void);
extern uint8_t  BatteryGetAwayPercent    (void);
extern uint8_t  BatteryGetHomePercent    (void);
extern int32_t  BatteryGetRestMa         (void);
extern uint32_t BatteryGetPlotRestSeconds(void);
extern int8_t   BatteryGetPlotDir        (void);
extern uint8_t  BatteryGetPlotIncPercent (void);
extern uint8_t  BatteryGetPlotMaxPercent (void);
extern uint8_t  BatteryGetPlotMinPercent (void);
extern uint32_t BatteryGetCalAs          (void);
extern int16_t  BatteryGetCalMv          (void);
extern uint32_t BatteryGetCalAsPerMv     (void);
extern uint32_t BatteryGetCalTime        (void);
extern uint32_t BatteryGetCalMinAs       (void);

extern uint32_t BatteryGetRestedSeconds  (void);
extern char     BatteryGetCalOkToStart   (void);