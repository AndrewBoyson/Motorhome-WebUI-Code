#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>   //File date

#include "file.h"
#include "http-response.h"
#include "http-request.h"
#include "../http-this.h"

static  int include                  (char *item, char* p0, char* p1, char* p2, char* p3, char* p4, char* p5, char* p6, char* p7 );  //Declaration required to allow recursive sheet addition

static char _includeValue[100];
static uint32_t _includeCount;

int HttpIncludeExpandFile(char* filename, char* p0, char* p1, char* p2, char* p3, char* p4, char* p5, char* p6, char* p7) {	
	char fullpath[100];
	int r = HttpThisMakeFullPath(filename, fullpath, sizeof(fullpath));
	if (r)
	{
		HttpResponseType = 500; //Internal Server Error
		snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "Could not include file %s", fullpath);
		return -1;
	}
	
	FILE* fp = fopen(fullpath, "r");
	if (fp == NULL)
	{
		char *pExt = PathExt(fullpath); //This is never null but it could be an empty string.
		if (*pExt == 0) strcat(fullpath, ".inc");
		fp = fopen(fullpath, "r");
		if (fp == NULL)
		{
			HttpResponseType = 500; //Internal Server Error
			snprintf(HttpResponseMessage, HTTP_RESPONSE_MESSAGE_SIZE, "Could not include file %s", fullpath);
			return -1;
		}
	}

	char name[100];
	int  n = 0;
	char isName = 0;
	while (1)
	{
		int c = fgetc(fp);
		if (c == EOF) break;
		if (c ==   0) continue;
		if (c == '\r') continue; //Ignore extraneous <CR>s so that javascript can easily split lines on <LF>s
		if (c == '~')
		{
			if (isName)
			{
				name[n] = 0; //Terminate name string
				n = 0;       //Reset the start of name
				include(name, p0, p1, p2, p3, p4, p5, p6, p7);
			}
			isName = !isName;
			continue;
		}
		if (isName)  name[n++] = (char)c;
		else         HttpResponseAddChar((char)c);
	}

	fclose(fp);
	return 0;
}

