/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       main.cpp
* Description           : 	    
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>

#include "ClientIO.h"

static void PrintUsage(char *i_strProcName);

const char * g_strServerAddr="gwm-000-cn-0448.bcloud365.net";//"127.0.0.1";//"gwm-000-cn-0448.bcloud365.net";
unsigned short g_wServerPort=9128;//9128;//9160;
const char * g_strLocalID="P-12345678901234567890";

/*****************************************************************************
-Fuction        : Peer2PeerClientProc
-Description    : strLocalID strPeerID strServerAddr ServerPort strLocalIP
设备端 (被动端/受控端)strLocalID NULL strServerAddr ServerPort NULL
    设备端LocalID用序列号，具有唯一性，便于查找
客户端 (主动端/控制端)strLocalID strPeerID strServerAddr ServerPort strLocalIP
    客户端LocalID便于统计成功率，可以固定值
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int p2pClientProc(const char * i_strLocalID,const char * i_strPeerID,const char * i_strServerAddr,unsigned short i_wServerPort,const char * i_strLocalIP)
{
    int iRet = -1;
    ClientIO *pClientIO = new ClientIO(i_strServerAddr,i_wServerPort);
    iRet = pClientIO->Proc(i_strLocalID,i_strPeerID,i_strLocalIP);//阻塞
    delete pClientIO;
    return iRet;
}
/*****************************************************************************
-Fuction        : main
-Description    : main
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int main(int argc, char* argv[]) 
{
    int iRet = -1;
    const char * strLocalID=g_strLocalID;
    const char * strServerAddr=g_strServerAddr;
    unsigned short wServerPort=g_wServerPort;//9160;
    const char * strPeerID=NULL;
    const char * strLocalIP=NULL;
    
    if(argc >1 && argc != 2 && argc != 4 && argc != 5 && argc != 6)
    {
        PrintUsage(argv[0]);
        return iRet;
    }
    if(argc==1)
    {
        PrintUsage(argv[0]);
    }
    if(argc == 2||argc == 4||argc == 5||argc == 6)
    {
        strLocalID=(const char *)argv[1];
    }
    if(argc == 4||argc == 5||argc == 6)
    {
        strServerAddr=(const char *)argv[2];
        wServerPort=atoi(argv[3]);
    }
    if(argc == 5||argc == 6)
    {
        strLocalIP=(const char *)argv[4];
    }
    if(argc == 6)
    {
        strPeerID=(const char *)argv[5];
    }
    iRet=p2pClientProc(strLocalID, strPeerID, strServerAddr,wServerPort, strLocalIP);
    return iRet;
}
/*****************************************************************************
-Fuction        : PrintUsage
-Description    : 
p2pClient strLocalID strPeerID strServerAddr ServerPort strLocalIP
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
static void PrintUsage(char *i_strProcName)
{
    printf("Usage: %s strLocalID strServerAddr ServerPort strLocalIP strPeerID\r\n",i_strProcName);
    printf("run default args: %s %s %s %d 127.0.0.1 NULL\r\n",i_strProcName,g_strLocalID,g_strServerAddr,g_wServerPort);
}

