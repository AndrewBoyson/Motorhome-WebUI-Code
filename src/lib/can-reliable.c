#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "log.h"
#include "can.h"

#define MAX_ENTRIES 10
#define MAX_RESENDS 15

static uint64_t _values[MAX_ENTRIES];
static int      _lens  [MAX_ENTRIES];
static int32_t  _ids   [MAX_ENTRIES];
static int      _counts[MAX_ENTRIES];

void CanReliablePoll()
{
	//Divide by 2 to give two seconds
	static bool odd = 0;
	odd = !odd;
	if (!odd) return;
	
	//See if there is an existing entry and resend it
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		if (_lens[i] != 0)
		{
			Log('d', "CanReliablePoll     -  resent entry id %03x, len %d", _ids[i], _lens[i]);
			CanSend(_ids[i], _lens[i], &_values[i]);
			_counts[i]++;
			break;
		}
	}
	
	//See if any entries need to be reeped
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		if (_lens[i] != 0 && _counts[i] >= MAX_RESENDS)
		{
			Log('d', "CanReliablePoll     -  reeped entry id %03x, len %d", _ids[i], _lens[i]);
			_lens[i] = 0;
		}
	}
}

void CanReliableSend(int32_t id, int len, void* pData)
{
	
	//Send the message
	CanSend(id, len, pData);
	
	//See if there is an existing entry and update it
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		if (_lens[i] != 0 && _ids[i] == id)
		{
			Log('d', "CanReliableSend - updated entry id %03x, len %d", id, len);
			_lens[i] = len;
			memcpy(&_values[i], pData, len);
			_counts[i] = 0;
			return;
		}
	}
	
	//See if there is a space and add the entry
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		if (_lens[i] == 0)
		{
			Log('d', "CanReliableSend     -   added entry id %03x, len %d", id, len);
			_lens[i] = len;
			_ids[i] = id;
			memcpy(&_values[i], pData, len);
			_counts[i] = 0;
			return;
		}
	}
	
	//Out of room so warn
	Log('e', "CanReliableSend entry table full id %03x, len %d", id, len);
}

void CanReliableReceived(int32_t id, int len, void* pData)
{
	//See if there is an existing entry and clear it
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		if (_lens[i] == len && _ids[i] == id && memcmp(&_values[i], pData, len) == 0)
		{
			Log('d', "CanReliableReceived - removed entry id %03x, len %d", id, len);
			_lens[i] = 0;
			return;
		}
	}
}