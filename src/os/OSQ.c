
#include "OSQ.h"

/*********************************************************************
*
*       OS_Q_Create
*
*  Function description:
*    Create a message queue.
*
*  Parameter:
*    pQ:     Pointer to the control structure of type OS_Q
*    pData:  Pointer to the buffer for data storage
*    Size:   Size of the data buffer in bytes
*
*  Context on entry
*    May be called from main, task, interrupt or timer
*    Interrupts:    Don't care
*    RegionCnt :    Don't care
*    DICnt     :    Don't care
*
*  Context on exit
*    Interrupts :   Restored acc. to DICnt.
*    Crit.region:   Unchanged.
*    DICnt      :   Unchanged.
*
*  Return value:
*    NONE (void)
*/
void OS_Q_Create( OS_Q* pQ, void* pData, OS_UINT Size )
{
	memset( pData, 0, Size );

	if( ((OS_U32) pData & (sizeof(int) - 1)) != 0 )
	{
		Size -= (sizeof(int) - ((OS_U32) pData & (sizeof(int) - 1)));
	}

	pData = (void*)OS_U32_TO_PTR_TYPE( (((OS_U32) pData + (sizeof(int) -1)) & ~(sizeof(int) - 1)) );
	//
	// Initialize the Q structure
	//
	memset( pQ, 0, sizeof(OS_Q) );

	pQ->Size = Size;
	pQ->pData = (OS_U8*)pData;

	if( OS_pQHead )
	{
		pQ->pNext = OS_pQHead;
	}

	OS_pQHead = pQ;

    pthread_mutex_init( &pQ->Qmutex, NULL );
    pthread_cond_init( &pQ->Qcond, NULL );
}

/*********************************************************************
*
*       OS_Q_GetPtr
*
*  Function description
*    Retrieve one message (pointer) from the message queue.
*
*  Parameter:
*    pQ:     Pointer to the control structure of type OS_Q
*    ppData: Address of the pointer which shall point to the data
*
*  Context on entry
*    May be called from a task only (blocking function)
*    Interrupts:    Don't care
*    RegionCnt :    Don't care
*    DICnt     :    Don't care
*
*  Context on exit
*    Interrupts :   Restored acc. to DICnt.
*    Crit.region:   Unchanged.
*    DICnt      :   Unchanged.
*
*  Return value:
*    Size of message in bytes
*    *ppData points to the data
*/
int OS_Q_GetPtr( OS_Q* pQ, void**ppData )
{
	int r;
	OS_Q_LINK* pLink;


	pthread_mutex_lock( &pQ->Qmutex );

	while( 1 )  //lint !e716 Loop until pQ->MsgCnt > 0
	{
		if( pQ->MsgCnt )
		{
			pLink = (OS_Q_LINK*)(pQ->pData + pQ->offFirst); //lint !e826 !e740 Suppress warning about pointer conversion and unusal pointer cast
			*ppData = (void*)(pLink + 1);
			r = (int)pLink->Size;
			pQ->InUse = 1;
			break;
		}
		else 
		{
			pthread_cond_wait( &pQ->Qcond, &pQ->Qmutex );
		}
	}

	pthread_mutex_unlock( &pQ->Qmutex );
	return r;
}

/*********************************************************************
*
*       OS_Q_Purge
*
*  Function description
*    Deletes the last message.
*    Has to be called for every pointer (data) retreived from the queue
*    before the next message may be retrieved
*
*  Parameter:
*    pQ:     Pointer to the control structure of type OS_Q
*
*  Context on entry
*    May be called from a task, interrupt ot timer (non blocking function)
*    Interrupts:    Don't care
*    RegionCnt :    Don't care
*    DICnt     :    Don't care
*
*  Context on exit
*    Interrupts :   Restored acc. to DICnt.
*    Crit.region:   Unchanged.
*    DICnt      :   Unchanged.
*
*  Return value:
*    NONE (void)
*/
void OS_Q_Purge( OS_Q* pQ )
{
	OS_Q_LINK* pLink;
	OS_UINT    Off;

	pthread_mutex_lock( &pQ->Qmutex );

	pLink = (OS_Q_LINK*)(pQ->pData + pQ->offFirst); //lint !e826 !e740 Suppress warning about pointer conversion and unusal pointer cast
	//
	// Calculate position of next entry from Size
	//
	Off  = sizeof(OS_Q_LINK);
	Off += (pLink->Size + sizeof(int) - 1) & ~(sizeof(int) - 1);  //lint !e737 Suppress warning about Loos of precision // Add size of data padded acc. to alignment
	pQ->MsgCnt--;    // Adjust message count
	pQ->offFirst += Off;
	//
	// Check if end of Q-buffer is reached
	//
	if( (pQ->Size - pQ->offFirst) <= sizeof(OS_Q_LINK) )
	{
		//
		// Less than sizeof(OS_Q_LINK) bytes at the end of buffer, too small for a Queue entry
		//
		pQ->offFirst = 0;
	}
	else
	{
		//
		// If messages follow, check whether there is a valid message behind the current message
		//
		if( pQ->MsgCnt != 0 )
		{
			pLink = (OS_Q_LINK*)(pQ->pData + pQ->offFirst); //lint !e826 !e740 Suppress warning about pointer conversion and unusal pointer cast
			if( pLink->Size == 0 )
			{  // No space for a complete message at the end of the buffer. (A size of zero was mrked by OS_Q_Put())
				pQ->offFirst = 0;      // The next message is located at the start of the buffer
			}
		}
	}
	pQ->InUse = 0;   // Mark queue as not in use
	//
	//  Wake up task(s) waiting for this queue (New since version 3.84b with new function OS_Q_PutBlocked())
	//
	pthread_mutex_unlock( &pQ->Qmutex );
}

