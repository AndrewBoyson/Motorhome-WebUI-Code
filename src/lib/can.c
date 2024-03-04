#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can/raw.h>
#include <pthread.h>
#include <errno.h>

#include "log.h"
#include "can-reliable.h"

#define MAX_LENGTH 8

static int canSocket = 0;
static pthread_mutex_t sendMutex = PTHREAD_MUTEX_INITIALIZER; //If using default values then pthread_mutex_init is not required
static char initialised = 0;

void CanInit()
{
	struct sockaddr_can addr;
	struct ifreq ifr;
	int r = 0;
	
	canSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (canSocket == -1)
	{
		LogErrno("CanInit socket error");
		return;
	}
	
	strcpy(ifr.ifr_name, "can0");
	
	//Wait for can0 to be up
	int count = 0;
	while (1)
	{
		count++;
		r = ioctl(canSocket, SIOCGIFFLAGS, &ifr);
		if (r == -1)
		{
			if (errno != 19 || count > 20) //Don't display no such device until there has been time for the kernel to start it
			{
				LogErrno("CanInit ioctl SIOCGIFFLAGS error");
			}
		}
		else
		{
			char isUp = !!(ifr.ifr_flags & IFF_UP);
			char isRunning = !!(ifr.ifr_flags & IFF_RUNNING);
			if (isUp && isRunning) break;
			if (count > 30) Log('e', "Can is not up and running"); //Don't display can not up until there has been time for the kernel to start it
		}
		sleep(1);
	}
	
	r = ioctl(canSocket, SIOCGIFINDEX, &ifr);
	if (r == -1)
	{
		LogErrno("CanInit ioctl SIOCGIFINDEX error");
		sleep(2);
	}

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	r = bind(canSocket, (struct sockaddr *)&addr, sizeof(addr));
	if (r == -1)
	{
		LogErrno("CanInit bind error");
		return;
	}
	
	Log('i', "Can is initialised");
	initialised = 1;
}

int CanReadOrWait(int32_t* pId, int* pLen, void* pData) //Only called by the CAN thread and only after being initialised
{
	*pId = 0;
	*pLen = 0;
	
	struct can_frame frame;

	int nbytes = read(canSocket, &frame, sizeof(frame)); //Thread blocks here

	if (nbytes < sizeof(frame)) Log('e', "CAN read %d less than frame size %d", nbytes, sizeof(frame));

	if (nbytes < 0)
	{
		LogErrno("CAN read error");
		return -1;
	}
	*pId  = frame.can_id;
	*pLen = frame.can_dlc;
	if (*pLen > MAX_LENGTH) *pLen = MAX_LENGTH;
	if (*pLen < 0         ) *pLen = 0;
	memcpy(pData, frame.data, *pLen);
	
	CanReliableConfirm(*pId, *pLen, pData);

	return 0; //returns 0 on success, anything else on failure
}
int CanSend(int32_t id, int len, void* pData) //This can be called by multiple threads so seems wise to put a mutex around the send
{
	if (!initialised)
	{
		Log('e', "CanSend - not initialised");
		return -1;
	}
	
	struct can_frame frame; //Thread safe as memory allocated from the stack - don't make static however.

	frame.can_id = id;
	frame.can_dlc = len <= MAX_LENGTH ? (uint8_t)len : MAX_LENGTH;
	memcpy(frame.data, pData, len);

	int r = pthread_mutex_lock(&sendMutex);
	if (r) LogNumber("CanSend error locking mutex", r);
	
	int nbytes = send(canSocket, &frame, sizeof(frame), 0); //flags = 0 but could put MSG_DONTWAIT to prevent blocking);
	
	r = pthread_mutex_unlock(&sendMutex);
	if (r) LogNumber("CanSend error unlocking mutex", r);
	
	static char wasOk = 1;
	if (nbytes < 0)
	{
		if (errno != 105) LogErrno("CAN send error");
		if (wasOk) Log('w', "CAN has gone off-line");
		wasOk = 0;
		return -1;
	}
	
	if (!wasOk) Log('w', "CAN is back on-line");
	wasOk = 1;
	
	return nbytes != len; //return 0 on success
}
