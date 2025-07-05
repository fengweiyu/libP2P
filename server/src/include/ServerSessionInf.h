/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ServerSessionInf.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef SERVER_SESSION_INF_H
#define SERVER_SESSION_INF_H


#include "ServerSessionCom.h"


/*****************************************************************************
-Class          : ServerSessionInf
-Description    : ServerSessionInf
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class ServerSessionInf
{
public:
	ServerSessionInf(ThreadSafeQueue<QueueMessage> * i_pMgrQueue,T_Peer2PeerCfg * i_ptPeer2PeerCfg);
	virtual ~ServerSessionInf();
    int Proc(char * i_strReq,int i_iReqLen,char *o_strRes,int i_iResMaxLen);//return ResLen,<0 err
private:
    void * m_pHandle;
};










#endif
