/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	UdpHoleHandle.cpp
* Description		: 	UdpHoleHandle operation center
* Created			: 	2020.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include "UdpHoleHandle.h"
#include <string.h>
#include <iostream>

using std::cout;//需要<iostream>
using std::endl;





/*****************************************************************************
-Fuction		: UdpHoleHandle
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
UdpHoleHandle::UdpHoleHandle(T_UdpHoleHandleCb * i_ptUdpHoleHandleCb)
{
    m_pIoHandle=NULL;
    memcpy(&m_tUdpHoleHandleCb,i_ptUdpHoleHandleCb,sizeof(T_UdpHoleHandleCb));
    m_iPeerSendedMsgToLocalFlag=0;
}

/*****************************************************************************
-Fuction		: ~UdpHoleHandle
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
UdpHoleHandle::~UdpHoleHandle()
{
    /*if(NULL != m_pIoHandle)
    {
        m_tUdpHoleHandleCb.Close(m_pIoHandle);
    }*/
}

/*****************************************************************************
-Fuction		: SendToPeerFirst
-Description	: 主控端 
-Input			: 
-Output 		: 
-Return 		: <0 err,0 ok,> 0 Len
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int UdpHoleHandle::SendToPeerFirst(int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort)
{
    int iRet = -1;
    const char * strSendMsg="nice to meet you";
    unsigned char abResMsg[256];
    int iResMsgLen = 0;

    if (NULL == i_strPeerPublicIP||i_iPeerPublicPort<=0)
    {
        P2P_LOGE("UdpHoleHandle::SendToPeerFirst NULL err %d %d\r\n",i_iPeerNatType,i_iPeerPublicPort);
        return iRet;
    }
    if(NULL == m_tUdpHoleHandleCb.SendData||NULL == m_tUdpHoleHandleCb.RecvData||NULL == m_tUdpHoleHandleCb.ChangePeerAddr||NULL == m_tUdpHoleHandleCb.pIoHandleObj)
    {
        P2P_LOGE("NULL == m_tUdpHoleHandleCb. err %d %d\r\n",i_iPeerPublicPort,i_iPeerNatType);
        return iRet;
    }
    
    iRet = m_tUdpHoleHandleCb.ChangePeerAddr(m_tUdpHoleHandleCb.pIoHandleObj,i_strPeerPublicIP, i_iPeerPublicPort);
    iRet = m_tUdpHoleHandleCb.SendData(m_tUdpHoleHandleCb.pIoHandleObj,(unsigned char *)strSendMsg,strlen(strSendMsg));
    if (iRet<0)
    {
        P2P_LOGE("UdpHoleHandle.SendToPeerFirst err : %s %d\r\n",i_strPeerPublicIP,i_iPeerPublicPort);
        return iRet;
    }

    /* 完整步骤不适合当前结构，后续优化为多线程+消息队列同步的形式
    m_pIoHandle = m_tUdpHoleHandleCb.Init("UDP",i_strPeerPublicIP, i_iPeerPublicPort);
    iRet =m_tUdpHoleHandleCb.SendData(m_pIoHandle,strSendMsg,strlen(strSendMsg));
    if (iRet<0)
    {
        P2P_LOGE("m_tUdpHoleHandleCb.SendData err %d %d\r\n",iRet,sizeof(abResMsg));
        return iRet;
    }

    
    iRet =m_tUdpHoleHandleCb.ReqPeerSendMsgToLocal(m_tUdpHoleHandleCb.pReqObj);

    iRet =m_tUdpHoleHandleCb.SendData(m_pIoHandle,strSendMsg,strlen(strSendMsg));
    iRet =m_tUdpHoleHandleCb.RecvData(m_pIoHandle,abResMsg,sizeof(abResMsg));
    if (iRet<=0)
    {
        P2P_LOGE("m_tUdpHoleHandleCb.RecvData err %d %d\r\n",iRet,sizeof(abResMsg));
        return -1;
    }
    iResMsgLen = iRet;
    */
    return 0;
}

