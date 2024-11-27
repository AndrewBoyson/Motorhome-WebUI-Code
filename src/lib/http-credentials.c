#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "settings.h"
#include "log.h"

static char _id[9];
static char _password[20];

void CredentialsMakeCookie(char* cookie)
{
	char id[100];
	int r = SettingsGetString("credentialsId", id, sizeof(id));
	if (r) id[0] = 0;
	strcpy(cookie, "id=");
	strcat(cookie, id);
}
char CredentialsVerifyCookie(char* cookie)
{
	char target[100];
	CredentialsMakeCookie(target);
	return strstr(cookie, target) != 0; //strstr returns the location of target in cookie or 0 if not found
}
char CredentialsVerifyPassword(char* password)
{
	char target[100];
	int r = SettingsGetString("credentialsPassword", target, sizeof(target));
	if (r) target[0] = 0;
	return strcmp(target, password) == 0;
}
char* CredentialsGetId()
{
	return _id;
}
void CredentialsSetPassword(char* text)
{
	strncpy(_password, text, sizeof(_password));
	SettingsSetString("credentialsPassword", _password);
	
	//Change the id
	srand((unsigned)time(0));
	
	for (int i = 0; i < sizeof(_id) - 1; i++)
	{
		unsigned char c = (unsigned char)(rand() & 0x3F);
		if      (c  < 26) _id[i] = c       + 'A';
		else if (c  < 52) _id[i] = c  - 26 + 'a';
		else if (c  < 62) _id[i] = c  - 52 + '0';
		else if (c == 62) _id[i] =           '+';
		else if (c == 63) _id[i] =           '/';
	}
	_id[sizeof(_id)-1] = 0;
	SettingsSetString("credentialsId", _id);
}
char* CredentialsGetPassword()
{
	return _password;
}
void CredentialsInit()
{
	int r = SettingsGetString("credentialsPassword", _password, sizeof(_password));
	if (r) _password[0] = 0;
	r = SettingsGetString("credentialsId", _id, sizeof(_id));
	if (r) _id[0] = 0;
}