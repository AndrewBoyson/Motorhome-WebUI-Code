
extern void LinThisReceive();
extern void LinThisInit();
extern char LinThisCalculateCheckSum(int headerLen, char* pHeader, int bufferLen, char* pBuffer);

#define TRACE_FID_18_CHANGES_TO_SEND 0
#define TRACE_SID_B2_READ_BY_ID      1
#define TRACE_SID_BA_UPLOAD_COMMAND  1
#define TRACE_SID_BB_DOWNLOAD_STATUS 0
#define TRACE_SID_B9_HEARTBEAT       0
