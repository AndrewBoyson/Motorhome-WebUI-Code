#include <stdint.h>

extern char   LinTrace;
extern char   LinAllowBusWrites;

extern void (*LinUnhandledBytesHandler)(void); //Set by LinThisInit
extern void (*LinIdHandler            )(void); //Set by LinThisInit
extern void (*LinDataHandler          )(void); //Set by LinThisInit
extern int    LinDataLength;                   //Set by LinIdHandler - must be 0 or positive to read bytes or specify ignore or unhandled

extern char LinGetBusIsActive(void);

extern int   LinGetUnhandledBytesCount(void);
extern char* LinGetUnhandledBytes(void);

extern char  LinGetProtectedId(void);
extern char  LinGetFrameId(void);
extern char* LinGetDataPointer(void);

extern void LinInit             (void);                 //Safe to be called multiple times
extern void LinReadOrWait       (void);                 //Called by LinThis
extern int  LinSend             (int len, void* pData); //Returns 0 on success or -1 on failure
extern int  LinSendBreak        (void);                 //Returns 0 on success or -1 on fail
extern void LinSendFrameResponse(int len, char* pData, char trace);
