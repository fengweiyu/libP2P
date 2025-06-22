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

#include <stdlib.h>
#include <stdio.h>
#include <string>

using std::string;

typedef struct UdpHoleHandleCb
{
    void * (*Init)(const char * strProtocolType,const char * i_strPeerIP,int i_iPeerPort);
    int (*SendData)(void *i_pIoHandle,unsigned char * i_pbSendBuf,int i_iSendLen);
    int (*RecvData)(void *i_pIoHandle,unsigned char *o_pbRecvBuf,int i_iRecvBufMaxLen);//返回非正值，则认为无响应无法收到数据
    int (*Close)(void *i_pIoHandle);
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
    int SendToPeer(int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort);
    int SetPeerSendedMsgToLocalFlag(int i_iPeerSendedMsgToLocalFlag);
    
private:
    int SendToPeerAfterPeerSendToLocal(int i_iLocalNatType,int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort);

    void * m_pIoHandle;
    T_UdpHoleHandleCb m_tUdpHoleHandleCb;
    int m_iPeerSendedMsgToLocalFlag;//0 还没发过，1发过了
};









#endif
