#include <stdio.h>      //snprintf
#include <errno.h>      //errno
#include <string.h>     //strerror
#include <sys/socket.h> //connect read write and socket options
#include <netinet/in.h> //protocol options
#include <netinet/tcp.h>//tcp protocol options
#include <unistd.h>     //close
#include <netdb.h>      //addrinfo
#include <arpa/inet.h>  //inet_pton
#include <resolv.h>


#include "log.h"
#include "socket.h"

#define LISTEN_LIMIT 32
#define TIMEOUT_SECONDS 5

/* Explanation

       |   Datagram  |  Stream
       |     UDP     |    TCP
       | Unconnected |  Connected
-------+-------------+-------------
Server |    bind *   |   bind *
       |             |   listen
	   |             |   accept
	   |   recvfrom  |   recv/read
	   |   sendto    |   send/write
-------+-------------+-------------
Client |    ***      |   connect **
       |   sendto    |   send/write
	   |  recvfrom   |   recv/read

*   Bind binds the socket to a port: it is only used by a server. It is used for both UDP and TCP.
**  Connect connects a socket to a remote server: it is only used by a client. It is usually only used for TCP.
*** The act of sending the request will have bound the socket to an unused port number.
    This will have been used as the source of the request, so should match the destination of the reply.
    The socket is therefore correctly bound to receive the reply.

socket domain = protocol family = AF_INET     or AF_INET6   = IPv4 or IPv6
socket type   = type            = SOCK_STREAM or SOCK_DGRAM = TCP  or UDP
*/

//Options
static int setSocketRecvTimeout(int sfd, int seconds) {
	struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = seconds;
	if (setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &tv,  sizeof tv))
	{
		LogErrno("setSocketRecvTimeout-setsockopt");
		return -1;
	}
	return 0;
}
static int setSocketSendTimeout(int sfd, int seconds) {
	struct timeval tv;
	tv.tv_usec = 0;	
	tv.tv_sec = seconds;
	if (setsockopt(sfd, SOL_SOCKET, SO_SNDTIMEO, &tv,  sizeof tv))
	{
		LogErrno("setSocketSendTimeout-setsockopt");
		return -1;
	}
	return 0;
}
static int setSocketCorkState(int sfd, int state) {
	if (setsockopt(sfd, IPPROTO_TCP, TCP_CORK, &state,  sizeof state))
	{
		LogErrno("setSocketCorkState-setsockopt");
		return -1;
	}
	return 0;
}
static int setSocketDualStack(int sfd) {
	int optval = 0;
	if (setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY, &optval,  sizeof optval))
	{
		LogErrno("setSocketDualStack");
		return -1;
	}
	return 0;
}
static int setSocketReuseAddress(int sfd) {
	int optval = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval,  sizeof optval))
	{
		LogErrno("setSocketReuseAddress");
		return -1;
	}
	return 0;
}

//Create
static int makeSocketWithTimeout(int protocol, int recvTimeout, int sendTimeout) {
	
	//Create a dual stack socket
	int sfd = socket(PF_INET6, protocol, 0);
	if (sfd < 0) { LogErrno("makeSocket - socket"); return -1; }
	
	//Set socket options
	if (               setSocketReuseAddress(sfd)             ) { close(sfd); return -1; }
	if (               setSocketDualStack   (sfd)             ) { close(sfd); return -1; }
	if (recvTimeout && setSocketRecvTimeout (sfd, recvTimeout)) { close(sfd); return -1; }
	if (sendTimeout && setSocketSendTimeout (sfd, sendTimeout)) { close(sfd); return -1; }
	
	return sfd;
}

//Connect
static struct addrinfo * getAddresses(char *server, char *port, int protocol) {         //returns a pointer to a list of addresses or NULL
	struct addrinfo hints;
	struct addrinfo *result;
	memset(&hints, 0, sizeof(struct addrinfo));
	//If hints.ai_flags specifies AI_V4MAPPED, and hints.ai_family is AF_INET6, and no matching IPv6 addresses could be found, then include IPv4-mapped IPv6 addresses.
	hints.ai_family   = AF_INET6;
	hints.ai_socktype = protocol;
	hints.ai_flags    = AI_V4MAPPED;
	hints.ai_protocol = 0;        // Any protocol
	int s = getaddrinfo(server, port, &hints, &result);
	if (s != 0)
	{
		Log('e', "getAddresses - server: %s; port %s; getaddrinfo: %s", server, port, gai_strerror(s));
		return NULL;
	}
	return result;
}
static int connectToAddressInList(struct addrinfo *addresses, int timeout) {                     //Returns a socket if successful or -1 if not
	int sfd;
	struct addrinfo *rp;
	for (rp = addresses; rp != NULL; rp = rp->ai_next)
	{	
		if (rp->ai_family != PF_INET6) continue;
		sfd = makeSocketWithTimeout(rp->ai_socktype, timeout, timeout);
		if (sfd == -1) continue;
		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) return sfd; // Success

		close(sfd);
	}
	return -1; //No address succeeded
}

