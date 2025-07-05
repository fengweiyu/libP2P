/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ClientSession.cpp
* Description           : 	    P2P C-S协议实现
* Created               :       2022.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "ClientSession.h"
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

using std::string;




/*****************************************************************************
-Fuction        : ClientSession
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
ClientSession::ClientSession(T_ClientSessionCb * i_ptClientSessionCb,T_ClientSessionCfg * i_ptClientSessionCfg)
{
    memcpy(&m_tClientSessionCb,i_ptClientSessionCb,sizeof(T_ClientSessionCb));
    memcpy(&m_tClientSessionCfg,i_ptClientSessionCfg,sizeof(T_ClientSessionCfg));
    m_pPeer2PeerHandle = new Peer2PeerHandle();
    memset(&m_tLocalNatInfo,0,sizeof(T_NatInfo));
    memset(&m_tPeerNatInfo,0,sizeof(T_NatInfo));
    m_iPeer2PeerHandleSuccessFlag=0;
    m_eStatus=CLIENT_SESSION_LOGIN;
    m_iP2PConnectResultFlag=0;
}
/*****************************************************************************
-Fuction        : ~ClientSession
-Description    : ~ClientSession
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
ClientSession::~ClientSession()
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
-Return         : <0 err ,=0 OK ,>0 need send len
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ClientSession::Proc(char * i_strReq,int i_iReqLen,char *o_strRes,int i_iResMaxLen)
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
            T_NatInfo tPeerNatInfo;
            memset(strPeerID,0,sizeof(strPeerID));
            memset(&tPeerNatInfo,0,sizeof(T_NatInfo));

            iRet=ParseSendMsgToPeerReq(i_strReq,strPeerID,sizeof(strPeerID),&tPeerNatInfo.iNatType,tPeerNatInfo.strIP,sizeof(tPeerNatInfo.strIP),&tPeerNatInfo.iPort);
            if(iRet < 0)
            {
                P2P_LOGE("ClientSession::Proc ParseSendMsgToPeerReq err %d \r\n",iRet);
                return iRet;
            }
            if(CLIENT_SESSION_IDLE!=m_eStatus)//保险起见要判断一下不是主控端 m_tClientSessionCfg.strPeerID
            {
                P2P_LOGE("ClientSession::Proc SendMsgToPeer err %d \r\n",m_eStatus);
                iRet=-1;
            }
            else
            {
                iRet=m_pPeer2PeerHandle->SendMsgToPeer(tPeerNatInfo.iNatType, (const char *)tPeerNatInfo.strIP,tPeerNatInfo.iPort);
            }
            iRet = CreateSendMsgToPeerRes(m_tClientSessionCfg.strLocalID,strPeerID,iRet,tPeerNatInfo.iNatType,(const char *)tPeerNatInfo.strIP,tPeerNatInfo.iPort,
            o_strRes,i_iResMaxLen);
            if(iRet <= 0)
            {
                P2P_LOGE("ClientSession::Proc CreateSendMsgToPeerRes err %d \r\n",iRet);
                return iRet;
            }
            if(strlen(m_tPeerNatInfo.strIP)<=0)
                memcpy(&m_tPeerNatInfo,&tPeerNatInfo,sizeof(T_NatInfo));
            m_eStatus=CLIENT_SESSION_PEER_2_PEER_CONNECT_TEST;
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
                tPeer2PeerCb.pIoHandleObj= m_tClientSessionCb.pIoHandleObj;
                tPeer2PeerCb.ChangePeerAddr= m_tClientSessionCb.ChangePeerAddr;
                tPeer2PeerCb.pSessionHandle = this;
                tPeer2PeerCb.ReportLocalNatInfo = ClientSession::ReportResultCb;
                iRet=m_pPeer2PeerHandle->Proc(&tPeer2PeerCb,m_tClientSessionCfg.strLocalIP,(const char *)tPeer2PeerCfg.strStunServer1Addr,
                tPeer2PeerCfg.iStunServer1Port,(const char *)tPeer2PeerCfg.strStunServer2Addr,tPeer2PeerCfg.iStunServer2Port);
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
                else
                {
                    m_eStatus=CLIENT_SESSION_IDLE;
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
                memset(&tPeerNatInfo,0,sizeof(T_NatInfo));
                iRet=this->ParsePeerNatInfoRes(i_strReq,m_tClientSessionCfg.strPeerID,&tPeerNatInfo.iNatType,tPeerNatInfo.strIP,sizeof(tPeerNatInfo.strIP),&tPeerNatInfo.iPort);
                if(iRet < 0)
                {
                    P2P_LOGE("ClientSession::Proc ParsePeerNatInfoRes err %d \r\n",m_eStatus);
                    return iRet;
                }
                if(strlen(m_tPeerNatInfo.strIP)<=0)
                    memcpy(&m_tPeerNatInfo,&tPeerNatInfo,sizeof(T_NatInfo));
                iRet=m_pPeer2PeerHandle->Peer2PeerHoleHandle(m_tLocalNatInfo.iNatType,tPeerNatInfo.iNatType,tPeerNatInfo.strIP, tPeerNatInfo.iPort);
                if(iRet < 0)
                {
                    P2P_LOGE("ClientSession::Proc Peer2PeerHoleHandle err %d \r\n",m_eStatus);
                    return iRet;
                }
                if(0 == iRet)
                {
                    iRet = this->CreatePeerSendMsgReq(m_tClientSessionCfg.strLocalID,m_tClientSessionCfg.strPeerID, m_tLocalNatInfo.iNatType, (const char *)m_tLocalNatInfo.strIP,
                                                    m_tLocalNatInfo.iPort,o_strRes,i_iResMaxLen);
                    if(iRet <= 0)
                    {
                        P2P_LOGE("ClientSession::Proc CreatePeerSendMsgReq err %d \r\n",m_eStatus);
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
                T_NatInfo tPeerNatInfo;
                memset(&tPeerNatInfo,0,sizeof(T_NatInfo));
                iRet=this->ParsePeerSendMsgRes(i_strReq,m_tClientSessionCfg.strPeerID,
                &tPeerNatInfo.iNatType,tPeerNatInfo.strIP,sizeof(tPeerNatInfo.strIP),&tPeerNatInfo.iPort,strResResultDesc,sizeof(strResResultDesc));
                if(iRet < 0)
                {
                    P2P_LOGE("ClientSession::Proc ParsePeerSendMsgRes err %d \r\n",m_eStatus);
                    return iRet;
                }
                iRet=m_pPeer2PeerHandle->SetPeerSendedMsgToLocalFlag(1);
                iRet=m_pPeer2PeerHandle->Peer2PeerHoleHandle(m_tLocalNatInfo.iNatType,tPeerNatInfo.iNatType,tPeerNatInfo.strIP, tPeerNatInfo.iPort);
                if(iRet <= 0)
                {
                    P2P_LOGE("ClientSession::Proc SetPeerSendedMsgToLocalFlag Peer2PeerHoleHandle err %d \r\n",m_eStatus);
                    m_iPeer2PeerHandleSuccessFlag=0;
                    m_eStatus=CLIENT_SESSION_PEER_2_PEER_HANDLE_REPORT;
                    return iRet;
                }
                m_iPeer2PeerHandleSuccessFlag=1;
                m_eStatus=CLIENT_SESSION_PEER_2_PEER_HANDLE_REPORT;
            }
            break;
        }
        case CLIENT_SESSION_PEER_2_PEER_HANDLE_REPORT:
        {
            iRet = this->CreateReportResultReq(m_tClientSessionCfg.strLocalID,m_tClientSessionCfg.strPeerID,m_iPeer2PeerHandleSuccessFlag==0?-1:0,o_strRes,i_iResMaxLen);
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
                int iCurStatus=-1;
                iRet=this->ParseReportResultRes(i_strReq,m_tClientSessionCfg.strLocalID,m_tClientSessionCfg.strPeerID,&iSuccessCnt,&iFailCnt,&iCurStatus);
                if(iRet < 0)
                {
                    P2P_LOGE("ClientSession::Proc ParseReportResultRes err %d \r\n",iRet);
                    return iRet;
                }
                P2P_LOGI("ReportResultAck SuccessCnt %d ,FailCnt %d ,CurStatus %s\r\n",iSuccessCnt,iFailCnt,0==iCurStatus?"SUCCESS":"FAIL");
                m_iP2PConnectResultFlag=m_iPeer2PeerHandleSuccessFlag==0?-1:1;
                m_eStatus=CLIENT_SESSION_IDLE;
            }
            break;
        }
        case CLIENT_SESSION_PEER_2_PEER_CONNECT_TEST:
        {
            iRet=m_pPeer2PeerHandle->RecvMsgFromPeer(m_tPeerNatInfo.iNatType,m_tPeerNatInfo.strIP, m_tPeerNatInfo.iPort);
            if(iRet < 0)
            {
                P2P_LOGE("ClientSession::Proc RecvMsgFromPeer err %d \r\n",m_eStatus);
                m_iP2PConnectResultFlag=-1;
                m_eStatus=CLIENT_SESSION_IDLE;
                return iRet;
            }
            if(iRet > 0)
            {
                P2P_LOGD("ClientSession::Proc RecvMsgFromPeer OK %d \r\n",m_eStatus);
                m_iP2PConnectResultFlag=1;
                m_eStatus=CLIENT_SESSION_IDLE;
                return 0;
            }
            iRet=0;
            break;
        }
        case CLIENT_SESSION_IDLE://可以做受控端，给对端发收msg
        {
            iRet=0;
            break;
        }
        default:
        {
            P2P_LOGW("ClientSession::Proc eClientSessionStatus err %d \r\n",m_eStatus);
            break;
        }

    }
    if(iRet < 0 /*&& i_iReqLen>0*/)
    {
        P2P_LOGE("ClientSession::Proc err m_eStatus %d \r\n",m_eStatus);
    }
    return iRet;
}

