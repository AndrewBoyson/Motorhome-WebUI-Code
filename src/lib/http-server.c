
#include "../lib/socket.h"
#include "http-request.h"
#include "http-response.h"

#define PORT 80

static int newsockfd = -1;

char HttpServerReadNextChar() { //Blocks until a character is available then return that character or 0 if no more characters are available (close or error)
	if (newsockfd < 0) return 0;
	char c = 0;
	int m = TcpRecv(newsockfd, &c, 1);
	if (m < 1)
	{
		TcpClose(newsockfd);
		newsockfd = -1;
		return 0;
	}
	return c;
}
int HttpServerSendBytes(char* buffer, int length) { //Blocks until space is available then returns 0 on success or 1 on failure
	if (newsockfd < 0) return 1;
	int m = TcpSend(newsockfd, buffer, length);
	if (m)
	{
		TcpClose(newsockfd);
		newsockfd = -1;
		return 1;
	}
	return m;
}

void HttpServer()
{
	//Initialise local part
	int sockfd = TcpMakeListeningSocket(PORT);
	if (sockfd < 0) return;
	
	while(1)
	{
		//Wait for a request and acquire new socket
		newsockfd = TcpSpawnSocket(sockfd);
		if (newsockfd < 0) continue;
		
		//Receive the request - HttpServerReadNextChar is called as often as needed
		HttpRequestReceive();
		if (newsockfd < 0) continue;
		
		//Send the response - HttpServerSendBytes is called as often as needed
		HttpResponseSend();
		if (newsockfd < 0) continue;
		
		//Send FIN
		TcpShutdown(newsockfd);
		
		//Close the new socket
		TcpClose(newsockfd);
		newsockfd = -1;
	}
    TcpClose(sockfd);
}