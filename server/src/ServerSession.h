/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ServerSession.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef SERVER_SESSION_H
#define SERVER_SESSION_H

#include "HttpServer.h"




typedef struct Peer2PeerCfg
{
    char strStunServer1Addr[128];
    int iStunServer1Port;
    char strStunServer2Addr[128];
    int iStunServer2Port;

}T_Peer2PeerCfg;


/*****************************************************************************
-Class          : HttpServer
-Description    : HttpServer
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class ServerSession
{
public:
	ServerSession();
	virtual ~ServerSession();
    int HandleHttpReq(const char * i_strReq,char *o_strRes,int i_iResMaxLen);//return ResLen,<0 err
private:
    int HandleReq(char * i_strURL,char * i_strBody,char **o_ppRes);
    int Regex(const char *i_strPattern,char *i_strBuf,string * o_aMatch,int i_iMatchMaxCnt);
    ThreadSafeQueue<QueueMessage> m_oSessionQueue;
    ThreadSafeQueue<QueueMessage> * m_pMgrQueue;
};













#endif
