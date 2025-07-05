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
    int (*ChangePeerAddr)(void *i_pIoHandle,const char *i_pstrPeerAddr,int i_iPeerPort);//打洞的socket(端口)要和上报给对方的一致
    int (*SendData)(void *i_pIoHandle,unsigned char * i_pbSendBuf,int i_iSendLen);//打洞的socket(端口) 使用固定的地址端口,以便保持固定的外网地址端口映射
    int (*RecvData)(void *i_pIoHandle,unsigned char *o_pbRecvBuf,int i_iRecvBufMaxLen);//返回非正值，则认为无响应无法收到数据
    void * pIoHandleObj;//使用同样的对象(外部传入的,已经绑定过端口的),固定使用一个内网ip和端口，穿透成功后用的也是这个(内网IP端口和外网一一对应即固定映射关系)
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
    int m_iPeerSendedMsgToLocalFlag;//0 还没发过，1发过了
};









#endif
