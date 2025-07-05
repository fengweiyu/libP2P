/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ServerSessionInf.cpp
* Description           : 	    接口层，防止曝露内部文件
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "ServerSessionInf.h"
#include "ServerSession.h"

/*****************************************************************************
-Fuction        : ServerSessionInf
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
ServerSessionInf::ServerSessionInf(ThreadSafeQueue<QueueMessage> * i_pMgrQueue,T_Peer2PeerCfg * i_ptPeer2PeerCfg)
{
    m_pHandle = NULL;
    m_pHandle = new ServerSession(i_pMgrQueue,i_ptPeer2PeerCfg);
}
/*****************************************************************************
-Fuction        : ~ServerSessionInf
-Description    : ~ServerSessionInf
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
ServerSessionInf::~ServerSessionInf()
{
    if(NULL != m_pHandle)
    {
        ServerSession *pServerSession = (ServerSession *)m_pHandle;
        delete pServerSession;
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
int ServerSessionInf::Proc(char * i_strReq,int i_iReqLen,char *o_strRes,int i_iResMaxLen)
{
    if(NULL != m_pHandle)
    {
        ServerSession *pServerSession = (ServerSession *)m_pHandle;
        return pServerSession->Proc(i_strReq,i_iReqLen,o_strRes,i_iResMaxLen);
    }  
    return -1;
}

