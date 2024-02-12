#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "log.h"
#include "can.h"

#define MAX_ENTRIES 10
#define MAX_RESENDS 15

struct entry
{
	uint64_t value;
	int      len;
	int32_t  id;
	int      resends;
} _entries[MAX_ENTRIES];

void CanReliablePoll()
{
	//Divide by 2 to give two seconds
	static bool odd = 0;
	odd = !odd;
	if (!odd) return;
	
	//See if there is an existing entry and resend it
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		if (_entries[i].len != 0)
		{
			Log('d', "CanReliablePoll     -  resent entry id %03x, len %d", _entries[i].id, _entries[i].len);
			CanSend(_entries[i].id, _entries[i].len, &_entries[i].value);
			_entries[i].resends++;
			break;
		}
	}
	
	//See if any entries need to be reeped
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		if (_entries[i].len != 0 && _entries[i].resends >= MAX_RESENDS)
		{
			Log('d', "CanReliablePoll     -  reeped entry id %03x, len %d", _entries[i].id, _entries[i].len);
			_entries[i].len = 0;
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
		if (_entries[i].len != 0 && _entries[i].id == id)
		{
			Log('d', "CanReliableSend - updated entry id %03x, len %d", id, len);
			_entries[i].len = len;
			memcpy(&_entries[i].value, pData, len);
			_entries[i].resends = 0;
			return;
		}
	}
	
	//See if there is a space and add the entry
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		if (_entries[i].len == 0)
		{
			Log('d', "CanReliableSend     -   added entry id %03x, len %d", id, len);
			_entries[i].len = len;
			_entries[i].id = id;
			memcpy(&_entries[i].value, pData, len);
			_entries[i].resends = 0;
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
		if (_entries[i].len == len && _entries[i].id == id && memcmp(&_entries[i].value, pData, len) == 0)
		{
			Log('d', "CanReliableReceived - removed entry id %03x, len %d", id, len);
			_entries[i].len = 0;
			return;
		}
	}
}