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

#include "Peer2PeerHandle.h"


typedef enum ClientSessionStatus
{
    CLIENT_SESSION_IDLE=0,
    CLIENT_SESSION_LOGIN,
    CLIENT_SESSION_LOGIN_ACK,
    CLIENT_SESSION_REPORT_NAT_INFO,
    CLIENT_SESSION_REPORT_NAT_INFO_ACK,
    CLIENT_SESSION_PEER_2_PEER_HANDLE_PREPARE,
    CLIENT_SESSION_PEER_2_PEER_HANDLE_START,
    CLIENT_SESSION_PEER_2_PEER_HANDLE_PROC,
    CLIENT_SESSION_PEER_2_PEER_HANDLE_REPORT,
    CLIENT_SESSION_PEER_2_PEER_HANDLE_REPORT_ACK,
    CLIENT_SESSION_PEER_2_PEER_CONNECT_TEST,
}E_ClientSessionStatus;



typedef struct ClientSessionCb
{
    int (*Init)(void *i_pIoHandle,const char * i_strPeerIP,int i_iPeerPort);
    int (*SendData)(void *i_pIoHandle,unsigned char * i_pbSendBuf,int i_iSendLen);
    int (*RecvData)(void *i_pIoHandle,unsigned char *o_pbRecvBuf,int i_iRecvBufMaxLen);//返回非正值，则认为无响应无法收到数据
    void * pIoHandleObj;//固定使用一个内网ip和端口，穿透成功后用的也是这个(内网IP端口和外网一一对应即固定映射关系)
    int (*ChangePeerAddr)(void *i_pIoHandle,const char *i_pstrPeerAddr,int i_iPeerPort);
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
    int Proc(char * i_strReq,int i_iReqLen,char *o_strRes,int i_iResMaxLen);//return ResLen,<0 err
    int GetP2PConnectResult();//-1失败0 未出结果1成功
    int SaveLocalNatInfo(int i_iNatType,const char * i_strPublicIP,int i_iPublicPort);
    static int ReportResultCb(void *i_pReportObj,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort);
private:
    int CreateLoginReq(const char * i_strLocalID,char * o_pcReqBuf,int i_iBufMaxLen);
    int CreateReportNatInfoReq(const char * i_strLocalID,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort,char * o_pcReqBuf,int i_iBufMaxLen);
    int CreatePeerNatInfoReq(const char * i_strLocalID,const char * i_strPeerID,char * o_pcReqBuf,int i_iBufMaxLen);
    int CreatePeerSendMsgReq(const char * i_strLocalID,const char * i_strPeerID,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort,char * o_pcReqBuf,int i_iBufMaxLen);
    int CreateSendMsgToPeerRes(const char * i_strLocalID,const char * i_strPeerID,int i_iResCode,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort,char * o_pcResBuf,int i_iBufMaxLen);
    int CreateReportResultReq(const char * i_strLocalID,const char * i_strPeerID,int i_iResultCode,char * o_pcReqBuf,int i_iBufMaxLen);
    int ParseServerMsg(char * i_strMsg,int * o_iReqOrRes,char * o_pcCmdBufMaxLen,int i_iCmdBufMaxLen);
    int ParseLoginRes(char * i_strMsg,T_Peer2PeerCfg * o_ptPeer2PeerCfg);
    int ParseReportNatInfoRes(char * i_strMsg,char * o_pcResBuf,int i_iBufMaxLen);
    int ParsePeerNatInfoRes(char * i_strMsg,const char * i_strPeerID,int * o_iNatType,char * o_strPublicIP,int i_iIPBufMaxLen,int * o_iPublicPort);
    int ParsePeerSendMsgRes(char * i_strMsg,const char * i_strPeerID,int * o_iNatType,char * o_strPublicIP,int i_iIPBufMaxLen,int * o_iPublicPort,char * o_pcResBuf,int i_iBufMaxLen);
    int ParseSendMsgToPeerReq(char * i_strMsg,char * o_strPeerID,int i_iIDBufMaxLen,int * o_iNatType,char * i_strPublicIP,int i_iIPBufMaxLen,int * o_iPublicPort);
    int ParseReportResultRes(char * i_strMsg,char * i_strLocalID,char * i_strPeerID,int * o_iSuccessCnt,int * o_iFailCnt,int * o_iCurStatus);
        
    E_ClientSessionStatus m_eStatus;
    T_ClientSessionCb m_tClientSessionCb;
    T_ClientSessionCfg m_tClientSessionCfg;
    Peer2PeerHandle * m_pPeer2PeerHandle;
    T_NatInfo m_tLocalNatInfo;
    T_NatInfo m_tPeerNatInfo;
    int m_iPeer2PeerHandleSuccessFlag;//0 失败1成功
    int m_iP2PConnectResultFlag;//-1失败0 未出结果1成功
};













#endif
