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
    virtual int Send(unsigned char * i_pbSendBuf,int i_iSendLen,int i_iSocketFd=-1)=0;//һ��ĳ��������ʼָ��Ĭ��ֵ,���ұߵ����в���������ָ��Ĭ��ֵ.
    virtual int Recv(unsigned char *o_pbRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iSocketFd=-1,int i_iTimeOutMs=1000)=0;//�ڵ��þ���Ĭ�ϲ����ĺ���ʱ, ��ĳ��ʵ��Ĭ��,���ұߵ�����ʵ�ζ�Ӧ��Ĭ��
    virtual void Close(int i_iSocketFd=-1)=0;//�������Ƕ��廹�ǵ��õ�ʱ��Ĭ�ϵĶ��÷ŵ����棬https://www.cnblogs.com/LubinLew/p/DefaultParameters.html  
    virtual int SetPeerAddrPort(const char *i_pstrPeerIP,unsigned short i_wPeerPort)=0;
};


/*****************************************************************************
-Class          : UdpServer
-Description    : UDPʵ����û�пͻ��˷���˵ĸ��
UDP�Ƕ˵���ͨ�ţ���Ҫ��֪���Զ˵�ip�Ͷ˿�
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
    int Recv(unsigned char *o_pbRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iSocketFd=-1,int i_iTimeOutMs=1000);//-1 ���� 0 ������ >0��ʱʱ��
    int SetPeerAddrPort(const char *i_pstrPeerAddr,unsigned short i_wPeerPort);
    void Close(int i_iSocketFd=-1);
    int GetLocalSocketFd();
private:
    int             m_iServerSocketFd;
    string          m_strClientIP;//Ŀ�Ŀͻ���IP���˿ڲ���䣬���Է��ڳ�Ա������
    unsigned short  m_wClientPort;
};


/*****************************************************************************
-Class          : UdpClient
-Description    : UDPʵ����û�пͻ��˷���˵ĸ��
�����client��ʾ�ȷ��ͣ�server��ʾ�Ƚ���
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
    int Recv(unsigned char *o_pbRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iSocketFd=-1,int i_iTimeOutMs=1000);//-1 ���� 0 ������ >0��ʱʱ��
    int SetPeerAddrPort(const char *i_pstrPeerAddr,unsigned short i_wPeerPort);
    void Close(int i_iSocketFd=-1);
    int GetLocalSocketFd();
private:
    int             m_iClientSocketFd;
    string          m_strServerIP;//Ŀ�ķ�����IP���˿ڲ���䣬���Է��ڳ�Ա������
    unsigned short  m_wServerPort;
};




#endif
