extern void CanReliablePoll();
extern void CanReliableSend(int32_t id, int len, void* pData);
extern void CanReliableReceived(int32_t id, int len, void* pData);