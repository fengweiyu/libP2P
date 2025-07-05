/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ServerDemo.c
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "ServerDemo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <utility>

using std::make_pair;

/*****************************************************************************
-Fuction		: ServerDemo
-Description	: ServerDemo
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
ServerDemo :: ServerDemo(int i_iServerPort)
{
    m_pServerHandle=NULL;
    TcpServer::Init(NULL,i_iServerPort);
}

/*****************************************************************************
-Fuction		: ~HttpServerDemo
-Description	: ~HttpServerDemo
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2023/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
ServerDemo :: ~ServerDemo()
{
    if(NULL != m_pServerHandle)
    {
        delete m_pServerHandle;
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
* 2023/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int ServerDemo :: Proc()
{
    int iClientSocketFd=-1;
    T_Peer2PeerCfg tPeer2PeerCfg;
    ThreadSafeQueue<QueueMessage> oThreadSafeQueue;
    ServerIO *pServerIO = NULL;

    memset(&tPeer2PeerCfg,0,sizeof(T_Peer2PeerCfg));
    snprintf(tPeer2PeerCfg.strStunServer1Addr,sizeof(tPeer2PeerCfg.strStunServer1Addr),"%s","stun.voipbuster.com");
    tPeer2PeerCfg.iStunServer1Port=3478;
    snprintf(tPeer2PeerCfg.strStunServer2Addr,sizeof(tPeer2PeerCfg.strStunServer2Addr),"%s","gwm-000-cn-0448.bcloud365.net");
    tPeer2PeerCfg.iStunServer2Port=3478;
    if(NULL == m_pServerHandle)
    {
        m_pServerHandle= new ServerHandle(&oThreadSafeQueue);
    }  
    while(1)
    {
        iClientSocketFd=TcpServer::Accept();//非阻塞，后续优化为线程阻塞效率更高些
        if(iClientSocketFd<0)  
        {  
            SleepMs(10);
            CheckMapServerIO();
            continue;
        } 
        pServerIO = new ServerIO(iClientSocketFd,&oThreadSafeQueue,&tPeer2PeerCfg);
        AddMapServerIO(pServerIO,iClientSocketFd);
        P2P_LOGD("m_ServerIOMap size %d\r\n",m_ServerIOMap.size());
    }
    return 0;
}

/*****************************************************************************
-Fuction        : CheckMapServerIO
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int ServerDemo::CheckMapServerIO()
{
    int iRet = -1;
    ServerIO *pServerIO=NULL;

    std::lock_guard<std::mutex> lock(m_MapMtx);//std::lock_guard对象会在其作用域结束时自动释放互斥量
    for (map<int, ServerIO *>::iterator iter = m_ServerIOMap.begin(); iter != m_ServerIOMap.end(); )
    {
        pServerIO=iter->second;
        if(0 == pServerIO->GetProcFlag())
        {
            delete pServerIO;
            iter=m_ServerIOMap.erase(iter);// 擦除元素并返回下一个元素的迭代器
        }
        else
        {
            iter++;// 继续遍历下一个元素
        }
    }
    return 0;
}

/*****************************************************************************
-Fuction        : AddMapHttpSession
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2023/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int ServerDemo::AddMapServerIO(ServerIO * i_pServerIO,int i_iClientSocketFd)
{
    int iRet = -1;

    if(NULL == i_pServerIO)
    {
        P2P_LOGE("AddMapServerIO NULL!!!%p\r\n",i_pServerIO);
        return -1;
    }
    std::lock_guard<std::mutex> lock(m_MapMtx);//std::lock_guard对象会在其作用域结束时自动释放互斥量
    m_ServerIOMap.insert(make_pair(i_iClientSocketFd,i_pServerIO));
    return 0;
}


