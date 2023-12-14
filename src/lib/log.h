extern void LogInit(void);
extern void LogSetLevel(char logLevel);
extern char LogGetLevel(void);
extern char LogIsAtLevel(char type);
extern void LogP(char type, const char *t1, const char *t2);
extern void LogPN(char type, char *t1, int n1, char *t2, int n2);
extern void Log(char type, const char *fmt, ...);
extern void LogErrno(char *);
extern void LogNumber(char *, int);