/*********************************************************************
*
*       OS_Q_Put
*
*  Function description:
*       Copy the given message into the message queue.
*
*  Parameter:
*    pQ:     Pointer to the control structure of type OS_Q
*    pSrc:   Pointer to first byte of data to store
*    Size:   Size of the data
*
*  Context on entry
*    May be called from main, task, interrupt or timer
*    Interrupts:    Don't care
*    RegionCnt :    Don't care
*    DICnt     :    Don't care
*
*  Context on exit
*    Interrupts :   Restored acc. to DICnt.
*    Crit.region:   Unchanged.
*    DICnt      :   Unchanged.
*
*  Return value:
*    0 : Success, message stored
*    1 : No more space in the queue, message NOT stored
*/
int OS_Q_Put( OS_Q* pQ, const void* pSrc, OS_UINT Size )
{
	int        Off;
	OS_UINT    StorageSize;
	OS_Q_LINK* pLink;
	int        LinkSize;

	pthread_mutex_lock( &pQ->Qmutex );
	//
	// Calculate total size required to store the message
	//
	StorageSize = sizeof(OS_Q_LINK) + ((Size+sizeof(int)-1) & ~(sizeof(int)-1));
	//
	// Compute offset to free space
	//
	if( pQ->MsgCnt )
	{
		pLink    = (OS_Q_LINK*)(pQ->pData + pQ->offLast);       //lint !e826 !e740 Suppress warning about pointer conversion and unusal pointer cast
		Off      =  (int)pQ->offLast + (int)sizeof(OS_Q_LINK);  // Off = start of data
		LinkSize = pLink->Size ;
		//
		// If the Link size is negative, a previous transfer into the queue is in progress
		//
		if( LinkSize < 0 )
		{
			LinkSize = 0 - LinkSize;
		}
		LinkSize &= ~(1uL << ((sizeof(int) << 3) - 1));            //lint !e737 Suppress warning about Loos of precision // Adjust to next word boundary
		Off += (LinkSize + sizeof(int) - 1) & ~(sizeof(int) - 1);  //lint !e713 !e737: Suppress warning about Loos of precision // Add size of data padded acc. to alignment
		//
		// Check if there is enough space behind the last message
		//
		if( pQ->offFirst < (OS_UINT) Off )         // We cast to unsingned int to avoid a compiler warning
		{
			if( (pQ->Size - Off) < StorageSize )    //lint !e737 Suppress warning about Loos of precision 
			{
				//
				// Check if there is enough space before the first message
				//
				if( pQ->offFirst < StorageSize ) 
				{
					Off = -1;      // No space
				}
				else
				{
					//
					// Message will be stored at the first buffer address.
					// Mark end of buffer invalid when there is enough space for at least the samllest possible message
					// This information is needed by OS_Q_Purge() to wrap around
					//
					if( (pQ->Size - Off) > sizeof(OS_Q_LINK) )    //lint !e737 Suppress warning about Loos of precision 
					{
						pLink       = (OS_Q_LINK*)(pQ->pData + Off); //lint !e826 !e740 Suppress warning about pointer conversion and unusal pointer cast
						pLink->Size = 0;
					}
					Off = 0;    // Set pointer to the start address of the Q-buffer
				}
			}
		}
		else
		{
			if( pQ->offFirst-Off < StorageSize)  //lint !e737 Suppress warning about Loos of precision
			{
				Off = -1;        // No space
			}
		}
	}
	else
	{
		Off = (StorageSize <= pQ->Size) ? 0 : -1;
		pQ->offFirst = 0;
	}
	//
	// If we found space, insert it in the list, copy data and wake up task
	//
	if( Off >= 0 )	
	{
		pQ->offLast = (OS_UINT)Off;   // Remember the offset for the next call of OS_Q_Put
		pQ->MsgCnt++;
		//
		// Copy data into the queue buffer
		//
		pLink = (OS_Q_LINK*)(pQ->pData + Off); //lint !e826 !e740 Suppress warning about pointer conversion and unusal pointer cast
		//
		//  Mark data as "in progress" before re-enbling interrupts
		//
		pLink->Size = 0 - Size;           //lint !e713 Suppress warning about Loos of precision // Negative Size flags "data not valid", in progress
		pQ->InProgressCnt++;              // Mark transfer in progress (needed to adjust MsgCnt when calling OS_Q_GetMessageCnt() from int)
		memcpy( pLink + 1, pSrc, Size );  // Copy data with interrupts enabled to keep latency short
		pLink->Size = (OS_INT)Size;       // Mark data valid
		pQ->InProgressCnt--;              // Mark transfer done
		//
		//  Wake up task(s) waiting for this queue
		//
		pthread_cond_signal( &pQ->Qcond );
	}
    pthread_mutex_unlock( &pQ->Qmutex );

	return (Off >= 0) ? 0 : 1;
}

