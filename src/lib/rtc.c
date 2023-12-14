
#include <unistd.h> //sleep

#include <time.h>   //clock_nanosleep

#include "log.h"

static char hadTime = 0;
static struct timespec tp;
int RtcWaitForNextSecond(time_t* pT) //Returns 0 on success
{
	int r = 0;
	if (!hadTime)
	{
		r = clock_gettime(CLOCK_REALTIME, &tp);
		if (r)
		{
			LogErrno("TickMain - clock_gettime");
			return -1;
		}
	}
	tp.tv_sec++;
	tp.tv_nsec = 0;
	r = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &tp, NULL);
	if (r)
	{
		Log('e', "TickMain - clock_nanosleep had error %d", r);
		return -1;
	}
	hadTime = 1;
	*pT = tp.tv_sec;
	return 0;
}