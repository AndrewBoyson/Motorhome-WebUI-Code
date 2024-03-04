#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "log.h"
#include "can.h"

#define MAX_ENTRIES 10
#define MAX_RESENDS 15

#define DEBUG 0

struct entry
{
	uint64_t value;
	int      len;
	int32_t  id;
	int      resends;
} _entries[MAX_ENTRIES];

void CanReliablePoll()
{	
	//See if there is an existing entry and resend it
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		if (_entries[i].len != 0)
		{
			if (_entries[i].resends == 0)
			{
				_entries[i].resends++;
				continue;
			}
			else
			{
				Log('d', "CanReliablePoll     - id %03x, len %d: entry resent", _entries[i].id, _entries[i].len);
				CanSend(_entries[i].id, _entries[i].len, &_entries[i].value);
				_entries[i].resends++;
				break;
			}
		}
	}
	
	//See if any entries need to be reeped
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		if (_entries[i].len != 0 && _entries[i].resends >= MAX_RESENDS)
		{
			Log('d', "CanReliablePoll     - id %03x, len %d: entry reeped", _entries[i].id, _entries[i].len);
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
		if (_entries[i].len && _entries[i].id == id)
		{
			Log('d', "CanReliableSend     - id %03x, len %d: entry updated", id, len);
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
			if (DEBUG) Log('d', "CanReliableSend     - id %03x, len %d: entry added", id, len);
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

void CanReliableConfirm(int32_t id, int len, void* pData)
{
	//See if there is an existing entry and clear it
	for (int i = 0; i < MAX_ENTRIES; i++)
	{
		if (_entries[i].len && _entries[i].id == id)
		{
			if (_entries[i].len == len && memcmp(&_entries[i].value, pData, len) == 0)
			{
				if (DEBUG) Log('d', "CanReliableConfirm  - id %03x, len %d: entry removed", id, len);
				_entries[i].len = 0;
			}
			else
			{
				if (_entries[i].len == len) Log('d', "CanReliableConfirm  - id %03x, len %d: entry left as data not matched", id, len);
				else                        Log('d', "CanReliableConfirm  - id %03x, len %d: entry left as length not matched", id, len);
			}
			return;
		}
	}
}