/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ServerIO.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef SERVER_IO_H
#define SERVER_IO_H

#include "TcpSocket.h"
#include "ServerSessionInf.h"
#include <thread>
#include <mutex>

using std::thread;
using std::mutex;

/*****************************************************************************
-Class			: ServerIO
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class ServerIO : TcpServer
{
public:
	ServerIO(int i_iClientSocketFd,ThreadSafeQueue<QueueMessage> * i_pMgrQueue,T_Peer2PeerCfg * i_ptPeer2PeerCfg);
	virtual ~ServerIO();
    int Proc(ThreadSafeQueue<QueueMessage> * i_pMgrQueue,T_Peer2PeerCfg * i_ptPeer2PeerCfg);
    int GetProcFlag();
private:
	int m_iClientSocketFd;
	
    ServerSessionInf * m_pServerSessionInf;

    thread * m_pServerIOProc;
	int m_iServerIOFlag;
};

#endif
