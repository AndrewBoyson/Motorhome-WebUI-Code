#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 

#include "http-server.h"
#include "http-get.h"
#include "http-post.h"
#include "http-response.h"
#include "log.h"
#include "../global.h"
#include "http-credentials.h"

#define HEADER_NAME_SIZE   20 // eg 'Content-Length'
#define HEADER_VALUE_SIZE 100 // eg '1000'

#define METHOD_SIZE    10 // eg 'POST', 'GET', 'HEAD'.
#define RESOURCE_SIZE  50 // eg '/' or '/favicon.ico'. Leave room to add absolute path to www and '.inc'.
#define QUERY_SIZE   2000 // eg 'batterycapacity=50000'
#define COOKIE_SIZE HEADER_VALUE_SIZE // 'id=f4Gt5'

char HttpRequestMethod  [  METHOD_SIZE]; // POST, GET, HEAD
char HttpRequestResource[RESOURCE_SIZE]; // '/' or '/favicon.ico' etc
char HttpRequestQuery   [   QUERY_SIZE]; // 'batterycapacity=50000' etc
char HttpRequestCookie  [  COOKIE_SIZE]; // 'id=f4Gt5'

char HttpRequestAuthorised = 0; //Reset at the start of each request and set if a valid cookie or a valid password is received.
	
static int _contentLength = 0;
static int _contentCount  = 0;

static char getNextChar() { //Returns 0 if error or no more characters
	return HttpServerReadNextChar();
}

char HttpRequestGetContentChar() { //Returns 0 if error or no more characters
	if (_contentCount >= _contentLength) return 0;
	_contentCount++;
	return getNextChar();
}
//Read the request and work out what to do
static int handleRequestHeader    (char *name,    char *value)    {
	if (strcmp(name, "Content-Length") == 0)
	{
		int r = sscanf(value, "%d", &_contentLength);
		if (r < 0)
		{
			HttpResponseType = 400; // Bad request
			strcpy(HttpResponseMessage, "Content-Length header invalid value");
			return -1;
		}
	}
	if (strcmp(name, "Cookie") == 0)
	{
		strncpy(HttpRequestCookie, value, COOKIE_SIZE);
		HttpRequestCookie[COOKIE_SIZE-1] = 0; //Guarantee a null terminated string
		if (CredentialsVerifyCookie(HttpRequestCookie)) HttpRequestAuthorised = 1;	//Establish if the request is authorised by cookie
	}
	return 0;
}
static int handleRequestTopLine   (char* method, char* resource, char* query) {

//eg GET /image?text=hello HTTP1.1 <CR><LF>

    char isMethod   = 1;
    char isResource = 0;
    char isQuery    = 0;

	int i = 0;

	method  [0] = 0;
	resource[0] = 0;
    query   [0] = 0;

	while (1)
	{
		char c = getNextChar();
		switch(c)
		{
			case '\n': return  0; //Leave the pointer at the start of the next line
			case '\r':  continue ; //Ignore carriage returns
            case   0 :
				HttpResponseType = 400; // Bad request
				strcpy(HttpResponseMessage, "Top line ended before EOL");
				return -1; //Line has finished before it should - error
			case  ' ':
				if (isMethod)
				{
					isMethod    = 0;
					isResource  = 1;
					isQuery     = 0;
				}
				else
				{
					isMethod    = 0;
					isResource  = 0;
					isQuery     = 0;
				}
				i = 0;
				break;
			case '?':
				if (isResource)
				{
					isMethod    = 0;
					isResource  = 0;
					isQuery     = 1;
				}
				else
				{
					HttpResponseType = 400; // Bad request
					strcpy(HttpResponseMessage, "Top line had ? in the wrong place");
					return -1;
				}
				i = 0;
				break;
			default:
				if (isMethod   && i <   METHOD_SIZE - 1) { method  [i] = c; method  [i+1] = 0; }
				if (isResource && i < RESOURCE_SIZE - 1) { resource[i] = c; resource[i+1] = 0; }
				if (isQuery    && i <    QUERY_SIZE - 1) { query   [i] = c; query   [i+1] = 0; }
				i++;
				break;
		}
	}
	return 0;
}
static int handleRequestHeaderLine(void)                          {
	char name[HEADER_NAME_SIZE];
	int n = 0;
	char value[HEADER_VALUE_SIZE];
	int v = 0;
	char isName = 1;
	while (1)
	{
		char c = getNextChar();
		if (c ==  0  ) return 0; //No more characters are available
		if (c == '\r') continue; //Ignore carriage returns 
		if (c == '\n') break   ; //Leave the pointer at the start of the next line
		if (c == ':' )
		{
			isName = 0;
		}
		else
		{
			if (isName)
			{
				if (n < HEADER_NAME_SIZE - 1) name [n++] = c;
			}
			else
			{
				if (v == 0 && c == ' ') continue;
				if (v < HEADER_VALUE_SIZE - 1) value[v++] = c;
			}
		}
	}
	name [n] = 0;
	value[v] = 0;

	if (handleRequestHeader(name, value)) return -1;
	return n + v;
}

void HttpRequestReceive(void) {

	HttpRequestAuthorised = 0;
	
	HttpResponseType = 0;

	//Handle the top line
	strcpy(HttpResponseMessage, "handleRequestTopLine");
	if (handleRequestTopLine(HttpRequestMethod, HttpRequestResource, HttpRequestQuery)) return;
	
	//Handle the header lines
	_contentLength = 0;
	_contentCount  = 0;
	strcpy(HttpResponseMessage, "handleRequestHeaderLines");
	while(1)
	{
		int lineLength = handleRequestHeaderLine(); //This may set HttpRequestAuthorised from a cookie
		if (lineLength  < 0) return;
		if (lineLength == 0) break; //Reached the end of the request's header lines
	}

	//Handle the request top line
	char isGet  = strcmp(HttpRequestMethod, "GET" ) == 0;
	char isHead = strcmp(HttpRequestMethod, "HEAD") == 0;
	char isPost = strcmp(HttpRequestMethod, "POST") == 0;

	//Check request type is supported
	if (!isGet && !isHead && !isPost)
	{
		HttpResponseType = 501; // Not Implemented
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "We only support GET, HEAD and POST; you sent '%s'", HttpRequestMethod);
		return;
	}
	
	//Check resource is ok
	if (!*HttpRequestResource)
	{
		HttpResponseType = 400; // Bad request
		strcpy(HttpResponseMessage, "No resource requested");
		return;
	}

	//Handle query content
	if (HttpRequestQuery[0])
	{
		strcpy(HttpResponseMessage, "HttpGet");
		int r = HttpGet(HttpRequestQuery); //This may set HttpRequestAuthorised from a password
		if (r) return;
	}
	
	//Handle no authorization
	if (!HttpRequestAuthorised)
	{
		HttpResponseType = 200; //Success as we are going to show the login page
		strcpy(HttpRequestResource, "/login");
		return;
	}
	
	//Handle posted content
	if (isPost)
	{
		strcpy(HttpResponseMessage, "HttpPost");
		int r = HttpPost();
		if (r) return;
	}
	
	//Success
	HttpResponseType = 200;
}