//Bind
static struct sockaddr_storage makeSocketAddress(unsigned short port, char* ip) {
	struct sockaddr_in6 sa;
	bzero((char *) &sa, sizeof(sa));
	sa.sin6_family = AF_INET6;
	inet_pton(AF_INET6, ip, &sa.sin6_addr);
	sa.sin6_port = htons(port);
	
	struct sockaddr_storage sas;
	memcpy(&sas, &sa, sizeof(sa));
	
	return sas;
}
static int bindSocket(int sfd, unsigned short port) {
	
	//Make a socket address
	struct sockaddr_storage sa = makeSocketAddress(port, "::"); //IN6ADDR_ANY_INIT
	
	//Bind the socket to the address
	if (bind(sfd, (struct sockaddr *) &sa, sizeof(sa)) < 0)
	{
		LogErrno("bindSocket - bind");
		return -1;
	}
	return 0;
}

//Connected streamed TCP
int TcpMakeTalkingSocket  (char *server, char *port, int timeout) { //Server can be name or ip. Returns socket if successful or -1 if not.
	// Obtain addresses matching the host and port, remember to free it
	struct addrinfo *addresses = getAddresses(server, port, SOCK_STREAM);
	if (!addresses) return -1;
	
	//Try each address until we successfully connect.
	//Normally, the application should try using the addresses in the order in which they are returned
	//The sorting function used within getaddrinfo() is defined in RFC 3484; the order can be tweaked for a particular system by editing /etc/gai.conf
	int sfd = connectToAddressInList(addresses, timeout);
	
	//Whatever happens free the addresses
	freeaddrinfo(addresses);
	
	return sfd;
}
int TcpMakeListeningSocket(unsigned short port) {                   //Returns a socket if successful or -1 if not
	
	int sfd = makeSocketWithTimeout(SOCK_STREAM, 0, 0); //Create a socket without timeout
	if (sfd < 0) return -1;
	
	if (bindSocket(sfd, port))
	{
		close(sfd);
		return -1;
	}
	
	int r = listen(sfd, LISTEN_LIMIT);
	if (r < 0)
	{
		LogErrno("TcpMakeListeningSocket-listen");
		close(sfd);
		return -1;
	}
	
	return sfd;
}
int TcpSpawnSocket        (int listeningSocket) {
	int newSocket = accept(listeningSocket, NULL, NULL);
	if (newSocket < 0)
	{
		LogErrno("TcpSpawnSocket-accept");
		return -1;
	}
	int r = setSocketRecvTimeout (newSocket, TIMEOUT_SECONDS);
	if (r)
	{
		close(newSocket);
		return -1;
	}
	r = setSocketSendTimeout (newSocket, TIMEOUT_SECONDS);
	if (r)
	{
		close(newSocket);
		return -1;
	}
	r = setSocketCorkState(newSocket, 1);
	if (r)
	{
		close(newSocket);
		return -1;
	}
	return newSocket;
}

