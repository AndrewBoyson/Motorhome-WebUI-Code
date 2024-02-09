#include <pthread.h> //thread stuff
#include <semaphore.h>
#include <unistd.h> //sleep

#include "lib/thread.h"
#include "lib/log.h"
#include "lib/http-server.h"
#include "lib/usbdrive.h"
#include "lib/rtc.h"

#include "can-this.h"
#include "alert.h"
#include "tank.h"
#include "battery.h"

static struct Thread threadCan;
static struct Thread threadHttp;
static struct Thread threadPoll;
static struct Thread threadTick;

static void *workerHttp(void *arg)
{
	if (ThreadWorkerInit(&threadHttp)) return NULL;
	HttpServer();
    return NULL; 
}

static void *workerPoll(void *arg)
{
	if (ThreadWorkerInit(&threadPoll)) return NULL;
	AlertInit();
	TankInit();
	BatteryInit();
	while(1)
	{
		sleep(1); //Sleep takes a float so 10 is ten seconds and 0.1 is 100 ms
		
		static int secondsAfterInit = 0;
		if (secondsAfterInit < 1000) secondsAfterInit++;

		UsbDrivePoll(); //Mount or unmount the UsbDrive
				
		if (secondsAfterInit > 15) //This gives long enough after a reset for all CAN information to have been received at least once
		{
			AlertPoll();    //Check for alerts
			TankPoll();     //Manages the tank - actually just records water and lpg levels
			BatteryPoll();  //Manage the battery
		}
	}
	return NULL;
}

static void *workerTick(void *arg)
{
	if (ThreadWorkerInit(&threadTick)) return NULL;
	sleep(60); //Give time for the can interface to be up and for the rtc to be stable
	while(1)
	{
		time_t t = 0;
		int r = RtcWaitForNextSecond(&t); //returns 0 on success
		if (r) continue;
		CanThisSendServerTime(t);
	}
	return NULL;
}

static void *workerCan(void *arg)
{
	if (ThreadWorkerInit(&threadCan)) return NULL;
	CanThisInit();
	while(1) CanThisReceive();
	return NULL;
}

//External routines
void ThreadsStart()
{	
	threadCan .Name = "can";
	threadHttp.Name = "http";
	threadPoll.Name = "poll";
	threadTick.Name = "tick";
	
	threadCan .Worker = workerCan;
	threadHttp.Worker = workerHttp;
	threadPoll.Worker = workerPoll;
	threadTick.Worker = workerTick;
	
	threadCan .NormalPriority = 0;
	threadHttp.NormalPriority = 0;
	threadPoll.NormalPriority = 0;
	threadTick.NormalPriority = 0;
	
	ThreadStart(&threadCan );
	ThreadStart(&threadHttp);
	ThreadStart(&threadPoll);
	ThreadStart(&threadTick);
	sleep(1); //Give time for all the threads to start
}
void ThreadsWait()
{
	ThreadJoin(&threadCan );
	ThreadJoin(&threadHttp);
	ThreadJoin(&threadPoll);
	ThreadJoin(&threadTick);
}
void ThreadsKill()
{
	ThreadCancel(&threadCan );
	ThreadCancel(&threadHttp);
	ThreadCancel(&threadPoll);
	ThreadCancel(&threadTick);
	sleep(1); //pause thread for one second to allow the threads time to exit
}
