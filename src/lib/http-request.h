extern char HttpRequestMethod  [];
extern char HttpRequestResource[];
extern char HttpRequestQuery   [];
extern char HttpRequestCookie  [];

extern char HttpRequestAuthorised;

extern void HttpRequestReceive(void);
extern char HttpRequestGetContentChar(void);
