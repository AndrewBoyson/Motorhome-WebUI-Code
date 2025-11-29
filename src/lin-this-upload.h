
extern uint16_t LinThisUploadGetSecondsSinceCommandSent(void);
extern void     LinThisUploadHadStatus(void); //Save the time of the last status and cancel the send ongoing indicator

extern void     LinThisUploadSetCommandSendWanted(void);
extern char     LinThisUploadGetCommandSendOngoing(void);

extern char     LinThisUploadPollSendWanted(void);
extern void     LinThisUploadHadSendAcknowledgement(char id, char result);

extern void     LinThisUpload(void);
extern void     LinThisUploadInit(void);