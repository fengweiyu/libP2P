/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	Peer2PeerHandle.h
* Description		: 	Peer2PeerHandle operation center
* Created			: 	2020.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef PEER_2_PEER_H
#define PEER_2_PEER_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "Peer2PeerAdapter.h"


using std::string;

typedef struct Peer2PeerCb
{
    int (*Init)(void *i_pIoHandle,const char * i_strPeerIP,int i_iPeerPort);
    int (*SendData)(void *i_pIoHandle,unsigned char * i_pbSendBuf,int i_iSendLen);
    int (*RecvData)(void *i_pIoHandle,unsigned char *o_pbRecvBuf,int i_iRecvBufMaxLen);//返回非正值，则认为无响应无法收到数据
    int (*ChangePeerAddr)(void *i_pIoHandle,const char *i_pstrPeerAddr,int i_iPeerPort);
    void * pIoHandleObj;//固定使用一个内网ip和端口，穿透成功后用的也是这个(内网IP端口和外网一一对应即固定映射关系)
    void * pSessionHandle;
    int (*ReportLocalNatInfo)(void *i_pSessionHandle,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort);
    int (*GetPeerNatInfo)(void *i_pSessionHandle,char * o_pcIP, int i_iMaxLenIP,int * o_iPort,int * o_iNatType);//可以为NULL,表示不获取对方net信息，被动等待对方来连
    int (*ReqPeerSendMsgToLocal)(void *i_pSessionHandle);//可以为NULL,表示不获取对方net信息，被动等待对方来连的情况
}T_Peer2PeerCb;


/*****************************************************************************
-Class			: Peer2PeerHandle
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class Peer2PeerHandle
{
public:
    Peer2PeerHandle();
    virtual ~Peer2PeerHandle();
    int Proc(T_Peer2PeerCb *i_ptPeer2PeerCb,const char * i_strLocalIP,const char * i_strStunServer1IP,int i_iStunServer1Port,const char * i_strStunServer2IP,int i_iStunServer2Port);
    int Peer2PeerHoleHandle(int i_iLocalNatType,int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort);
    int SetPeerSendedMsgToLocalFlag(int i_iPeerSendedMsgToLocalFlag);
    int SendMsgToPeer(int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort);
    int RecvMsgFromPeer(int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort);

    static int ReportResultCb(void * i_pReportObj,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort);
    int LocalNatInfoHandle(int i_iNatType,const char * i_strPublicIP,int i_iPublicPort);
private:

    T_Peer2PeerCb m_tPeer2PeerCb;//
    void * m_pUdpHoleHandle;
    void * m_pNatDetect;
};









#endif
