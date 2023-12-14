#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

#include <sys/stat.h>   //File date

#include "http-response.h"
#include "http-include.h"
#include "http-request.h"
#include "http-server.h"
#include "../http-this.h"
#include "../global.h"
#include "../lib/log.h"
#include "../lib/file.h"

#define PATH_SIZE 100

int  HttpResponseType;                                //Only 0 (no error), 400 Bad Request, 404 Not Found, 500 Internal Server Error, or 501 Not Implemented are understood
char HttpResponseMessage[HTTP_RESPONSE_MESSAGE_SIZE]; //This will be sent as the body of a warning message to supplement the above

static void addChar(char c) {
	HttpServerSendBytes(&c, 1);
}
static void addText(char* text) {
	while (1)
	{
		char c = *text++;
		if (!c) break;
		addChar(c);
	}
}
static void addTextN(char* text, int size) {
	while (1)
	{
		if (size-- <= 0) break;
		char c = *text++;
		if (!c) break;
		addChar(c);
	}
}

//Local convention is that:
//Paths     starting with a '/' are absolute and should be used as is
//Paths not starting with a '/' are relative to WWW_FOLDER and need to be prepended with '/home/pi/server/www/'

//Utilities for handling files

static time_t fileTimeOrZero(char* file) {
	char fullpath[PATH_SIZE];
	int r = HttpThisMakeFullPath(file, fullpath, PATH_SIZE);
	if (r) return 0;
	return FileDateOrZero(fullpath);
}

//Utilities for writing into non text area of buffer. On error: set error type and return failure; server will respond with a 4xx or 5xx response
static void  addHttpDate     (time_t t       ) {
	char sDate[30];
	struct tm tm;

	if (gmtime_r(&t, &tm) == 0)
	{
		LogErrno("addHttpDate - gmtime-r");
		return;
	}
	int length = strftime(sDate, 30, "%a, %e %b %Y %H:%M:%S GMT", &tm);
	if (length < 0)
	{
		LogErrno("addHttpDate - strftime");
		return;
	}
	addText(sDate);
	return;
}
static void  addBinaryFile   ( char *filename) {
	char fullpath[PATH_SIZE];
	int r = HttpThisMakeFullPath(filename, fullpath, PATH_SIZE);
	if (r) return;
	
	FILE *fp = fopen(fullpath, "r");
	if (fp == NULL)
	{
		Log('e', "addBinaryFile - fopen '%.100s'", fullpath);
		LogErrno("addBinaryFile - fopen");
		return;
	}

	while (1)
	{
		int c = fgetc(fp);
		if (c == EOF) break;
		addChar((char)c);
	}

	fclose(fp);
}

