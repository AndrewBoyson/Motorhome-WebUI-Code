#include <stdint.h>

extern void (*LinUnhandledBytesHandler)(void); //Set by LinThisInit
extern void (*LinIdHandler            )(void); //Set by LinThisInit
extern void (*LinDataHandler          )(void); //Set by LinThisInit
extern int    LinDataLength;                   //Set by LinIdHandler - must be -1 if id is not handled or zero if no bytes to read

extern int   LinGetUnhandledBytesCount(void);
extern char* LinGetUnhandledBytes(void);

extern char  LinGetProtectedId(void);
extern char  LinGetFrameId(void);
extern char* LinGetDataPointer(void);
extern char  LinGetCheckSum(void);

extern void LinInit      (void);                 //Safe to be called multiple times
extern void LinReadOrWait(void);                 //Called by LinThis
extern int  LinSend      (int len, void* pData); //Returns 0 on success or -1 on failure
extern int  LinSendBreak (void);                 //Returns 0 on success or -1 on fail

extern char LinCheckIdParityIsOk(char id); //Returns 1 if ok, 0 if not
extern char LinStripIdParity    (char id); //Returns the raw id
extern char LinAddIdParity      (char id); //Returns the id with parity
extern char LinGetCheckSumWithId(char id, int bufferLen, char* pBuffer);
extern char LinGetCheckSumWithoutId(      int bufferLen, char* pBuffer);