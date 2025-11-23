
extern char   LinTransportNad;

extern char   LinTransportRequest[];
extern int    LinTransportRequestLength;

extern char   LinTransportResponse[];
extern void   LinTransportSetResponseLengthAndTrace(int length, char trace);

extern void (*LinTransportHandler            )(void);
extern void   LinTransportHandleRequestPacket (void);
extern void   LinTransportHandleResponsePacket(void);