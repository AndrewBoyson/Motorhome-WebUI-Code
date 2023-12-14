#include <sys/socket.h>
#include <stdint.h> 


extern int TcpMakeTalkingSocket  (char *server, char *port, int timeout);
extern int TcpMakeListeningSocket(unsigned short port);
extern int TcpSpawnSocket        (int listeningsfd);

extern int TcpSend       (int sfd, void *buffer, int size);
extern int TcpSendString (int sfd, char *text);
extern int TcpRecv       (int sfd, void *buffer, int size);
extern int TcpRecvAll    (int sfd, void *buffer, int size);
extern int TcpDownload   (int sfd, void *buffer, int size, char* filename);

extern int TcpClose      (int sfd);
extern int TcpShutdown   (int sfd);


extern int UdpMakeTalkingSocket  (int timeout);
extern int UdpMakeListeningSocket(unsigned short port);

extern int UdpResolve    (char *server, char *port, struct sockaddr_storage *pSockaddr);

extern int UdpSend       (int sfd, struct sockaddr_storage sockaddr, void *pBuffer, int size);
extern int UdpRecv       (int sfd, struct sockaddr_storage *pSockaddr, void *pBuffer, int size);

extern int UdpClose      (int sfd);


extern int SocketRemoteAddress   (int sfd, struct sockaddr_storage *pSockaddr);
extern int SocketIPv6NetworkOrder(struct sockaddr_storage sockaddr, uint32_t* pIp0, uint32_t* pIp1, uint32_t* pIp2, uint32_t* pIp3);
extern int SocketIsIPv4          (struct sockaddr_storage sockaddr);
extern int SocketIPv4NetworkOrder(struct sockaddr_storage sockaddr, uint32_t* pIp);
extern int SocketIpAsString      (struct sockaddr_storage sockaddr, char *ip, int len);
extern int SocketGetFQHostName   (struct sockaddr_storage sockaddr, char *name, char length);
extern int SocketGetHostName     (struct sockaddr_storage sockaddr, char *name, char length);
