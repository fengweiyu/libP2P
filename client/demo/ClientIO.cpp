/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ClientIO.c
* Description           : 	    本文件目的是实现业务和IO分离
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "ClientIO.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <unistd.h>
#include <thread>

using std::thread;

#define IO_RECV_MAX_LEN (1024)
#define IO_SEND_MAX_LEN (1024)

/*****************************************************************************
-Fuction		: ClientIO
-Description	: ClientIO
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
ClientIO :: ClientIO(const char * i_strServerAddr,unsigned short i_wServerPort)
{
    int iRet = -1;
    string strAddr;

    if(NULL == i_strServerAddr)
    {
        P2P_LOGE("ClientIO NULL == i_strServerAddr err %d\r\n",i_wServerPort);
        return;
    }
    
    strAddr.assign(i_strServerAddr);
    iRet = TcpClient::Init(&strAddr,i_wServerPort);
    if(iRet < 0)
    {
        P2P_LOGE("TcpClient::Init err %s %d\r\n",i_strServerAddr,i_wServerPort);
        return;
    }

    m_iClientSocketFd=TcpClient::GetClientSocket();
    m_iIOProcFlag = 0;
    m_pClientSession = NULL;
}

/*****************************************************************************
-Fuction		: ~ClientIO
-Description	: ~ClientIO
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
ClientIO :: ~ClientIO()
{
    if(NULL!= m_pClientSession)
    {
        delete m_pClientSession;
        m_pClientSession = NULL;
    }
}

/*****************************************************************************
-Fuction		: Proc
-Description	: 阻塞
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int ClientIO :: Proc(const char * i_strLocalID,const char * i_strPeerID,const char * i_strLocalIP)
{
    int iRet=-1;
    char *pcRecvBuf=NULL;
    int iRecvLen=-1;
    char *pcSendBuf=NULL;
    int iSendLen=-1;
    timeval tTimeValue;
    T_ClientSessionCb tClientSessionCb;
    T_ClientSessionCfg tClientSessionCfg;


    if(NULL == i_strLocalID)
    {
        P2P_LOGE("NULL == i_strLocalID err\r\n");
        return -1;
    }
    if(m_iClientSocketFd < 0)
    {
        P2P_LOGE("ClientIO m_iClientSocketFd < 0 err\r\n");
        return -1;
    }

    memset(&tClientSessionCb,0,sizeof(T_ClientSessionCb));
    tClientSessionCfg.Init = ClientIO::IoInit;
    tClientSessionCfg.SendData = ClientIO::IoSendData;
    tClientSessionCfg.RecvData = ClientIO::IoRecvData;
    tClientSessionCfg.Close= ClientIO::IoClose;
    memset(&tClientSessionCfg,0,sizeof(T_ClientSessionCfg));
    snprintf(tClientSessionCfg.strLocalID,sizeof(tClientSessionCfg.strLocalID),"%s",i_strLocalID);
    if(NULL!= i_strLocalIP)
    {
        snprintf(tClientSessionCfg.strLocalIP,sizeof(tClientSessionCfg.strLocalIP),"%s",i_strLocalIP);
    }
    else
    {
        snprintf(tClientSessionCfg.strLocalIP,sizeof(tClientSessionCfg.strLocalIP),"%s","127.0.0.1");
    }
    if(NULL!= i_strPeerID)
    {
        snprintf(tClientSessionCfg.strPeerID,sizeof(tClientSessionCfg.strPeerID),"%s",i_strPeerID);
    }
    if(NULL== m_pClientSession)
    {
        m_pClientSession = new ClientSession(&tClientSessionCb,&tClientSessionCfg);
    }


    
    pcRecvBuf = new char[IO_RECV_MAX_LEN];
    if(NULL == pcRecvBuf)
    {
        P2P_LOGE("IO NULL == pcRecvBuf err\r\n");
        return -1;
    }
    pcSendBuf = new char[IO_SEND_MAX_LEN];
    if(NULL == pcSendBuf)
    {
        P2P_LOGE("IO NULL == pcSendBuf err\r\n");
        delete[] pcRecvBuf;
        return -1;
    }

    
    m_iIOProcFlag = 1;
    P2P_LOGW("IO start Proc\r\n");
    iRecvLen = 0;
    memset(pcRecvBuf,0,IO_RECV_MAX_LEN);
    iSendLen=0;
    memset(pcSendBuf,0,IO_SEND_MAX_LEN);
    while(m_iIOProcFlag)
    {
        memset(pcSendBuf,0,IO_SEND_MAX_LEN);
        iRet=m_pClientSession->Proc(pcRecvBuf,iRecvLen,pcSendBuf,IO_SEND_MAX_LEN)
        if(iRet > 0)
        {
            iSendLen=iRet;
            iRet=TcpClient::Send(pcSendBuf,iSendLen,m_iClientSocketFd);
            if(iRet<0)
            {
                P2P_LOGE("TcpClient::Send err exit %d\r\n",iRet);
                break;
            }
        }

        iRecvLen = 0;
        memset(pcRecvBuf,0,IO_RECV_MAX_LEN);
        milliseconds timeMS(300);// 表示300毫秒
        iRet=TcpClient::Recv(pcRecvBuf,&iRecvLen,IO_RECV_MAX_LEN,m_iClientSocketFd,&timeMS);
        if(iRet < 0)
        {
            P2P_LOGE("TcpClient::Recv err exit %d\r\n",iRecvLen);
            break;
        }
        if(iRecvLen<=0)
        {
            continue;
        }
    }
    
    if(m_iClientSocketFd>=0)
    {
        TcpClient::Close(m_iClientSocketFd);//主动退出,
        P2P_LOGW("HttpServerIO::Close m_iClientSocketFd Exit%d\r\n",m_iClientSocketFd);
    }
    if(NULL != pcSendBuf)
    {
        delete[] pcSendBuf;
    }
    if(NULL != pcRecvBuf)
    {
        delete[] pcRecvBuf;
    }
    
    m_iIOProcFlag=0;
    return 0;
}

/*****************************************************************************
-Fuction		: GetProcFlag
-Description	: HttpServerIO
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int ClientIO :: GetProcFlag()
{
    return m_iIOProcFlag;//多线程竞争注意优化
}

/*****************************************************************************
-Fuction		: IoInit
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
void * ClientIO::IoInit(const char * strProtocolType,const char * i_strPeerIP,int i_iPeerPort)
{
    int iRet = -1;
    UdpClient * pUdpClient = NULL;
    
    if(NULL == strProtocolType ||0!=strcmp("UDP",strProtocolType)||NULL == i_strPeerIP||i_iPeerPort<0)
    {
        P2P_LOGE("IoInit err %d %d\r\n",iRet,i_iPeerPort);
        return pUdpClient;
    }
    if(0!=strcmp("UDP",strProtocolType))
    {
        P2P_LOGE("IoInit err %d %d\r\n",iRet,i_iPeerPort);
        return pUdpClient;
    }
    pUdpClient = new UdpClient();
    iRet = pUdpClient->Init(i_strPeerIP, (unsigned short)i_iPeerPort);
    if(iRet<0)
    {
        P2P_LOGE("pUdpClient->Init err %d %d\r\n",iRet,i_iPeerPort);
        delete pUdpClient;
        pUdpClient = NULL;
    }
    return pUdpClient;
}

/*****************************************************************************
-Fuction		: IoSendData
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int ClientIO::IoSendData(void *i_pIoHandle, unsigned char * i_pbSendBuf,int i_iSendLen)
{
    int iRet = -1;
    UdpClient * pUdpClient = NULL;

    if(NULL == i_pIoHandle ||NULL == i_pbSendBuf ||i_iSendLen<0)
    {
        P2P_LOGE("IoSendData err %d %d\r\n",iRet,i_iSendLen);
        return iRet;
    }
    pUdpClient = (UdpClient *)i_pIoHandle;    
    iRet = pUdpClient->Send(i_pbSendBuf,i_iSendLen);
    return iRet;
}

/*****************************************************************************
-Fuction		: IoRecvData
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int ClientIO::IoRecvData(void *i_pIoHandle, unsigned char * o_pbRecvBuf,int i_iRecvBufMaxLen)
{
    int iRet = -1;
    UdpClient * pUdpClient = NULL;
    int iRecvLen=0;
    
    if(NULL == i_pIoHandle ||NULL == o_pbRecvBuf)
    {
        P2P_LOGE("IoRecvData err %d %d\r\n",iRet,i_iRecvBufMaxLen);
        return iRet;
    }
    pUdpClient = (UdpClient *)i_pIoHandle;    
    iRet = pUdpClient->Recv(o_pbRecvBuf,&iRecvLen ,i_iRecvBufMaxLen);
    if(iRecvLen<=0)
    {
        P2P_LOGE("IoRecvData pUdpClient->Recv err %d %d\r\n",iRet,i_iRecvBufMaxLen);
        return -1;
    }
    return iRecvLen;
}

/*****************************************************************************
-Fuction		: IoSendData
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int ClientIO::IoClose(void *i_pIoHandle)
{
    int iRet = -1;
    UdpClient * pUdpClient = NULL;
    
    if(NULL == i_pIoHandle)
    {
        P2P_LOGE("IoClose err %d r\n",iRet);
        return iRet;
    }
    pUdpClient = (UdpClient *)i_pIoHandle;    
    pUdpClient->Close();
    delete pUdpClient;
    return iRet ;
}

