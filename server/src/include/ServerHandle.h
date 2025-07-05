/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ServerHandle.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef SERVER_HANDLE_H
#define SERVER_HANDLE_H

#include "ServerSessionCom.h"
#include <thread>
#include <mutex>

using std::thread;
using std::mutex;


/*****************************************************************************
-Class          : Proc
-Description    : Proc
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class ServerHandle
{
public:
	ServerHandle(ThreadSafeQueue<QueueMessage> * i_pMgrQueue);
	virtual ~ServerHandle();
    int Proc(ThreadSafeQueue<QueueMessage> * i_pMgrQueue);//10 ms³¬Ê±  
private:
    void * m_pHandle;
    
    thread * m_pServerHandleProc;
	int m_iServerHandleFlag;
};










#endif
