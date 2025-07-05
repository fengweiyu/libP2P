/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       Peer2PeerManager.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef PEER_2_PEER_MANAGER_H
#define PEER_2_PEER_MANAGER_H

#include <thread>
#include <mutex>
#include <string>
#include <list>
#include <map>
#include "ServerSessionCom.h"

using std::map;
using std::string;
using std::list;
using std::mutex;
using std::thread;








/*****************************************************************************
-Class			: Peer2PeerManager
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class NatInfo
{
public:
	NatInfo(){pSession=NULL;strPublicIP.clear();iNatType=-1;iPublicPort=-1;};
	virtual ~NatInfo(){};
public:
    void * pSession;
    int iNatType;
    string strPublicIP;
    int iPublicPort;
};

/*****************************************************************************
-Class			: Peer2PeerResult
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class Peer2PeerResult
{
public:
	Peer2PeerResult(){strLocalID.clear();strPeerID.clear();iSuccessCnt=0;iFailCnt=0;};
	virtual ~Peer2PeerResult(){};
public:
    string strLocalID;
    string strPeerID;
    int iSuccessCnt;
    int iFailCnt;
    int iCurStatus;//-1 Ê§°Ü,0 ³É¹¦
};


/*****************************************************************************
-Class			: Peer2PeerManager
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class Peer2PeerManager
{
public:
	Peer2PeerManager();
	virtual ~Peer2PeerManager();
    int Proc(ThreadSafeQueue<QueueMessage> * i_pMgrQueue);
private:

    map<string,NatInfo> m_NatInfoMap;
    map<string,Peer2PeerResult> m_Peer2PeerResultMap;
    ThreadSafeQueue<QueueMessage> * m_pMgrQueue;
};

#endif