/*****************************************************************************
-Fuction		: SetPeerSendedMsgToLocalFlag
-Description	: 主控端 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int UdpHoleHandle::SetPeerSendedMsgToLocalFlag(int i_iPeerSendedMsgToLocalFlag)
{
    int iRet = -1;
    unsigned char abResMsg[256];
    int iResMsgLen = 0;
    const char * strRecvedMsg="nice to meet you";//udp对象中已经有对方身份比对，当然后续可以优化为更严格的消息内容比对

    m_iPeerSendedMsgToLocalFlag=i_iPeerSendedMsgToLocalFlag;
    
    if(NULL == m_tUdpHoleHandleCb.SendData||NULL == m_tUdpHoleHandleCb.RecvData||
    NULL == m_tUdpHoleHandleCb.ChangePeerAddr||NULL == m_tUdpHoleHandleCb.pIoHandleObj)
    {
        P2P_LOGE("NULL == m_tUdpHoleHandleCb. err %d \r\n",i_iPeerSendedMsgToLocalFlag);
        return iRet;
    }
    iRet =m_tUdpHoleHandleCb.RecvData(m_tUdpHoleHandleCb.pIoHandleObj,abResMsg,sizeof(abResMsg));// b发a收测试
    if (iRet<=0)
    {
        P2P_LOGW("UdpHoleHandle.SetPeerSendedMsgToLocalFlag no RecvData %d %d\r\n",iRet,sizeof(abResMsg));
        return -1;
    }
    P2P_LOGD("UdpHoleHandle.SetPeerSendedMsgToLocalFlag RecvData %d , %s\r\n",iRet,(char *)abResMsg);
    return 0;
}


/*****************************************************************************
-Fuction		: SendToPeerAfterPeerSendToLocal
-Description	: 主控端 
-Input			: 
-Output 		: 
-Return 		: <0 err,0 ok,> 0 Len
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int UdpHoleHandle::SendToPeerAfterPeerSendToLocal(int i_iLocalNatType,int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort)
{
    int iRet = -1;
    const char * strSendMsg="nice to meet you";
    unsigned char abResMsg[256];
    int iResMsgLen = 0;
    const char * strRecvedMsg="nice to meet you too";//udp对象中已经有对方身份比对，当然后续可以优化为更严格的消息内容比对

    if (NULL == i_strPeerPublicIP||i_iPeerPublicPort<=0)
    {
        P2P_LOGE("UdpHoleHandle::SendToPeerAfterPeerSendToLocal NULL err %d %d\r\n",i_iPeerNatType,i_iPeerPublicPort);
        return iRet;
    }
    if(NULL == m_tUdpHoleHandleCb.SendData||NULL == m_tUdpHoleHandleCb.RecvData||
    NULL == m_tUdpHoleHandleCb.ChangePeerAddr||NULL == m_tUdpHoleHandleCb.pIoHandleObj)
    {
        P2P_LOGE("NULL == m_tUdpHoleHandleCb. err %d %d\r\n",i_iPeerPublicPort,i_iPeerNatType);
        return iRet;
    }
    
    iRet = m_tUdpHoleHandleCb.ChangePeerAddr(m_tUdpHoleHandleCb.pIoHandleObj,i_strPeerPublicIP, i_iPeerPublicPort);
    iRet =m_tUdpHoleHandleCb.SendData(m_tUdpHoleHandleCb.pIoHandleObj,(unsigned char*)strSendMsg,strlen(strSendMsg));
    P2P_LOGD("UdpHoleHandle.SendToPeer %s %d,%s\r\n",i_strPeerPublicIP,i_iPeerPublicPort,strSendMsg);
    iRet =m_tUdpHoleHandleCb.RecvData(m_tUdpHoleHandleCb.pIoHandleObj,abResMsg,sizeof(abResMsg));// a发b收测试
    if (iRet<=0)
    {
        P2P_LOGE("UdpHoleHandle.RecvData err %d %d\r\n",iRet,sizeof(abResMsg));
        return -1;
    }
    iResMsgLen = iRet;
    P2P_LOGD("UdpHoleHandle.RecvFromPeer %d , %s\r\n",iRet,(char *)abResMsg);
    return iResMsgLen;
}

/*****************************************************************************
-Fuction		: Proc
-Description	: 主控端 
-Input			: 
-Output 		: 
-Return 		: -1 err,0 send again,1 success
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int UdpHoleHandle::Proc(int i_iLocalNatType,int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort)
{
    int iRet = -1;
    
    if(0 == m_iPeerSendedMsgToLocalFlag)
    {
        iRet=this->SendToPeerFirst(i_iPeerNatType,i_strPeerPublicIP,i_iPeerPublicPort);
        return m_iPeerSendedMsgToLocalFlag;
    }
    iRet=this->SendToPeerAfterPeerSendToLocal(i_iLocalNatType,i_iPeerNatType,i_strPeerPublicIP,i_iPeerPublicPort);
    if(iRet < 0)
    {
        return -1;
    }
    return m_iPeerSendedMsgToLocalFlag;
}

/*****************************************************************************
-Fuction		: SendToPeer
-Description	: 受控端逻辑
-Input			: 
-Output 		: 
-Return 		: <0 err,0 ok,> 0 Len
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int UdpHoleHandle::SendToPeer(int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort)
{
    int iRet = -1;
    const char * strSendMsg="nice to meet you";
    unsigned char abResMsg[256];
    int iResMsgLen = 0;

    if (NULL == i_strPeerPublicIP||i_iPeerPublicPort<=0)
    {
        P2P_LOGE("UdpHoleHandle::SendToPeer NULL err %d %d\r\n",i_iPeerNatType,i_iPeerPublicPort);
        return iRet;
    }
    if(NULL == m_tUdpHoleHandleCb.SendData||NULL == m_tUdpHoleHandleCb.RecvData||NULL == m_tUdpHoleHandleCb.ChangePeerAddr||NULL == m_tUdpHoleHandleCb.pIoHandleObj)
    {
        P2P_LOGE("NULL == m_tUdpHoleHandleCb. err %d %d\r\n",i_iPeerPublicPort,i_iPeerNatType);
        return iRet;
    }
    
    iRet = m_tUdpHoleHandleCb.ChangePeerAddr(m_tUdpHoleHandleCb.pIoHandleObj,i_strPeerPublicIP, i_iPeerPublicPort);
    m_tUdpHoleHandleCb.RecvData(m_tUdpHoleHandleCb.pIoHandleObj,abResMsg,sizeof(abResMsg));//正常情况对方发过来是收不到的，以防万一取完缓存
    iRet = m_tUdpHoleHandleCb.SendData(m_tUdpHoleHandleCb.pIoHandleObj,(unsigned char *)strSendMsg,strlen(strSendMsg));
    if (iRet<0)
    {
        P2P_LOGE("UdpHoleHandle.SendToPeer err : %s %d\r\n",i_strPeerPublicIP,i_iPeerPublicPort);
        return iRet;
    }
    P2P_LOGD("UdpHoleHandle.SendToPeer %s %d,%s\r\n",i_strPeerPublicIP,i_iPeerPublicPort,strSendMsg);
    return 0;
}

/*****************************************************************************
-Fuction		: RecvFromPeer
-Description	: 受控端逻辑
-Input			: 
-Output 		: 
-Return 		: <0 err,0 ok,> 0 Len
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int UdpHoleHandle::RecvFromPeer(int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort)
{
    int iRet = -1;
    const char * strSendMsg="nice to meet you too";
    unsigned char abResMsg[256];
    int iResMsgLen = 0;
    const char * strRecvedMsg="nice to meet you";//udp对象中已经有对方身份比对，当然后续可以优化为更严格的消息内容比对

    if (NULL == i_strPeerPublicIP||i_iPeerPublicPort<=0)
    {
        P2P_LOGE("UdpHoleHandle::RecvFromPeer NULL err %d %d\r\n",i_iPeerNatType,i_iPeerPublicPort);
        return iRet;
    }
    if(NULL == m_tUdpHoleHandleCb.SendData||NULL == m_tUdpHoleHandleCb.RecvData||NULL == m_tUdpHoleHandleCb.ChangePeerAddr||NULL == m_tUdpHoleHandleCb.pIoHandleObj)
    {
        P2P_LOGE("NULL == m_tUdpHoleHandleCb. err %d %d\r\n",i_iPeerPublicPort,i_iPeerNatType);
        return iRet;
    }
    
    iRet = m_tUdpHoleHandleCb.ChangePeerAddr(m_tUdpHoleHandleCb.pIoHandleObj,i_strPeerPublicIP, i_iPeerPublicPort);
    iRet = m_tUdpHoleHandleCb.RecvData(m_tUdpHoleHandleCb.pIoHandleObj,abResMsg,sizeof(abResMsg));
    if (iRet<=0)
    {
        P2P_LOGE("UdpHoleHandle.RecvFromPeer RecvData err %d %d\r\n",iRet,sizeof(abResMsg));
        return 0;
    }
    iResMsgLen = iRet;
    P2P_LOGD("UdpHoleHandle.RecvFromPeer %d , %s\r\n",iRet,(char *)abResMsg);
    iRet = m_tUdpHoleHandleCb.SendData(m_tUdpHoleHandleCb.pIoHandleObj,(unsigned char *)strSendMsg,strlen(strSendMsg));
    if (iRet<0)
    {
        P2P_LOGE("UdpHoleHandle.RecvFromPeer err : %s %d\r\n",i_strPeerPublicIP,i_iPeerPublicPort);
        return iRet;
    }
    P2P_LOGD("UdpHoleHandle.SendToPeer %s %d,%s\r\n",i_strPeerPublicIP,i_iPeerPublicPort,strSendMsg);
    return iResMsgLen;
}


