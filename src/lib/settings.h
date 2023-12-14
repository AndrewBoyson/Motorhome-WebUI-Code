#include <time.h>
#include <stdint.h>

extern int SettingsGetV     (const char *name, const char *format, ...);
extern int SettingsGetS8    (const char* name, int8_t*   pValue);
extern int SettingsGetS16   (const char* name, int16_t*  pValue);
extern int SettingsGetS32   (const char* name, int32_t*  pValue);
extern int SettingsGetU8    (const char* name, uint8_t*  pValue);
extern int SettingsGetU16   (const char* name, uint16_t* pValue);
extern int SettingsGetU32   (const char* name, uint32_t* pValue);
extern int SettingsGetTime  (const char* name, time_t*   pValue);
extern int SettingsGetString(const char* name, char*       text);
extern int SettingsGetChar  (const char* name, char*     pValue);

extern int SettingsSetV     (const char *name, const char *format, ...);
extern int SettingsSetS8    (const char* name, int8_t     value);
extern int SettingsSetS16   (const char* name, int16_t    value);
extern int SettingsSetS32   (const char* name, int32_t    value);
extern int SettingsSetU8    (const char* name, uint8_t    value);
extern int SettingsSetU16   (const char* name, uint16_t   value);
extern int SettingsSetU32   (const char* name, uint32_t   value);
extern int SettingsSetTime  (const char* name, time_t     value);
extern int SettingsSetString(const char* name, char*       text);
extern int SettingsSetChar  (const char* name, char       value);