/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	Peer2PeerHandle.cpp
* Description		: 	Peer2PeerHandle operation center
* Created			: 	2020.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include "Peer2PeerHandle.h"
#include <string.h>

/*****************************************************************************
-Fuction		: Peer2PeerHandle
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Peer2PeerHandle::Peer2PeerHandle()
{
    memset(&m_tPeer2PeerCb,0,sizeof(T_Peer2PeerCb));
    m_pNatDetect=NULL;
    m_pUdpHoleHandle=NULL;
}

/*****************************************************************************
-Fuction		: ~Peer2PeerHandle
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Peer2PeerHandle::~Peer2PeerHandle()
{
    if(NULL != m_pNatDetect)
    {
        delete m_pNatDetect;
    }
    if(NULL != m_pUdpHoleHandle)
    {
        delete m_pUdpHoleHandle;
    }
}

/*****************************************************************************
-Fuction		: Proc
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Peer2PeerHandle::Proc(T_Peer2PeerCb *i_ptPeer2PeerCb,const char * i_strLocalIP,const char * i_strStunServer1IP,int i_iStunServer1Port,const char * i_strStunServer2IP,int i_iStunServer2Port)
{
    int iRet = -1;
    T_NatDetectCb tNatDetectCb;

    
    if(NULL == i_strStunServer1IP||NULL == i_strStunServer2IP||NULL == i_strLocalIP||
    NULL == i_ptPeer2PeerCb||i_iStunServer1Port<=0||i_iStunServer2Port<=0)
    {
        P2P_LOGE("Peer2PeerHandle Proc NULL err %d %d\r\n",i_iStunServer1Port,i_iStunServer2Port);
        return -1;
    }
    
    memcpy(&m_tPeer2PeerCb,i_ptPeer2PeerCb,sizeof(T_Peer2PeerCb));
    if(NULL == m_tPeer2PeerCb.SendData||NULL == m_tPeer2PeerCb.RecvData||NULL == m_tPeer2PeerCb.Init||NULL == m_tPeer2PeerCb.Close||
    NULL == m_tPeer2PeerCb.ReportLocalNatInfo||NULL == m_tPeer2PeerCb.pSessionHandle)
    {
        P2P_LOGE("NULL == m_tPeer2PeerCb. err %d %d\r\n",i_iStunServer1Port,i_iStunServer2Port);
        return iRet;
    }

    memset(&tNatDetectCb,0,sizeof(T_NatDetectCb));
    tNatDetectCb.pReportObj = this;
    tNatDetectCb.ReportResult = Peer2PeerHandle::ReportResultCb;
    tNatDetectCb.Init = i_ptPeer2PeerCb->Init;
    tNatDetectCb.SendData = i_ptPeer2PeerCb->SendData;
    tNatDetectCb.RecvData = i_ptPeer2PeerCb->RecvData;
    tNatDetectCb.Close= i_ptPeer2PeerCb->Close;
    if(NULL == m_pNatDetect)
        m_pNatDetect=new NatDetect(&tNatDetectCb,i_strLocalIP,i_strStunServer1IP,i_iStunServer1Port,i_strStunServer2IP,i_iStunServer2Port);
    return m_pNatDetect->Proc();
}

/*****************************************************************************
-Fuction		: ReportResultCb
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Peer2PeerHandle::ReportResultCb(void * i_pReportObj,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort)
{
    int iRet = -1;
    Peer2PeerHandle * pPeer2PeerHandle = NULL;
    
    if(NULL == i_strPublicIP)
    {
        P2P_LOGE("ReportResultCb err i_iNatType %d ,i_iPublicPort %d \r\n",i_iNatType,i_iPublicPort);
        return iRet;
    }
    if(NULL == i_pReportObj)
    {
        P2P_LOGE("ReportResultCb NULL == i_pReportObj i_iNatType %d ,i_iPublicPort %d \r\n",i_iNatType,i_iPublicPort);
        return iRet;
    }
    pPeer2PeerHandle = (Peer2PeerHandle *)i_pReportObj;
    return pPeer2PeerHandle->LocalNatInfoHandle(i_iNatType,i_strPublicIP,i_iPublicPort);
}

/*****************************************************************************
-Fuction		: ReportResultCb
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Peer2PeerHandle::LocalNatInfoHandle(int i_iNatType,const char * i_strPublicIP,int i_iPublicPort)
{
    int iRet = -1;
    char strPeerPublicIP[128];
    int iPeerNatType=0;
    int iPeerPublicPort=0;
    T_UdpHoleHandleCb tUdpHoleHandleCb;
    
    if(NULL != m_tPeer2PeerCb.ReportLocalNatInfo)
    {
        iRet=m_tPeer2PeerCb.ReportLocalNatInfo(m_tPeer2PeerCb.pSessionHandle,i_iNatType,i_strPublicIP,i_iPublicPort);
    }
    return iRet ;
    
    /* 完整步骤(回调方式)不适合当前(逻辑要简单)结构，后续优化为多线程+消息队列同步的形式
    if(NULL != m_tPeer2PeerCb.ReportLocalNatInfo)
    {
        iRet=m_tPeer2PeerCb.ReportLocalNatInfo(m_tPeer2PeerCb.pSessionHandle,i_iNatType,i_strPublicIP,i_iPublicPort);
    }
    if(NULL == i_strPublicIP)
    {
        P2P_LOGE("ReportResultCb err i_iNatType %d ,i_iPublicPort %d \r\n",i_iNatType,i_iPublicPort);
        return iRet;
    }
    if(i_iNatType >= 4||i_iNatType<0)
    {
        P2P_LOGW("ReportResultCb warn NatType %d ,PublicIP %s,PublicPort %d \r\n",i_iNatType,i_strPublicIP,i_iPublicPort);
        return iRet;
    }
    if(NULL == m_tPeer2PeerCb.GetPeerNatInfo)
    {
        P2P_LOGW("ReportResultCb GetPeerNatInfo NULL NatType %d ,PublicIP %s,PublicPort %d \r\n",i_iNatType,i_strPublicIP,i_iPublicPort);
        return 0;
    }
    STUN_LOGI("ReportResultCb NatType %d ,PublicIP %s,PublicPort %d \r\n",i_iNatType,i_strPublicIP,i_iPublicPort);
    memset(strPeerPublicIP,0,sizeof(strPeerPublicIP));
    iRet=m_tPeer2PeerCb.GetPeerNatInfo(m_tPeer2PeerCb.pSessionHandle,strPeerPublicIP,sizeof(strPeerPublicIP)-1,&iPeerPublicPort,&iPeerNatType);
    if(iRet <= 0)
    {
        P2P_LOGW("ReportResultCb GetPeerNatInfo err NatType %d ,PublicIP %s,PublicPort %d \r\n",i_iNatType,i_strPublicIP,i_iPublicPort);
        return iRet;
    }
    if(iPeerNatType >= 4||iPeerNatType<0)
    {
        P2P_LOGW("GetPeerNatInfo warn NatType %d ,PublicIP %s,PublicPort %d \r\n",iPeerNatType,strPeerPublicIP,iPeerPublicPort);
        return iRet;
    }

    memset(&tUdpHoleHandleCb,0,sizeof(T_UdpHoleHandleCb));
    tUdpHoleHandleCb.pReqObj = m_tPeer2PeerCb.pSessionHandle;
    tUdpHoleHandleCb.ReqPeerSendMsgToLocal = m_tPeer2PeerCb.ReqPeerSendMsgToLocal;
    tUdpHoleHandleCb.Init = m_tPeer2PeerCb.Init;
    tUdpHoleHandleCb.SendData = m_tPeer2PeerCb.SendData;
    tUdpHoleHandleCb.RecvData = m_tPeer2PeerCb.RecvData;
    tUdpHoleHandleCb.Close= m_tPeer2PeerCb.Close;
    iRet=m_pUdpHoleHandle->Proc(&tUdpHoleHandleCb,i_iNatType,iPeerNatType,strPeerPublicIP,iPeerPublicPort);
    return iRet ;
    */
}


