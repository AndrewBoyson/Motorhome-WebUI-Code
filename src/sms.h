#include <stdarg.h>

extern void  SmsSetUserName(char* text);
extern char* SmsGetUserName(void);
extern void  SmsSetPassword(char* text);
extern char* SmsGetPassword(void);
extern void  SmsInit(void);

extern void  SmsSend (char* number, char* text);
extern void  SmsSendV(char* number, const char *format, va_list args);
extern void  SmsSendF(char* number, const char *format, ...);
extern void  SmsHandleRequest(char* sender, char* request);
