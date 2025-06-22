/*****************************************************************************
* Copyright (C) 2017-2018 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module		: 	UdpSocket.cpp
* Description		: 	UdpSocket operation center
* Created			: 	2017.09.21.
* Author			: 	Yu Weifeng
* Function List		: 	
* Last Modified 	: 	
* History			: 	
******************************************************************************/
#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <sys/types.h>

#include <fcntl.h>
#include <errno.h>


#ifdef _WIN32
#include <WinSock2.h> //win 没有epoll只有类似的IOCP 
#include <ws2tcpip.h> 
//#include <iphlpapi.h> //
#else
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h> 
#include <ifaddrs.h>
#endif


#include "NetAdapter.h"
#include "UdpSocket.h"

using std::cout;
using std::endl;
using std::string;


/*****************************************************************************
-Fuction		: UdpSocket
-Description	: UdpSocket
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
UdpSocket::UdpSocket()
{
}

/*****************************************************************************
-Fuction		: ~UdpSocket
-Description	: ~
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
UdpSocket::~UdpSocket()
{
}

/*****************************************************************************
-Fuction		: ResolveDomain
-Description	: ResolveDomain
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int UdpSocket::GetLocalAddr(string * o_strIP) 
{  
#ifdef _WIN32
    return -1;//
#if 0
    ULONG outBufLen = 0;  
    DWORD dwRetVal = 0;  

    // 第一次调用，获取缓冲区大小  
    if (GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &outBufLen) == ERROR_BUFFER_OVERFLOW) 
    {  
        IP_ADAPTER_ADDRESSES *pAddresses = (IP_ADAPTER_ADDRESSES *) malloc(outBufLen);  
        if (pAddresses == NULL) 
        {  
            printf("Memory allocation failed\n");  
            return -1;  
        }  
        if ((dwRetVal = GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen)) == NO_ERROR) 
        {  
            IP_ADAPTER_ADDRESSES *pCurr = pAddresses;  
            while (pCurr != NULL) 
            {  
                // 遍历 unicast 地址链表  
                IP_ADAPTER_UNICAST_ADDRESS *pUnicast = pCurr->FirstUnicastAddress;  
                while (pUnicast != NULL) 
                {  
                    if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) 
                    {  
                        char ip[INET_ADDRSTRLEN];  
                        struct sockaddr_in *sa_in = (struct sockaddr_in *)pUnicast->Address.lpSockaddr;  
                        inet_ntop(AF_INET, &(sa_in->sin_addr), ip, INET_ADDRSTRLEN);
                        o_strIP->assign(ip);
                        printf("IPv4 Address: %s\n", ip);  
                        free(pAddresses);  
                        return 0; // 只取第一个IPv4地址  
                    }  
                    pUnicast = pUnicast->Next;  
                }  
                pCurr = pCurr->Next;  
            }  
        } 
        else 
        {  
            printf("GetAdaptersAddresses() failed with error: %ld\n", dwRetVal);  
        }  
        free(pAddresses);  
    } 
    else 
    {  
        printf("GetAdaptersAddresses() failed to get buffer size\n");  
    }  
    return -1;  

    //旧版本win sdk
    DWORD   dwSize = 0;  
    DWORD   dwRetVal = 0;  

    // 第一次调用，获取缓冲区大小  
    if (GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &dwSize) == ERROR_BUFFER_OVERFLOW) 
    {  
        IP_ADAPTER_ADDRESSES *pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(dwSize);  
        if (pAddresses == NULL) 
        {  
            printf("Memory allocation failed\n");  
            return -1;  
        }  

        if ((dwRetVal = GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &dwSize)) == NO_ERROR) 
        {  
            IP_ADAPTER_ADDRESSES *pCurrAddresses = pAddresses;  
            while (pCurrAddresses) 
            {  
                IP_ADDR_STRING *pIpAddr = &pCurrAddresses->IpAddressList;  
                while (pIpAddr) 
                {  
                    if (strcmp(pIpAddr->IpAddress.String, "127.0.0.1") != 0) 
                    {  
                        o_strIP->assign(pIpAddr->IpAddress.String);
                        printf("IPv4 Address: %s\n", pIpAddr->IpAddress.String);  
                        free(pAddresses);  
                        return 0;  // 只取第一个  
                    }  
                }  
                pCurrAddresses = pCurrAddresses->Next;  
            }  
        }  
        free(pAddresses);  
    }  
    printf("Failed to get IP addresses\n");  
    return -1;  
#endif
#else
    struct ifaddrs *ifaddr, *ifa;  
    char ip[INET_ADDRSTRLEN] = {0};  

    if (getifaddrs(&ifaddr) == -1) 
    {  
        perror("getifaddrs");  
        return -1;  
    }  
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {  
        if (ifa->ifa_addr == NULL)  
            continue;  

        if (ifa->ifa_addr->sa_family == AF_INET) 
        { // IPv4  
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;  
            inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);  
            // 跳过本地回环地址  
            if (strcmp(ip, "127.0.0.1") != 0) 
            {  
                printf("IPv4 Address: %s\n", ip);
                o_strIP->assign(ip);
                break; // 只取第一个  
            }  
        }  
    }  
    freeifaddrs(ifaddr);  
    return 0;  
#endif
} 

/*****************************************************************************
-Fuction		: ResolveDomain
-Description	: ResolveDomain
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int UdpSocket::ResolveDomain(const char * i_strDomain,string * o_strIP) 
{  
    int iRet = -1;
    
	if(i_strDomain==NULL ||o_strIP==NULL)
	{
        UDP_LOGE("ResolveDomain NULL\r\n");
        return iRet;
	}
#ifdef _WIN32
    // 初始化WinSock  
    WSADATA wsaData;  
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {  
        //std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;  
        UDP_LOGE("WSAStartup failed:\r\n");
        return iRet;
    }  
#endif
    struct addrinfo hints, *res;  
    memset(&hints, 0, sizeof(hints));  
    
    // 设置类型为IPv4  
    hints.ai_family = AF_INET;  //IPv4   AF_UNSPEC;       // 支持IPv4或IPv6  
    hints.ai_socktype = SOCK_DGRAM; //UDP  SOCK_STREAM tcp

    do
    {
        // 调用getaddrinfo  
        int status = getaddrinfo(i_strDomain, NULL, &hints, &res);  
        if (status != 0) 
        {  
            //std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;  
            UDP_LOGE("getaddrinfo error::\r\n");
            break;;  
        }  
        // 遍历结果  
        for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) 
        {  
            void* addr;  
            // 获取IPv4地址  
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;  
            addr = &(ipv4->sin_addr);  
        
            // 转换为可读的IPv4地址  
            char ipstr[INET_ADDRSTRLEN];  
            inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));  
            
            //std::cout << "Resolved IP: " << ipstr << std::endl;  
            UDP_LOGD("Resolved IP: %s\r\n",ipstr);
            o_strIP->assign(ipstr);
            iRet = 0;
        }  
    }while(0);
    // 释放结果  
    freeaddrinfo(res); 
#ifdef _WIN32
    WSACleanup(); // 清理WinSock  
#endif
    return iRet;  
} 

/*****************************************************************************
-Fuction		: UdpServer
-Description	: UdpServer
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
UdpServer::UdpServer()
{
    m_iServerSocketFd = -1;
    m_strClientIP.clear();
    m_wClientPort=0;
}

/*****************************************************************************
-Fuction        : ~UdpServer
-Description    : ~UdpServer
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
UdpServer::~UdpServer()
{
}

/*****************************************************************************
-Fuction        : Init
-Description    : Init
-Input          : 
-Output         : 
-Return         : 失败返回-1，成功返回0
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int UdpServer::Init(const char *i_pstrClientIP,unsigned short i_wClientPort,const char *i_pstrIP,unsigned short i_wPort)
{
    int iRet=FALSE;
    int iSocketFd=-1;
    unsigned short wPort=i_wPort;
    struct sockaddr_in tServerAddr;
    string strIP("");

    if(m_iServerSocketFd !=-1)
    {
        UDP_LOGD("UDP already inited:\r\n");
        iRet=TRUE;
        return iRet;
    }
    iRet=UdpSocket::ResolveDomain(i_pstrClientIP,&strIP);
    if(iRet < 0)
    {
        UDP_LOGE("TcpClient::ResolveDomain err exit %s,%s\r\n",i_pstrClientIP,strIP.c_str());
        return iRet;
    }
#ifdef _WIN32
    // 初始化WinSock  
    WSADATA wsaData;  
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {  
        UDP_LOGE("WSAStartup failed:%d\r\n",WSAGetLastError());
        return iRet;
    }  
#endif

    do
    {
        iSocketFd=socket(AF_INET,SOCK_DGRAM,0);//IPPROTO_UDP 0
        if(iSocketFd<0)
        {
            perror(NULL);
            UDP_LOGE("UdpSocketInit err\r\n");
            break;
        }
        // Set Sockfd NONBLOCK //暂时使用阻塞形式的
        //iSocketStatus=fcntl(iSocketFd, F_GETFL, 0);
        //fcntl(iSocketFd, F_SETFL, iSocketStatus | O_NONBLOCK);    
        
        // Connect to server
        //this->GetIpAndPort(i_URL,&IP,&wPort);
        memset(&tServerAddr,0, sizeof(tServerAddr));
        tServerAddr.sin_family = AF_INET;
        tServerAddr.sin_port = htons(wPort);//
        if(NULL == i_pstrIP)
            tServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);//也可以使用htonl(INADDR_ANY),表示使用本机的所有IP
        else
            tServerAddr.sin_addr.s_addr = inet_addr(i_pstrIP);//也可以使用htonl(INADDR_ANY),表示使用本机的所有IP
        if(bind(iSocketFd,(struct sockaddr*)&tServerAddr,sizeof(tServerAddr))<0)
        {
            perror(NULL);
            UDP_LOGE("UdpSocket bind err\r\n");
            break;
        }
        m_iServerSocketFd=iSocketFd;
        if(i_pstrClientIP!=NULL && i_wClientPort!=0)
        {
            m_strClientIP.assign(strIP.c_str());
            m_wClientPort=i_wClientPort;
        }
        iRet=TRUE;
    }while(0);

    if(iRet!=TRUE)
    {
#ifdef _WIN32
        if(iSocketFd>=0)
            closesocket(iSocketFd);  
        WSACleanup();  
#else
        if(iSocketFd>=0)
            close(iSocketFd);  
#endif
    }
    return iRet;
}

/*****************************************************************************
-Fuction        : Send
-Description    : 阻塞的操作形式
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int UdpServer::Send(unsigned char * i_pbSendBuf,int i_iSendLen,int i_iSocketFd)
{
    int iRet=FALSE;
    struct sockaddr_in tClientAddr;
    unsigned char * pbSendBuf=i_pbSendBuf;
    int iSendLen=i_iSendLen;
	int iSocketFd=-1;


    if(i_pbSendBuf==NULL ||i_iSendLen<=0||m_wClientPort==0||m_strClientIP.length()<=0)
    {
        //cout<<"Send err"<<m_wClientPort<<endl;
        UDP_LOGE("Send NULL err %d %d\r\n",i_iSendLen,m_wClientPort);
        return iRet;
    }

    if(i_iSocketFd<0)
    {
        iSocketFd=m_iServerSocketFd;
    }
    else
    {
        iSocketFd=i_iSocketFd;
    }
    if(iSocketFd<0)
    {
        UDP_LOGE("UdpServer::Send iSocketFd<0 err:\r\n");
        return iRet;
    }
    memset(&tClientAddr,0, sizeof(tClientAddr));
    tClientAddr.sin_family = AF_INET;
    tClientAddr.sin_port = htons(m_wClientPort);//
    tClientAddr.sin_addr.s_addr = inet_addr(m_strClientIP.c_str());
    while(1)
    {
        iRet=sendto(iSocketFd,(char *)pbSendBuf,iSendLen,0,(struct sockaddr*)&tClientAddr,sizeof(tClientAddr));
        if(iRet<0)
        {
            UDP_LOGE("sendto err %d %d\r\n",i_iSendLen,m_wClientPort);
            break;
        }
        if(iRet<iSendLen)
        {
            pbSendBuf+=iRet;
            iSendLen-=iRet;
        }
        else
        {
            iRet=TRUE;
            break;
        }
    }
    //string strSend(i_acSendBuf);
    //cout<<"Send :\r\n"<<strSend<<"iRet:"<<iRet<<endl;

    return iRet;
}

/*****************************************************************************
-Fuction        : Recv
-Description    : 阻塞的操作形式
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int UdpServer::Recv(unsigned char *o_pbRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iSocketFd,int i_iTimeOutMs)
{
    int iRet=FALSE;
    fd_set tReadFds;
    timeval tTimeValue;
    timeval *ptTimeValue=NULL;
    char acRecvBuf[UDP_MTU];
    unsigned char *pbRecvBuf=o_pbRecvBuf;
    //int iRecvAllLen=0;
    struct sockaddr_in tClientAddr;
    struct sockaddr_in tRecvClientAddr;
    socklen_t iSockaddrLen=sizeof(struct sockaddr_in);
    int iRecvLen=0;
	int iSocketFd=-1;

    if(o_pbRecvBuf==NULL ||o_piRecvLen==NULL ||i_iRecvBufMaxLen<=0)
    {
        UDP_LOGE("Recv NULL err %d %d\r\n",i_iRecvBufMaxLen,i_iSocketFd);
        return iRet;
    }
    if(i_iSocketFd<0)
    {
        iSocketFd=m_iServerSocketFd;
    }
    else
    {
        iSocketFd=i_iSocketFd;
    }
    if(iSocketFd<0)
    {
        UDP_LOGE("UdpServer::Recv iSocketFd<0 err:\r\n");
        return iRet;
    }
    if(i_iTimeOutMs < 0)
    {
        ptTimeValue=NULL;
    }
    else
    {
        tTimeValue.tv_sec = i_iTimeOutMs/1000;//超时时间，超时返回错误
        tTimeValue.tv_usec = i_iTimeOutMs%1000*1000;
        ptTimeValue=&tTimeValue;
    }
    memset(o_pbRecvBuf,0,i_iRecvBufMaxLen);;
    FD_ZERO(&tReadFds); //清空描述符集合    
    FD_SET(iSocketFd, &tReadFds); //设置描述符集合
        
    //while(1)//udp是面向数据报的，没有粘包,不需要一直读
    {
        iRet = select(iSocketFd + 1, &tReadFds, NULL, NULL, ptTimeValue);//调用select（）监控函数//NULL 一直等到有变化
        if(iRet<0)  
        {
            perror("select Recv err\n");  
            UDP_LOGE("select Recv err %d %d\r\n",i_iRecvBufMaxLen,i_iSocketFd);
            return iRet;
        }
        if (FD_ISSET(iSocketFd, &tReadFds))   //测试fd1是否可读  
        {
            memset(acRecvBuf,0,UDP_MTU);//不需要组包，udp就是一个个数据包的
            iRecvLen=recvfrom(iSocketFd,acRecvBuf,sizeof(acRecvBuf),0,(struct sockaddr*)&tRecvClientAddr,&iSockaddrLen);
            if(iRecvLen<0)
            {
                perror("recvfrom Recv err\n");  
                UDP_LOGE("recvfrom Recv err %d %d\r\n",i_iRecvBufMaxLen,i_iSocketFd);
                return iRet;
            }
            if(iRecvLen>i_iRecvBufMaxLen)
            {
                iRet=FALSE;
                UDP_LOGE("Recv err, iRecvLen%d>i_iRecvBufMaxLen%d \r\n",iRecvLen,i_iRecvBufMaxLen);
                return iRet;
            }
            memcpy(pbRecvBuf,acRecvBuf,iRecvLen);
            pbRecvBuf+=iRecvLen;
            iRet=TRUE;
            //break;
        }
        else
        {
            //break;
        }
    }
    
    if(iRecvLen>0 && iRet==TRUE)
    {
        if(m_strClientIP.length()>0 && m_wClientPort!=0)
        {
            /*bzero(&tClientAddr, sizeof(tClientAddr));
            tClientAddr.sin_family = AF_INET;
            tClientAddr.sin_port = htons(m_wClientPort);//
            tClientAddr.sin_addr.s_addr = inet_addr(m_strClientIP.c_str());
            if(0!=memcmp(&tClientAddr,&tRecvClientAddr,sizeof(struct sockaddr_in)))//判断数据来源
            {
                string strRecv(inet_ntoa(tRecvClientAddr.sin_addr));
                cout<<"Recv data from err IP:\r\n"<<strRecv<<" Port:"<<ntohs(tRecvClientAddr.sin_port)<<endl;
                iRet=FALSE;
            }
            else*/
            {
                *o_piRecvLen=iRecvLen;
                iRet=TRUE;
            }
        }
        else
        {
            *o_piRecvLen=iRecvLen;
            //cout<<"Recv :\r\n"<<strRecv<<endl;
            iRet=TRUE;
        }
    }
    else
    {
        //cout<<"Recv err:"<<iRecvLen<<endl;
        iRet=FALSE;
    }
    return iRet;
}



