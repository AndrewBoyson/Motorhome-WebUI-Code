#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "http-response.h"
#include "http-request.h"
#include "../http-this.h"
#include "../global.h"
#include "../values.h"
#include "../lib/log.h"

//static char *p;  //Current position
//static char *pl; //Last character to be handled (set after a POST from Content-Length)


//Local convention is that:
//Paths     starting with a '/' are absolute and should be used as is
//Paths not starting with a '/' are relative to WWW_FOLDER and need to be prepended with '/home/pi/server/www/'

static char getNextChar() { //Returns 0 if error or no more characters
	return HttpRequestGetContentChar();
}
//Handle posts
static int decodePercent(char* r) {
	char sHex[3];
	
	//First hex digit
	char c = getNextChar();
	if (!c) return -1;
	sHex[0] = c;

	//Second hex digit
	c = getNextChar();
	if (!c) return -1;
	sHex[1] = c;

	//Convert hex digits to ascii
	int i;
	if (sscanf(sHex, "%x", &i) < 1)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "Invalid URL encoded sequence '%%%2s'", sHex);
		return -1;
	}
	*r = (char)i;
	
	return 0;
}
static int handlePostReadName     (int length, char *name )               { //Reads the name and leaves the pointer at the start of the value
	name[0] = 0;
	int n = 0;
	while (1)
	{
		char c = getNextChar();
		switch (c)
		{
			case  0 : return 0; //Reached end of content
			case '=': return 0; //reached end of name
			case '%':
			{
				int r = decodePercent(&c);
				if (r) return -1;
				break;
			}
			case '+':
			{
				c = ' ';
				break;
			}
		}
		if (n < length - 1)
		{
			name[n++] = c;
			name[n]   = 0;
		}
	}
}
static int handlePostReadValue    (int length, char *value)               { //Reads the value and leave the pointer at the start of the next name
	value[0] = 0;
	int v = 0;

	while (1)
	{
		char c = getNextChar();
		switch (c)
		{
			case  0 : return 0; //Reached end of content
			case '&': return 0; //Reached end of value
			case '%':
			{
				int r = decodePercent(&c);
				if (r) return -1;
				break;
			}
			case '+':
			{
				c = ' ';
				break;
			}
		}
		if (v < length - 1)
		{
			value[v++] = c;
			value[v] = 0;
		}
	}
}
static int handlePostEditSave     (char *filename)
{
	char fullpath[100];
	HttpThisMakeFullPath(filename, fullpath, sizeof(fullpath));
	
	//Log('w' ,"Saving to file %s", filename);
	FILE *fp = fopen(fullpath, "w");
	if (fp == NULL)
	{
		HttpResponseType = 400; //Bad Request
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "Could not open file to save POST %.100s", filename);
		return -1;
	}
	char finished = 0;
	while (1)
	{
		char c = getNextChar();
		switch (c)
		{
			case   0 : finished = 1; break; //Reached end of content
			case  '&': finished = 1; break; //Reached end of value
			case '\r': finished = 1; break; //Reached end of value
			case '%':
			{
				int r = decodePercent(&c);
				if (r)
				{
					fclose(fp);
					return -1;
				}
				break;
			}
			case '+':
			{
				c = ' ';
				break;
			}
		}
		if (finished) break;
		if (fputc(c, fp) == EOF)
		{
			HttpResponseType = 500; //Internal Server Error
			snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "ERROR writing to file to save POST %s", filename);
			fclose(fp);
			return -1;
		}
	}
	if (fflush(fp))
	{
		HttpResponseType = 500; //Internal Server Error
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "ERROR flushing file to save POST %s", filename);
		fclose(fp);
		return -1;
	}
	if (fclose(fp))
	{
		HttpResponseType = 500; //Internal Server Error
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "Could not close file to save POST %s", filename);
		return -1;
	}

	return 0;
}
static int handlePostEdit         (char *filename)            {

	//Read the name
	char name[20];
	if (handlePostReadName(sizeof(name), name)) return -1;
	
	//Log('w', "Name='%s'", name);

	if (strcmp(name, "content") == 0) return handlePostEditSave(filename);

	HttpResponseType = 400; //Bad Request
	snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "Handle POST name 'save': expected 'content' but had '%s'", name);
	return -1;
}
int HttpPost             () {
	
	//Read the name
	char  name[20];
	char value[100];
	if (handlePostReadName (sizeof(name ), name )) return -1;
	if (handlePostReadValue(sizeof(value), value)) return -1;

	//Log('w', "Name='%s', Value='%s'", name, value);

	//Handle the name
	if (strcmp(name, "save"          ) == 0) return handlePostEdit    (value);

	//Deal with unknown POST
	HttpResponseType = 400; //Bad Request
	snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "POST: did not recognise '%s'", name);
	return -1;
}
