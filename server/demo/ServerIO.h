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
#include "ServerSession.h"
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
	ServerIO(int i_iClientSocketFd);
	virtual ~ServerIO();
    int Proc();
    int GetProcFlag();
private:
	int m_iClientSocketFd;
	
    ServerSession * m_pServerSession;

    thread * m_pServerIOProc;
	int m_iServerIOFlag;
};

#endif
