#include <stdint.h>

extern void CanInit      (void);                                 //Safe to be called multiple times
extern int  CanReadOrWait(int32_t* pId, int* pLen, void* pData); //Returns 0 on success
extern int  CanSend      (int32_t   id, int   len, void* pData); //Returns 0 on success