/*****************************************************************************
-Fuction		: GetP2PConnectResult
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2020/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int ClientSession::GetP2PConnectResult()
{
    return m_iP2PConnectResultFlag;
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
int ClientSession::SaveLocalNatInfo(int i_iNatType,const char * i_strPublicIP,int i_iPublicPort)
{
    int iRet = -1;
    
    if(NULL == i_strPublicIP)
    {
        P2P_LOGE("SaveLocalNatInfo err i_iNatType %d ,i_iPublicPort %d \r\n",i_iNatType,i_iPublicPort);
        return iRet;
    }
    P2P_LOGI("SaveLocalNatInfo NatType %d ,PublicIP %s,PublicPort %d \r\n",i_iNatType,i_strPublicIP,i_iPublicPort);
    snprintf(m_tLocalNatInfo.strIP,sizeof(m_tLocalNatInfo.strIP),"%s",i_strPublicIP);
    m_tLocalNatInfo.iNatType=i_iNatType;
    m_tLocalNatInfo.iPort=i_iPublicPort;
    return 0 ;
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
int ClientSession::ReportResultCb(void *i_pReportObj,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort)
{
    int iRet = -1;
    
    if(NULL == i_strPublicIP)
    {
        P2P_LOGE("ReportResultCb err i_iNatType %d ,i_iPublicPort %d \r\n",i_iNatType,i_iPublicPort);
        return iRet;
    }
    P2P_LOGI("ReportResultCb NatType %d ,PublicIP %s,PublicPort %d \r\n",i_iNatType,i_strPublicIP,i_iPublicPort);
    ClientSession *pClientSession=(ClientSession *)i_pReportObj;
    
    return pClientSession->SaveLocalNatInfo(i_iNatType,i_strPublicIP,i_iPublicPort);
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
int ClientSession::CreateLoginReq(const char * i_strLocalID,char * o_pcReqBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == i_strLocalID||0 == strlen(i_strLocalID)||NULL == o_pcReqBuf)
    {
        P2P_LOGE("CreateLoginReq err %d \r\n",i_iBufMaxLen);
        return iRet;
    }
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"req","login");
    
    cJSON* pData = cJSON_CreateObject();
    cJSON_AddStringToObject(pData, "LocalID", i_strLocalID);
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
-Fuction        : CreateReportNatInfoReq
-Description    : 
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ClientSession::CreateReportNatInfoReq(const char * i_strLocalID,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort,char * o_pcReqBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == i_strLocalID||0 == strlen(i_strLocalID)||NULL == i_strPublicIP||NULL == o_pcReqBuf)
    {
        P2P_LOGE("CreateReportNatInfoReq err %d \r\n",i_iNatType);
        return iRet;
    }
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"req","ReportNatInfo");
    
    cJSON* pData = cJSON_CreateObject();
    cJSON_AddStringToObject(pData, "LocalID", i_strLocalID);
    
    cJSON* pNatInfo = cJSON_CreateObject();
    cJSON_AddNumberToObject(pNatInfo, "NatType", i_iNatType);
    cJSON_AddStringToObject(pNatInfo, "PublicIP", i_strPublicIP);
    cJSON_AddNumberToObject(pNatInfo, "PublicPort", i_iPublicPort);
    cJSON_AddItemToObject(pData, "LocalNatInfo", pNatInfo);

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
int ClientSession::CreatePeerNatInfoReq(const char * i_strLocalID,const char * i_strPeerID,char * o_pcReqBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == i_strLocalID||0 == strlen(i_strLocalID)||NULL == i_strPeerID||NULL == o_pcReqBuf)
    {
        P2P_LOGE("CreateReportNatInfoReq err %d \r\n",i_iBufMaxLen);
        return iRet;
    }
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"req","PeerNatInfo");
    
    cJSON* pData = cJSON_CreateObject();
    cJSON_AddStringToObject(pData, "LocalID", i_strLocalID);
    cJSON_AddStringToObject(pData, "PeerID", i_strPeerID);

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
int ClientSession::CreatePeerSendMsgReq(const char * i_strLocalID,const char * i_strPeerID,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort,char * o_pcReqBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == i_strLocalID||0 == strlen(i_strLocalID)||NULL == i_strPeerID||0 == strlen(i_strPeerID)||NULL == i_strPublicIP||NULL == o_pcReqBuf)
    {
        P2P_LOGE("CreatePeerSendMsgReq err %d \r\n",i_iNatType);
        return iRet;
    }
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"req","PeerSendMsg");
    
    cJSON* pData = cJSON_CreateObject();
    cJSON_AddStringToObject(pData, "LocalID", i_strLocalID);
    
    cJSON* pNatInfo = cJSON_CreateObject();
    cJSON_AddNumberToObject(pNatInfo, "NatType", i_iNatType);
    cJSON_AddStringToObject(pNatInfo, "PublicIP", i_strPublicIP);
    cJSON_AddNumberToObject(pNatInfo, "PublicPort", i_iPublicPort);
    cJSON_AddItemToObject(pData, "LocalNatInfo", pNatInfo);
    
    cJSON_AddStringToObject(pData, "PeerID", i_strPeerID);

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
int ClientSession::CreateSendMsgToPeerRes(const char * i_strLocalID,const char * i_strPeerID,int i_iResCode,int i_iNatType,const char * i_strPublicIP,int i_iPublicPort,char * o_pcResBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == i_strLocalID||0 == strlen(i_strLocalID)||NULL == i_strPeerID||0 == strlen(i_strPeerID)||NULL == i_strPublicIP||NULL == o_pcResBuf)
    {
        P2P_LOGE("CreateSendMsgToPeerRes err %d \r\n",i_iNatType);
        return iRet;
    }
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"res","SendMsgToPeer");
    
    cJSON* pData = cJSON_CreateObject();
    cJSON_AddStringToObject(pData, "LocalID", i_strLocalID);
    cJSON_AddStringToObject(pData, "PeerID", i_strPeerID);

    cJSON* pNatInfo = cJSON_CreateObject();
    cJSON_AddNumberToObject(pNatInfo, "NatType", i_iNatType);
    cJSON_AddStringToObject(pNatInfo, "PublicIP", i_strPublicIP);
    cJSON_AddNumberToObject(pNatInfo, "PublicPort", i_iPublicPort);
    cJSON_AddItemToObject(pData, "PeerNatInfo", pNatInfo);
    
    cJSON_AddNumberToObject(pData, "ResultCode", i_iResCode);
    cJSON_AddStringToObject(pData, "ResultDesc", i_iResCode==0?"SUCCESS":"FAIL");

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
int ClientSession::CreateReportResultReq(const char * i_strLocalID,const char * i_strPeerID,int i_iResultCode,char * o_pcReqBuf,int i_iBufMaxLen)
{
    int iRet = -1;

    
    if(NULL == i_strLocalID||0 == strlen(i_strLocalID)||NULL == i_strPeerID||0 == strlen(i_strPeerID)||NULL == o_pcReqBuf)
    {
        P2P_LOGE("CreateReportNatInfoReq err %d \r\n",i_iBufMaxLen);
        return iRet;
    }
    cJSON * root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"req","ReportResult");
    
    cJSON* pData = cJSON_CreateObject();
    cJSON_AddStringToObject(pData, "LocalID", i_strLocalID);
    cJSON_AddStringToObject(pData, "PeerID", i_strPeerID);
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
-Output         : o_iReqOrRes 0 req 1 res -1 err
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ClientSession::ParseServerMsg(char * i_strMsg,int * o_iReqOrRes,char * o_pcCmdBufMaxLen,int i_iCmdBufMaxLen)
{
    int iRet = -1;
    int iReqOrRes = -1;
    
    if(NULL == i_strMsg||NULL == o_iReqOrRes||NULL == o_pcCmdBufMaxLen)
    {
        P2P_LOGE("CreateLoginReq err %d \r\n",i_iCmdBufMaxLen);
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
-Fuction        : LoginServer
-Description    : 
-Input          : 
-Output         : o_iReqOrRes 0 req 1 res -1 err
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ClientSession::ParseLoginRes(char * i_strMsg,T_Peer2PeerCfg * o_ptPeer2PeerCfg)
{
    int iRet = -1;
    
    if(NULL == i_strMsg||NULL == o_ptPeer2PeerCfg)
    {
        P2P_LOGE("ParseLoginRes err %d \r\n",iRet);
        return iRet;
    }
    
    cJSON * ptRootJson = NULL;
    cJSON * ptBodyJson = NULL;
    cJSON * ptNode = NULL;
    ptRootJson = cJSON_Parse(i_strMsg);
    if(NULL != ptRootJson)
    {
        ptNode = cJSON_GetObjectItem(ptRootJson,"res");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            ptNode = NULL;
        }
        ptBodyJson = cJSON_GetObjectItem(ptRootJson,"data");
        if(NULL != ptBodyJson)
        {
            ptNode = cJSON_GetObjectItem(ptBodyJson,"StunServer1Addr");
            if(NULL != ptNode && NULL != ptNode->valuestring)
            {
                iRet=snprintf(o_ptPeer2PeerCfg->strStunServer1Addr,sizeof(o_ptPeer2PeerCfg->strStunServer1Addr),"%s",ptNode->valuestring);
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"StunServer1Port");
            if(NULL != ptNode)
            {
                o_ptPeer2PeerCfg->iStunServer1Port=ptNode->valueint;
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"StunServer2Addr");
            if(NULL != ptNode && NULL != ptNode->valuestring)
            {
                iRet=snprintf(o_ptPeer2PeerCfg->strStunServer2Addr,sizeof(o_ptPeer2PeerCfg->strStunServer2Addr),"%s",ptNode->valuestring);
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"StunServer2Port");
            if(NULL != ptNode)
            {
                o_ptPeer2PeerCfg->iStunServer2Port=ptNode->valueint;
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
int ClientSession::ParseReportNatInfoRes(char * i_strMsg,char * o_pcResBuf,int i_iBufMaxLen)
{
    int iRet = -1;
    
    if(NULL == i_strMsg||NULL == o_pcResBuf)
    {
        P2P_LOGE("ParseReportNatInfoRes err %d \r\n",iRet);
        return iRet;
    }
    
    cJSON * ptRootJson = NULL;
    cJSON * ptBodyJson = NULL;
    cJSON * ptNode = NULL;
    ptRootJson = cJSON_Parse(i_strMsg);
    if(NULL != ptRootJson)
    {
        ptNode = cJSON_GetObjectItem(ptRootJson,"res");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            ptNode = NULL;
        }
        ptBodyJson = cJSON_GetObjectItem(ptRootJson,"data");
        if(NULL != ptBodyJson)
        {
            ptNode = cJSON_GetObjectItem(ptBodyJson,"ResultCode");
            if(NULL != ptNode)
            {
                iRet=ptNode->valueint;
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"ResultDesc");
            if(NULL != ptNode && NULL != ptNode->valuestring)
            {
                snprintf(o_pcResBuf,i_iBufMaxLen,"%s",ptNode->valuestring);
                ptNode = NULL;
            }
        }
        cJSON_Delete(ptRootJson);
    }

    return iRet;
}

/*****************************************************************************
-Fuction        : ParsePeerNatInfoRes
-Description    : 
-Input          : 
-Output         : o_iReqOrRes 0 req 1 res -1 err
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ClientSession::ParsePeerNatInfoRes(char * i_strMsg,const char * i_strPeerID,int * o_iNatType,char * o_strPublicIP,int i_iIPBufMaxLen,int * o_iPublicPort)
{
    int iRet = -1;
    char strPeerID[64];
    
    if(NULL == i_strMsg||NULL == i_strPeerID||NULL == o_iNatType||NULL == o_strPublicIP ||NULL == o_iPublicPort)
    {
        P2P_LOGE("ParsePeerNatInfoRes err %d \r\n",iRet);
        return iRet;
    }
    
    cJSON * ptRootJson = NULL;
    cJSON * ptBodyJson = NULL;
    cJSON * ptNatInfo = NULL;
    cJSON * ptNode = NULL;
    memset(strPeerID,0,sizeof(strPeerID));
    ptRootJson = cJSON_Parse(i_strMsg);
    if(NULL != ptRootJson)
    {
        ptNode = cJSON_GetObjectItem(ptRootJson,"res");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            ptNode = NULL;
        }
        ptBodyJson = cJSON_GetObjectItem(ptRootJson,"data");
        if(NULL != ptBodyJson)
        {
            ptNode = cJSON_GetObjectItem(ptBodyJson,"PeerID");
            if(NULL != ptNode&& NULL != ptNode->valuestring)
            {
                snprintf(strPeerID,sizeof(strPeerID),"%s",ptNode->valuestring);
                ptNode = NULL;
            }
            ptNatInfo = cJSON_GetObjectItem(ptBodyJson,"PeerNatInfo");
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
    if(0!=strcmp(strPeerID,i_strPeerID))
    {
        return -1;
    }
    return 0;
}

/*****************************************************************************
-Fuction        : ParsePeerSendMsgRes
-Description    : 
-Input          : 
-Output         : o_iReqOrRes 0 req 1 res -1 err
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ClientSession::ParsePeerSendMsgRes(char * i_strMsg,const char * i_strPeerID,int * o_iNatType,char * o_strPublicIP,int i_iIPBufMaxLen,int * o_iPublicPort,char * o_pcResBuf,int i_iBufMaxLen)
{
    int iRet = -1;
    char strPeerID[64];
    char strPublicIP[64];
    int iNatType = -1;
    int iPublicPort = -1;
    int iResultCode = -1;
    char strResultDesc[64];
    
    if(NULL == i_strMsg||NULL == i_strPeerID||NULL == o_iNatType||NULL == o_strPublicIP ||NULL == o_iPublicPort||NULL == o_pcResBuf)
    {
        P2P_LOGE("ParsePeerSendMsgRes err %d \r\n",iRet);
        return iRet;
    }
    
    cJSON * ptRootJson = NULL;
    cJSON * ptBodyJson = NULL;
    cJSON * ptNatInfo = NULL;
    cJSON * ptNode = NULL;
    memset(strPeerID,0,sizeof(strPeerID));
    memset(strPublicIP,0,sizeof(strPublicIP));
    memset(strResultDesc,0,sizeof(strResultDesc));
    ptRootJson = cJSON_Parse(i_strMsg);
    if(NULL != ptRootJson)
    {
        ptNode = cJSON_GetObjectItem(ptRootJson,"res");
        if(NULL != ptNode && NULL != ptNode->valuestring)
        {
            ptNode = NULL;
        }
        ptBodyJson = cJSON_GetObjectItem(ptRootJson,"data");
        if(NULL != ptBodyJson)
        {
            ptNode = cJSON_GetObjectItem(ptBodyJson,"PeerID");
            if(NULL != ptNode&& NULL != ptNode->valuestring)
            {
                snprintf(strPeerID,sizeof(strPeerID),"%s",ptNode->valuestring);
                ptNode = NULL;
            }
            ptNatInfo = cJSON_GetObjectItem(ptBodyJson,"PeerNatInfo");
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
            ptNode = cJSON_GetObjectItem(ptBodyJson,"ResultCode");
            if(NULL != ptNode)
            {
                iResultCode=ptNode->valueint;
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"ResultDesc");
            if(NULL != ptNode && NULL != ptNode->valuestring)
            {
                snprintf(o_pcResBuf,i_iBufMaxLen,"%s",ptNode->valuestring);
                ptNode = NULL;
            }
        }
        cJSON_Delete(ptRootJson);
    }
    if(0!=strcmp(strPeerID,i_strPeerID))
    {
        P2P_LOGE("strcmp(strPeerID %s,i_strPeerID %s) err \r\n",strPeerID,i_strPeerID);
        return -1;
    }
    return iResultCode;
}

/*****************************************************************************
-Fuction        : ParseSendMsgToPeerReq
-Description    : 
-Input          : 
-Output         : o_iReqOrRes 0 req 1 res -1 err
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int ClientSession::ParseSendMsgToPeerReq(char * i_strMsg,char * o_strPeerID,int i_iIDBufMaxLen,int * o_iNatType,char * i_strPublicIP,int i_iIPBufMaxLen,int * o_iPublicPort)
{
    int iRet = -1;
    char strPeerID[64];
    char strPublicIP[64];
    int iNatType = -1;
    int iPublicPort = -1;

    if(NULL == i_strMsg||NULL == o_strPeerID||NULL == o_iNatType||NULL == i_strPublicIP||NULL == o_iPublicPort)
    {
        P2P_LOGE("ParseSendMsgToPeerReq err %d \r\n",iRet);
        return iRet;
    }
    
    cJSON * ptRootJson = NULL;
    cJSON * ptBodyJson = NULL;
    cJSON * ptNatInfo = NULL;
    cJSON * ptNode = NULL;
    memset(strPeerID,0,sizeof(strPeerID));
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
            ptNode = cJSON_GetObjectItem(ptBodyJson,"PeerID");
            if(NULL != ptNode&& NULL != ptNode->valuestring)
            {
                snprintf(strPeerID,sizeof(strPeerID),"%s",ptNode->valuestring);
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
        }
        cJSON_Delete(ptRootJson);
    }
    *o_iNatType=iNatType;
    *o_iPublicPort=iPublicPort;
    snprintf(i_strPublicIP,i_iIPBufMaxLen,"%s",strPublicIP);
    snprintf(o_strPeerID,i_iIDBufMaxLen,"%s",strPeerID);
    return 0;
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
int ClientSession::ParseReportResultRes(char * i_strMsg,char * i_strLocalID,char * i_strPeerID,int * o_iSuccessCnt,int * o_iFailCnt,int * o_iCurStatus)
{
    int iRet = -1;
    char strPeerID[64];
    char strLocalID[64];

    if(NULL == i_strMsg||NULL == o_iSuccessCnt||NULL == o_iFailCnt||NULL == o_iCurStatus)
    {
        P2P_LOGE("ParseReportResultRes err %d \r\n",iRet);
        return iRet;
    }
    
    cJSON * ptRootJson = NULL;
    cJSON * ptBodyJson = NULL;
    cJSON * ptNode = NULL;
    memset(strPeerID,0,sizeof(strPeerID));
    memset(strLocalID,0,sizeof(strLocalID));
    ptRootJson = cJSON_Parse(i_strMsg);
    if(NULL != ptRootJson)
    {
        ptNode = cJSON_GetObjectItem(ptRootJson,"res");
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
                snprintf(strLocalID,sizeof(strLocalID),"%s",ptNode->valuestring);
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"PeerID");
            if(NULL != ptNode && NULL != ptNode->valuestring)
            {
                snprintf(strPeerID,sizeof(strPeerID),"%s",ptNode->valuestring);
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"CurStatus");
            if(NULL != ptNode && NULL != ptNode->valuestring)
            {
                if(0 == strcmp("SUCCESS",ptNode->valuestring))
                {
                    *o_iCurStatus=0;
                }
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"SuccessCnt");
            if(NULL != ptNode)
            {
                *o_iSuccessCnt=ptNode->valueint;
                ptNode = NULL;
            }
            ptNode = cJSON_GetObjectItem(ptBodyJson,"FailCnt");
            if(NULL != ptNode)
            {
                *o_iFailCnt=ptNode->valueint;
                ptNode = NULL;
            }
        }
        cJSON_Delete(ptRootJson);
    }
    if(0!=strcmp(strLocalID,i_strLocalID))
    {
        P2P_LOGE("strcmp(strLocalID %s,i_strLocalID %s) err \r\n",strLocalID,i_strLocalID);
        return -1;
    }
    if(0!=strcmp(strPeerID,i_strPeerID))
    {
        P2P_LOGE("strcmp(strPeerID %s,i_strPeerID %s) err \r\n",strPeerID,i_strPeerID);
        return -1;
    }
    return 0;
}


