/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ClientSession.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef CLIENT_SESSION_H
#define CLIENT_SESSION_H

#include "HttpServer.h"
#include "HttpTestCase.h"


typedef enum ClientSessionStatus
{
    CLIENT_SESSION_LOGIN,
    CLIENT_SESSION_LOGIN_ACK,
    CLIENT_SESSION_REPORT_NAT_INFO,
    CLIENT_SESSION_REPORT_NAT_INFO_ACK,
    CLIENT_SESSION_PEER_2_PEER_HANDLE_PREPARE,
    CLIENT_SESSION_PEER_2_PEER_HANDLE_START,
    CLIENT_SESSION_PEER_2_PEER_HANDLE_PROC,
    CLIENT_SESSION_PEER_2_PEER_HANDLE_REPORT,
    CLIENT_SESSION_PEER_2_PEER_HANDLE_REPORT_ACK,
}E_ClientSessionStatus;



typedef struct ClientSessionCb
{
    void * (*Init)(const char * strProtocolType,const char * i_strPeerIP,int i_iPeerPort);
    int (*SendData)(void *i_pIoHandle,unsigned char * i_pbSendBuf,int i_iSendLen);
    int (*RecvData)(void *i_pIoHandle,unsigned char *o_pbRecvBuf,int i_iRecvBufMaxLen);//返回非正值，则认为无响应无法收到数据
    int (*Close)(void *i_pIoHandle);
}T_ClientSessionCb;

typedef struct ClientSessionCfg
{
    char strLocalIP[32];
    char strLocalID[64];
    char strPeerID[64];

}T_ClientSessionCfg;

typedef struct Peer2PeerCfg
{
    char strStunServer1Addr[128];
    int iStunServer1Port;
    char strStunServer2Addr[128];
    int iStunServer2Port;

}T_Peer2PeerCfg;

typedef struct NatInfo
{
    int iNatType;//
    char strIP[16];
    int iPort;//
}T_NatInfo;

/*****************************************************************************
-Class          : ClientSession
-Description    : ClientSession
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class ClientSession
{
public:
	ClientSession(T_ClientSessionCb * i_ptClientSessionCb,T_ClientSessionCfg * i_ptClientSessionCfg);
	virtual ~ClientSession();
    int HandleHttpReq(const char * i_strReq,char *o_strRes,int i_iResMaxLen);//return ResLen,<0 err
private:
    E_ClientSessionStatus m_eStatus;
    T_ClientSessionCb m_tClientSessionCb;
    T_ClientSessionCfg m_tClientSessionCfg;
    Peer2PeerHandle * m_pPeer2PeerHandle;
    T_NatInfo m_tLocalNatInfo;
    T_NatInfo m_tPeerNatInfo;
    int m_iPeer2PeerHandleSuccessFlag;//0 失败1成功
};













#endif