/*********************************************************************
*
*       OS_Q_GetPtrTimed
*
*  Function description
*    Retrieve one message (pointer) from message queue with timeout.
*
*  Parameter:
*    pQ:      Pointer to the control structure of type OS_Q
*    ppData:  Address of the pointer which shall point to the data
*    Timeout: Maximum waiting time in timer ticks
*
*  Context on entry
*    May be called from a task only (blocking function)
*    Interrupts:    Don't care
*    RegionCnt :    Don't care
*    DICnt     :    Don't care
*
*  Context on exit
*    Interrupts :   Restored acc. to DICnt.
*    Crit.region:   Unchanged.
*    DICnt      :   Unchanged.
*
*  Return value:
*    0  : No data available, timeout
*    > 0: Size of message in bytes
*    *ppData points to the data
*/
int OS_Q_GetPtrTimed( OS_Q* pQ, void**ppData, OS_TIME Timeout )
{
	OS_Q_LINK* pLink;
    struct timespec abstime;
    struct timeval now;
    int nsec;

    pthread_mutex_lock( &pQ->Qmutex );

    gettimeofday(&now, NULL);
    nsec = now.tv_usec * 1000 + (Timeout % 1000) * 1000000;
    abstime.tv_nsec = nsec % 1000000000;
    abstime.tv_sec = now.tv_sec + nsec / 1000000000 + Timeout / 1000;

	while( pQ->MsgCnt == 0 )
	{
		if( pthread_cond_timedwait( &pQ->Qcond, &pQ->Qmutex, &abstime ) == ETIMEDOUT )
		{
			pthread_mutex_unlock( &pQ->Qmutex );
			return 0;
		}
	}

	pLink     = (OS_Q_LINK*)(pQ->pData + pQ->offFirst); //lint !e826 Suppress warning about pointer conversion
	*ppData   = (void*)(pLink + 1);
	pQ->InUse = 1;
  
	pthread_mutex_unlock( &pQ->Qmutex );

	return pLink->Size;
}
void OS_Q_Delete( OS_Q* pQ ) 
{
	pthread_mutex_unlock( &pQ->Qmutex );
	pthread_mutex_destroy( &pQ->Qmutex );	
	pthread_cond_destroy( &pQ->Qcond );

	//
	// Take it out of the list
	//
	{ //lint !e539 Suppress warning message about wrong indent
		OS_Q** ppQ;
		for( ppQ = (OS_Q **) &OS_pQHead; *ppQ; ppQ = (OS_Q **) & ((*ppQ)->pNext) ) 
		{
			if( *ppQ == pQ ) 
			{
				*ppQ = (OS_Q *)pQ->pNext; /* We found it. Lets unlink */
				break;
			}
		}
	}
	//
	// Mark the queue empty and invalid
	//  
	pQ->MsgCnt = 0;
}

void OS_Q_DisplayInfo(OS_Q* pQ )
{
    pthread_mutex_lock( &pQ->Qmutex );	
	printf("pdata=0x%08x,size=%d,in:0x%08x,out:0x%08x,msg_cnt=%d\n",(unsigned int)pQ->pData,pQ->Size,pQ->offLast,pQ->offFirst,pQ->MsgCnt);
	pthread_mutex_unlock( &pQ->Qmutex );
}

