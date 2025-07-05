/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       Peer2PeerManager.c
* Description           : 	
* Created               :       2023.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "Peer2PeerManager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <unistd.h>
#include <thread>





using std::thread;


/*****************************************************************************
-Fuction		: Peer2PeerManager
-Description	: Peer2PeerManager
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Peer2PeerManager :: Peer2PeerManager()
{
    m_pMgrQueue = NULL;

}

/*****************************************************************************
-Fuction		: ~NatInfoHandle
-Description	: ~NatInfoHandle
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Peer2PeerManager :: ~Peer2PeerManager()
{
}
/*****************************************************************************
-Fuction        : proc
-Description    : proc
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int Peer2PeerManager :: Proc(ThreadSafeQueue<QueueMessage> * i_pMgrQueue)
{
    int iRet = -1;

    if(NULL == i_pMgrQueue)
    {
        P2P_LOGE("Peer2PeerManager :: Proc NULL \r\n");
        return iRet;
    }
    m_pMgrQueue=i_pMgrQueue;
    QueueMessage oMsg;
    if (0!=m_pMgrQueue->WaitAndPop(oMsg, 10)) // 10 ms超时  
    { 
        return iRet;
    }
    // 处理消息
    iRet = -1;
    switch(oMsg.iMsgID)
    {
        case REPORT_NAT_INFO_MSG_ID:
        {
            if(sizeof(T_NatInfoMsg)!= oMsg.iDataSize||NULL == oMsg.pSender)
            {
                P2P_LOGE("Peer2PeerManager :: Proc REPORT_NAT_INFO_MSG_ID err %d %d \r\n",sizeof(T_NatInfoMsg),oMsg.iDataSize);
                return iRet;
            }
            string strID;
            NatInfo oNatInfo;
            T_NatInfoMsg * ptNatInfoMsg;
            ptNatInfoMsg=(T_NatInfoMsg *)oMsg.pbData;
            oNatInfo.pSession=oMsg.pSender;
            oNatInfo.iNatType=ptNatInfoMsg->iNatType;
            oNatInfo.iPublicPort=ptNatInfoMsg->iPublicPort;
            oNatInfo.strPublicIP.assign(ptNatInfoMsg->strPublicIP);
            strID.assign(ptNatInfoMsg->strID);

            auto search = m_NatInfoMap.find(strID);
            if (search == m_NatInfoMap.end())
            {
                m_NatInfoMap.insert(make_pair(strID,oNatInfo));
            }
            else
            {
                search->second.pSession=oNatInfo.pSession;
                search->second.iNatType=oNatInfo.iNatType;
                search->second.iPublicPort=oNatInfo.iPublicPort;
                search->second.strPublicIP.assign(ptNatInfoMsg->strPublicIP);
            }
            iRet=0;
            break;
        }
        case GET_NAT_INFO_MSG_ID:
        {
            char strPeerID[64];
            if(sizeof(strPeerID)!= oMsg.iDataSize)
            {
                P2P_LOGE("Peer2PeerManager :: Proc GET_NAT_INFO_MSG_ID err %d %d \r\n",sizeof(strPeerID),oMsg.iDataSize);
                return iRet;
            }
            memset(strPeerID,0,sizeof(strPeerID));
            memcpy(strPeerID,(char *)oMsg.pbData,sizeof(strPeerID));
            string strPeer;
            strPeer.assign(strPeerID);
            
            auto search = m_NatInfoMap.find(strPeer);
            if (search == m_NatInfoMap.end())
            {
                P2P_LOGE("Peer2PeerManager :: Proc m_NatInfoMap find err %s \r\n",strPeerID);
                return iRet;
            }
            T_NatInfoMsg tNatInfoMsg;
            memset(&tNatInfoMsg,0,sizeof(T_NatInfoMsg));
            tNatInfoMsg.iNatType=search->second.iNatType;
            tNatInfoMsg.iPublicPort=search->second.iPublicPort;
            snprintf(tNatInfoMsg.strID,sizeof(tNatInfoMsg.strID),"%s",strPeerID);
            snprintf(tNatInfoMsg.strPublicIP,sizeof(tNatInfoMsg.strPublicIP),"%s",search->second.strPublicIP.c_str());
            ThreadSafeQueue<QueueMessage> * pQueue=(ThreadSafeQueue<QueueMessage> *)oMsg.pSender;//search->second.pSession;
            QueueMessage oNatInfoMsg(GET_NAT_INFO_ACK_MSG_ID,(unsigned char *)&tNatInfoMsg,sizeof(T_NatInfoMsg));
            iRet=pQueue->Push(oNatInfoMsg);  
            if(iRet < 0)
            {
                P2P_LOGE("Peer2PeerManager::Proc GET_NAT_INFO_ACK_MSG_ID err %d \r\n",iRet);
            }
            break;
        }
        case REQ_PEER_SEND_MSG_MSG_ID:
        {
            if(sizeof(T_ReqPeerSendMsg)!= oMsg.iDataSize||NULL == oMsg.pSender)
            {
                P2P_LOGE("Peer2PeerManager :: Proc REQ_PEER_SEND_MSG_MSG_ID err %d %d \r\n",sizeof(T_NatInfoMsg),oMsg.iDataSize);
                return iRet;
            }
            string strPeerID;
            T_ReqPeerSendMsg * ptReqPeerSendMsg;
            ptReqPeerSendMsg=(T_ReqPeerSendMsg *)oMsg.pbData;
            strPeerID.assign(ptReqPeerSendMsg->strPeerID);
            auto search = m_NatInfoMap.find(strPeerID);
            if (search == m_NatInfoMap.end())
            {
                P2P_LOGE("Peer2PeerManager :: Proc m_NatInfoMap find strPeerID err %s \r\n",strPeerID.c_str());
                return iRet;
            }
            ThreadSafeQueue<QueueMessage> * pQueue=(ThreadSafeQueue<QueueMessage> *)search->second.pSession;
            QueueMessage oReqPeerSendMsg(REQ_SEND_MSG_TO_PEER_MSG_ID,(unsigned char *)&ptReqPeerSendMsg->tLocalNatInfo,sizeof(T_NatInfoMsg),oMsg.pSender);
            iRet=pQueue->Push(oReqPeerSendMsg);
            if(iRet < 0)
            {
                P2P_LOGE("Peer2PeerManager::Proc REQ_SEND_MSG_TO_PEER_MSG_ID err %d \r\n",iRet);
            }
            break;
        }
        case REQ_SEND_MSG_TO_PEER_ACK_MSG_ID:
        {
            if(sizeof(T_ReqSendMsgToPeerResultMsg)!= oMsg.iDataSize)
            {
                P2P_LOGE("Peer2PeerManager :: Proc REQ_SEND_MSG_TO_PEER_ACK_MSG_ID err %d %d \r\n",sizeof(T_ReqSendMsgToPeerResultMsg),oMsg.iDataSize);
                return iRet;
            }
            string strLocalID;
            string strPeerID;
            T_ReqSendMsgToPeerResultMsg * ptReqSendMsgToPeerResultMsg;
            ptReqSendMsgToPeerResultMsg=(T_ReqSendMsgToPeerResultMsg *)oMsg.pbData;
            strPeerID.assign(ptReqSendMsgToPeerResultMsg->strPeerID);
            strLocalID.assign(ptReqSendMsgToPeerResultMsg->strLocalID);
            
            auto search = m_NatInfoMap.find(strPeerID);
            if (search == m_NatInfoMap.end())
            {
                P2P_LOGE("Peer2PeerManager :: Proc m_NatInfoMap find strPeerID err %s \r\n",strPeerID.c_str());
                return iRet;
            }
            auto it = m_NatInfoMap.find(strLocalID);
            if (it == m_NatInfoMap.end())
            {
                P2P_LOGE("Peer2PeerManager :: Proc m_NatInfoMap find strLocalID err %s \r\n",strLocalID.c_str());
                return iRet;
            }
            T_ReqPeerSendAckMsg tReqPeerSendAckMsg;
            tReqPeerSendAckMsg.iResult=ptReqSendMsgToPeerResultMsg->iResult;
            tReqPeerSendAckMsg.tPeerNatInfo.iNatType=it->second.iNatType;
            tReqPeerSendAckMsg.tPeerNatInfo.iPublicPort=it->second.iPublicPort;
            snprintf(tReqPeerSendAckMsg.tPeerNatInfo.strID,sizeof(tReqPeerSendAckMsg.tPeerNatInfo.strID),"%s",ptReqSendMsgToPeerResultMsg->strLocalID);
            snprintf(tReqPeerSendAckMsg.tPeerNatInfo.strPublicIP,sizeof(tReqPeerSendAckMsg.tPeerNatInfo.strPublicIP),"%s",it->second.strPublicIP.c_str());
            
            ThreadSafeQueue<QueueMessage> * pQueue=(ThreadSafeQueue<QueueMessage> *)search->second.pSession;
            QueueMessage oReqPeerSendAckMsg(REQ_PEER_SEND_MSG_ACK_MSG_ID,(unsigned char *)&tReqPeerSendAckMsg,sizeof(T_ReqPeerSendAckMsg));
            iRet=pQueue->Push(oReqPeerSendAckMsg);
            if(iRet < 0)
            {
                P2P_LOGE("Peer2PeerManager::Proc REQ_PEER_SEND_MSG_ACK_MSG_ID err %d \r\n",iRet);
            }
            break;
        }
        case REPORT_P2P_RESULT_MSG_ID:
        {
            if(sizeof(T_ReqSendMsgToPeerResultMsg)!= oMsg.iDataSize||NULL == oMsg.pSender)
            {
                P2P_LOGE("Peer2PeerManager :: Proc REPORT_P2P_RESULT_MSG_ID err %d %d \r\n",sizeof(T_ReqSendMsgToPeerResultMsg),oMsg.iDataSize);
                return iRet;
            }
            int iSuccessCnt=0;
            int iFailCnt=0;
            int iCurStatus=-1;
            string strKey;
            Peer2PeerResult oPeer2PeerResult;
            string strLocalID;
            string strPeerID;
            T_ReqSendMsgToPeerResultMsg * ptReqSendMsgToPeerResultMsg;
            ptReqSendMsgToPeerResultMsg=(T_ReqSendMsgToPeerResultMsg *)oMsg.pbData;
            strPeerID.assign(ptReqSendMsgToPeerResultMsg->strPeerID);
            strLocalID.assign(ptReqSendMsgToPeerResultMsg->strLocalID);
            strKey.assign(strLocalID.c_str());
            strKey.append(strPeerID.c_str());
            
            auto search = m_Peer2PeerResultMap.find(strKey);
            if (search == m_Peer2PeerResultMap.end())
            {
                Peer2PeerResult oPeer2PeerResult;
                oPeer2PeerResult.strPeerID.assign(ptReqSendMsgToPeerResultMsg->strPeerID);
                oPeer2PeerResult.strLocalID.assign(ptReqSendMsgToPeerResultMsg->strLocalID);
                if(ptReqSendMsgToPeerResultMsg->iResult<0)
                {
                    oPeer2PeerResult.iFailCnt++;
                    oPeer2PeerResult.iCurStatus=-1;
                    iFailCnt=oPeer2PeerResult.iFailCnt;
                }
                else
                {
                    oPeer2PeerResult.iSuccessCnt++;
                    oPeer2PeerResult.iCurStatus=0;
                    iSuccessCnt=oPeer2PeerResult.iSuccessCnt;
                }
                iCurStatus=oPeer2PeerResult.iCurStatus;
                m_Peer2PeerResultMap.insert(make_pair(strKey,oPeer2PeerResult));
            }
            else
            {
                if(ptReqSendMsgToPeerResultMsg->iResult<0)
                {
                    search->second.iFailCnt++;
                    search->second.iCurStatus=-1;
                    iFailCnt=search->second.iFailCnt;
                }
                else
                {
                    search->second.iSuccessCnt++;
                    search->second.iCurStatus=0;
                    iSuccessCnt=search->second.iSuccessCnt;
                }
                iCurStatus=search->second.iCurStatus;
            }
            T_PeerToPeerResultMsg tPeerToPeerResultMsg;
            memset(&tPeerToPeerResultMsg,0,sizeof(T_PeerToPeerResultMsg));
            tPeerToPeerResultMsg.iSuccessCnt=iSuccessCnt;
            tPeerToPeerResultMsg.iFailCnt=iFailCnt;
            tPeerToPeerResultMsg.iCurStatus=iCurStatus;
            snprintf(tPeerToPeerResultMsg.strLocalID,sizeof(tPeerToPeerResultMsg.strLocalID),"%s",strLocalID.c_str());
            snprintf(tPeerToPeerResultMsg.strPeerID,sizeof(tPeerToPeerResultMsg.strPeerID),"%s",strPeerID.c_str());
            ThreadSafeQueue<QueueMessage> * pQueue=(ThreadSafeQueue<QueueMessage> *)oMsg.pSender;
            QueueMessage oPeerToPeerResultMsg(REPORT_P2P_RESULT_ACK_MSG_ID,(unsigned char *)&tPeerToPeerResultMsg,sizeof(T_PeerToPeerResultMsg));
            iRet=pQueue->Push(oPeerToPeerResultMsg);
            if(iRet < 0)
            {
                P2P_LOGE("Peer2PeerManager::Proc REPORT_P2P_RESULT_ACK_MSG_ID err %d \r\n",iRet);
            }
            break;
        }
        default:
        {
            P2P_LOGE("Peer2PeerManager :: Proc oMsg.iMsgID err %d %d \r\n",oMsg.iMsgID,oMsg.iDataSize);
            break;
        }
    }
    
    return iRet;
}

