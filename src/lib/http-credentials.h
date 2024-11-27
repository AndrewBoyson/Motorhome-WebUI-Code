extern void  CredentialsMakeCookie(char* cookie);
extern char  CredentialsVerifyCookie(char* cookie);
extern char  CredentialsVerifyPassword(char* password);
extern char* CredentialsGetId(void);
extern void  CredentialsSetPassword(char* password);
extern char* CredentialsGetPassword(void);
extern void  CredentialsInit(void);