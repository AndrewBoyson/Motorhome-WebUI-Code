#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/settings.h"
#include "lib/log.h"


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
	//return strcmp(target, cookie) == 0;
}
char CredentialsVerifyPassword(char* password)
{
	char target[100];
	int r = SettingsGetString("credentialsPassword", target, sizeof(target));
	if (r) target[0] = 0;
	return strcmp(target, password) == 0;
}
void CredentialsResetId()
{
	static char runOnce = 0;
	if (!runOnce) srand((unsigned)time(0));
	runOnce = 1;
	unsigned randomNumber = rand();
	char idText[20];
	sprintf(idText, "%X", randomNumber);
	SettingsSetString("credentialsId", idText);
}
void CredentialsGetId(char* id, int bufLen)
{
	int r = SettingsGetString("credentialsId", id, bufLen);
	if (r) id[0] = 0;
}
void CredentialsSetPassword(char* password)
{
	SettingsSetString("credentialsPassword", password);
}
void CredentialsGetPassword(char* password, int bufLen)
{
	int r = SettingsGetString("credentialsPassword", password, bufLen);
	if (r) password[0] = 0;
}