static char* getNextParameterOrNull(char* item) {
	while (1)
	{
		if (*item == '^')
		{
			*item = 0;
			item++;
			return item;
		}
		if (*item == 0) return 0;
		item++;
	}
}
static char* assignHashedValue(char number, char* p0, char* p1, char* p2, char* p3, char* p4, char* p5, char* p6, char* p7)
{
	switch (number)
	{
		case '0': return p0;
		case '1': return p1;
		case '2': return p2;
		case '3': return p3;
		case '4': return p4;
		case '5': return p5;
		case '6': return p6;
		case '7': return p7;
		default:  return 0;
	}
}
static  int include(char *item, char* p0, char* p1, char* p2, char* p3, char* p4, char* p5, char* p6, char* p7) { // Called recursively

	char* name   = item;
	char* value0 = 0;
	char* value1 = 0;
	char* value2 = 0;
	char* value3 = 0;
	char* value4 = 0;
	char* value5 = 0;
	char* value6 = 0;
	char* value7 = 0;
	char* value8 = 0;
	
	//Get each parameter
	if (item  ) value0 = getNextParameterOrNull(item  );
	if (value0) value1 = getNextParameterOrNull(value0);
	if (value1) value2 = getNextParameterOrNull(value1);
	if (value2) value3 = getNextParameterOrNull(value2);
	if (value3) value4 = getNextParameterOrNull(value3);
	if (value4) value5 = getNextParameterOrNull(value4);
	if (value5) value6 = getNextParameterOrNull(value5);
	if (value6) value7 = getNextParameterOrNull(value6);
	if (value7) value8 = getNextParameterOrNull(value7);
	
	//If hashed replace a value with its previous number
	if (value0 && *value0 == '#') value0 = assignHashedValue(*(value0+1), p0, p1, p2, p3, p4, p5, p6, p7);
	if (value1 && *value1 == '#') value1 = assignHashedValue(*(value1+1), p0, p1, p2, p3, p4, p5, p6, p7);
	if (value2 && *value2 == '#') value2 = assignHashedValue(*(value2+1), p0, p1, p2, p3, p4, p5, p6, p7);
	if (value3 && *value3 == '#') value3 = assignHashedValue(*(value3+1), p0, p1, p2, p3, p4, p5, p6, p7);
	if (value4 && *value4 == '#') value4 = assignHashedValue(*(value4+1), p0, p1, p2, p3, p4, p5, p6, p7);
	if (value5 && *value5 == '#') value5 = assignHashedValue(*(value5+1), p0, p1, p2, p3, p4, p5, p6, p7);
	if (value6 && *value6 == '#') value6 = assignHashedValue(*(value6+1), p0, p1, p2, p3, p4, p5, p6, p7);
	if (value7 && *value7 == '#') value7 = assignHashedValue(*(value7+1), p0, p1, p2, p3, p4, p5, p6, p7);
	
	//Empty
	if (strcmp (name, ""                      ) == 0) { HttpResponseAddChar ( '~'); return 0; } //An empty include '~~' adds a single '~'
	if (strcmp (name, "tab"                   ) == 0) { HttpResponseAddChar ('\t'); return 0; } //Adds a tab character
	if (strcmp (name, "formfeed"              ) == 0) { HttpResponseAddChar ('\f'); return 0; } //Adds a formfeed character
	if (strcmp (name, "nop"                   ) == 0) {                             return 0; } //Adds nothing; used, for example, to hide </~nop~textarea> 

	//General
	if (strcmp (name, "expand"                ) == 0) { HttpIncludeExpandFile         (value0, value1, value2, value3, value4, value5, value6, value7, value8); return 0; } //This is recursive
	if (strcmp (name, "dir"                   ) == 0) { HttpResponseAddDir            (value0                                ); return 0; }
	if (strcmp (name, "file"                  ) == 0) { HttpResponseAddAsciiFile      (value0                                ); return 0; }
	if (strcmp (name, "filerows"              ) == 0) { HttpResponseAddAsciiFileRows  (value0                                ); return 0; }
	if (strcmp (name, "sheet"                 ) == 0) { HttpResponseAddString         (HttpRequestResource);                    return 0; } //Adds the current sheet name eg '/battery'
	if (strcmp (name, "setvalue"              ) == 0) { strncpy(_includeValue, value0, sizeof(_includeValue)); _includeValue[sizeof(_includeValue)-1] = 0; return 0; } //Save a value
	if (strcmp (name, "getvalue"              ) == 0) { HttpResponseAddString(_includeValue);                         return 0; } //add the saved value
	if (strcmp (name, "zerocount"             ) == 0) { _includeCount = 0;                                            return 0; } //Zero the count
	if (strcmp (name, "getcount"              ) == 0) { HttpResponseAddU32(_includeCount++);                          return 0; } //add the current count and increment
	if (strcmp (name, "p0"                    ) == 0) { if (p0) HttpResponseAddString(p0);                            return 0; }
	if (strcmp (name, "p1"                    ) == 0) { if (p1) HttpResponseAddString(p1);                            return 0; }
	if (strcmp (name, "p2"                    ) == 0) { if (p2) HttpResponseAddString(p2);                            return 0; }
	if (strcmp (name, "p3"                    ) == 0) { if (p3) HttpResponseAddString(p3);                            return 0; }
	if (strcmp (name, "p4"                    ) == 0) { if (p4) HttpResponseAddString(p4);                            return 0; }
	if (strcmp (name, "p5"                    ) == 0) { if (p5) HttpResponseAddString(p5);                            return 0; }
	if (strcmp (name, "p6"                    ) == 0) { if (p6) HttpResponseAddString(p6);                            return 0; }
	if (strcmp (name, "p7"                    ) == 0) { if (p7) HttpResponseAddString(p7);                            return 0; }

	//User stuff
	int r = HttpThisInclude(name, value0);
	
	if (r)
	{
		//Handle name which could not be found
		HttpResponseAddString("ERROR Could not find ");
		HttpResponseAddString(name);
	}
	return 0;
}

