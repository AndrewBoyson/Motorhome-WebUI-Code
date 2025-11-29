#include <stdint.h>
#include <time.h>

#include "lib/lin.h"
#include "lib/lin-transport.h"
#include "lib/log.h"

#include "lin-this.h"
#include "truma.h"

#define RETRY_SECONDS 30
#define WALL_TIME_UPLOAD_HOUR 2 //Use 02 as local time over dst goes spring 00:58 00:59 02:00 02:01; autumn 01:58 01:59 01:00 01:01

static time_t _timeOfLastCommandUpload  = 0;
static char   _uploadCommandWanted      = 0;
static char   _uploadCommandOngoing     = 0; //Set by gui, reset when the upload is acknowledged or a matching status is received
static char   _uploadCommandId          = 0;

static time_t _timeOfLastWallTimeUpload = 0;
static char   _uploadWallTimeWanted     = 0;
static char   _uploadWallTimeOngoing    = 0; //Set at 02h00, reset when the upload is acknowledged
static char   _uploadWallTimeId         = 0;

static uint8_t _uploadId                = 0; //This is set randomly by init and increments for every upload - used to match with acknowledgements

uint16_t LinThisUploadGetSecondsSinceCommandSent()
{
	time_t elapsedSeconds = time(0) - _timeOfLastCommandUpload;
	if (elapsedSeconds > 65535) elapsedSeconds = 65535;
	if (elapsedSeconds <     0) elapsedSeconds =     0;
	return (uint16_t)(elapsedSeconds);
}
void LinThisUploadSetCommandSendWanted()
{
	_uploadCommandWanted     = 1;
	_uploadCommandOngoing    = 1;
	_timeOfLastCommandUpload = time(0);
}
static void setWallTimeSendWanted()
{
	_uploadWallTimeWanted     = 1;
	_uploadWallTimeOngoing    = 1;
	_timeOfLastWallTimeUpload = time(0);
}
static char oneShotWallTimeUploadHour(time_t t, int uploadHour)
{
	struct tm *ptm = localtime(&t); //This takes into account daylight saving time
	static int _prevHour = -1;
	char result = _prevHour != uploadHour && ptm->tm_hour == uploadHour;
	_prevHour = ptm->tm_hour;
	return result;
}
char LinThisUploadPollSendWanted() //Polled by the CP every few seconds
{
	//Calculate time
	time_t t = time(0);             //This is utc so will be correct even spanning dst changes
	
	//command
	if (_uploadCommandOngoing && !_uploadCommandWanted && t - _timeOfLastCommandUpload > RETRY_SECONDS) LinThisUploadSetCommandSendWanted(); //Retry
	if (_uploadCommandWanted) return 1;
	
	//wall time
	if (oneShotWallTimeUploadHour(t, WALL_TIME_UPLOAD_HOUR)) setWallTimeSendWanted();
	if (_uploadWallTimeOngoing && !_uploadWallTimeWanted && t - _timeOfLastWallTimeUpload > RETRY_SECONDS) setWallTimeSendWanted(); //Retry
	if (_uploadWallTimeWanted) return 1;

	return 0;
}
char LinThisUploadGetCommandSendOngoing()      { return _uploadCommandOngoing; }

