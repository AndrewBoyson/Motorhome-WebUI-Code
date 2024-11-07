#include <stdint.h>
#include <string.h>
#include <stdio.h>    //fopen
#include <sys/stat.h> //mkdir
#include <errno.h>    //errno and ENOENT
#include <stdarg.h>   //va_list, va_start, va_end, vfprintf
#include <time.h>

#include "log.h"

static FILE* openForRead (const char* name) { //returns 0 on failure, fp on success

	char filename[100];
	strcpy(filename, "settings/");
	strcat(filename, name);
	
	FILE* fp = fopen(filename, "r");
	if (!fp && errno != ENOENT) LogErrno("Settings - fopen: "); //Log any errors except no such file or directory
	return fp;
}
static FILE* openForWrite(const char* name) { //returns 0 on failure, fp on success

	char filename[100];
	strcpy(filename, "settings/");
	mkdir(filename, 0777); //Create the settings folder and ignore errors
	strcat(filename, name);
	
	FILE* fp = fopen(filename, "w");
	if (!fp) LogErrno("Settings - fopen: ");
	return fp;
}

int SettingsGetV     (const char *name, const char *format, ...) { //Returns 0 on success
	
	FILE* fp = openForRead(name);
	if (!fp) return -1;
	
	va_list args;
	va_start (args, format);
	int count = vfscanf(fp, format, args);
	va_end (args);
	if (fclose(fp)) LogErrno("Settings - fclose: ");
	
	if (count < 0)
	{
		LogErrno("StateRead - vfscanf");
		return -1;
	}
	return 0;
}
int SettingsSetV     (const char *name, const char *format, ...) { //Returns 0 on success
	
	FILE* fp = openForWrite(name);
	if (!fp) return -1;
	
	va_list args;
	va_start (args, format);
	int length = vfprintf(fp, format, args);
	va_end (args);
	fclose(fp);
	
	if (length < 0)
	{
		LogErrno("StateWrite - vfprintf");
		return -1;
	}
	return 0;
}
int SettingsGetS32   (const char* name, int32_t*  pValue) { //Returns 0 on success

	FILE* fp = openForRead(name);
	if (!fp) return -1;
	
	errno = 0;
	if (fscanf(fp, "%d", pValue) < 0)
	{
		if (errno) LogErrno("Settings - fscanf: ");
		fclose(fp);
		return -1;
	}
	
	if (fclose(fp)) LogErrno("Settings - fclose: ");
	
	return 0;
}
int SettingsSetS32   (const char* name, int32_t    value) { //Returns 0 on success

	FILE* fp = openForWrite(name);
	if (!fp) return -1;
	
	if (fprintf(fp, "%d", value) < 0)
	{
		LogErrno("Settings - fprintf: ");
		fclose(fp);
		return -1;
	}
	
	if (fclose(fp)) LogErrno("Settings - fclose: ");
	
	return 0;
}
int SettingsGetS16   (const char* name, int16_t*  pValue) { //Returns 0 on success

	int32_t value = 0;
	int r = SettingsGetS32(name, &value);
	if (r) return r;
	if (value > INT16_MAX || value < INT16_MIN)
	{
		Log('e', "SettingsGetS16 - value too big");
		return 1;
	}
	*pValue = (int16_t)value;
	return 0;
}
int SettingsSetS16   (const char* name, int16_t    value) { //Returns 0 on success

	return SettingsSetS32(name, (int32_t)value);
}
int SettingsGetS8    (const char* name, int8_t*   pValue) { //Returns 0 on success

	int32_t value = 0;
	int r = SettingsGetS32(name, &value);
	if (r) return r;
	if (value > INT8_MAX || value < INT8_MIN)
	{
		Log('e', "SettingsGetS8 - value too big");
		return 1;
	}
	*pValue = (int8_t)value;
	return 0;
}
int SettingsSetS8    (const char* name, int8_t     value) { //Returns 0 on success

	return SettingsSetS32(name, (int32_t)value);
}
int SettingsGetTime  (const char* name, time_t*   pValue) { //Returns 0 on success

	return SettingsGetS32(name, (int32_t*)pValue);
}
int SettingsSetTime  (const char* name, time_t     value) { //Returns 0 on success

	return SettingsSetS32(name, (int32_t)value);
}
int SettingsGetU32   (const char* name, uint32_t* pValue) { //Returns 0 on success

	FILE* fp = openForRead(name);
	if (!fp) return -1;
	errno = 0;
	if (fscanf(fp, "%u", pValue) < 0)
	{
		if (errno) LogErrno("Settings - fscanf: ");
		fclose(fp);
		return -1;
	}
	
	if (fclose(fp)) LogErrno("Settings - fclose: ");
	
	return 0;
}
int SettingsSetU32   (const char* name, uint32_t   value) { //Returns 0 on success

	FILE* fp = openForWrite(name);
	if (!fp) return -1;
	
	if (fprintf(fp, "%u", value) < 0)
	{
		LogErrno("Settings - fprintf: ");
		fclose(fp);
		return -1;
	}
	
	if (fclose(fp)) LogErrno("Settings - fclose: ");
	
	return 0;
}
int SettingsGetU16   (const char* name, uint16_t* pValue) { //Returns 0 on success

	uint32_t value = 0;
	int r = SettingsGetU32(name, &value);
	if (r) return r;
	if (value > UINT16_MAX)
	{
		Log('e', "SettingsGetU16 - value too big");
		return 1;
	}
	*pValue = (uint16_t)value;
	return 0;
}
int SettingsSetU16   (const char* name, uint16_t   value) { //Returns 0 on success

	return SettingsSetU32(name, (uint32_t)value);
}
int SettingsGetU8    (const char* name, uint8_t*  pValue) { //Returns 0 on success

	uint32_t value = 0;
	int r = SettingsGetU32(name, &value);
	if (r) return r;
	if (value > UINT8_MAX)
	{
		Log('e', "SettingsGetU8 - value too big");
		return 1;
	}
	*pValue = (uint8_t)value;
	return 0;
}
int SettingsSetU8    (const char* name, uint8_t    value) { //Returns 0 on success

	return SettingsSetU32(name, (uint32_t)value);
}
int SettingsGetChar  (const char* name, char*     pValue) { //Returns 0 on success
	FILE* fp = openForRead(name);
	if (!fp) return -1;
	
	int c = fgetc(fp);
	if (c < 0)
	{
		LogErrno("Settings - fgetc: ");
		fclose(fp);
		return -1;
	}
	
	*pValue = (char)c;
	
	if (fclose(fp)) LogErrno("Settings - fclose: ");
	
	return 0;
}
int SettingsSetChar  (const char* name, char       value) { //Returns 0 on success
	FILE* fp = openForWrite(name);
	if (!fp) return -1;
	
	if (fputc(value, fp) < 0)
	{
		LogErrno("Settings - fputc: ");
		fclose(fp);
		return -1;
	}
	
	if (fclose(fp)) LogErrno("Settings - fclose: ");
	
	return 0;
}
int SettingsGetString(const char* name, char*       text, int buffLen) { //Returns 0 on success
/*
If the buffer length is zero then do nothing
If the buffer length is one then return a string with a single null but don't read any characters
If the buffer length is greater than one then read up to buffer length -1 characters and terminate with a null
*/

	if (buffLen == 0)
	{
		Log('e', "SettingsGetString - buffLen of zero");
		return -1;
	}
	if (!text)
	{
		Log('e', "SettingsGetString - NULL buffer");
		return -1;
	}
	
	FILE* fp = openForRead(name);
	if (!fp) return -1;
	
	char* p = text;
	while(p < text + buffLen - 1) //There is room in the buffer to add a character or NULL so continue
	{
		int c = fgetc(fp);        //Try to read the next character
		if (c > 0) *p++ = c;      //There is a character so add it and move on
		else        break;        //There is no character to read so leave
	}
	*p = 0;                       //Terminate the tring
	fclose(fp);
	return 0;
}
int OldSettingsGetString(const char* name, char*       text) { //Returns 0 on success

	FILE* fp = openForRead(name);
	if (!fp) return -1;
	
	errno = 0;
	if (fscanf(fp, "%s", text) < 0)
	{
		if (errno) LogErrno("Settings - fscanf: ");
		fclose(fp);
		return -1;
	}
	
	if (fclose(fp)) LogErrno("Settings - fclose: ");
	
	return 0;
}
int SettingsSetString(const char* name, char*       text) { //Returns 0 on success
	FILE* fp = openForWrite(name);
	if (!fp) return -1;
	
	if (fprintf(fp, "%s", text) < 0)
	{
		LogErrno("Settings - fprintf: ");
		fclose(fp);
		return -1;
	}
	
	if (fclose(fp)) LogErrno("Settings - fclose: ");
	
	return 0;
}