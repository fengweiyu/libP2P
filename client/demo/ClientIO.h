/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       ClientIO.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef CLIENT_IO_H
#define CLIENT_IO_H

#include "TcpSocket.h"
#include "UdpSocket.h"
#include <thread>
#include <mutex>

using std::thread;
using std::mutex;

/*****************************************************************************
-Class			: ClientIO
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class ClientIO : public TcpClient
{
public:
	ClientIO(const char * i_strServerAddr,unsigned short i_wServerPort);
	virtual ~ClientIO();
    int Proc(const char * i_strLocalID,const char * i_strPeerID,const char * i_strLocalIP);
    int GetProcFlag();

    static void * IoInit(const char * strProtocolType,const char * i_strPeerIP,int i_iPeerPort);//回调使用
    static int IoSendData(void *i_pIoHandle,unsigned char * i_pbSendBuf,int i_iSendLen);
    static int IoRecvData(void *i_pIoHandle,unsigned char *o_pbRecvBuf,int i_iRecvBufMaxLen);
    static int IoClose(void *i_pIoHandle);
private:
	int m_iClientSocketFd;
	
    ClientSession * m_pClientSession;

    thread * m_pHttpServerIOProc;
	int m_iIOProcFlag;
};

#endif
