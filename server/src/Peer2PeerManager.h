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
#ifndef NAT_INFO_HANDLE_H
#define NAT_INFO_HANDLE_H

#include <thread>
#include <mutex>
#include <string>
#include <list>
#include <map>

using std::map;
using std::string;
using std::list;
using std::mutex;
using std::thread;








/*****************************************************************************
-Class			: NatInfo
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class NatInfo
{
public:
	NatInfo(){strPublicIP.clear();};
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
	Peer2PeerResult(){strLocalID.clear();strPeerID.clear();};
	virtual ~Peer2PeerResult(){};
public:
    string strLocalID;
    string strPeerID;
    int iSuccessCnt;
    int iFailCnt;
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
    int Test(const char * i_strSrcFilePath,const char *i_strDstFilePath);
private:
    int ReadFile(const char * i_strSrcFilePath,unsigned char **o_ppBuffer);
    int Proc(const char * i_strSrcFilePath,const char *i_strDstFilePath);

    map<string,NatInfo> m_NatInfoMap;
    map<string,Peer2PeerResult> m_Peer2PeerResultMap;
};

#endif
