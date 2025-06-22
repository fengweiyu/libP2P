/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	TcpSocket.cpp
* Description		: 	TcpSocket  _WIN32 operation center
����֧��epoll��ʹ��wepoll
https://github.com/ZLMediaKit/ZLMediaKit.git/3rdpart/wepoll
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>


//#ifdef _WIN32
#include <WinSock2.h> //win û��epollֻ�����Ƶ�IOCP 
#include <ws2tcpip.h> 

#include "TcpSocket.h"
#include "NetAdapter.h"




using std::cout;
using std::endl;
using std::string;

/*****************************************************************************
-Fuction		: TcpSocket
-Description	: TcpSocket
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpSocket::TcpSocket()
{
}

/*****************************************************************************
-Fuction		: ~TcpSocket
-Description	: ~
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpSocket::~TcpSocket()
{
}

/*****************************************************************************
-Fuction		: ~ResolveDomain
-Description	: ~ResolveDomain
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpSocket::ResolveDomain(string * i_strDomain,string * o_strIP) 
{  
    int iRet = -1;
    
	if(i_strDomain==NULL ||o_strIP==NULL)
	{
        TCP_LOGE("ResolveDomain NULL\r\n");
        return iRet;
	}
    // ��ʼ��WinSock  
    WSADATA wsaData;  
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {  
        //std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;  
        TCP_LOGE("WSAStartup failed:\r\n");
        return iRet;
    }  

    struct addrinfo hints, *res = nullptr;  
    memset(&hints, 0, sizeof(hints));  

    // ����ΪIPv4��TCP  
    hints.ai_family = AF_INET;   
    hints.ai_socktype = SOCK_STREAM;  
    
    // ��ȡ��ַ��Ϣ  
    int status = getaddrinfo(i_strDomain->c_str(), NULL, &hints, &res);  
    if (status != 0) 
    {  
        //std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;  
        TCP_LOGE("getaddrinfo error::\r\n");
        WSACleanup(); // ����WinSock  
        return iRet;  
    }  

    // �����������ӡIP��ַ  
    for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) 
    {  
        void* addr;  
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;  
        addr = &(ipv4->sin_addr);  

        char ipstr[INET_ADDRSTRLEN];  
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));  
        //std::cout << "Resolved IP: " << ipstr << std::endl;  
        TCP_LOGD("Resolved IP: %s\r\n",ipstr);
        o_strIP->assign(ipstr);
        iRet = 0;
    }  

    freeaddrinfo(res); // �ͷ��ڴ�  
    WSACleanup(); // ����WinSock  

    return iRet;  
} 

/*****************************************************************************
-Fuction		: TcpServer
-Description	: TcpServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpServer::TcpServer()
{
    m_iServerSocketFd = -1;
}

/*****************************************************************************
-Fuction		: ~TcpServer
-Description	: ~TcpServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpServer::~TcpServer()
{
    if (m_iServerSocketFd != INVALID_SOCKET)
    {
        closesocket(m_iServerSocketFd);
        m_iServerSocketFd = INVALID_SOCKET;
        WSACleanup();
    }
}

/*****************************************************************************
-Fuction		: Init
-Description	: Init
-Input			: 
-Output 		: 
-Return 		: ʧ�ܷ���-1���ɹ�����0
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServer::Init(string *i_strIP,unsigned short i_wPort)
{
	int iRet=-1;
	int iSocketFd=-1;
	unsigned short wPort=i_wPort;
	struct sockaddr_in tServerAddr;

	if(m_iServerSocketFd !=-1)
	{
        return 0;
	}
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;//WSADATA�ṹ������ĵ�ֵַ
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        TCP_LOGE("TcpSocketInit WSAStartup err\r\n");
        return iRet;
    }
    do
	{
        iSocketFd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(iSocketFd<0)
        {
            TCP_LOGE("TcpSocketInit socket err\r\n");
            break;
        }
        // Set Sockfd NONBLOCK //��ʱʹ��������ʽ��
        //iSocketStatus=fcntl(iSocketFd, F_GETFL, 0);
        //fcntl(iSocketFd, F_SETFL, iSocketStatus | O_NONBLOCK);    
        int tmp = 1;
        if (setsockopt(iSocketFd, SOL_SOCKET, SO_REUSEADDR, (char *)&tmp, sizeof(tmp)) < 0) 
        {
            TCP_LOGE("TcpSocket setsockopt err\r\n");
            break;
        }
        int chOpt = 1;
        if (setsockopt(iSocketFd, IPPROTO_TCP, TCP_NODELAY, (char *)&chOpt, sizeof(chOpt)) < 0) 
        {
            TCP_LOGE("TcpSocket setsockopt TCP_NODELAY err\r\n");
            break;
        }
        // Connect to server
        //this->GetIpAndPort(i_URL,&IP,&wPort);
		memset(&tServerAddr,0, sizeof(tServerAddr));
        tServerAddr.sin_family = AF_INET;
        tServerAddr.sin_port = htons(wPort);//
        if(i_strIP == NULL)
        {
            tServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);//Ҳ����ʹ��htonl(INADDR_ANY),��ʾʹ�ñ���������IP
        }
        else
        {
            tServerAddr.sin_addr.s_addr = inet_addr(i_strIP->c_str());//Ҳ����ʹ��htonl(INADDR_ANY),��ʾʹ�ñ���������IP
        }
        if(bind(iSocketFd,(struct sockaddr*)&tServerAddr,sizeof(tServerAddr))<0)
        {
            TCP_LOGE("TcpSocket setsockopt bind err\r\n");
            break;
        }
        //��ǰ������ip�Ͷ˿ں�����������ӵĿͻ��˸���Ϊ100
        if(listen(iSocketFd,SOMAXCONN)<0) //�ȴ����Ӹ���,Ҳ�����������ӵĿͻ��˸���100
        {
            TCP_LOGE("TcpSocket setsockopt listen err\r\n");
            break;
        }
        m_iServerSocketFd=iSocketFd;
        iRet=0;
    }while(0);
    if(iRet < 0)
    {
        if (iSocketFd != INVALID_SOCKET)
        {
            closesocket(iSocketFd);
            iSocketFd = INVALID_SOCKET;
        }
        WSACleanup();
    }
	return iRet;
}

/*****************************************************************************
-Fuction		: Accept
-Description	: �����Ĳ�����ʽ
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServer::Accept()
{
    int iClientSocketFd=-1;
    SOCKADDR_IN    tClientAddr; 
    int iLen = sizeof(tClientAddr);
    
    //have connect request use accept  
    iLen=sizeof(tClientAddr);  
    iClientSocketFd=accept(m_iServerSocketFd,(struct sockaddr*)&tClientAddr,&iLen);  //�����ȴ��ͻ�������
    if(iClientSocketFd<0)  
    {  
        TCP_LOGE("cannot accept client connect request\r\n");  
        closesocket(m_iServerSocketFd);
        m_iServerSocketFd = INVALID_SOCKET;
        WSACleanup();
        return iClientSocketFd;
    } 
    // ����socketΪ������ģʽ //������select,�ݲ����� 
    u_long mode = 1;
    //if (ioctlsocket(iClientSocketFd, FIONBIO, &mode) == SOCKET_ERROR) 
    {
        //TCP_LOGE("Failed to set socket to non-blocking mode\r\n");
        //closesocket(iClientSocketFd);
        //iClientSocketFd = INVALID_SOCKET;
    }
	return iClientSocketFd;
}

/*****************************************************************************
-Fuction		: Send
-Description	: �����Ĳ�����ʽ
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServer::Send(char * i_acSendBuf,int i_iSendLen,int i_iClientSocketFd)
{
	int iRet=-1;

	
	if(i_acSendBuf==NULL ||i_iSendLen<=0)
	{
        TCP_LOGE("Send err\r\n");
        return iRet;
	}
    iRet=send(i_iClientSocketFd,i_acSendBuf,i_iSendLen,0);
    if(iRet<0)
    {
        Close(i_iClientSocketFd);
        return iRet;
    }
    iRet=0;
    string strSend(i_acSendBuf);
    //TCP_LOGD("Send : %s\r\n",strSend.c_str());

	return iRet;
}

/*****************************************************************************
-Fuction		: Recv
-Description	: �����Ĳ�����ʽ
-Input			: 
-Output 		: o_piRecvLen ����С��Ҫ�������ݣ����ʾ�����ˣ�Ĭ�ϳ�ʱ1s
-Return 		: 
iRet=-1;��ʾ�׽��ֳ���
iRet = 0; ��ʾû�г���������ʱ
o_piRecvLen ��ʾ���յ��ĳ���
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServer::Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iClientSocketFd,milliseconds *i_pTime)
{
    int iRecvLen=-1;
    int iRet=-1;
    fd_set tReadFds;
    timeval tTimeValue;
    char *pcRecvBuf=o_acRecvBuf;
    int iLeftRecvLen=i_iRecvBufMaxLen;

    if(NULL == o_acRecvBuf ||NULL == o_piRecvLen ||i_iRecvBufMaxLen <= 0)
    {
        TCP_LOGE("TcpServer::Recv NULL\r\n");
        return iRet;
    }   
    memset(o_acRecvBuf,0,i_iRecvBufMaxLen);;
    while(iLeftRecvLen > 0)
    {
        FD_ZERO(&tReadFds); //�������������    
        FD_SET(i_iClientSocketFd, &tReadFds); //��������������
        tTimeValue.tv_sec      = 1;//��ʱʱ�䣬��ʱ���ش���
        tTimeValue.tv_usec     = 0;
        if(NULL != i_pTime)
        {
            // ��ȡ����ʱ������ֵ
            long long millisecondsValue = i_pTime->count();
            tTimeValue.tv_sec      = millisecondsValue/1000;//��ʱʱ�䣬��ʱ���ش���
            tTimeValue.tv_usec     = millisecondsValue%1000*1000;
        }
            
        iRet = select(i_iClientSocketFd + 1, &tReadFds, NULL, NULL, &tTimeValue);//����select������غ���//NULL һֱ�ȵ��б仯
        if(iRet<0)  
        {
            TCP_LOGE("select Recv err\n");  
            Close(i_iClientSocketFd);
            iRet=-1;
            break;
        }
        else if(0 == iRet)
        {
            //perror("select Recv timeout\r\n");
            iRet = 0;
            break;
        }
        else
        {
        }
        if (FD_ISSET(i_iClientSocketFd, &tReadFds))   //����fd1�Ƿ�ɶ�  
        {
            iRecvLen=recv(i_iClientSocketFd,pcRecvBuf,iLeftRecvLen,0);  
            if(iRecvLen<=0)
            {
                if(errno != EINTR)
                {
                    TCP_LOGE("errno Recv err%d\r\n",iRecvLen); 
                    iRet=-1;
                    break;
                }
            }
            else
            {
                iLeftRecvLen = iLeftRecvLen-iRecvLen;
                pcRecvBuf += iRecvLen;
                iRet = 0;
            }
        }
        else
        {
            TCP_LOGE("errno FD_ISSET err"); 
            iRet=-1;
        	break;
        }
    }
    if(iLeftRecvLen < i_iRecvBufMaxLen)
    {
        string strRecv(o_acRecvBuf);
        *o_piRecvLen = i_iRecvBufMaxLen - iLeftRecvLen;
        TCP_LOGE("SvcRecv :%d\r\n",*o_piRecvLen);
    }
    else
    {
        //TCP_LOG("Recv err:"<<iRecvAllLen);
    }
    return iRet;
}


/*****************************************************************************
-Fuction		: Close
-Description	: Close
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
void TcpServer::Close(int i_iClientSocketFd)
{
	if(i_iClientSocketFd!=-1)
	{
		closesocket(i_iClientSocketFd);
		i_iClientSocketFd =- 1;
	}
	else
	{
		TCP_LOGE("Close err:%d\r\n",i_iClientSocketFd);
	}
}

/*****************************************************************************
-Fuction		: TcpClient
-Description	: TcpClient
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpClient ::TcpClient()
{
    m_iClientSocketFd = -1;
}

/*****************************************************************************
-Fuction		: ~TcpSocket
-Description	: ~TcpSocket
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpClient ::~TcpClient()
{

}

/*****************************************************************************
-Fuction		: Init
-Description	: Init
-Input			: 
-Output 		: 
-Return 		: ʧ�ܷ���-1���ɹ�����0
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpClient::Init(string *i_strIP,unsigned short i_wPort)
{
	int iRet=-1;
	int iSocketFd=-1;
    string strIP("");
    
	if(i_strIP==NULL)
	{
        TCP_LOGE("TcpClient Init i_strIP NULL\r\n");
        return iRet;
	}
    iRet=TcpSocket::ResolveDomain(i_strIP,&strIP);
    if(iRet < 0)
    {
        TCP_LOGE("TcpClient::ResolveDomain err exit %s,%s\r\n",i_strIP->c_str(),strIP.c_str());
        return iRet;
    }
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;//WSADATA�ṹ������ĵ�ֵַ
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        TCP_LOGE("TcpClient WSAStartup err\r\n");
        return iRet;
    }
	
	do
	{
        struct sockaddr_in tServerAddr;
        iSocketFd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(iSocketFd<0)
        {
            TCP_LOGE("TcpSocketInit err\r\n");
            break;
        }
		// Set Sockfd NONBLOCK //��ʱʹ��������ʽ��
		//iSocketStatus=fcntl(iSocketFd, F_GETFL, 0);
		//fcntl(iSocketFd, F_SETFL, iSocketStatus | O_NONBLOCK);	
        int tmp = 1;
        if (setsockopt(iSocketFd, SOL_SOCKET, SO_REUSEADDR, (char *)&tmp, sizeof(tmp)) < 0) 
        {
            TCP_LOGE("TcpClient setsockopt err\r\n");
            break;
        }
		// Connect to server
		memset(&tServerAddr,0, sizeof(tServerAddr));
		tServerAddr.sin_family = AF_INET;
		tServerAddr.sin_port = htons(i_wPort);
		tServerAddr.sin_addr.s_addr = inet_addr(strIP.c_str());
		if(connect(iSocketFd, (struct sockaddr *)&tServerAddr, sizeof(tServerAddr)) < 0) 
		{
			TCP_LOGE("TcpSocket connect err\r\n");
            break;
		}
        m_iClientSocketFd=iSocketFd;
        iRet=0;
	}while(0);
    if(iRet < 0)
    {
        if (iSocketFd != INVALID_SOCKET)
        {
            closesocket(iSocketFd);
            iSocketFd = INVALID_SOCKET;
        }
        WSACleanup();
    }
	return iRet;


}

/*****************************************************************************
-Fuction		: Send
-Description	: �����Ĳ�����ʽ
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpClient::Send(char * i_acSendBuf,int i_iSendLen,int i_iClientSocketFd)
{
	int iRet=-1;
	int iSocketFd=0;
	
	if(i_acSendBuf==NULL ||i_iSendLen<=0)
	{
        TCP_LOGE("Send err\r\n");
        return iRet;
	}
	if(i_iClientSocketFd> 0)
	{
        iSocketFd = i_iClientSocketFd;
	}
	else
	{
        iSocketFd = m_iClientSocketFd;
	}
    iRet=send(iSocketFd,i_acSendBuf,i_iSendLen,0);
    if(iRet<0)
    {
        Close(iSocketFd);
        return iRet;
    }
    iRet=0;
    string strSend(i_acSendBuf);
    //TCP_LOGD("Send : %s\r\n",strSend.c_str());
    
	return iRet;
}

/*****************************************************************************
-Fuction		: Recv
-Description	: �����Ĳ�����ʽ
-Input			: 
-Output 		: o_piRecvLen ����С��Ҫ�������ݣ����ʾ�����ˣ�Ĭ�ϳ�ʱ1s
-Return 		: 
iRet=-1;��ʾ�׽��ֳ���
iRet = 0; ��ʾû�г���������ʱ
o_piRecvLen ��ʾ���յ��ĳ���
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpClient::Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iClientSocketFd,milliseconds *i_pTime)
{
    int iRecvLen=-1;
    int iRet=-1;
    fd_set tReadFds;
    timeval tTimeValue;
    char *pcRecvBuf=o_acRecvBuf;
    int iLeftRecvLen=i_iRecvBufMaxLen;
	int iSocketFd=0;
	
    if(NULL == o_acRecvBuf ||NULL == o_piRecvLen ||i_iRecvBufMaxLen <= 0)
    {
        TCP_LOGE("TcpClient::Recv NULL\r\n");
        return iRet;
    }   
	if(i_iClientSocketFd> 0)
	{
        iSocketFd = i_iClientSocketFd;
	}
	else
	{
        iSocketFd = m_iClientSocketFd;
	}
    while(iLeftRecvLen > 0)
    {
        FD_ZERO(&tReadFds); //�������������    
        FD_SET(iSocketFd, &tReadFds); //��������������
        tTimeValue.tv_sec  =1;//��ʱʱ�䣬��ʱ���ش���
        tTimeValue.tv_usec = 0;
        if(NULL != i_pTime)
        {
            // ��ȡ����ʱ������ֵ
            long long millisecondsValue = i_pTime->count();
            tTimeValue.tv_sec      = millisecondsValue/1000;//��ʱʱ�䣬��ʱ���ش���
            tTimeValue.tv_usec     = millisecondsValue%1000*1000;
        }
        iRet = select(iSocketFd + 1, &tReadFds, NULL, NULL, &tTimeValue);//����select������غ���//NULL һֱ�ȵ��б仯
        if(iRet<0)  
        {
            TCP_LOGE("select Recv err\n");  
            Close(iSocketFd);
            iRet=-1;
            break;
        }
        else if(0 == iRet)
        {
            //perror("select Recv timeout\r\n");
            iRet = 0;
            break;
        }
        else
        {
        }
        if (FD_ISSET(iSocketFd, &tReadFds))   //����fd1�Ƿ�ɶ�  
        {
            iRecvLen=recv(iSocketFd,pcRecvBuf,iLeftRecvLen,0);  
            if(iRecvLen<=0)
            {
                if(errno != EINTR)
                {
                    TCP_LOGE("errno Recv err%d\r\n",iRecvLen); 
                    iRet=-1;
                    break;
                }
            }
            else
            {
                iLeftRecvLen = iLeftRecvLen-iRecvLen;
                pcRecvBuf += iRecvLen;
                iRet = 0;
                
            }
        }
        else
        {
            TCP_LOGE("errno FD_ISSET err"); 
            iRet=-1;
        	break;
        }
    }
    if(iLeftRecvLen < i_iRecvBufMaxLen)
    {
        string strRecv(o_acRecvBuf);
        *o_piRecvLen = i_iRecvBufMaxLen - iLeftRecvLen;
        TCP_LOGE("Recv :%d\r\n",*o_piRecvLen);
    }
    else
    {
        //TCP_LOG("Recv err:"<<iRecvAllLen);
    }
    return iRet;
}


/*****************************************************************************
-Fuction		: Close
-Description	: Close
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
void TcpClient::Close(int i_iClientSocketFd)
{
	int iClientSocketFd=-1;

    if(i_iClientSocketFd<=0)
    {
        iClientSocketFd=m_iClientSocketFd;
    }
    else
    {
        iClientSocketFd=i_iClientSocketFd;
    }
	if(iClientSocketFd!=-1)
	{
		closesocket(iClientSocketFd);
		m_iClientSocketFd=-1;
	}
	else
	{
		TCP_LOGE("Close err:%d\r\n",iClientSocketFd);
	}
}

/*****************************************************************************
-Fuction        : GetClientSocket
-Description    : GetClientSocket
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int TcpClient::GetClientSocket()
{
    return m_iClientSocketFd;
}

/*****************************************************************************
-Fuction		: TcpServer
-Description	: TcpServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpServerEpoll::TcpServerEpoll()
{
    m_iServerSocketFd = -1;
    m_iServerEpollFd = -1;
    m_iClientEpollFd = -1;
    m_iMaxListenSocket = 1000;
}

/*****************************************************************************
-Fuction		: ~TcpServer
-Description	: ~TcpServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
TcpServerEpoll::~TcpServerEpoll()
{
    CloseServer();
}

/*****************************************************************************
-Fuction		: Init
-Description	: Init
-Input			: 
-Output 		: 
-Return 		: ʧ�ܷ���-1���ɹ�����0
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServerEpoll::Init(unsigned short i_wPort,char * i_strIP)
{
	int iRet=-1;


	return iRet;
}

/*****************************************************************************
-Fuction		: Accept
-Description	: �������Ĳ�����ʽ
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServerEpoll::Accept()
{
	int iRet=-1;


	return iRet;
}

/*****************************************************************************
-Fuction		: Send
-Description	: �����Ĳ�����ʽ
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServerEpoll::Send(char * i_acSendBuf,int i_iSendLen,int i_iClientSocketFd)
{
	int iRet=-1;
	return iRet;
}

/*****************************************************************************
-Fuction		: Recv
-Description	: �������Ĳ�����ʽ
-Input			: i_iTimeoutMs Ĭ��5ms
-Output 		: o_piRecvLen ����С��Ҫ�������ݣ����ʾ�����ˣ�
-Return 		: 
iRet=-1;��ʾ�׽��ֳ���
iRet = 0; ��ʾû�г���������ʱ
o_piRecvLen ��ʾ���յ��ĳ���
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int TcpServerEpoll::Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iClientSocketFd,int i_iTimeoutMs)
{
    int iRecvLen=-1;
    int iRet=-1;
    return iRet;
}


/*****************************************************************************
-Fuction		: Close
-Description	: Close
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
void TcpServerEpoll::CloseClient(int i_iClientSocketFd)
{
}


/*****************************************************************************
-Fuction		: Close
-Description	: Close
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
void TcpServerEpoll::CloseServer()
{
}



