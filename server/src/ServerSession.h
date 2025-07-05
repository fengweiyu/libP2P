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

#include "ServerSessionCom.h"






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
	ServerSession(ThreadSafeQueue<QueueMessage> * i_pMgrQueue,T_Peer2PeerCfg * i_ptPeer2PeerCfg);
	virtual ~ServerSession();
    int Proc(char * i_strReq,int i_iReqLen,char *o_strRes,int i_iResMaxLen);//return ResLen,<0 err
private:
    int ParseClientMsg(char * i_strMsg,int * o_iReqOrRes,char * o_pcCmdBufMaxLen,int i_iCmdBufMaxLen);
    int ParseLoginReq(char * i_strMsg,char * o_pcReqBuf,int i_iBufMaxLen);
    int ParseReportNatInfoReq(char * i_strMsg,char * o_strID,int i_iIDBufMaxLen,int * o_iNatType,char * o_strPublicIP,int i_iIPBufMaxLen,int * o_iPublicPort);
    int ParsePeerNatInfoReq(char * i_strMsg,char * o_strID,int i_iIDBufMaxLen);
    int ParsePeerSendMsgReq(char * i_strMsg,char * o_strID,int i_iIDBufMaxLen);
    int ParseSendMsgToPeerRes(char * i_strMsg,char * o_strLocalID,int i_iLocalBufMaxLen,char * o_strPeerID,int i_iPeerBufMaxLen);
    int ParseReportResultReq(char * i_strMsg,char * o_strLocalID,int i_iLocalBufMaxLen,char * o_strPeerID,int i_iPeerBufMaxLen);
    int CreateLoginRes(T_Peer2PeerCfg * i_ptPeer2PeerCfg,char * o_pcResBuf,int i_iBufMaxLen);
    int CreateReportNatInfoRes(int i_iResultCode,char * o_pcReqBuf,int i_iBufMaxLen);
    int CreatePeerNatInfoRes(const char * i_strID,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort,char * o_pcReqBuf,int i_iBufMaxLen);
    int CreatePeerSendMsgRes(int i_iResultCode,const char * i_strID,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort,char * o_pcReqBuf,int i_iBufMaxLen);
    int CreateSendMsgToPeerReq(const char * i_strPeerID,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort,char * o_pcReqBuf,int i_iBufMaxLen);
    int CreateReportResultRes(const char * i_strLocalID,const char * i_strPeerID,int i_iSuccessCnt,int i_iFailCnt,int i_iCurStatus,char * o_pcResBuf,int i_iBufMaxLen);


    ThreadSafeQueue<QueueMessage> m_oSessionQueue;
    ThreadSafeQueue<QueueMessage> * m_pMgrQueue;

    T_Peer2PeerCfg m_tPeer2PeerCfg;
    char m_strLocalID[64];
    T_NatInfoMsg m_tLocalNatInfoMsg;
};













#endif
