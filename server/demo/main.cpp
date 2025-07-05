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

#include "ServerDemo.h"

static void PrintUsage(char *i_strProcName);

unsigned short g_wServerPort=9128;//9128;//9160;
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
    
    int dwServerPort=g_wServerPort;
    
    if(argc !=2)
    {
        PrintUsage(argv[0]);
    }
    else
    {
        dwServerPort=atoi(argv[1]);
    }
    ServerDemo *pServerDemo = new ServerDemo(dwServerPort);
    iRet=pServerDemo->Proc();//×èÈû
    
    return iRet;
}
/*****************************************************************************
-Fuction        : PrintUsage
-Description    : http://localhost:9208/file/H264AAC.flv/test.mp4
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
static void PrintUsage(char *i_strProcName)
{
    printf("Usage: %s ServerPort \r\n",i_strProcName);
    printf("run default args: %s %d \r\n",i_strProcName,g_wServerPort);
}