void LinThisUploadHadSendAcknowledgement(char id, char result)
{
	if (_uploadCommandOngoing && id == _uploadCommandId)
	{
		if (result == 0) TrumaHadSendAcknowledgement(); //0 means successful so we know the CP has received the new target values so update them without waiting for a status
		
		_uploadCommandOngoing = 0;
		_timeOfLastCommandUpload = 0;
		return;
	}
	if (_uploadWallTimeOngoing && id == _uploadWallTimeId)
	{
		_uploadWallTimeOngoing = 0;
	}
}
void LinThisUploadHadStatus() //Save the time of the last status and cancel the send ongoing indicator
{
	if (TrumaHasSameActualAsTarget())
	{
		_uploadCommandOngoing = 0;
		_timeOfLastCommandUpload = 0;
	}
}
static void uploadCommand()
{	
	uint16_t rawWantedRoomTemp = 0;
	uint8_t  rawWantedFanMode = 0;
	if (TrumaGetWantedRoomOn())
	{
		rawWantedRoomTemp = ((uint16_t)TrumaGetWantedRoomTemp() + 273) * 10;
		switch (TrumaGetWantedFanMode())
		{
			case 'H': rawWantedFanMode = 10; break;
			case 'E': rawWantedFanMode =  1; break;
			default : rawWantedFanMode =  0; break;
		}
	}
	uint16_t rawWantedWaterTemp = 0;
	if (TrumaGetWantedWaterOn())
	{
		switch (TrumaGetWantedWaterTemp())
		{
			case 'E': rawWantedWaterTemp = ( 40 + 273) * 10; break;
			case 'H': rawWantedWaterTemp = ( 60 + 273) * 10; break;
			case 'B': rawWantedWaterTemp = (200 + 273) * 10; break;
			default:  rawWantedWaterTemp =    0;
		}
	}
	uint8_t rawWantedEnergySel = 0;
	switch (TrumaGetWantedEnergySel())
	{
		case 'G': rawWantedEnergySel = 1; break;
		case 'E': rawWantedEnergySel = 2; break;
		case 'B':
		case 'M': rawWantedEnergySel = 3; break;
		default : rawWantedEnergySel = 0; break;
	}
	uint16_t rawWantedElecPower = 0;
	switch (TrumaGetWantedElecPower())
	{
		case '2': rawWantedElecPower = 1800; break;
		case '1': rawWantedElecPower =  900; break;
		default : rawWantedElecPower =    0; break;
	}
	
	_uploadCommandId = _uploadId++;
	
	if (LinTrace && TRACE_SID_BA_UPLOAD_COMMAND)
	{
		Log ('d',"Upload command (message type 0x32, id %02X)", _uploadCommandId);
		Log ('d',"Target room temp:   truma %04X, app %d"  , rawWantedRoomTemp,   TrumaGetWantedRoomTemp ());
		Log ('d',"Target fan mode:    truma   %02X, app %c", rawWantedFanMode,    TrumaGetWantedFanMode  ());
		Log ('d',"Target water temp:  truma %04X, app %c"  , rawWantedWaterTemp,  TrumaGetWantedWaterTemp());
		Log ('d',"Target elec power:  truma %04X, app %c"  , rawWantedElecPower,  TrumaGetWantedElecPower());
		Log ('d',"Target energy sel:  truma %04X, app %c"  , rawWantedEnergySel,  TrumaGetWantedEnergySel());
	}
	
	LinTransportResponse[ 0] = 0xFA; //
	LinTransportResponse[ 1] = 0x00; //
	LinTransportResponse[ 2] = 0x1F; //
	LinTransportResponse[ 3] = 0x00; //
	LinTransportResponse[ 4] = 0x1E; //
	LinTransportResponse[ 5] = 0x00; //
	LinTransportResponse[ 6] = 0x00; //
	LinTransportResponse[ 7] = 0x22; //
	LinTransportResponse[ 8] = 0xFF; //
	LinTransportResponse[ 9] = 0xFF; //
	LinTransportResponse[10] = 0xFF; //
	LinTransportResponse[11] = 0x54; //message start
	LinTransportResponse[12] = 0x01; //
	LinTransportResponse[13] = 0x0C; //message length 12
	LinTransportResponse[14] = 0x32; //message type of request -1
	LinTransportResponse[15] = _uploadCommandId; //message id
	                                 //16 message checksum
	char* pResp = LinTransportResponse + 17;
	*(uint16_t*)(pResp +  0) = rawWantedRoomTemp;        //17
	           *(pResp +  2) = rawWantedFanMode;         //19
	           *(pResp +  3) = 0;                        //20 Recv status
	*(uint16_t*)(pResp +  4) = rawWantedElecPower;       //21
	*(uint16_t*)(pResp +  6) = rawWantedWaterTemp;       //23
	*(uint8_t* )(pResp +  8) = *(uint8_t* )(pResp +  4); //25
	*(uint8_t* )(pResp +  9) = *(uint8_t* )(pResp +  5); //26
	*(uint8_t* )(pResp + 10) = rawWantedEnergySel;       //27
	*(uint8_t* )(pResp + 11) = *(uint8_t* )(pResp + 10); //28
	
	LinTransportResponse[16] = LinThisCalculateCheckSum(5, LinTransportResponse + 11, 12, LinTransportResponse + 17);
	
	LinTransportSetResponseLengthAndTrace(41, TRACE_SID_BA_UPLOAD_COMMAND);  //41 bytes
	
	_uploadCommandWanted = 0; 
}
static void uploadWallTime()
{	
	_uploadWallTimeId = _uploadId++;

	time_t t = time(0);
	struct tm *ptm = localtime(&t); //This takes into account daylight saving time
	
	if (LinTrace && TRACE_SID_BA_UPLOAD_COMMAND)
	{
		Log ('d',"Upload wall time (message type 0x14, id %02X) to %02d:%02d:%02d",_uploadWallTimeId, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	}
	
	LinTransportResponse[ 0] = 0xFA; //
	LinTransportResponse[ 1] = 0x00; //
	LinTransportResponse[ 2] = 0x1F; //
	LinTransportResponse[ 3] = 0x00; //
	LinTransportResponse[ 4] = 0x1E; //
	LinTransportResponse[ 5] = 0x00; //
	LinTransportResponse[ 6] = 0x00; //
	LinTransportResponse[ 7] = 0x22; //
	LinTransportResponse[ 8] = 0xFF; //
	LinTransportResponse[ 9] = 0xFF; //
	LinTransportResponse[10] = 0xFF; //
	LinTransportResponse[11] = 0x54; //message start
	LinTransportResponse[12] = 0x01; //
	LinTransportResponse[13] = 0x0A; //message length 10
	LinTransportResponse[14] = 0x14; //message type
	LinTransportResponse[15] = _uploadWallTimeId; //message id
	
	char* pResp = LinTransportResponse + 17;
	*(pResp +  0) =  0;
	*(pResp +  1) = ptm->tm_hour; //Hours
	*(pResp +  2) = ptm->tm_min;  //Minutes
	*(pResp +  3) = ptm->tm_sec;  //Seconds though unsure if they are seen
    *(pResp +  4) =  0;
	*(pResp +  5) =  0;
	*(pResp +  6) =  0;
	*(pResp +  7) =  0;
	*(pResp +  8) =  0;
	*(pResp +  9) =  0;
	
	LinTransportResponse[16] = LinThisCalculateCheckSum(5, LinTransportResponse + 11, 10, LinTransportResponse + 17); //message checksum
	
	LinTransportSetResponseLengthAndTrace(41, TRACE_SID_BA_UPLOAD_COMMAND);  //41 bytes
	
	_uploadWallTimeWanted = 0; 
}
void LinThisUpload()
{
	if      (_uploadCommandWanted ) uploadCommand();
	else if (_uploadWallTimeWanted) uploadWallTime();
	else Log('e', "LinThisUpload - nothing to do");
}
void LinThisUploadInit()
{
	_uploadId = (uint8_t)time(0); //Put a random starting id
}