int TcpSend      (int sfd, void *pBuffer, int size) { //Returns 0 if successful or -1 if not
	if (sfd == -1)
	{
		Log('e', "TcpSend socket not connected");
		return -1;
	}
	while (size > 0)
	{
		int nwrite = write(sfd, pBuffer, size);
		if (nwrite <= 0)
		{
			if (errno != EPIPE) LogErrno("TcpSend-write"); //Don't alarm if the client has closed the connection
			return -1;
		}
		pBuffer += nwrite;
		size    -= nwrite;
	}
	return 0;
}
int TcpSendString(int sfd, char *text) {
	return TcpSend(sfd, text, strlen(text));
}
int TcpRecv      (int sfd, void *pBuffer, int size) { //Returns number of characters read if successful or -1 if not
	if (sfd == -1)
	{
		Log('e', "TcpRecv socket not connected");
		return -1;
	}
	int nread = read(sfd, pBuffer, size);
	if (nread == -1)
	{
		//if (errno != EAGAIN) LogErrno("TcpRecv-read"); //Don't alarm on timeouts
		if (errno != ECONNRESET &&  //Don't alarm on resets                       
		    errno != EAGAIN       ) //Don't alarm on timeouts
		{
			LogErrno("TcpRecv-read"); 
		}
		return -1;
	}
	return nread;
}
int TcpRecvAll   (int sfd, void *pBuffer, int size) { //Returns number of characters read if successful or -1 if not
	if (sfd == -1)
	{
		Log('e', "TcpRecvAll socket not connected");
		return -1;
	}
	int total = 0;
	while(1)
	{
		int nread = read(sfd, pBuffer, size);
		if (nread == 0) break; //Stop once all data has been read
		if (nread == -1)
		{
			if (errno != EAGAIN) LogErrno("TcpRecvAll-read");
			return -1;
		}
		pBuffer += nread;
		total += nread;
		size -= nread;
	}
	return total;
}
int TcpDownload  (int sfd, void *pBuffer, int size, char *localfilename) { //Returns number of characters downloaded if successful or -1 if not
	FILE *outfile = fopen(localfilename, "w");
	if (!outfile)
	{
		LogErrno("TcpDownload-fopen");
		return -1;
	}
	
	int bytesread = 0;
	while (1)
	{
		int bytes = read(sfd, pBuffer, size);
		if (bytes == 0) break;
		bytesread += bytes;
		write(fileno(outfile), pBuffer, bytes);
	}
	fclose(outfile);
	return bytesread;
}
int TcpShutdown(int sfd) {
	if (shutdown(sfd, SHUT_WR)) //Causes FIN to be sent
	{
		LogErrno("TcpShutdown-shutdown");
		return -1;
	}
	return 0;
}
int TcpClose     (int sfd) {

	if (close(sfd))
	{
		LogErrno("TcpClose-close");
		return -1;
	}
	return 0;
}

//Unconnected datagram UDP
int UdpMakeTalkingSocket  (int timeout) { //returns a socket if succuessful or -1 if not ok.
	return makeSocketWithTimeout(SOCK_DGRAM, timeout, timeout);
}
int UdpMakeListeningSocket(unsigned short port) { //returns a socket if successful or -1 if not
	int sfd = makeSocketWithTimeout(SOCK_DGRAM, 0, 0);
	if (sfd < 0) return -1;
	if (bindSocket(sfd, port)) return -1;
	return sfd;
}
int UdpResolve   (char *server, char *port, struct sockaddr_storage *pSockaddr) { //Returns 0 if successful or -1 if not
	//Obtain addresses matching the host and port, remember to free it
	struct addrinfo *addresses = getAddresses(server, port, SOCK_DGRAM);
	if (!addresses) return -1;
	
	//Use the first address
	memcpy(pSockaddr, addresses->ai_addr, addresses->ai_addrlen);
		
	//Whatever happens free the addresses
	freeaddrinfo(addresses);

	return 0;
}
int UdpRecv      (int sfd, struct sockaddr_storage *pSockaddr, void *pBuffer, int size) {
	if (sfd == -1)
	{
		Log('e', "UdpRecv socket not connected");
		return -1;
	}
	socklen_t destAddressSize = sizeof(struct sockaddr_storage);
    int nread = recvfrom(sfd, pBuffer, size, 0, (struct sockaddr *)pSockaddr, &destAddressSize);
	if (nread == -1)
	{
		LogErrno("UdpRecv-recvfrom");
		return -1;
	}
	return nread;
}
int UdpSend      (int sfd, struct sockaddr_storage sockaddr, void *pBuffer, int size) {
	socklen_t sockaddrlen = sizeof(struct sockaddr_storage);
	int nwrite = sendto(sfd, pBuffer, size, 0, (struct sockaddr *)&sockaddr, sockaddrlen);
	if (nwrite <= 0)
	{
		LogErrno("UdpSend-sendto");
		return -1;
	}
	return 0;
}
int UdpClose     (int sfd) {
	if (close(sfd))
	{
		LogErrno("UdpClose-close");
		return -1;
	}
	return 0;
}


