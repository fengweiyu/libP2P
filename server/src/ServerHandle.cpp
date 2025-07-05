/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ServerHandle.cpp
* Description           : 	    接口层，防止曝露内部文件
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "ServerHandle.h"
#include "Peer2PeerManager.h"

/*****************************************************************************
-Fuction        : ServerHandle
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
ServerHandle::ServerHandle(ThreadSafeQueue<QueueMessage> * i_pMgrQueue)
{
    m_pHandle = NULL;
    m_pHandle = new Peer2PeerManager();
    
    m_iServerHandleFlag = 0;
    m_pServerHandleProc = new thread(&ServerHandle::Proc,this,i_pMgrQueue);
}
/*****************************************************************************
-Fuction        : ~ServerHandle
-Description    : ~ServerHandle
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
ServerHandle::~ServerHandle()
{
    if(NULL!= m_pServerHandleProc)
    {
        P2P_LOGW("~~ServerHandle start exit\r\n");
        m_iServerHandleFlag = 0;
        m_pServerHandleProc->join();//
        delete m_pServerHandleProc;
        m_pServerHandleProc = NULL;
    }
    P2P_LOGW("~~ServerHandle exit\r\n");
    if(NULL != m_pHandle)
    {
        Peer2PeerManager *pPeer2PeerManager = (Peer2PeerManager *)m_pHandle;
        delete pPeer2PeerManager;
    }  
}

/*****************************************************************************
-Fuction        : Proc
-Description    : Proc
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerHandle::Proc(ThreadSafeQueue<QueueMessage> * i_pMgrQueue)
{
    int iRet = -1;
    
    m_iServerHandleFlag = 1;
    P2P_LOGW("ServerHandle start Proc\r\n");
    while(m_iServerHandleFlag)
    {
        Peer2PeerManager *pPeer2PeerManager = (Peer2PeerManager *)m_pHandle;
        iRet = pPeer2PeerManager->Proc(i_pMgrQueue);
    }
    m_iServerHandleFlag=0;
    return iRet;
}

