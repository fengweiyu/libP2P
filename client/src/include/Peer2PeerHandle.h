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
#include "NatDetect.h"
#include "UdpHoleHandle.h"


using std::string;

typedef struct Peer2PeerCb
{
    void * (*Init)(const char * strProtocolType,const char * i_strPeerIP,int i_iPeerPort);
    int (*SendData)(void *i_pIoHandle,unsigned char * i_pbSendBuf,int i_iSendLen);
    int (*RecvData)(void *i_pIoHandle,unsigned char *o_pbRecvBuf,int i_iRecvBufMaxLen);//���ط���ֵ������Ϊ����Ӧ�޷��յ�����
    int (*Close)(void *i_pIoHandle);
    void * pSessionHandle;
    int (*ReportLocalNatInfo)(void *i_pSessionHandle,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort);
    int (*GetPeerNatInfo)(void *i_pSessionHandle,char * o_pcIP, int i_iMaxLenIP,int * o_iPort,int * o_iNatType);//����ΪNULL,��ʾ����ȡ�Է�net��Ϣ�������ȴ��Է�����
    int (*ReqPeerSendMsgToLocal)(void *i_pSessionHandle);//����ΪNULL,��ʾ����ȡ�Է�net��Ϣ�������ȴ��Է����������
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
    int SendMsgToPeer(int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort);
    int Peer2PeerHoleHandle(int i_iLocalNatType,int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort);
    int SetPeerSendedMsgToLocalFlag(int i_iPeerSendedMsgToLocalFlag);

    static int ReportResultCb(void * i_pReportObj,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort);
    
private:
    int LocalNatInfoHandle(int i_iNatType,const char * i_strPublicIP,int i_iPublicPort);

    T_Peer2PeerCb m_tPeer2PeerCb;//
    UdpHoleHandle * m_pUdpHoleHandle;
    NatDetect * m_pNatDetect;
};









#endif
