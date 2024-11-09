extern void CredentialsMakeCookie(char* cookie);
extern char CredentialsVerifyCookie(char* cookie);
extern char CredentialsVerifyPassword(char* password);
//extern void CredentialsResetId(void);
extern void CredentialsGetId(char* id, int bufLen);
extern void CredentialsSetPassword(char* password);
extern void CredentialsGetPassword(char* password, int bufLen);