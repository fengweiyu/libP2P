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
ServerSession::ServerSession(ThreadSafeQueue<QueueMessage> * i_pMgrQueue,T_Peer2PeerCfg * i_ptPeer2PeerCfg)
{
    //m_oSessionQueue.clear();

    m_pMgrQueue=i_pMgrQueue;

    memset(&m_tLocalNatInfoMsg,0,sizeof(T_NatInfoMsg));
    memset(&m_strLocalID,0,sizeof(m_strLocalID));
    memcpy(&m_tPeer2PeerCfg,i_ptPeer2PeerCfg,sizeof(T_Peer2PeerCfg));
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
        P2P_LOGE("ServerSession::Proc NULL err %d \r\n",i_iResMaxLen);
        return iRet;
    }
    memset(strCmdBuf,0,sizeof(strCmdBuf));
    if(NULL != i_strReq&&i_iReqLen>0)
    {
        iRet=ParseClientMsg(i_strReq,&iReqOrRes,strCmdBuf,sizeof(strCmdBuf));
        if(iRet < 0)
        {
            P2P_LOGE("ServerSession::Proc ParseServerMsg err %d,%s\r\n",i_iReqLen,i_strReq);
            return iRet;
        }
        P2P_LOGD("ParseClientMsg LocalID %s , %s \r\n",m_strLocalID,i_strReq);
    }
    if(1 == iReqOrRes)//res
    {
        if(0==strcmp(strCmdBuf,"SendMsgToPeer"))
        {
            T_ReqSendMsgToPeerResultMsg tReqSendMsgToPeerResultMsg;
            memset(&tReqSendMsgToPeerResultMsg,0,sizeof(T_ReqSendMsgToPeerResultMsg));
            iRet=this->ParseSendMsgToPeerRes(i_strReq,tReqSendMsgToPeerResultMsg.strLocalID,sizeof(tReqSendMsgToPeerResultMsg.strLocalID),
            tReqSendMsgToPeerResultMsg.strPeerID,sizeof(tReqSendMsgToPeerResultMsg.strPeerID));
            if(iRet < 0||0 != strcmp(tReqSendMsgToPeerResultMsg.strLocalID,m_strLocalID))
            {
                P2P_LOGE("ServerSession::Proc ParseSendMsgToPeerRes err %d ,%s ,%s\r\n",iRet,tReqSendMsgToPeerResultMsg.strLocalID,m_strLocalID);
                iRet=-1;
            }
            tReqSendMsgToPeerResultMsg.iResult=iRet;
            QueueMessage oReqSendResultMsg(REQ_SEND_MSG_TO_PEER_ACK_MSG_ID,(unsigned char *)&tReqSendMsgToPeerResultMsg,sizeof(T_ReqSendMsgToPeerResultMsg),&m_oSessionQueue);
            iRet=m_pMgrQueue->Push(oReqSendResultMsg);  
            return iRet;
        }
        return iRet;
    }
    if(0 == iReqOrRes && 0==strcmp(strCmdBuf,"login"))
    {
        iRet=this->ParseLoginReq(i_strReq,m_strLocalID,sizeof(m_strLocalID));
        if(iRet < 0)
        {
            P2P_LOGE("ServerSession::Proc ParseLoginReq err %d \r\n",iRet);
            return iRet;
        }
        iRet = this->CreateLoginRes(&m_tPeer2PeerCfg, o_strRes,i_iResMaxLen);
        if(iRet <= 0)
        {
            P2P_LOGE("ServerSession::Proc CreateLoginRes err %d \r\n",iRet);
            return -1;
        }
        //QueueMessage oMsg(LOGIN_MSG_ID,(unsigned char *)m_oSessionQueue,4);
        //iRet=m_pMgrQueue->Push(oMsg);  
        return iRet;
    }
    if(0 == iReqOrRes && 0==strcmp(strCmdBuf,"ReportNatInfo"))
    {
        T_NatInfoMsg tNatInfoMsg;
        memset(&tNatInfoMsg,0,sizeof(T_NatInfoMsg));
        iRet=this->ParseReportNatInfoReq(i_strReq,tNatInfoMsg.strID,sizeof(tNatInfoMsg.strID),&tNatInfoMsg.iNatType,
        tNatInfoMsg.strPublicIP,sizeof(tNatInfoMsg.strPublicIP),&tNatInfoMsg.iPublicPort);
        if(0!=strcmp(tNatInfoMsg.strID,m_strLocalID)||iRet < 0)
        {
            P2P_LOGE("ServerSession::Proc ParseReportNatInfoReq err %d ,%s ,%s\r\n",iRet,tNatInfoMsg.strID,m_strLocalID);
            return iRet;
        }
        memcpy(&m_tLocalNatInfoMsg,&tNatInfoMsg,sizeof(T_NatInfoMsg));
        QueueMessage oReportMsg(REPORT_NAT_INFO_MSG_ID,(unsigned char *)&tNatInfoMsg,sizeof(T_NatInfoMsg),&m_oSessionQueue);
        iRet=m_pMgrQueue->Push(oReportMsg);  
        iRet = this->CreateReportNatInfoRes(iRet, o_strRes,i_iResMaxLen);
        if(iRet <= 0)
        {
            P2P_LOGE("ServerSession::Proc CreateReportNatInfoRes err %d \r\n",iRet);
            return -1;
        }
        return iRet;
    }
    if(0 == iReqOrRes && 0==strcmp(strCmdBuf,"PeerNatInfo"))
    {
        char strPeerID[64];
        memset(strPeerID,0,sizeof(strPeerID));
        iRet=this->ParsePeerNatInfoReq(i_strReq,strPeerID,sizeof(strPeerID));
        if(strlen(strPeerID)<=0 ||iRet < 0)
        {
            P2P_LOGE("ServerSession::Proc ParsePeerNatInfoReq err %d ,%s ,%s\r\n",iRet,strPeerID,m_strLocalID);
            return iRet;
        }
        QueueMessage oGetMsg(GET_NAT_INFO_MSG_ID,(unsigned char *)strPeerID,sizeof(strPeerID),&m_oSessionQueue);
        iRet=m_pMgrQueue->Push(oGetMsg);  
        if(iRet < 0)
        {
            P2P_LOGE("ServerSession::Proc GET_NAT_INFO_MSG_ID err %d \r\n",iRet);
        }
        return iRet;
    }
    if(0 == iReqOrRes && 0==strcmp(strCmdBuf,"PeerSendMsg"))
    {
        char strPeerID[64];
        memset(&strPeerID,0,sizeof(strPeerID));
        iRet=this->ParsePeerSendMsgReq(i_strReq,strPeerID,sizeof(strPeerID));
        if(iRet < 0)
        {
            P2P_LOGE("ServerSession::Proc ParsePeerSendMsgReq err %d ,%s ,%s\r\n",iRet,strPeerID,m_strLocalID);
            return iRet;
        }
        T_ReqPeerSendMsg tReqPeerSendMsg;
        memset(&tReqPeerSendMsg,0,sizeof(T_ReqPeerSendMsg));
        memcpy(tReqPeerSendMsg.strPeerID,strPeerID,sizeof(strPeerID));
        memcpy(&tReqPeerSendMsg.tLocalNatInfo,&m_tLocalNatInfoMsg,sizeof(T_NatInfoMsg));
        QueueMessage oReqSendMsg(REQ_PEER_SEND_MSG_MSG_ID,(unsigned char *)&tReqPeerSendMsg,sizeof(T_ReqPeerSendMsg),&m_oSessionQueue);
        iRet=m_pMgrQueue->Push(oReqSendMsg);  
        return iRet;
    }
    if(0 == iReqOrRes && 0==strcmp(strCmdBuf,"ReportResult"))
    {
        T_ReqSendMsgToPeerResultMsg tReqSendMsgToPeerResultMsg;
        memset(&tReqSendMsgToPeerResultMsg,0,sizeof(T_ReqSendMsgToPeerResultMsg));
        iRet=this->ParseReportResultReq(i_strReq,tReqSendMsgToPeerResultMsg.strLocalID,sizeof(tReqSendMsgToPeerResultMsg.strLocalID),
        tReqSendMsgToPeerResultMsg.strPeerID,sizeof(tReqSendMsgToPeerResultMsg.strPeerID));
        if(iRet < 0)
        {
            P2P_LOGE("ServerSession::Proc ParseReportResultReq err %d ,strLocalID %s ,strPeerID %s\r\n",iRet,tReqSendMsgToPeerResultMsg.strLocalID,tReqSendMsgToPeerResultMsg.strPeerID);
        }
        tReqSendMsgToPeerResultMsg.iResult=iRet;
        QueueMessage oPeer2PeerResultMsg(REPORT_P2P_RESULT_MSG_ID,(unsigned char *)&tReqSendMsgToPeerResultMsg,sizeof(T_ReqSendMsgToPeerResultMsg),&m_oSessionQueue);
        iRet=m_pMgrQueue->Push(oPeer2PeerResultMsg);  
        return iRet;
    }
    QueueMessage oMsg;
    if (0!=m_oSessionQueue.WaitAndPop(oMsg, 10)) // 10ms超时  
    { 
        return iRet;
    }
    // 处理消息
    switch(oMsg.iMsgID)
    {
        case GET_NAT_INFO_ACK_MSG_ID:
        {
            T_NatInfoMsg tNatInfoMsg;
            if(sizeof(T_NatInfoMsg)!= oMsg.iDataSize)
            {
                P2P_LOGE("ServerSession :: Proc GET_NAT_INFO_ACK_MSG_ID err %d %d \r\n",sizeof(T_NatInfoMsg),oMsg.iDataSize);
                return iRet;
            }
            memset(&tNatInfoMsg,0,sizeof(T_NatInfoMsg));
            memcpy(&tNatInfoMsg,(T_NatInfoMsg *)oMsg.pbData,sizeof(T_NatInfoMsg));
        
            iRet = this->CreatePeerNatInfoRes(tNatInfoMsg.strID,tNatInfoMsg.iNatType,tNatInfoMsg.strPublicIP, tNatInfoMsg.iPublicPort,o_strRes,i_iResMaxLen);
            if(iRet <= 0)
            {
                P2P_LOGE("ServerSession::Proc CreatePeerNatInfoRes err %d \r\n",iRet);
                return -1;
            }
            break;
        }
        case REQ_SEND_MSG_TO_PEER_MSG_ID:
        {
            T_NatInfoMsg tPeerNatInfoMsg;
            if(sizeof(T_NatInfoMsg)!= oMsg.iDataSize)
            {
                P2P_LOGE("ServerSession :: Proc REQ_SEND_MSG_TO_PEER_MSG_ID err %d %d \r\n",sizeof(T_NatInfoMsg),oMsg.iDataSize);
                return iRet;
            }
            memset(&tPeerNatInfoMsg,0,sizeof(T_NatInfoMsg));
            memcpy(&tPeerNatInfoMsg,(T_NatInfoMsg *)oMsg.pbData,sizeof(T_NatInfoMsg));
            iRet = this->CreateSendMsgToPeerReq(tPeerNatInfoMsg.strID,tPeerNatInfoMsg.iNatType,tPeerNatInfoMsg.strPublicIP, tPeerNatInfoMsg.iPublicPort,o_strRes,i_iResMaxLen);
            if(iRet <= 0)
            {
                P2P_LOGE("ServerSession::Proc CreateSendMsgToPeerReq err %d \r\n",iRet);
                return -1;
            }
            break;
        }
        case REQ_PEER_SEND_MSG_ACK_MSG_ID:
        {
            T_ReqPeerSendAckMsg tReqPeerSendAckMsg;
            if(sizeof(T_ReqPeerSendAckMsg)!= oMsg.iDataSize)
            {
                P2P_LOGE("ServerSession :: Proc REQ_PEER_SEND_MSG_ACK_MSG_ID err %d %d \r\n",sizeof(T_ReqPeerSendAckMsg),oMsg.iDataSize);
                return iRet;
            }
            memset(&tReqPeerSendAckMsg,0,sizeof(T_ReqPeerSendAckMsg));
            memcpy(&tReqPeerSendAckMsg,(T_NatInfoMsg *)oMsg.pbData,sizeof(T_ReqPeerSendAckMsg));
            iRet = this->CreatePeerSendMsgRes(tReqPeerSendAckMsg.iResult,tReqPeerSendAckMsg.tPeerNatInfo.strID,tReqPeerSendAckMsg.tPeerNatInfo.iNatType,
            tReqPeerSendAckMsg.tPeerNatInfo.strPublicIP,tReqPeerSendAckMsg.tPeerNatInfo.iPublicPort,o_strRes,i_iResMaxLen);
            if(iRet <= 0)
            {
                P2P_LOGE("ServerSession::Proc CreatePeerSendMsgRes err %d \r\n",iRet);
                return -1;
            }
            break;
        }
        case REPORT_P2P_RESULT_ACK_MSG_ID:
        {
            T_PeerToPeerResultMsg tPeerToPeerResultMsg;
            if(sizeof(T_PeerToPeerResultMsg)!= oMsg.iDataSize)
            {
                P2P_LOGE("ServerSession :: Proc REQ_PEER_SEND_MSG_ACK_MSG_ID err %d %d \r\n",sizeof(T_PeerToPeerResultMsg),oMsg.iDataSize);
                return iRet;
            }
            memset(&tPeerToPeerResultMsg,0,sizeof(T_PeerToPeerResultMsg));
            memcpy(&tPeerToPeerResultMsg,(T_PeerToPeerResultMsg *)oMsg.pbData,sizeof(T_PeerToPeerResultMsg));
            iRet = this->CreateReportResultRes(tPeerToPeerResultMsg.strLocalID,tPeerToPeerResultMsg.strPeerID,tPeerToPeerResultMsg.iSuccessCnt,
            tPeerToPeerResultMsg.iFailCnt,tPeerToPeerResultMsg.iCurStatus,o_strRes,i_iResMaxLen);
            if(iRet <= 0)
            {
                P2P_LOGE("ServerSession::Proc CreateReportResultRes err %d \r\n",iRet);
                return -1;
            }
            break;
        }
        default:
        {
            P2P_LOGW("ServerSession::Proc iMsgID err %d \r\n",oMsg.iMsgID);
            break;
        }

    }
    if(iRet < 0)
    {
        P2P_LOGE("ServerSession::Proc err %d \r\n",iRet);
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
        P2P_LOGE("ParsePeerSendMsgReq NULL err %d \r\n",iRet);
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
                ptNode = NULL;
            }
            ptNatInfo = cJSON_GetObjectItem(ptBodyJson,"LocalNatInfo");
            if(NULL != ptNatInfo)
            {
                ptNode = cJSON_GetObjectItem(ptNatInfo,"NatType");
                if(NULL != ptNode)
                {
                    ptNode = NULL;
                }
                ptNode = cJSON_GetObjectItem(ptNatInfo,"PublicIP");
                if(NULL != ptNode&& NULL != ptNode->valuestring)
                {
                    ptNode = NULL;
                }
                ptNode = cJSON_GetObjectItem(ptNatInfo,"PublicPort");
                if(NULL != ptNode)
                {
                    ptNode = NULL;
                }
                ptNatInfo = NULL;
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
int ServerSession::ParseSendMsgToPeerRes(char * i_strMsg,char * o_strLocalID,int i_iLocalBufMaxLen,char * o_strPeerID,int i_iPeerBufMaxLen)
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
                snprintf(o_strLocalID,i_iLocalBufMaxLen,"%s",ptNode->valuestring);
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
int ServerSession::ParseReportResultReq(char * i_strMsg,char * o_strLocalID,int i_iLocalBufMaxLen,char * o_strPeerID,int i_iPeerBufMaxLen)
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
                snprintf(o_strLocalID, i_iLocalBufMaxLen,"%s",ptNode->valuestring);
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
int ServerSession::CreateReportResultRes(const char * i_strLocalID,const char * i_strPeerID,int i_iSuccessCnt,int i_iFailCnt,int i_iCurStatus,char * o_pcResBuf,int i_iBufMaxLen)
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
    cJSON_AddStringToObject(pData, "CurStatus", 0==i_iCurStatus?"SUCCESS":"FAIL");

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


