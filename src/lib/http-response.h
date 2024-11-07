#include <stdint.h>
#include <time.h>

extern char* HttpResponseGetBufferStart(void);
extern int   HttpResponseGetLength(void);

extern void HttpResponseAddDir          (                  char *folder);
extern void HttpResponseAddAsciiFile    (                  char *file  );
extern void HttpResponseAddAsciiFileRows(                  char *file  );
extern void HttpResponseAddString       (                  char *value );
extern void HttpResponseAddChar         (                   char value );
extern void HttpResponseAddTime         ( char *format,   time_t value );
extern void HttpResponseAddBool         ( char *format,     char value );
extern void HttpResponseAddFloat        ( char *format,    float value );
extern void HttpResponseAddDouble       ( char *format,   double value );
extern void HttpResponseAddS8           (                 int8_t value );
extern void HttpResponseAddU8           (                uint8_t value );
extern void HttpResponseAddX8           (                uint8_t value );
extern void HttpResponseAddS16          (                int16_t value );
extern void HttpResponseAddU16          (               uint16_t value );
extern void HttpResponseAddX16          (               uint16_t value );
extern void HttpResponseAddS32          (                int32_t value );
extern void HttpResponseAddU32          (               uint32_t value );
extern void HttpResponseAddX32          (               uint32_t value );
extern void HttpResponseAddS64          (                int64_t value );
extern void HttpResponseAddU64          (               uint64_t value );
extern void HttpResponseAddX64          (               uint64_t value );

extern void HttpResponseSend     (void);


/*Error handling
Four possibilities:
-	Fatal error ie can do nothing about, must bomb out and try again.
	An example is the client closing the socket unexpectedly.
	Log the error.
	Ignore the error type.
	Set the return value to FAILURE (-1).
	
-	Cannot handle request - will result in a 1xx, 3xx, 4xx, or 5xx response.
	An example is that could not decipher the request.
	Log the error only if debugging - normally don't need to do this.
	Set the error type to eg: 500 (Internal Server Error) or 400 (Bad Request).
	Set the error message to what you want the user to see.
	Set the return value to SUCCESS (0).
	
-	Cannot deal with an element in part of the response - will result in a 200 OK response.
	An example is not being able to open a file in the index.
	Put an explanation instead of the text that would have been displayed in that part of the page.
	Set the error type to eg: 200 (OK).
	Set the return value to SUCCESS (0).
	
-	No problem
	Set the error type to eg: 200 (OK).
	Set the return value to SUCCESS (0).
*/
#define HTTP_RESPONSE_MESSAGE_SIZE 1024
extern int  HttpResponseType;      //Only 200 OK, 400 Bad Request, 404 Not Found, 500 Internal Server Error, or 501 Not Implemented are understood
extern char HttpResponseMessage[]; //This will be sent as the body of a warning message to supplement the above

#define HTTP_RESPONSE_COOKIE_SIZE 20
extern char HttpResponseCookie[];
extern int  HttpResponseCookieAge;