//Utilities
int SocketRemoteAddress   (int sfd, struct sockaddr_storage *pSockaddr) {
	socklen_t sockaddrlen = sizeof(struct sockaddr_storage);
	if (getpeername(sfd, (struct sockaddr*)pSockaddr, &sockaddrlen))
	{
		LogErrno("SocketRemoteAddress - getpeername");
		return -1;
	}
	return 0;
}
int SocketIPv6NetworkOrder(struct sockaddr_storage sockaddr, uint32_t* pIp0, uint32_t* pIp1, uint32_t* pIp2, uint32_t* pIp3) {
		
	if (sockaddr.ss_family != AF_INET6) 
	{
		Log('e', "SocketIp6InHostOrder - Socket address is not IPv6");
		return -1;
	}
	
	struct sockaddr_in6 *psockaddrIP6 = (struct sockaddr_in6 *)&sockaddr;
	struct sockaddr_in6 sockaddr_in6 = *psockaddrIP6;
	
	//Addresses are held in network byte order which is big endian
	*pIp0 = sockaddr_in6.sin6_addr.s6_addr32[0]; //MSB
	*pIp1 = sockaddr_in6.sin6_addr.s6_addr32[1];
	*pIp2 = sockaddr_in6.sin6_addr.s6_addr32[2];
	*pIp3 = sockaddr_in6.sin6_addr.s6_addr32[3]; //LSB

	return 0;
}
int SocketIsIPv4          (struct sockaddr_storage sockaddr) {
	
	if (sockaddr.ss_family == AF_INET) return 1;
	if (sockaddr.ss_family != AF_INET6) return 0;

	uint32_t addr0, addr1, addr2, addr3;
	SocketIPv6NetworkOrder(sockaddr, &addr0, &addr1, &addr2, &addr3);
	
	return addr0 == 0 && addr1 == 0 && addr2  == htonl(0xffff);
}
int SocketIPv4NetworkOrder(struct sockaddr_storage sockaddr, uint32_t* pIp) {
	if (sockaddr.ss_family == AF_INET)
	{
		struct sockaddr_in *psockaddrIP4 = (struct sockaddr_in *)&sockaddr;
		struct sockaddr_in sockaddr_in = *psockaddrIP4;
		*pIp = sockaddr_in.sin_addr.s_addr; //s_addr is stored in network byte order
		return 0;
	}
	if (sockaddr.ss_family == AF_INET6)
	{
		uint32_t pIp0, pIp1, pIp2;
		SocketIPv6NetworkOrder(sockaddr, &pIp0, &pIp1, &pIp2, pIp); //Network order == Big Endian == MSB in [0] and LSB in [x] == IP4 address is in [3]
		return 0;
	}
	Log('e', "SocketIp4InHostOrder - Socket address is not IPv4 or IPv6");
	return -1;
}
int SocketIpAsString      (struct sockaddr_storage sockaddr, char *ip, int len) {
	socklen_t sockaddrlen = sizeof(struct sockaddr_storage);
	int r = getnameinfo((struct sockaddr*)&sockaddr, sockaddrlen, ip, len, 0, 0, NI_NUMERICHOST);
	if (r)
	{
		Log('e', "SocketIpAsString - ip: %s; getnameinfo: %s", ip, gai_strerror(r));
		return -1;
	}
	return 0;
}
int SocketGetFQHostName   (struct sockaddr_storage sockaddr, char *name, char length) {
	socklen_t destAddressSize = sizeof(struct sockaddr_storage);
	int r = getnameinfo((struct sockaddr *)&sockaddr, destAddressSize, name, length, NULL, 0, 0);
	if (r)
	{
		Log('e', "SocketGetFQHostName - host: %s; getnameinfo: %s", name, gai_strerror(r));
		return -1;
	}
	return 0;
}
int SocketGetHostName     (struct sockaddr_storage sockaddr, char *name, char length) {
	//Get the client
	if (SocketGetFQHostName(sockaddr, name, length)) return -1;
		
	//Get the host name part of the client - ie the bit before the '.'
	if (res_init()) { Log('e', "SocketGetHostName - res_init"); return -1; }
	char* localdomain = _res.defdname;
	
	int i = strlen(name) - 1;                                          //Position i at the last character of the destination
	while (name[i] != '.' && i >= 0) i--;                              //Position i at the last '.' in destination or at -1 if no '.' found
	if (i >= 0 && strcmp(name + i + 1, localdomain) == 0) name[i] = 0; //If found then remove the local domain part by replacing the last '.' with a 0.
	return 0;
}

