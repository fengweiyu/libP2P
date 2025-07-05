/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ServerIO.c
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "ServerIO.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <unistd.h>
#include <thread>

using std::thread;

#define IO_RECV_MAX_LEN (1024)
#define IO_SEND_MAX_LEN (1024)

/*****************************************************************************
-Fuction		: HttpServerIO
-Description	: 
或者分离线程函数使用静态函数，线程函数内阻塞处理并释放对象
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
ServerIO :: ServerIO(int i_iClientSocketFd,ThreadSafeQueue<QueueMessage> * i_pMgrQueue,T_Peer2PeerCfg * i_ptPeer2PeerCfg)
{
    m_iClientSocketFd=i_iClientSocketFd;
    m_pServerSessionInf = NULL;

    m_iServerIOFlag = 0;
    m_pServerIOProc = new thread(&ServerIO::Proc,this,i_pMgrQueue,i_ptPeer2PeerCfg);
    //m_pHttpSessionProc->detach();//注意线程回收
}

/*****************************************************************************
-Fuction		: ~HttpServerIO
-Description	: 析构函数由外部调用
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
ServerIO :: ~ServerIO()
{
    if(NULL!= m_pServerSessionInf)
    {
        delete m_pServerSessionInf;
        m_pServerSessionInf = NULL;
    }
    if(NULL!= m_pServerIOProc)
    {
        P2P_LOGW("~~ServerIO start exit\r\n");
        m_iServerIOFlag = 0;
        //while(0 == m_iExitProcFlag){usleep(10);};
        m_pServerIOProc->join();//
        delete m_pServerIOProc;
        m_pServerIOProc = NULL;
    }
    P2P_LOGW("~~ServerIO exit\r\n");
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
int ServerIO :: Proc(ThreadSafeQueue<QueueMessage> * i_pMgrQueue,T_Peer2PeerCfg * i_ptPeer2PeerCfg)
{
    int iRet=-1;
    char *pcRecvBuf=NULL;
    char *pcSendBuf=NULL;
    int iRecvLen=-1;
    int iSendLen=-1;
    timeval tTimeValue;

    
    if(NULL== m_pServerSessionInf)
    {
        m_pServerSessionInf = new ServerSessionInf(i_pMgrQueue,i_ptPeer2PeerCfg);
    }
    if(m_iClientSocketFd < 0)
    {
        P2P_LOGE("ServerIO m_iClientSocketFd < 0 err\r\n");
        return -1;
    }
    pcRecvBuf = new char[IO_RECV_MAX_LEN];
    if(NULL == pcRecvBuf)
    {
        P2P_LOGE("ServerIO NULL == pcRecvBuf err\r\n");
        return -1;
    }
    pcSendBuf = new char[IO_SEND_MAX_LEN];
    if(NULL == pcSendBuf)
    {
        P2P_LOGE("ServerIO NULL == pcSendBuf err\r\n");
        delete[] pcRecvBuf;
        return -1;
    }
    m_iServerIOFlag = 1;
    P2P_LOGW("ServerIO start Proc\r\n");
    iRecvLen = 0;
    while(m_iServerIOFlag)
    {
        memset(pcSendBuf,0,IO_SEND_MAX_LEN);
        iRet = m_pServerSessionInf->Proc(pcRecvBuf, iRecvLen, pcSendBuf, IO_SEND_MAX_LEN);
        if(iRet > 0)
        {
            iSendLen=iRet;
            iRet=TcpServer::Send(pcSendBuf,iSendLen,m_iClientSocketFd);
            if(iRet<0)
            {
                P2P_LOGE("TcpServer::Send err exit %d\r\n",iRet);
                break;
            }
        }
    
        iRecvLen = 0;
        memset(pcRecvBuf,0,IO_RECV_MAX_LEN);
        milliseconds timeMS(300);// 表示300毫秒
        iRet=TcpServer::Recv(pcRecvBuf,&iRecvLen,IO_RECV_MAX_LEN,m_iClientSocketFd,&timeMS);
        if(iRet < 0)
        {
            P2P_LOGE("TcpServer::Recv err exit %d\r\n",iRecvLen);
            break;
        }
        if(iRecvLen<=0)
        {
            continue;
        }
    }
    
    if(m_iClientSocketFd>=0)
    {
        TcpServer::Close(m_iClientSocketFd);//主动退出,
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
    
    m_iServerIOFlag=0;
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
int ServerIO :: GetProcFlag()
{
    return m_iServerIOFlag;//多线程竞争注意优化
}

