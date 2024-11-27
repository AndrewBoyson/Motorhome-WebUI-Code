#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "http-response.h"
#include "http-request.h"
#include "http-credentials.h"
#include "../http-this.h"
#include "log.h"

#define SIZE_NAME 100
#define SIZE_VALUE 1000

static char *p;  //Current position

//Handle saves
int HttpGetParseX64  (char *text, uint64_t* pValue) {
	if (sscanf(text, "%llx",  pValue) <= 0)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseX64: invalid '%s'", text);
		return 1;
	}
	return 0;
}
int HttpGetParseS64  (char *text,  int64_t* pValue) {
	if (sscanf(text, "%'lld",  pValue) <= 0)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseS64: invalid '%s'", text);
		return 1;
	}
	return 0;
}
int HttpGetParseU64  (char *text, uint64_t* pValue) {
	if (sscanf(text, "%'llu",  pValue) <= 0)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseU64: invalid '%s'", text);
		return 1;
	}
	return 0;
}
int HttpGetParseS32  (char *text,  int32_t* pValue) {
	if (sscanf(text, "%'d",  pValue) <= 0)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseS32: invalid '%s'", text);
		return 1;
	}
	return 0;
}
int HttpGetParseU32  (char *text, uint32_t* pValue) {
	if (sscanf(text, "%'u",  pValue) <= 0)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseU32: invalid '%s'", text);
		return 1;
	}
	return 0;
}
int HttpGetParseS16  (char *text,  int16_t* pValue) {
	int32_t value = 0;
	if (sscanf(text, "%'d",  &value) <= 0)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseS16: invalid '%s'", text);
		return 1;
	}
	if (value > INT16_MAX || value < INT16_MIN)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseS16: too big '%s'", text);
		return 1;
	}
	*pValue = (int16_t)value;
	return 0;
}
int HttpGetParseU16  (char *text, uint16_t* pValue) {
	uint32_t value = 0;
	if (sscanf(text, "%'u",  &value) <= 0)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseU16: invalid '%s'", text);
		return 1;
	}
	if (value > UINT16_MAX)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseU16: too big '%s'", text);
		return 1;
	}
	*pValue = (uint16_t)value;
	return 0;
}
int HttpGetParseS8   (char *text,   int8_t* pValue) {
	int32_t value = 0;
	if (sscanf(text, "%'d",  &value) <= 0)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseS8: invalid '%s'", text);
		return 1;
	}
	if (value > INT8_MAX || value < INT8_MIN)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseS8: too big '%s'", text);
		return 1;
	}
	*pValue = (int8_t)value;
	return 0;
}
int HttpGetParseU8   (char *text,  uint8_t* pValue) {
	uint32_t value = 0;
	if (sscanf(text, "%'u",  &value) <= 0)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseU8: invalid '%s'", text);
		return 1;
	}
	if (value > UINT8_MAX)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseU8: too big '%s'", text);
		return 1;
	}
	*pValue = (uint8_t)value;
	return 0;
}
int HttpGetParseFloat(char *text,    float* pValue) {
	if (sscanf(text, "%f",  pValue) <= 0)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GetParseFloat: invalid '%s'", text);
		return 1;
	}
	return 0;
}

//Handle posts
static int getChar(char *r) { //returns -1 on error, +1 to finish and 0 to continue

	char sHex[3] = {0};

	//Decode the character if need be
	switch (*p)
	{
		case 0:
			*r = 0;
			return 1;
		case '+' :
			*r  = ' ';
			break;
		case '%':
			//First hex digit
			p++; if (!*p) { *r = 0; return 1; }
			sHex[0] = *p;

			//Second hex digit
			p++; if (!*p) { *r = 0; return 1; }
			sHex[1] = *p;

			//Convert hex digits to ascii
			int i;
			if (sscanf(sHex, "%x", &i) < 1)
			{
				HttpResponseType = 400; //Bad Request
				snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "Invalid URL encoded sequence '%%%2s'", sHex);
				return -1;
			}
			*r = (char)i;
			break;
		default:
			*r = *p;
			break;
	}

	//Advance the pointer
	p++;
	return 0;
}
static int readNameValue  (char* name, char* value) { //returns -1 on error, +1 to finish and 0 to continue. Leaves the pointer at the start of the next pair

	char c = 0;
	int  n = 0;
	int  v = 0;
	char isName = 1;
	*name = 0;
	*value = 0;
	
	while (1)
	{
		int r = getChar(&c);
		if (r) return r;
		switch (c)
		{
			case   0: return 1;
			case '&': return 0;
			case '=':
				isName = 0;
				break;
			default:
				if (isName)
				{
					if (n < SIZE_NAME-1)
					{
						name[n] = c;
						n++;
						name[n] = 0;
					}
				}
				else
				{
					if (v < SIZE_VALUE-1)
					{
						value[v] = c;
						v++;
						value[v] = 0;
					}
				}
				break;
		}
	}
}
static int handleNameValue(unsigned rid, char* name, char* value) { //returns -1 to indicate an error is to be handled
	//Log('d', "%u: '%s'='%s'", rid, name, value);
	int r = HttpThisNameValue(rid, name, value); //returns -1 if an error is to be handled, 1 if not handled, 0 if handled ok
	if (r == -1) return -1;
	if (r)
	{
		//Deal with unknown GET
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "GET: did not recognise '%s'='%s'", name,value);
		return -1;
	}
	return 0;
}
int HttpGet(char* query) { //returns -1 to indicate an error is to be handled

	//Log('d', "'%s'", query);

	//Keep a request id to enable multiple name value pairs to be linked when handled
	static unsigned rid = 0;
	rid++;
	
	//Read the name
	char  name[SIZE_NAME];
	char value[SIZE_VALUE];
	
	p = query;
	while(1)
	{
		int r = readNameValue(name, value); //returns -1 on error, +1 to finish and 0 to continue.
		
		if (r == -1) return -1; //Handle error
			
		if (strcmp(name, "password") == 0)
		{
			if (CredentialsVerifyPassword(value))
			{
				HttpRequestAuthorised = 1;
				CredentialsMakeCookie(HttpResponseCookie);
				HttpResponseCookieAge = 400 * 24 * 3600;
			}
			break; //Finish
		}
		if (r) break; //Finish
	}
	
	//Don't handle any request unless authorised
	if (!HttpRequestAuthorised) return 0; //Quietly return without indication of an error
	
	p = query;
	while(1)
	{
		int r = readNameValue(name, value); //returns -1 on error, +1 to finish and 0 to continue.
		if (r == -1) return -1;
		if (*name && strcmp(name, "password")) //Only handle names which are not empty and which aren't system names
		{
			if (handleNameValue(rid, name, value)) return -1; //returns 0 on success; -1 on error
		}
		if (r) break; //Finish
	}
	return 0;
}