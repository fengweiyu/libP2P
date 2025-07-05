/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ServerDemo.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef SERVER_DEMO_H
#define SERVER_DEMO_H

#include <mutex>
#include <string>
#include <list>
#include <map>
#include "ServerIO.h"
#include "ServerHandle.h"

using std::map;
using std::string;
using std::list;
using std::mutex;

/*****************************************************************************
-Class			: ServerDemo
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class ServerDemo : public TcpServer
{
public:
	ServerDemo(int i_iServerPort);
	virtual ~ServerDemo();
    int Proc();
    
private:
    int CheckMapServerIO();
    int AddMapServerIO(ServerIO * i_pServerIO,int i_iClientSocketFd);
    
    map<int, ServerIO *>  m_ServerIOMap;
    mutex m_MapMtx;
    ServerHandle * m_pServerHandle;
};

#endif
