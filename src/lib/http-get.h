#include <stdint.h>

extern int HttpGetParseS64  (char *text,  int64_t* pValue);
extern int HttpGetParseU64  (char *text, uint64_t* pValue);
extern int HttpGetParseX64  (char *text, uint64_t* pValue);
extern int HttpGetParseS32  (char *text,  int32_t* pValue);
extern int HttpGetParseU32  (char *text, uint32_t* pValue);
extern int HttpGetParseS16  (char *text,  int16_t* pValue);
extern int HttpGetParseU16  (char *text, uint16_t* pValue);
extern int HttpGetParseS8   (char *text,   int8_t* pValue);
extern int HttpGetParseU8   (char *text,  uint8_t* pValue);
extern int HttpGetParseFloat(char *text,    float* pValue);
extern int HttpGetParseChar (char *text,     char* pValue);

extern int HttpGet(char* query);