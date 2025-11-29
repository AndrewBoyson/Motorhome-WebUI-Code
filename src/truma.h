#include <stdint.h>


extern char     TrumaGetWantedRoomOn   (void);
extern char     TrumaGetWantedWaterOn  (void);
extern uint8_t  TrumaGetWantedRoomTemp (void);
extern char     TrumaGetWantedWaterTemp(void);
extern char     TrumaGetWantedFanMode  (void);
extern char     TrumaGetWantedEnergySel(void);
extern char     TrumaGetWantedElecPower(void);

extern void     TrumaSetWantedRoomOn   (char    v);
extern void     TrumaSetWantedWaterOn  (char    v);
extern void     TrumaSetWantedRoomTemp (uint8_t v);
extern void     TrumaSetWantedWaterTemp(char    v);
extern void     TrumaSetWantedFanMode  (char    v);
extern void     TrumaSetWantedEnergySel(char    v);
extern void     TrumaSetWantedElecPower(char    v);

extern uint8_t TrumaTargetRoomTemp  ;
extern char    TrumaTargetWaterTemp ;
extern char    TrumaTargetFanMode   ;
extern char    TrumaTargetEnergySel ;
extern char    TrumaTargetElecPower ;

extern uint16_t TrumaActualRoomTemp  ;
extern uint16_t TrumaActualWaterTemp ;
extern uint8_t  TrumaActualRecvStatus;
extern uint8_t  TrumaActualOpStatus  ;
extern uint8_t  TrumaActualErrorCode ;

extern void TrumaHadSendAcknowledgement(void);
extern char TrumaHasSameActualAsTarget(void);

extern void TrumaInit(void);