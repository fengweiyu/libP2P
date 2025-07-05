/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	UdpHoleHandle.h
* Description		: 	UdpHoleHandle operation center
* Created			: 	2020.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef UDP_HOLE_HANDLE_H
#define UDP_HOLE_HANDLE_H

#include "Peer2PeerAdapter.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>

using std::string;

typedef struct UdpHoleHandleCb
{
    int (*ChangePeerAddr)(void *i_pIoHandle,const char *i_pstrPeerAddr,int i_iPeerPort);//�򶴵�socket(�˿�)Ҫ���ϱ����Է���һ��
    int (*SendData)(void *i_pIoHandle,unsigned char * i_pbSendBuf,int i_iSendLen);//�򶴵�socket(�˿�) ʹ�ù̶��ĵ�ַ�˿�,�Ա㱣�̶ֹ���������ַ�˿�ӳ��
    int (*RecvData)(void *i_pIoHandle,unsigned char *o_pbRecvBuf,int i_iRecvBufMaxLen);//���ط���ֵ������Ϊ����Ӧ�޷��յ�����
    void * pIoHandleObj;//ʹ��ͬ���Ķ���(�ⲿ�����,�Ѿ��󶨹��˿ڵ�),�̶�ʹ��һ������ip�Ͷ˿ڣ���͸�ɹ����õ�Ҳ�����(����IP�˿ں�����һһ��Ӧ���̶�ӳ���ϵ)
}T_UdpHoleHandleCb;

/*****************************************************************************
-Class			: UdpHoleHandle
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class UdpHoleHandle
{
public:
    UdpHoleHandle(T_UdpHoleHandleCb * i_ptUdpHoleHandleCb);
    virtual ~UdpHoleHandle();
    int Proc(int i_iLocalNatType,int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort);
    int SetPeerSendedMsgToLocalFlag(int i_iPeerSendedMsgToLocalFlag);
    int SendToPeer(int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort);
    int RecvFromPeer(int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort);
private:
    int SendToPeerFirst(int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort);
    int SendToPeerAfterPeerSendToLocal(int i_iLocalNatType,int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort);

    void * m_pIoHandle;
    T_UdpHoleHandleCb m_tUdpHoleHandleCb;
    int m_iPeerSendedMsgToLocalFlag;//0 ��û������1������
};









#endif
