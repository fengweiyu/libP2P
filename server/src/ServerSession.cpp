/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ServerSession.cpp
* Description           : 	    P2P C-S协议实现
* Created               :       2022.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "ServerSession.h"
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

using std::string;




/*****************************************************************************
-Fuction        : ServerSession
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
ServerSession::ServerSession(ThreadSafeQueue<QueueMessage> * i_pMgrQueue,T_ClientSessionCb * i_ptClientSessionCb,T_ClientSessionCfg * i_ptClientSessionCfg)
{
    m_oSessionQueue.clear();

    m_pMgrQueue=i_pMgrQueue;
    // 从输入队列获取消息  
    QueueMessage msg;  
    if (m_pMgrQueue->WaitAndPop(msg, 100)<0) 
    { // 100ms超时  
        // 处理消息并回复  
        Message reply;  
        reply.sender_id = thread_id;  
        reply.content = "Reply to: " + msg.content;  
        out_queue.Push(reply);  
    }  

    memcpy(&m_tClientSessionCb,i_ptClientSessionCb,sizeof(T_ClientSessionCb));
    memcpy(&m_tClientSessionCfg,i_ptClientSessionCfg,sizeof(T_ClientSessionCfg));
    m_pPeer2PeerHandle = new Peer2PeerHandle();
    memset(&m_tLocalNatInfo,0,sizeof(T_NatInfo));
    memset(&m_tPeerNatInfo,0,sizeof(T_NatInfo));
    m_iPeer2PeerHandleSuccessFlag=0;
    m_eStatus=CLIENT_SESSION_LOGIN;
}
/*****************************************************************************
-Fuction        : ~ServerSession
-Description    : ~ServerSession
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
ServerSession::~ServerSession()
{
    if(NULL != m_pPeer2PeerHandle)
    {
        delete m_pPeer2PeerHandle;
        m_pPeer2PeerHandle = NULL;
    }
}
/*****************************************************************************
-Fuction        : Proc
-Description    : 
-Input          : 
-Output         : 
-Return         : <0 err
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::Proc(char * i_strReq,int i_iReqLen,char *o_strRes,int i_iResMaxLen)
{
    int iRet = -1;
    int iReqOrRes=-1;
    char strCmdBuf[64];
    char strResResultDesc[16];

    if(NULL == o_strRes)
    {
        P2P_LOGE("ClientSession::Proc NULL err %d \r\n",i_iResMaxLen);
        return iRet;
    }
    memset(strCmdBuf,0,sizeof(strCmdBuf));
    if(NULL != i_strReq&&i_iReqLen>0)
    {
        iRet=ParseServerMsg(i_strReq,&iReqOrRes,strCmdBuf,sizeof(strCmdBuf));
        if(iRet < 0)
        {
            P2P_LOGE("ClientSession::Proc ParseServerMsg err %d \r\n",iRet);
            return iRet;
        }
    }
    if(0 == iReqOrRes)
    {
        if(0==strcmp(strCmdBuf,"SendMsgToPeer"))
        {
            char strPeerID[64];
            int iPeerNatType=-1;
            char strPeerPublicAddr[64];
            int iPeerPublicPort=-1;
            memset(strPeerID,0,sizeof(strPeerID));
            memset(strPeerPublicAddr,0,sizeof(strPeerPublicAddr));
            iRet=ParseSendMsgToPeerReq(i_strReq,strPeerID,sizeof(strPeerID),&iPeerNatType,strPeerPublicAddr,sizeof(strPeerPublicAddr),&iPeerPublicPort);
            if(iRet < 0)
            {
                P2P_LOGE("ClientSession::Proc ParseSendMsgToPeerReq err %d \r\n",iRet);
                return iRet;
            }
            iRet=m_pPeer2PeerHandle->SendMsgToPeer(iPeerNatType, (const char *)strPeerPublicAddr,iPeerPublicPort);
            iRet = CreateSendMsgToPeerRes(m_tClientSessionCfg.strLocalID,strPeerID,iRet,iPeerNatType,(const char *)strPeerPublicAddr,iPeerPublicPort,
            o_strRes,i_iResMaxLen);
            if(iRet <= 0)
            {
                P2P_LOGE("ClientSession::Proc CreateSendMsgToPeerRes err %d \r\n",iRet);
            }
            return iRet;
        }
        return iRet;
    }
    switch(m_eStatus)
    {
        case CLIENT_SESSION_LOGIN:
        {
            iRet = CreateLoginReq(m_tClientSessionCfg.strLocalID,o_strRes,i_iResMaxLen);
            if(iRet <= 0)
            {
                P2P_LOGE("ClientSession::Proc CreateLoginReq err %d \r\n",iRet);
                return iRet;
            }
            m_eStatus=CLIENT_SESSION_LOGIN_ACK;
            break;
        }
        case CLIENT_SESSION_LOGIN_ACK:
        {
            T_Peer2PeerCfg tPeer2PeerCfg;
            memset(&tPeer2PeerCfg,0,sizeof(T_Peer2PeerCfg));
            if(1 == iReqOrRes && 0==strcmp(strCmdBuf,"login"))
            {
                iRet=this->ParseLoginRes(i_strReq,&tPeer2PeerCfg);
                if(iRet < 0)
                {
                    P2P_LOGE("ClientSession::Proc ParseLoginRes err %d \r\n",iRet);
                    return iRet;
                }
                T_Peer2PeerCb tPeer2PeerCb;
                memset(&tPeer2PeerCb,0,sizeof(T_Peer2PeerCb));
                tPeer2PeerCb.Init = m_tClientSessionCb.Init;
                tPeer2PeerCb.SendData = m_tClientSessionCb.SendData;
                tPeer2PeerCb.RecvData = m_tClientSessionCb.RecvData;
                tPeer2PeerCb.Close= m_tClientSessionCb.Close;
                tPeer2PeerCb.pSessionHandle = this;
                tPeer2PeerCb.ReportLocalNatInfo = ClientSession::ReportResultCb;
                iRet=m_pPeer2PeerHandle->Proc(&tPeer2PeerCb,,(const char *)tPeer2PeerCfg.strStunServer1Addr,
                tPeer2PeerCfg.iStunServer1Port,(const char *)tPeer2PeerCfg.strStunServer1Addr,tPeer2PeerCfg.iStunServer2Port);
                if(iRet < 0)
                {
                    P2P_LOGE("ClientSession::Proc m_pPeer2PeerHandle err %d \r\n",iRet);
                    return iRet;
                }
                m_eStatus=CLIENT_SESSION_REPORT_NAT_INFO;
            }
            break;
        }
        case CLIENT_SESSION_REPORT_NAT_INFO:
        {
            iRet = this->CreateReportNatInfoReq(m_tClientSessionCfg.strLocalID,m_tLocalNatInfo.iNatType, (const char *)m_tLocalNatInfo.strIP,
            m_tLocalNatInfo.iPort,o_strRes,i_iResMaxLen);
            if(iRet <= 0)
            {
                P2P_LOGE("ClientSession::Proc CreateReportNatInfoReq err %d \r\n",iRet);
                return iRet;
            }
            m_eStatus=CLIENT_SESSION_REPORT_NAT_INFO_ACK;
            break;
        }
        case CLIENT_SESSION_REPORT_NAT_INFO_ACK:
        {
            if(1 == iReqOrRes && 0==strcmp(strCmdBuf,"ReportNatInfo"))
            {
                iRet=this->ParseReportNatInfoRes(i_strReq,strResResultDesc,sizeof(strResResultDesc));
                if(iRet < 0)
                {
                    P2P_LOGE("ClientSession::Proc ParseReportNatInfoRes err %d \r\n",iRet);
                    return iRet;
                }
                if(strlen(m_tClientSessionCfg.strPeerID)>0)
                {
                    m_eStatus=CLIENT_SESSION_PEER_2_PEER_HANDLE_PREPARE;
                }
            }
            break;
        }
        case CLIENT_SESSION_PEER_2_PEER_HANDLE_PREPARE:
        {
            iRet = this->CreatePeerNatInfoReq(m_tClientSessionCfg.strLocalID,m_tClientSessionCfg.strPeerID,o_strRes,i_iResMaxLen);
            if(iRet <= 0)
            {
                P2P_LOGE("ClientSession::Proc CreatePeerNatInfoReq err %d \r\n",iRet);
                return iRet;
            }
            m_eStatus=CLIENT_SESSION_PEER_2_PEER_HANDLE_START;
            break;
        }
        case CLIENT_SESSION_PEER_2_PEER_HANDLE_START:
        {
            if(1 == iReqOrRes && 0==strcmp(strCmdBuf,"PeerNatInfo"))
            {
                T_NatInfo tPeerNatInfo;
                memset(&tNatInfo,0,sizeof(T_NatInfo));
                iRet=this->ParsePeerNatInfoRes(i_strReq,m_tClientSessionCfg.strPeerID,&tPeerNatInfo.iNatType,tPeerNatInfo.strIP,sizeof(tPeerNatInfo.strIP),&tPeerNatInfo.iPort)
                if(iRet < 0)
                {
                    P2P_LOGE("ClientSession::Proc ParsePeerNatInfoRes err %d \r\n",iRet);
                    return iRet;
                }
                iRet=m_pPeer2PeerHandle->Peer2PeerHoleHandle(m_tLocalNatInfo.iNatType,tPeerNatInfo.iNatType,tPeerNatInfo.strIP, tPeerNatInfo.iPort);
                if(iRet < 0)
                {
                    P2P_LOGE("ClientSession::Proc Peer2PeerHoleHandle err %d \r\n",iRet);
                    return iRet;
                }
                if(0 == iRet)
                {
                    iRet = this->CreatePeerSendMsgReq(m_tClientSessionCfg.strLocalID,m_tClientSessionCfg.strPeerID, m_tLocalNatInfo.iNatType, (const char *)m_tLocalNatInfo.strIP,
                                                    m_tLocalNatInfo.iPort,o_strRes,i_iResMaxLen);
                    if(iRet <= 0)
                    {
                        P2P_LOGE("ClientSession::Proc CreatePeerSendMsgReq err %d \r\n",iRet);
                        return iRet;
                    }
                    m_eStatus=CLIENT_SESSION_PEER_2_PEER_HANDLE_PROC;
                }
            }
            break;
        }
        case CLIENT_SESSION_PEER_2_PEER_HANDLE_PROC:
        {
            if(1 == iReqOrRes && 0==strcmp(strCmdBuf,"PeerSendMsg"))
            {
                iRet=this->ParsePeerSendMsgRes(i_strReq,m_tClientSessionCfg.strPeerID,strResResultDesc,sizeof(strResResultDesc));
                if(iRet < 0)
                {
                    P2P_LOGE("ClientSession::Proc ParsePeerSendMsgRes err %d \r\n",iRet);
                    return iRet;
                }
                m_pPeer2PeerHandle->SetPeerSendedMsgToLocalFlag(1);
                iRet=m_pPeer2PeerHandle->Peer2PeerHoleHandle(m_tLocalNatInfo.iNatType,tPeerNatInfo.iNatType,tPeerNatInfo.strIP, tPeerNatInfo.iPort);
                if(iRet <= 0)
                {
                    P2P_LOGE("ClientSession::Proc Peer2PeerHoleHandle err %d \r\n",iRet);
                    return iRet;
                }
                m_iPeer2PeerHandleSuccessFlag=1;
                m_eStatus=CLIENT_SESSION_PEER_2_PEER_HANDLE_REPORT;
            }
            break;
        }
        case CLIENT_SESSION_PEER_2_PEER_HANDLE_REPORT:
        {
            iRet = this->CreateReportResultReq(m_tClientSessionCfg.strLocalID,m_tClientSessionCfg.strPeerID,m_iPeer2PeerHandleSuccessFlag,o_strRes,i_iResMaxLen);
            if(iRet <= 0)
            {
                P2P_LOGE("ClientSession::Proc CreateReportResultReq err %d \r\n",iRet);
                return iRet;
            }
            m_eStatus=CLIENT_SESSION_PEER_2_PEER_HANDLE_REPORT_ACK;
            break;
        }
        case CLIENT_SESSION_PEER_2_PEER_HANDLE_REPORT_ACK:
        {
            if(1 == iReqOrRes && 0==strcmp(strCmdBuf,"ReportResult"))
            {
                int iSuccessCnt=0;
                int iFailCnt=0;
                iRet=this->ParseReportResultRes(i_strReq,m_tClientSessionCfg.strLocalID,m_tClientSessionCfg.strPeerID,&iSuccessCnt,&iFailCnt);
                if(iRet < 0)
                {
                    P2P_LOGE("ClientSession::Proc ParseReportResultRes err %d \r\n",iRet);
                    return iRet;
                }
                P2P_LOGI("ReportResult iSuccessCnt %d ,iFailCnt %d \r\n",iSuccessCnt,iFailCnt);
            }
            break;
        }
        default:
        {
            P2P_LOGW("ClientSession::Proc eClientSessionStatus err %d \r\n",m_eStatus);
            break;
        }

    }
    if(iRet < 0)
    {
        P2P_LOGE("ClientSession::Proc err m_eStatus %d \r\n",m_eStatus);
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : ParseClientMsg
-Description    : 
-Input          : 
-Output         : o_iReqOrRes 0 req 1 res -1 err
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::ParseClientMsg(char * i_strMsg,int * o_iReqOrRes,char * o_pcCmdBufMaxLen,int i_iCmdBufMaxLen)
{
    int iRet = -1;
    int iReqOrRes = -1;
    
    if(NULL == i_strMsg||NULL == o_iReqOrRes||NULL == o_pcCmdBufMaxLen)
    {
        P2P_LOGE("ParseClientMsg err %d \r\n",i_iCmdBufMaxLen);
        return iRet;
    }
    cJSON * ptBodyJson = NULL;
    cJSON * ptNode = NULL;
    ptBodyJson = cJSON_Parse(i_strMsg);
    if(NULL != ptBodyJson)
    {
        ptNode = cJSON_GetObjectItem(ptBodyJson,"res");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            iRet = snprintf(o_pcCmdBufMaxLen,i_iCmdBufMaxLen,"%s",ptNode->valuestring);
            iReqOrRes = 1;
            ptNode = NULL;
        }
        ptNode = cJSON_GetObjectItem(ptBodyJson,"req");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            iRet = snprintf(o_pcCmdBufMaxLen,i_iCmdBufMaxLen,"%s",ptNode->valuestring);
            iReqOrRes = 0;
            ptNode = NULL;
        }
        cJSON_Delete(ptBodyJson);
    }
    *o_iReqOrRes=iReqOrRes;
    return iRet;
}

/*****************************************************************************
-Fuction        : ParseLoginReq
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::ParseLoginReq(char * i_strMsg,char * o_pcReqBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == i_strMsg||0 == i_iBufMaxLen ||NULL == o_pcReqBuf)
    {
        P2P_LOGE("ParseLoginReq err %d \r\n",i_iBufMaxLen);
        return iRet;
    }
    
    cJSON * ptRootJson = NULL;
    cJSON * ptBodyJson = NULL;
    cJSON * ptNode = NULL;
    ptRootJson = cJSON_Parse(i_strMsg);
    if(NULL != ptRootJson)
    {
        ptNode = cJSON_GetObjectItem(ptRootJson,"req");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            ptNode = NULL;
        }
        ptBodyJson = cJSON_GetObjectItem(ptRootJson,"data");
        if(NULL != ptBodyJson)
        {
            ptNode = cJSON_GetObjectItem(ptBodyJson,"LocalID");
            if(NULL != ptNode && NULL != ptNode->valuestring)
            {
                iRet=snprintf(o_pcReqBuf,i_iBufMaxLen,"%s",ptNode->valuestring);
                ptNode = NULL;
            }
        }
        cJSON_Delete(ptRootJson);
    }

    return iRet;
}

/*****************************************************************************
-Fuction        : ParseReportNatInfoRes
-Description    : 
-Input          : 
-Output         : o_iReqOrRes 0 req 1 res -1 err
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::ParseReportNatInfoReq(char * i_strMsg,char * o_strID,int i_iIDBufMaxLen,int * o_iNatType,char * o_strPublicIP,int i_iIPBufMaxLen,int * o_iPublicPort)
{
    int iRet = -1;
    
    if(NULL == i_strMsg||NULL == o_strID||NULL == o_iNatType||NULL == o_strPublicIP||NULL == o_iPublicPort)
    {
        P2P_LOGE("ParseReportNatInfoReq err %d \r\n",iRet);
        return iRet;
    }
    
    cJSON * ptRootJson = NULL;
    cJSON * ptBodyJson = NULL;
    cJSON * ptNatInfo = NULL;
    cJSON * ptNode = NULL;
    ptRootJson = cJSON_Parse(i_strMsg);
    if(NULL != ptRootJson)
    {
        ptNode = cJSON_GetObjectItem(ptRootJson,"req");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            ptNode = NULL;
        }
        ptBodyJson = cJSON_GetObjectItem(ptRootJson,"data");
        if(NULL != ptBodyJson)
        {
            ptNode = cJSON_GetObjectItem(ptBodyJson,"LocalID");
            if(NULL != ptNode&& NULL != ptNode->valuestring)
            {
                iRet = snprintf(o_strID,i_iIDBufMaxLen,"%s",ptNode->valuestring);
                ptNode = NULL;
            }
            ptNatInfo = cJSON_GetObjectItem(ptBodyJson,"LocalNatInfo");
            if(NULL != ptNatInfo)
            {
                ptNode = cJSON_GetObjectItem(ptNatInfo,"NatType");
                if(NULL != ptNode)
                {
                    *o_iNatType=ptNode->valueint;
                    ptNode = NULL;
                }
                ptNode = cJSON_GetObjectItem(ptNatInfo,"PublicIP");
                if(NULL != ptNode&& NULL != ptNode->valuestring)
                {
                    snprintf(o_strPublicIP,i_iIPBufMaxLen,"%s",ptNode->valuestring);
                    ptNode = NULL;
                }
                ptNode = cJSON_GetObjectItem(ptNatInfo,"PublicPort");
                if(NULL != ptNode)
                {
                    *o_iPublicPort=ptNode->valueint;
                    ptNode = NULL;
                }
                ptNatInfo = NULL;
            }
        }
        cJSON_Delete(ptRootJson);
    }

    return iRet;

}

/*****************************************************************************
-Fuction        : ParsePeerNatInfoReq
-Description    : 
-Input          : 
-Output         : o_iReqOrRes 0 req 1 res -1 err
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::ParsePeerNatInfoReq(char * i_strMsg,char * o_strID,int i_iIDBufMaxLen)
{
    int iRet = -1;
    
    if(NULL == i_strMsg||NULL == o_strID)
    {
        P2P_LOGE("ParsePeerNatInfoReq NULL err %d \r\n",iRet);
        return iRet;
    }
    
    cJSON * ptRootJson = NULL;
    cJSON * ptBodyJson = NULL;
    cJSON * ptNode = NULL;
    ptRootJson = cJSON_Parse(i_strMsg);
    if(NULL != ptRootJson)
    {
        ptNode = cJSON_GetObjectItem(ptRootJson,"req");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            ptNode = NULL;
        }
        ptBodyJson = cJSON_GetObjectItem(ptRootJson,"data");
        if(NULL != ptBodyJson)
        {
            ptNode = cJSON_GetObjectItem(ptBodyJson,"LocalID");
            if(NULL != ptNode&& NULL != ptNode->valuestring)
            {
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"PeerID");
            if(NULL != ptNode&& NULL != ptNode->valuestring)
            {
                iRet = snprintf(o_strID,i_iIDBufMaxLen,"%s",ptNode->valuestring);
                ptNode = NULL;
            }
        }
        cJSON_Delete(ptRootJson);
    }

    return iRet;

}

/*****************************************************************************
-Fuction        : ParsePeerSendMsgReq
-Description    : 
-Input          : 
-Output         : o_iReqOrRes 0 req 1 res -1 err
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::ParsePeerSendMsgReq(char * i_strMsg,char * o_strID,int i_iIDBufMaxLen)
{
    int iRet = -1;
    
    if(NULL == i_strMsg||NULL == o_strID)
    {
        P2P_LOGE("ParsePeerNatInfoReq NULL err %d \r\n",iRet);
        return iRet;
    }
    
    cJSON * ptRootJson = NULL;
    cJSON * ptBodyJson = NULL;
    cJSON * ptNode = NULL;
    ptRootJson = cJSON_Parse(i_strMsg);
    if(NULL != ptRootJson)
    {
        ptNode = cJSON_GetObjectItem(ptRootJson,"req");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            ptNode = NULL;
        }
        ptBodyJson = cJSON_GetObjectItem(ptRootJson,"data");
        if(NULL != ptBodyJson)
        {
            ptNode = cJSON_GetObjectItem(ptBodyJson,"LocalID");
            if(NULL != ptNode&& NULL != ptNode->valuestring)
            {
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"PeerID");
            if(NULL != ptNode&& NULL != ptNode->valuestring)
            {
                iRet = snprintf(o_strID,i_iIDBufMaxLen,"%s",ptNode->valuestring);
                ptNode = NULL;
            }
        }
        cJSON_Delete(ptRootJson);
    }

    return iRet;

}

/*****************************************************************************
-Fuction        : ParseSendMsgToPeerRes
-Description    : 
-Input          : 
-Output         : o ok -1 err
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::ParseSendMsgToPeerRes(char * i_strMsg,char * o_strLocalID,,int i_iLocalBufMaxLen,char * o_strPeerID,int i_iPeerBufMaxLen)
{
    int iRet = -1;
    char strDesrc[64];
    char strPublicIP[64];
    int iNatType = -1;
    int iPublicPort = -1;

    if(NULL == i_strMsg||NULL == o_strLocalID||NULL == o_strPeerID)
    {
        P2P_LOGE("ParseSendMsgToPeerRes NULL err %d \r\n",iRet);
        return iRet;
    }
    
    cJSON * ptRootJson = NULL;
    cJSON * ptBodyJson = NULL;
    cJSON * ptNatInfo = NULL;
    cJSON * ptNode = NULL;
    memset(strDesrc,0,sizeof(strDesrc));
    memset(strPublicIP,0,sizeof(strPublicIP));
    ptRootJson = cJSON_Parse(i_strMsg);
    if(NULL != ptRootJson)
    {
        ptNode = cJSON_GetObjectItem(ptRootJson,"req");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            ptNode = NULL;
        }
        ptBodyJson = cJSON_GetObjectItem(ptRootJson,"data");
        if(NULL != ptBodyJson)
        {
            ptNode = cJSON_GetObjectItem(ptBodyJson,"LocalID");
            if(NULL != ptNode&& NULL != ptNode->valuestring)
            {
                snprintf(o_strLocalID,o_strPeerID,"%s",ptNode->valuestring);
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"PeerID");
            if(NULL != ptNode&& NULL != ptNode->valuestring)
            {
                snprintf(o_strPeerID,i_iPeerBufMaxLen,"%s",ptNode->valuestring);
                ptNode = NULL;
            }
            ptNatInfo = cJSON_GetObjectItem(ptBodyJson,"PeerNatInfo");
            if(NULL != ptNatInfo)
            {
                ptNode = cJSON_GetObjectItem(ptNatInfo,"NatType");
                if(NULL != ptNode)
                {
                    iNatType=ptNode->valueint;
                    ptNode = NULL;
                }
                ptNode = cJSON_GetObjectItem(ptNatInfo,"PublicIP");
                if(NULL != ptNode&& NULL != ptNode->valuestring)
                {
                    snprintf(strPublicIP,sizeof(strPublicIP),"%s",ptNode->valuestring);
                    ptNode = NULL;
                }
                ptNode = cJSON_GetObjectItem(ptNatInfo,"PublicPort");
                if(NULL != ptNode)
                {
                    iPublicPort=ptNode->valueint;
                    ptNode = NULL;
                }
                ptNatInfo = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"ResultCode");
            if(NULL != ptNode)
            {
                iRet=ptNode->valueint;
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"ResultDesc");
            if(NULL != ptNode && NULL != ptNode->valuestring)
            {
                snprintf(strDesrc,sizeof(strDesrc),"%s",ptNode->valuestring);
                ptNode = NULL;
            }
        }
        cJSON_Delete(ptRootJson);
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : ParseReportResultReq
-Description    : 
-Input          : 
-Output         : o_iReqOrRes 0 req 1 res -1 err
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::ParseReportResultReq(char * i_strMsg,char * o_strLocalID,,int i_iLocalBufMaxLen,char * o_strPeerID,int i_iPeerBufMaxLen)
{
    int iRet = -1;
    char strDesrc[64];

    if(NULL == i_strMsg||NULL == o_strLocalID||NULL == o_strPeerID)
    {
        P2P_LOGE("ParseReportResultReq NULL err %d \r\n",iRet);
        return iRet;
    }
    
    cJSON * ptRootJson = NULL;
    cJSON * ptBodyJson = NULL;
    cJSON * ptNode = NULL;
    memset(strDesrc,0,sizeof(strDesrc));
    ptRootJson = cJSON_Parse(i_strMsg);
    if(NULL != ptRootJson)
    {
        ptNode = cJSON_GetObjectItem(ptRootJson,"req");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            ptNode = NULL;
        }
        ptBodyJson = cJSON_GetObjectItem(ptRootJson,"data");
        if(NULL != ptBodyJson)
        {
            ptNode = cJSON_GetObjectItem(ptBodyJson,"LocalID");
            if(NULL != ptNode&& NULL != ptNode->valuestring)
            {
                snprintf(o_strLocalID,o_strPeerID,"%s",ptNode->valuestring);
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"PeerID");
            if(NULL != ptNode&& NULL != ptNode->valuestring)
            {
                snprintf(o_strPeerID,i_iPeerBufMaxLen,"%s",ptNode->valuestring);
                ptNode = NULL;
            }
            
            ptNode = cJSON_GetObjectItem(ptBodyJson,"ResultCode");
            if(NULL != ptNode)
            {
                iRet=ptNode->valueint;
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"ResultDesc");
            if(NULL != ptNode && NULL != ptNode->valuestring)
            {
                snprintf(strDesrc,sizeof(strDesrc),"%s",ptNode->valuestring);
                ptNode = NULL;
            }
        }
        cJSON_Delete(ptRootJson);
    }
    return iRet;
}



/*****************************************************************************
-Fuction        : CreateLoginRes
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::CreateLoginRes(T_Peer2PeerCfg * i_ptPeer2PeerCfg,char * o_pcResBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == i_ptPeer2PeerCfg||0 == strlen(i_ptPeer2PeerCfg->strStunServer1Addr)||0 == strlen(i_ptPeer2PeerCfg->strStunServer2Addr)||0 == i_iBufMaxLen||NULL == o_pcResBuf)
    {
        P2P_LOGE("CreateLoginRes err %d \r\n",i_iBufMaxLen);
        return iRet;
    }
    
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"res","login");
    
    cJSON* pData = cJSON_CreateObject();
    cJSON_AddStringToObject(pData, "StunServer1Addr", i_ptPeer2PeerCfg->strStunServer1Addr);
    cJSON_AddNumberToObject(pData, "StunServer1Port", i_ptPeer2PeerCfg->iStunServer1Port);
    cJSON_AddStringToObject(pData, "StunServer2Addr", i_ptPeer2PeerCfg->strStunServer2Addr);
    cJSON_AddNumberToObject(pData, "StunServer2Port", i_ptPeer2PeerCfg->iStunServer2Port);
    cJSON_AddItemToObject(root, "data", pData)

    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        iRet = snprintf(o_pcResBuf,i_iBufMaxLen,"%s",buf);
        free(buf);
    }
    cJSON_Delete(root);

    return iRet;
}

/*****************************************************************************
-Fuction        : LoginServer
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::CreateReportNatInfoRes(int i_iResultCode,char * o_pcReqBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == o_pcReqBuf)
    {
        P2P_LOGE("CreateReportNatInfoRes err %d \r\n",i_iResultCode);
        return iRet;
    }
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"res","ReportNatInfo");
    
    cJSON* pData = cJSON_CreateObject();
    cJSON_AddNumberToObject(pData, "ResultCode", i_iResultCode);
    cJSON_AddStringToObject(pData, "ResultDesc", i_iResultCode==0?"SUCCESS":"FAIL");
    cJSON_AddItemToObject(root, "data", pData);

    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        iRet = snprintf(o_pcReqBuf,i_iBufMaxLen,"%s",buf);
        free(buf);
    }
    cJSON_Delete(root);

    return iRet;
}

/*****************************************************************************
-Fuction        : CreatePeerNatInfoRes
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::CreatePeerNatInfoRes(const char * i_strID,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort,char * o_pcReqBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == i_strID||0 == strlen(i_strID)||NULL == i_strPublicIP||NULL == o_pcReqBuf)
    {
        P2P_LOGE("CreatePeerNatInfoRes NULL err %d \r\n",i_iNatType);
        return iRet;
    }
    
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"res","PeerNatInfo");
    
    cJSON* pData = cJSON_CreateObject();
    cJSON_AddStringToObject(pData, "PeerID", i_strID);
    
    cJSON* pNatInfo = cJSON_CreateObject();
    cJSON_AddNumberToObject(pNatInfo, "NatType", i_iNatType);
    cJSON_AddStringToObject(pNatInfo, "PublicIP", i_strPublicIP);
    cJSON_AddNumberToObject(pNatInfo, "PublicPort", i_iPublicPort);
    cJSON_AddItemToObject(pData, "PeerNatInfo", pNatInfo);

    cJSON_AddItemToObject(root, "data", pData);

    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        iRet = snprintf(o_pcReqBuf,i_iBufMaxLen,"%s",buf);
        free(buf);
    }
    cJSON_Delete(root);

    return iRet;
}

/*****************************************************************************
-Fuction        : CreatePeerNatInfoRes
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::CreatePeerSendMsgRes(int i_iResultCode,const char * i_strID,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort,char * o_pcReqBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == i_strID||0 == strlen(i_strID)||NULL == i_strPublicIP||NULL == o_pcReqBuf)
    {
        P2P_LOGE("CreatePeerNatInfoRes NULL err %d \r\n",i_iNatType);
        return iRet;
    }
    
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"res","PeerSendMsg");
    
    cJSON* pData = cJSON_CreateObject();
    cJSON_AddStringToObject(pData, "PeerID", i_strID);
    
    cJSON* pNatInfo = cJSON_CreateObject();
    cJSON_AddNumberToObject(pNatInfo, "NatType", i_iNatType);
    cJSON_AddStringToObject(pNatInfo, "PublicIP", i_strPublicIP);
    cJSON_AddNumberToObject(pNatInfo, "PublicPort", i_iPublicPort);
    cJSON_AddItemToObject(pData, "PeerNatInfo", pNatInfo);

    cJSON_AddNumberToObject(pData, "ResultCode", i_iResultCode);
    cJSON_AddStringToObject(pData, "ResultDesc", i_iResultCode==0?"SUCCESS":"FAIL");

    cJSON_AddItemToObject(root, "data", pData);

    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        iRet = snprintf(o_pcReqBuf,i_iBufMaxLen,"%s",buf);
        free(buf);
    }
    cJSON_Delete(root);

    return iRet;
}

/*****************************************************************************
-Fuction        : LoginServer
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::CreateSendMsgToPeerReq(const char * i_strPeerID,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort,char * o_pcReqBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == i_strPeerID||0 == strlen(i_strPeerID)||NULL == i_strPublicIP||NULL == o_pcReqBuf)
    {
        P2P_LOGE("CreateSendMsgToPeerReq err %d \r\n",i_iNatType);
        return iRet;
    }
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"req","SendMsgToPeer");
    
    cJSON* pData = cJSON_CreateObject();
    cJSON_AddStringToObject(pData, "PeerID", i_strPeerID);

    cJSON* pNatInfo = cJSON_CreateObject();
    cJSON_AddNumberToObject(pNatInfo, "NatType", i_iNatType);
    cJSON_AddStringToObject(pNatInfo, "PublicIP", i_strPublicIP);
    cJSON_AddNumberToObject(pNatInfo, "PublicPort", i_iPublicPort);
    cJSON_AddItemToObject(pData, "PeerNatInfo", pNatInfo);

    cJSON_AddItemToObject(root, "data", pData);

    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        iRet = snprintf(o_pcReqBuf,i_iBufMaxLen,"%s",buf);
        free(buf);
    }
    cJSON_Delete(root);

    return iRet;
}

/*****************************************************************************
-Fuction        : LoginServer
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ServerSession::CreateReportResultRes(const char * i_strLocalID,const char * i_strPeerID,int i_iSuccessCnt,int i_iFailCnt,char * o_pcResBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == i_strLocalID||0 == strlen(i_strLocalID)||NULL == i_strPeerID||0 == strlen(i_strPeerID)||NULL == o_pcResBuf)
    {
        P2P_LOGE("CreateReportResultRes err %d \r\n",i_iBufMaxLen);
        return iRet;
    }
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"res","ReportResult");
    
    cJSON* pData = cJSON_CreateObject();
    cJSON_AddStringToObject(pData, "LocalID", i_strLocalID);
    cJSON_AddStringToObject(pData, "PeerID", i_strPeerID);
    cJSON_AddNumberToObject(pData, "SuccessCnt", i_iSuccessCnt);
    cJSON_AddNumberToObject(pData, "FailCnt", i_iFailCnt);

    cJSON_AddItemToObject(root, "data", pData);

    char * buf = cJSON_PrintUnformatted(root);
    if(buf)
    {
        iRet = snprintf(o_pcResBuf,i_iBufMaxLen,"%s",buf);
        free(buf);
    }
    cJSON_Delete(root);

    return iRet;
}