//Utilities for writing into buffer text part; any error is printed directly into the text so server will still respond with a 200 success response
void HttpResponseAddDir          (                  char *folder) {

	char fullpath[PATH_SIZE];
	int r = HttpThisMakeFullPath(folder, fullpath, PATH_SIZE);
	if (r) return;
	
	if (!FolderExists(fullpath)) return;
	
	void* d = FolderOpen(fullpath);
	if (!d)
	{
		addText("ERROR Could not open folder '");
		addText(folder);
		addText("'");
		return;
	}
	char* entry = 0;
	while(1)
	{
		entry = FolderNextEntry(d);
		if (!entry) break;
		addText(entry);
		addText("\r\n");
	}
	FolderClose(d);
}
void HttpResponseAddAsciiFile    (                  char *file  ) { //Adds the content of the file

	char fullpath[PATH_SIZE];
	int r = HttpThisMakeFullPath(file, fullpath, PATH_SIZE);
	if (r)
	{
		addText("HttpResponseAddAsciiFile - Could not make full path ");
		addText(fullpath);
		addText(" ");
		addText(strerror(errno));
		return;
	}
	
	FILE *fp = fopen(fullpath, "r");
	if (fp == NULL)
	{
		addText("HttpResponseAddAsciiFile - Could not open for read ");
		addText(fullpath);
		addText(" ");
		addText(strerror(errno));
		return;
	}
	while (1)
	{
		int c = fgetc(fp);
		if (c == EOF) break;
		addChar((char)c);
	}

	fclose(fp);
}
void HttpResponseAddAsciiFileRows(                  char *file  ) { //Adds a character for the number of rows needed to edit an ascii file

	char fullpath[PATH_SIZE];
	int r = HttpThisMakeFullPath(file, fullpath, PATH_SIZE);
	if (r)
	{
		addChar('1'); //Just add '1' if cannot determine length
		return;
	}
	
	FILE *fp = fopen(fullpath, "r");
	if (fp == NULL)
	{
		addChar('1'); //Just add '1' if cannot determine length
		return;
	}
	int lines = 0;
	while (1)
	{
		int c = fgetc(fp);
		if (c == EOF) break;
        if (c == '\n') lines++;
	}
	fclose(fp);
	char text[10];
	int length = sprintf(text, "%d", lines + 1);
	if (length < 0)
	{
		addText("ERROR Could not print file rows");
	}
	else
	{
		addTextN(text, sizeof(text));
	}
	return;
}
void HttpResponseAddString       (                  char *value ) { //Adds a string
	addText(value);
}
void HttpResponseAddChar         (                   char value ) { //Adds a char
	addChar(value);
}
void HttpResponseAddTime         ( char *format,   time_t value ) { //Adds a formatted date
	struct tm tm;
	if (gmtime_r(&value, &tm))
	{
		addText("ERROR Could not convert date");
		return;
	}
	char text[100];
	int length = strftime(text, sizeof(text), format, &tm);
	if (length < 0) addText ("ERROR Could not print date");
	else     		addTextN(text, sizeof(text));
}
void HttpResponseAddBool         ( char *format,     char value ) { //Adds a boolean displayed as truetext or falsetext from a format of truetext/falsetext eg 'Y/N' or 'checked'

	char *i = format;
	char isFalseText = 0;
	while (1)
	{
		if (*i == 0) break;
		if (!isFalseText && *i == '/')
		{
			if (value) break;
			isFalseText = 1;
			i++;
			continue;
		}
		if ( value && !isFalseText) addChar(*i);
		if (!value &&  isFalseText) addChar(*i);
		i++;
	}
}
void HttpResponseAddFloat        ( char *format,    float value ) { //Adds a formatted float
	char text[100];
	int length = snprintf(text, sizeof(text), format, value);
	if (length < 0) addText("ERROR Could not print float");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddDouble       ( char *format,   double value ) { //Adds a formatted double
	char text[100];
	int length = snprintf(text, sizeof(text), format, value);
	if (length < 0) addText("ERROR Could not print double");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddS8           (                 int8_t value ) { //Adds a formatted signed integer
	char text[100];
	int length = snprintf(text, sizeof(text), "%hhd", value);
	if (length < 0) addText("ERROR Could not print integer ");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddU8           (                uint8_t value ) { //Adds a formatted unsigned integer
	char text[100];
	int length = snprintf(text, sizeof(text), "%hhu", value);
	if (length < 0) addText("ERROR Could not print integer ");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddX8           (                uint8_t value ) { //Adds a formatted unsigned integer
	char text[100];
	int length = snprintf(text, sizeof(text), "%02hhX", value);
	if (length < 0) addText("ERROR Could not print integer ");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddS16          (                int16_t value ) { //Adds a formatted signed integer
	char text[100];
	int length = snprintf(text, sizeof(text), "%hd", value);
	if (length < 0) addText("ERROR Could not print integer ");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddU16          (               uint16_t value ) { //Adds a formatted unsigned integer
	char text[100];
	int length = snprintf(text, sizeof(text), "%hu", value);
	if (length < 0) addText("ERROR Could not print integer ");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddX16          (               uint16_t value ) { //Adds a formatted unsigned integer
	char text[100];
	int length = snprintf(text, sizeof(text), "%04hu", value);
	if (length < 0) addText("ERROR Could not print integer ");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddS32          (                int32_t value ) { //Adds a formatted signed long
	char text[100];
	int length = snprintf(text, sizeof(text), "%d", value);
	if (length < 0) addText("ERROR Could not print integer ");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddU32          (               uint32_t value ) { //Adds a formatted unsigned long
	char text[100];
	int length = snprintf(text, sizeof(text), "%u", value);
	if (length < 0) addText("ERROR Could not print integer ");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddX32          (               uint32_t value ) { //Adds a formatted unsigned long
	char text[100];
	int length = snprintf(text, sizeof(text), "%08X", value);
	if (length < 0) addText("ERROR Could not print integer ");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddS64          (                int64_t value ) { //Adds a formatted signed long long
	char text[100];
	int length = snprintf(text, sizeof(text), "%lld", value);
	if (length < 0) addText("ERROR Could not print integer ");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddU64          (               uint64_t value ) { //Adds a formatted unsigned long long
	char text[100];
	int length = snprintf(text, sizeof(text), "%llu", value);
	if (length < 0) addText("ERROR Could not print integer ");
	else            addTextN(text, sizeof(text));
}
void HttpResponseAddX64          (               uint64_t value ) { //Adds a formatted unsigned long long as hex
	char text[100];
	int length = snprintf(text, sizeof(text), "%016llX", value);
	if (length < 0) addText("ERROR Could not print integer ");
	else            addTextN(text, sizeof(text));
}

//Add response headers
static void addResponseHeaderDate        (void) {

	addText("Date: ");
	time_t t;
	if (time(&t) == -1)
	{
		LogErrno ("ERROR with time");
		addText("Error with time");
	}
	else
	{
		addHttpDate(t);
	}
	addText("\r\n");
}
void addStatusLine() {
	addText("HTTP/1.1");
	HttpResponseAddChar(' ');
	HttpResponseAddS32(HttpResponseType);
	HttpResponseAddChar(' ');
	switch (HttpResponseType)
	{
		case 200: addText("OK");                    break;
		case 400: addText("Bad Request");           break;
		case 404: addText("Not Found");             break;
		case 500: addText("Internal Server Error"); break;
		case 501: addText("Not Implemented");       break;
		default : addText("Unknown status");        break;
	}
	addText("\r\n");
}
static char _resourcePath[PATH_SIZE];
static char _resourceExt[10];
	
static time_t _resourceTimeLastModified ;

static void makeLocalResourcePathExtAndTime() {
	
	//Make resourcePath relative to www by removing any initial '/'
	if (*HttpRequestResource == '/') strncpy(_resourcePath, HttpRequestResource + 1, sizeof(_resourcePath));
	else                             strncpy(_resourcePath, HttpRequestResource    , sizeof(_resourcePath));
	_resourcePath[sizeof(_resourcePath)-1] = 0;
	
	//Make the default 'index' for '/'
	if (_resourcePath[0] == 0) strcpy(_resourcePath, "index");

	//Make our own copy of the extension to alter if need 
	char *pExt = PathExt(_resourcePath); //This is never null but it could be an empty string.
	strncpy(_resourceExt, pExt, sizeof(_resourceExt));
	_resourceExt[sizeof(_resourceExt)-1] = 0;

	//See if the file exists and, if it has no extension, try with '.inc'
	_resourceTimeLastModified = fileTimeOrZero(_resourcePath);
	if (!_resourceTimeLastModified)
	{
		if (_resourceExt[0] == 0)
		{
			strcat(_resourcePath, ".inc");
			strcpy(_resourceExt, "inc");
		}
		_resourceTimeLastModified = fileTimeOrZero(_resourcePath);
		if (!_resourceTimeLastModified)
		{
			HttpResponseType = 404; // Not Found
			snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "Could not find requested resource '%s'", _resourcePath);
		}
	}
}


void HttpResponseSend      () {
	
	if (!HttpResponseType)
	{
		Log ('e', "HttpResponseSend - No Status set - %s", HttpResponseMessage);
		return;
	}
	
/*	
	if (strlen(HttpRequestResource) > PATH_SIZE - 10)
	{
		Log ('e', "HttpResponseSend - Resource path is too long '%.100s", HttpRequestResource);
		return;
	}
	*/
	if (HttpResponseType == 200) makeLocalResourcePathExtAndTime(); //This may change HttpResponseType to 404 not found
	
	//Add the header and date
	addStatusLine();
	addResponseHeaderDate();
	addText("Connection: close\r\n");
	
	if (HttpResponseType == 200)
	{
		//Work out if it is an ajax file - used to determine the appropriate headers .
		char isAjax = strstr(_resourcePath, "-ajax") != 0; //isAjax contains a pointer to its occurrence or null if not found
	
		//Add the content type
		char noCache = 0;
		addText("Content-Type: ");
			 if (strcmp(_resourceExt, "ico" ) == 0) { addText("image/x-icon"             ); noCache = 0; }
		else if (strcmp(_resourceExt, "jpg" ) == 0) { addText("image/jpeg"               ); noCache = 0; }
		else if (strcmp(_resourceExt, "gif" ) == 0) { addText("image/gif"                ); noCache = 0; }
		else if (strcmp(_resourceExt, "png" ) == 0) { addText("image/png"                ); noCache = 0; }
		else if (strcmp(_resourceExt, "html") == 0) { addText("text/html; charset=UTF-8" ); noCache = 1; }
		else if (strcmp(_resourceExt, "inc" ) == 0)
		{
			if (isAjax)                             { addText("text/plain; charset=UTF-8"); noCache = 1; }
			else                                    { addText("text/html; charset=UTF-8" ); noCache = 1; }
		}
		else if (strcmp(_resourceExt, "css" ) == 0) { addText("text/css"                 ); noCache = 0; }
		else if (strcmp(_resourceExt, "js"  ) == 0) { addText("application/javascript"   ); noCache = 0; }
		else if (strcmp(_resourceExt, "txt" ) == 0) { addText("text/plain"               ); noCache = 1; }
		else                                        { addText("unknown"                  ); noCache = 1; }
		addText("\r\n");

		//Add cache control
		if (noCache)
		{
			addText("Cache-Control: no-cache\r\n");
		}
		else
		{
			addText("Last-Modified: ");
			addHttpDate(_resourceTimeLastModified);
			addText("\r\n");
		}
		
		//Disable CORS
		addText("Access-Control-Allow-Origin: *\r\n");
		
		//Add an extra <CR><LF> to show the end of headers
		addText("\r\n");
		
		//Add the content if not HEAD only
		if (strcmp(HttpRequestMethod, "HEAD") != 0)
		{
			if (strcmp(_resourceExt, "inc") == 0) HttpIncludeExpandFile(_resourcePath, 0, 0, 0, 0);
			else		          	              addBinaryFile        (_resourcePath);
		}
	}
	else
	{
		addText("Content-Type: text/plain\r\n");
		addText("\r\n");
		
		//Add the content if not HEAD only
		if (strcmp(HttpRequestMethod, "HEAD") != 0) addTextN(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE);
	}
}
