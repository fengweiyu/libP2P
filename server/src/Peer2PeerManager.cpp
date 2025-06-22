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
        TEST_LOGE("Test NULL \r\n");
        return iRet;
    }
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
    return 0;
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
int Peer2PeerManager :: AddNatInfo(const char * i_strSrcFilePath,const char *i_strDstFilePath)
{
    int iRet = -1;

    if(NULL == i_strSrcFilePath || NULL == i_strDstFilePath)
    {
        TEST_LOGE("Test NULL \r\n");
        return iRet;
    }
    if(Proc(i_strSrcFilePath,i_strDstFilePath)<=0)
    {
        return iRet;
    }
    return 0;
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
int Peer2PeerManager :: AddPeer2PeerResult(const char * i_strSrcFilePath,const char *i_strDstFilePath)
{
    unsigned char * pbSrcFileBuf=NULL;
    unsigned char * pbFileBuf=NULL;
	int iRet = -1,iReadLen = -1,iWriteLen=0;
	int iMaxLen=0;
    FILE *pDstFile=NULL;  


    iReadLen=ReadFile(i_strSrcFilePath,&pbSrcFileBuf);
    if(iReadLen <= 0)
    {
        TEST_LOGE("ReadFile err %s\r\n",i_strSrcFilePath);
        return iRet;
    } 
    do
    {
        iRet=InputData(pbSrcFileBuf,iReadLen,i_strSrcFilePath,i_strDstFilePath);
        if(iRet <= 0)
        {
            TEST_LOGE("InputData err %s %s\r\n",i_strSrcFilePath,i_strDstFilePath);
            //break;
        } 
        iMaxLen=iReadLen+(10*1024*1024) ;
        pbFileBuf = new unsigned char [iMaxLen];
        if(NULL == pbFileBuf)
        {
            TEST_LOGE("NULL == pbFileBuf err\r\n");
            break;
        } 
        iWriteLen=0;
        iRet=0;
        do
        {
            iWriteLen+=iRet;
            iRet=GetData(pbFileBuf+iWriteLen,iMaxLen-iWriteLen);
        } while(iRet>0);

        
        pDstFile = fopen(i_strDstFilePath,"wb");//
        if(NULL == pDstFile)
        {
            TEST_LOGE("fopen %s err\r\n",i_strDstFilePath);
            break;
        } 
        iRet = fwrite(pbFileBuf,1,iWriteLen, pDstFile);
        if(iRet != iWriteLen)
        {
            printf("fwrite err %d iWriteLen%d\r\n",iRet,iWriteLen);
            break;
        }
    }while(0);
    
    if(NULL != pbFileBuf)
    {
        delete [] pbFileBuf;
    } 
    if(NULL != pbSrcFileBuf)
    {
        delete [] pbSrcFileBuf;
    } 
    if(NULL != pDstFile)
    {
        fclose(pDstFile);//fseek(m_pMediaFile,0,SEEK_SET); 
    } 
    return iWriteLen;
}