/*****************************************************************************
-Fuction		: Peer2PeerHoleHandle
-Description	: 
-Input			: 
-Output 		: 
-Return 		: <0 err,0 ReqPeerSendMsgToLocal,>0 recv peer data
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Peer2PeerHandle::Peer2PeerHoleHandle(int i_iLocalNatType,int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort)
{
    int iRet = -1;
    char strPeerPublicIP[128];
    int iPeerNatType=0;
    int iPeerPublicPort=0;
    T_UdpHoleHandleCb tUdpHoleHandleCb;

    
    if(i_iLocalNatType >= 4||i_iLocalNatType<0)
    {
        P2P_LOGW("Peer2PeerHoleHandle warn i_iLocalNatType %d ,PeerPublicIP %s,PeerPublicPort %d \r\n",i_iLocalNatType,i_strPeerPublicIP,i_iPeerPublicPort);
        return iRet;
    }
    if(i_iPeerNatType >= 4||i_iPeerNatType<0)
    {
        P2P_LOGW("Peer2PeerHoleHandle warn i_iPeerNatType %d ,i_strPeerPublicIP %s,i_iPeerPublicPort %d \r\n",i_iPeerNatType,i_strPeerPublicIP,i_iPeerPublicPort);
        return iRet;
    }
    P2P_LOGI("Peer2PeerHoleHandle i_iPeerNatType %d ,i_strPeerPublicIP %s,i_iPeerPublicPort %d \r\n",i_iPeerNatType,i_strPeerPublicIP,i_iPeerPublicPort);

    memset(&tUdpHoleHandleCb,0,sizeof(T_UdpHoleHandleCb));
    tUdpHoleHandleCb.Init = m_tPeer2PeerCb.Init;
    tUdpHoleHandleCb.SendData = m_tPeer2PeerCb.SendData;
    tUdpHoleHandleCb.RecvData = m_tPeer2PeerCb.RecvData;
    tUdpHoleHandleCb.Close= m_tPeer2PeerCb.Close;
    if(NULL == m_pUdpHoleHandle)
    {
        m_pUdpHoleHandle=new UdpHoleHandle();
    }
    return m_pUdpHoleHandle->Proc(&tUdpHoleHandleCb,i_iLocalNatType,i_iPeerNatType,i_strPeerPublicIP,i_iPeerPublicPort);
}

/*****************************************************************************
-Fuction		: SetPeerSendedMsgToLocalFlag
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Peer2PeerHandle::SetPeerSendedMsgToLocalFlag(int i_iPeerSendedMsgToLocalFlag)
{
    if(NULL == m_pUdpHoleHandle)
    {
        P2P_LOGE("SetPeerSendedMsgToLocalFlag NULL == m_pUdpHoleHandle err %d \r\n",i_iPeerSendedMsgToLocalFlag);
        return -1;
    }
    return m_pUdpHoleHandle->SetPeerSendedMsgToLocalFlag(i_iPeerSendedMsgToLocalFlag);
}

/*****************************************************************************
-Fuction		: SendMsgToPeer
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Peer2PeerHandle::SendMsgToPeer(int i_iPeerNatType,const char * i_strPeerPublicIP,int i_iPeerPublicPort)
{
    if(NULL == m_pUdpHoleHandle)
    {
        P2P_LOGE("SendMsgToPeer NULL == m_pUdpHoleHandle err %d \r\n",i_iPeerNatType);
        return -1;
    }
    return m_pUdpHoleHandle->SendToPeer(i_iPeerNatType,i_strPeerPublicIP,i_iPeerPublicPort);
}