/*****************************************************************************
-Fuction        : Close
-Description    : Close
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
void UdpServer::Close(int i_iSocketFd)
{
	int iSocketFd=-1;

    if(i_iSocketFd<0)
    {
        iSocketFd=m_iServerSocketFd;
    }
    else
    {
        iSocketFd=i_iSocketFd;
    }
	if(iSocketFd!=-1)
	{
#ifdef _WIN32
        closesocket(iSocketFd);  
        WSACleanup();  
#else
        close(iSocketFd);  
#endif
	}
	else
	{
		UDP_LOGE("Close err:%d",iSocketFd);
	}
}

/*****************************************************************************
-Fuction        : UdpClient
-Description    : UdpClient
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
UdpClient::UdpClient()
{
    m_iClientSocketFd = -1;
    m_strServerIP.clear();
    m_wServerPort=0;
}

/*****************************************************************************
-Fuction        : ~UdpSocket
-Description    : ~UdpSocket
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
UdpClient::~UdpClient()
{

}

/*****************************************************************************
-Fuction        : Init
-Description    : 绑定端口确保是连续的rtp rtcp
-Input          : 
-Output         : 
-Return         : 失败返回-1，成功返回0
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int UdpClient::Init(const char *i_pstrServerIP,unsigned short i_wServerPort,const char *i_pstrIP,unsigned short i_wPort)
{
    int iRet=FALSE;
    int iSocketFd=-1;
    unsigned short wPort=i_wPort;
    struct sockaddr_in tServerAddr;
    struct sockaddr_in tClientAddr;
    string strIP("");

    if(m_iClientSocketFd !=-1)
    {
        UDP_LOGD("UdpClient already inited:\r\n");
        iRet=TRUE;
        return iRet;
    }
    if(i_pstrServerIP==NULL ||i_wServerPort<0)
    {
        UDP_LOGE("Init NULL err %d %d\r\n",i_wServerPort,i_wPort);
        return iRet;
    }
    iRet=UdpSocket::ResolveDomain(i_pstrServerIP,&strIP);
    if(iRet < 0)
    {
        UDP_LOGE("TcpClient::ResolveDomain err exit %s,%s\r\n",i_pstrServerIP,strIP.c_str());
        return iRet;
    }
#ifdef _WIN32
    // 初始化WinSock  
    WSADATA wsaData;  
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {  
        UDP_LOGE("WSAStartup failed:%d\r\n",WSAGetLastError());
        return iRet;
    }  
#endif
    do
    {
        iSocketFd=socket(AF_INET,SOCK_DGRAM,0);//IPPROTO_UDP 0
        if(iSocketFd<0)
        {
            perror(NULL);
            UDP_LOGE("UdpSocketInit err\r\n");
            break;
        }
        // Set Sockfd NONBLOCK //暂时使用阻塞形式的
        //iSocketStatus=fcntl(iSocketFd, F_GETFL, 0);
        //fcntl(iSocketFd, F_SETFL, iSocketStatus | O_NONBLOCK);    
        if(NULL != i_pstrIP && 0!=i_wPort)
        {//绑定端口确保端口是连续的rtp rtcp
            memset(&tClientAddr,0,sizeof(tClientAddr));
            tClientAddr.sin_family = AF_INET;
            tClientAddr.sin_port = htons(i_wPort);//
            tClientAddr.sin_addr.s_addr = inet_addr(i_pstrIP);//也可以使用htonl(INADDR_ANY),表示使用本机的所有IP
            if(bind(iSocketFd,(struct sockaddr*)&tClientAddr,sizeof(tClientAddr))<0)
            {
                perror(NULL);
                UDP_LOGE("UdpSocket bind err\r\n");
                break;
            }
        }
        memset(&tServerAddr,0, sizeof(tServerAddr));
        tServerAddr.sin_family = AF_INET;
        tServerAddr.sin_port = htons(i_wServerPort);
        tServerAddr.sin_addr.s_addr = inet_addr(strIP.c_str());
        if(connect(iSocketFd, (struct sockaddr *)&tServerAddr, sizeof(tServerAddr)) < 0 && errno != EINPROGRESS) 
        {//使用connect的目的是Init后就可以获取到本地连接的端口，否则只能sendto之后才能获取
            perror(NULL);
            UDP_LOGE("UdpSocket connect err\r\n");
            break;
        }
        m_strServerIP.assign(strIP.c_str());
        m_wServerPort=i_wServerPort;
        m_iClientSocketFd=iSocketFd;
        iRet=TRUE;
    }while(0);
    
    if(iRet!=TRUE)
    {
#ifdef _WIN32
        if(iSocketFd>=0)
            closesocket(iSocketFd);  
        WSACleanup();  
#else
        if(iSocketFd>=0)
            close(iSocketFd);  
#endif
    }
    
    return iRet;
}

/*****************************************************************************
-Fuction        : Send
-Description    : 阻塞的操作形式
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int UdpClient::Send(unsigned char * i_pbSendBuf,int i_iSendLen,int i_iSocketFd)
{
    int iRet=FALSE;
    struct sockaddr_in tServerAddr;
    unsigned char * pbSendBuf=i_pbSendBuf;
    int iSendLen=i_iSendLen;
	int iSocketFd=-1;


    if(i_pbSendBuf==NULL ||i_iSendLen<=0||m_wServerPort==0||m_strServerIP.length()<=0)
    {
        UDP_LOGE("UdpClient Send NULL err %d %d\r\n",i_iSendLen,m_wServerPort);
        return iRet;
    }

    if(i_iSocketFd<0)
    {
        iSocketFd=m_iClientSocketFd;
    }
    else
    {
        iSocketFd=i_iSocketFd;
    }
    if(iSocketFd<0)
    {
        UDP_LOGE("UdpClient::Send iSocketFd<0 err:\r\n");
        return iRet;
    }
    memset(&tServerAddr,0, sizeof(tServerAddr));
    tServerAddr.sin_family = AF_INET;
    tServerAddr.sin_port = htons(m_wServerPort);//
    tServerAddr.sin_addr.s_addr = inet_addr(m_strServerIP.c_str());
    while(1)
    {
        //iRet=sendto(iSocketFd,acSendBuf,iSendLen,0,(struct sockaddr*)&tClientAddr,sizeof(tClientAddr));
        iRet=send(iSocketFd,(char *)pbSendBuf,iSendLen,0);
        if(iRet<0)
        {
            UDP_LOGE("sendto err %d %d\r\n",i_iSendLen,m_wServerPort);
            break;
        }
        if(iRet<iSendLen)
        {
            pbSendBuf+=iRet;
            iSendLen-=iRet;
        }
        else
        {
            iRet=TRUE;
            break;
        }
    }
    //string strSend(i_acSendBuf);
    //cout<<"Send :\r\n"<<strSend<<"iRet:"<<iRet<<endl;
    return iRet;
}

/*****************************************************************************
-Fuction        : Recv
-Description    : 阻塞的操作形式
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
int UdpClient::Recv(unsigned char *o_pbRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen,int i_iSocketFd,int i_iTimeOutMs)
{
    int iRet=FALSE;
    fd_set tReadFds;
    timeval tTimeValue;
    timeval *ptTimeValue=NULL;
    char acRecvBuf[UDP_MTU];
    unsigned char *pbRecvBuf=o_pbRecvBuf;
    //int iRecvAllLen=0;
    struct sockaddr_in tServerAddr;
    struct sockaddr_in tRecvServerAddr;
    socklen_t iSockaddrLen=sizeof(struct sockaddr_in);
    int iRecvLen=0;
	int iSocketFd=-1;

    if(o_pbRecvBuf==NULL ||o_piRecvLen==NULL ||i_iRecvBufMaxLen<=0)
    {
        UDP_LOGE("Recv NULL err %d %d\r\n",i_iRecvBufMaxLen,i_iSocketFd);
        return iRet;
    }
    if(i_iSocketFd<0)
    {
        iSocketFd=m_iClientSocketFd;
    }
    else
    {
        iSocketFd=i_iSocketFd;
    }
    if(iSocketFd<0)
    {
        UDP_LOGE("UdpClient::Recv iSocketFd<0 err:\r\n");
        return iRet;
    }
    if(i_iTimeOutMs < 0)
    {
        ptTimeValue=NULL;
    }
    else
    {
        tTimeValue.tv_sec = i_iTimeOutMs/1000;//超时时间，超时返回错误
        tTimeValue.tv_usec = i_iTimeOutMs%1000*1000;
        ptTimeValue=&tTimeValue;
    }
    memset(o_pbRecvBuf,0,i_iRecvBufMaxLen);;
    FD_ZERO(&tReadFds); //清空描述符集合    
    FD_SET(iSocketFd, &tReadFds); //设置描述符集合
        
    //while(1)//udp是面向数据报的，没有粘包,不需要一直读
    {
        iRet = select(iSocketFd + 1, &tReadFds, NULL, NULL, ptTimeValue);//调用select（）监控函数//NULL 一直等到有变化
        if(iRet<0)  
        {
            perror("select Recv err\n");  
            UDP_LOGE("select Recv err %d %d\r\n",i_iRecvBufMaxLen,i_iSocketFd);
            return iRet;
        }
        if (FD_ISSET(iSocketFd, &tReadFds))   //测试fd1是否可读  
        {
            memset(acRecvBuf,0,UDP_MTU);//不需要组包，udp就是一个个数据包的
            iRecvLen=recvfrom(iSocketFd,acRecvBuf,sizeof(acRecvBuf),0,(struct sockaddr*)&tRecvServerAddr,&iSockaddrLen);
            if(iRecvLen<0)
            {
                perror("recvfrom Recv err\n");  
                UDP_LOGE("recvfrom Recv err %d %d\r\n",i_iRecvBufMaxLen,i_iSocketFd);
                return iRet;
            }
            if(iRecvLen>i_iRecvBufMaxLen)
            {
                iRet=FALSE;
                UDP_LOGE("Recv err, iRecvLen%d>i_iRecvBufMaxLen%d \r\n",iRecvLen,i_iRecvBufMaxLen);
                return iRet;
            }
            memcpy(pbRecvBuf,acRecvBuf,iRecvLen);
            pbRecvBuf+=iRecvLen;
            iRet=TRUE;
            //break;
        }
        else
        {
            //break;
        }
    }
    
    if(iRecvLen>0 && iRet==TRUE)
    {
        if(m_strServerIP.length()>0 && m_wServerPort!=0)
        {
            /*bzero(&tServerAddr, sizeof(tServerAddr));
            tServerAddr.sin_family = AF_INET;
            tServerAddr.sin_port = htons(m_wServerPort);//
            tServerAddr.sin_addr.s_addr = inet_addr(m_strServerIP.c_str());
            if(0!=memcmp(&tServerAddr,&tRecvServerAddr,sizeof(struct sockaddr_in)))
            {//判断数据来源
                string strRecv(inet_ntoa(tRecvServerAddr.sin_addr));
                cout<<"Recv data from err IP:\r\n"<<strRecv<<" Port:"<<ntohs(tRecvServerAddr.sin_port)<<endl;
                iRet=FALSE;
            }
            else*/
            {
                *o_piRecvLen=iRecvLen;
                iRet=TRUE;
            }
        }
        else
        {
            UDP_LOGE("Recv m_strServerIP err %d %d\r\n",iRecvLen,m_wServerPort);
            iRet=FALSE;
        }
    }
    else
    {
        //cout<<"Recv err:"<<iRecvLen<<endl;
        iRet=FALSE;
    }
    return iRet;
}


/*****************************************************************************
-Fuction        : Close
-Description    : Close
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version        Author           Modification
* -----------------------------------------------
* 2017/09/21      V1.0.0         Yu Weifeng       Created
******************************************************************************/
void UdpClient::Close(int i_iSocketFd)
{
    int iSocketFd=-1;

    if(i_iSocketFd<0)
    {
        iSocketFd=m_iClientSocketFd;
    }
    else
    {
        iSocketFd=i_iSocketFd;
    }
    if(iSocketFd!=-1)
    {
#ifdef _WIN32
        closesocket(iSocketFd);  
        WSACleanup();  
#else
        close(iSocketFd);  
#endif
    }
    else
    {
        UDP_LOGE("Close err:%d",iSocketFd);
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
int UdpClient::GetClientSocket()
{
    return m_iClientSocketFd;
}

