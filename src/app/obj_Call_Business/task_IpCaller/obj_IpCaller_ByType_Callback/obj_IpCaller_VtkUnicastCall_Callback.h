#ifndef _obj_Caller_VtkCdsCallSt_Callback_H
#define _obj_Caller_VtkCdsCallSt_Callback_H

void Callback_Caller_ToRedial_VtkUnicastCall(CALLER_STRUCT *msg);
void Callback_Caller_ToInvite_VtkUnicastCall(CALLER_STRUCT *msg);
void Callback_Caller_ToRinging_VtkUnicastCall(CALLER_STRUCT *msg)	;
void Callback_Caller_ToAck_VtkUnicastCall(CALLER_STRUCT *msg);
void Callback_Caller_ToBye_VtkUnicastCall(CALLER_STRUCT *msg);
void Callback_Caller_ToWaiting_VtkUnicastCall(CALLER_STRUCT *msg);
void Callback_Caller_ToUnlock_VtkUnicastCall(CALLER_STRUCT *msg);
void Callback_Caller_ToTimeout_VtkUnicastCall(CALLER_STRUCT *msg);
void Callback_Caller_ToError_VtkUnicastCall(CALLER_STRUCT *msg);
void Callback_Caller_ToForceClose_VtkUnicastCall(CALLER_STRUCT *msg);
#endif