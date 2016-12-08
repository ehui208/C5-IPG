#ifndef OSQ_H
#define OSQ_H

#include "RTOS.h"

#define OS_U32_TO_PTR_TYPE(exp) ((void *)(exp))

typedef struct OS_q_link OS_Q_LINK;
struct OS_q_link 
{
	OS_INT Size;     /* May be negative. A negative value marks data invalid, copying in progress. */
};

typedef struct OS_Q_STRUCT OS_Q;
struct OS_Q_STRUCT 
{
	pthread_mutex_t	Qmutex;
	pthread_cond_t	Qcond;
	OS_Q*			pNext;          /* ptr to next queue (for debugging / monitoring) */
	OS_U8*			pData;
	OS_UINT			Size;
	OS_UINT			MsgCnt;
	OS_UINT			offFirst;
	OS_UINT			offLast;
	OS_BOOL			InUse;
	OS_UINT			InProgressCnt;
};

OS_Q* OS_pQHead;

void OS_Q_Create( OS_Q* pQ, void* pData, OS_UINT Size );
int OS_Q_GetPtr( OS_Q* pQ, void**ppData );
void OS_Q_Purge( OS_Q* pQ );

int OS_Q_Put( OS_Q* pQ, const void* pSrc, OS_UINT Size );
int OS_Q_GetPtrTimed( OS_Q* pQ, void**ppData, OS_TIME Timeout );

void OS_Q_Delete( OS_Q* pQ );

void OS_Q_DisplayInfo(OS_Q* pQ );

#endif
