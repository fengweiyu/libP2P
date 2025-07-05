/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	UdpSocket.h
* Description		: 	UdpSocket operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H


#include <map>
#include <stdio.h>
#include <string>

using std::map;
using std::string;

#define UDP_MTU     1500
/*****************************************************************************
-Class			: UdpSocket
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class UdpSocket
{
public:
	UdpSocket();
	virtual ~UdpSocket();
    virtual int GetSocketAddrPort(int i_iSocketFd,string * o_strIP,int * o_iPort);
    virtual int ResolveDomain(const char * i_strDomain,string * o_strIP);
    static int GetLocalAddr(string * o_strIP);
    
    virtual int Init(const char *i_pstrDstIP,unsigned short i_wDstPort,const char *i_pstrIP,unsigned short i_wPort)=0;
    virtual int Send(unsigned char * i_pbSendBuf,int i_iSendLen,int i_iSocketFd=-1)=0;//一旦某个参数开始指定默认值,它右边的所有参数都必须指定默认值.
    virtual int Recv(unsigned char *o_pbRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iSocketFd=-1,int i_iTimeOutMs=1000)=0;//在调用具有默认参数的函数时, 若某个实参默认,其右边的所有实参都应该默认
    virtual void Close(int i_iSocketFd=-1)=0;//即无论是定义还是调用的时候默认的都得放到后面，https://www.cnblogs.com/LubinLew/p/DefaultParameters.html  
    virtual int SetPeerAddrPort(const char *i_pstrPeerIP,unsigned short i_wPeerPort)=0;
};


/*****************************************************************************
-Class          : UdpServer
-Description    : UDP实际上没有客户端服务端的概念，
UDP是端到端通信，需要先知道对端的ip和端口
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class UdpServer : public UdpSocket
{
public:
    UdpServer();
    virtual ~UdpServer();
    int Init(const char *i_pstrClientIP,unsigned short i_wClientPort,const char *i_pstrIP,unsigned short i_wPort);    
    int Send(unsigned char * i_pbSendBuf,int i_iSendLen,int i_iSocketFd=-1);
    int Recv(unsigned char *o_pbRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iSocketFd=-1,int i_iTimeOutMs=1000);//-1 阻塞 0 非阻塞 >0超时时间
    int SetPeerAddrPort(const char *i_pstrPeerAddr,unsigned short i_wPeerPort);
    void Close(int i_iSocketFd=-1);
    int GetLocalSocketFd();
private:
    int             m_iServerSocketFd;
    string          m_strClientIP;//目的客户端IP，端口不会变，所以放在成员变量里
    unsigned short  m_wClientPort;
};


/*****************************************************************************
-Class          : UdpClient
-Description    : UDP实际上没有客户端服务端的概念，
这里的client表示先发送，server表示先接收
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
class UdpClient : public UdpSocket
{
public:
    UdpClient();
    virtual ~UdpClient();
    int Init(const char *i_pstrServerIP,unsigned short i_wServerPort,const char *i_pstrIP=NULL,unsigned short i_wPort=0);
    int Send(unsigned char * i_pbSendBuf,int i_iSendLen,int i_iSocketFd=-1);
    int Recv(unsigned char *o_pbRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iSocketFd=-1,int i_iTimeOutMs=1000);//-1 阻塞 0 非阻塞 >0超时时间
    int SetPeerAddrPort(const char *i_pstrPeerAddr,unsigned short i_wPeerPort);
    void Close(int i_iSocketFd=-1);
    int GetLocalSocketFd();
private:
    int             m_iClientSocketFd;
    string          m_strServerIP;//目的服务器IP，端口不会变，所以放在成员变量里
    unsigned short  m_wServerPort;
};




#endif
