/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :    ServerSessionCom.h
* Description           :    ģ���ڲ����ⲿ�����߹�ͬ���������ŵ������include��
* Created               :    2020.01.13.
* Author                :    Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef SERVER_SESSION_COM_H
#define SERVER_SESSION_COM_H

#include <iostream>  
#include <thread>  
#include <mutex>  
#include <queue>  
#include <condition_variable>  
#include <memory>  
#include <chrono>  


#define  P2P_LOGW2(val,fmt,...)      printf("<%d>:"fmt,val,##__VA_ARGS__)
#define  P2P_LOGE2(val,fmt,...)      printf("<%d>:"fmt,val,##__VA_ARGS__)
#define  P2P_LOGD2(val,fmt,...)      printf("<%d>:"fmt,val,##__VA_ARGS__)
#define  P2P_LOGW(...)     printf(__VA_ARGS__)
#define  P2P_LOGE(...)     printf(__VA_ARGS__)
#define  P2P_LOGD(...)     printf(__VA_ARGS__)
#define  P2P_LOGI(...)     printf(__VA_ARGS__)



#ifdef _WIN32
#include <Windows.h>
#define SleepMs(val) Sleep(val)
#else
#include <unistd.h>
#define SleepMs(val) usleep(val*1000)
#endif

#define LOGIN_MSG_ID 6701
#define REPORT_NAT_INFO_MSG_ID 6702
#define GET_NAT_INFO_MSG_ID 6703
#define GET_NAT_INFO_ACK_MSG_ID 6704
#define REQ_PEER_SEND_MSG_MSG_ID 6705
#define REQ_PEER_SEND_MSG_ACK_MSG_ID 6706
#define REQ_SEND_MSG_TO_PEER_MSG_ID 6707
#define REQ_SEND_MSG_TO_PEER_ACK_MSG_ID 6708
#define REPORT_P2P_RESULT_MSG_ID 6709
#define REPORT_P2P_RESULT_ACK_MSG_ID 6710


typedef struct NatInfoMsg 
{
    char strID[64];
    int iNatType;
    char strPublicIP[64];
    int iPublicPort;
}T_NatInfoMsg;
typedef struct ReqPeerSendMsg 
{
    char strPeerID[64];
    T_NatInfoMsg tLocalNatInfo;
}T_ReqPeerSendMsg;
typedef struct ReqPeerSendAckMsg 
{
    int iResult;
    T_NatInfoMsg tPeerNatInfo;
}T_ReqPeerSendAckMsg;

typedef struct ReqSendMsgToPeerResultMsg
{
    char strLocalID[64];
    char strPeerID[64];
    int iResult;
}T_ReqSendMsgToPeerResultMsg;

typedef struct PeerToPeerResultMsg
{
    char strLocalID[64];
    char strPeerID[64];
    int iSuccessCnt;
    int iFailCnt;
    int iCurStatus;//-1 ʧ��,0 �ɹ�
}T_PeerToPeerResultMsg;

typedef struct Peer2PeerCfg
{
    char strStunServer1Addr[128];
    int iStunServer1Port;
    char strStunServer2Addr[128];
    int iStunServer2Port;

}T_Peer2PeerCfg;


#define QUEUE_MSG_MAX_NUM 1000

// ��Ϣ�ṹ��  //��ϢID���ݣ����ݸ���ID���Զ��壬���ݿ��Ը���ID��������
class QueueMessage 
{  
public:  
    // ���캯��  
    QueueMessage()
    {
        pbData = NULL;
        iDataSize = 0;
        iMsgID = -1;
        pSender = NULL;
    }
    QueueMessage(int i_iMsgID,unsigned char * i_pbData,int i_iDataSize,void * i_pSender=NULL)
    {
        pbData=NULL;
        iDataSize=0;
        if(NULL != i_pbData)
        {
            pbData=new unsigned char[i_iDataSize];
            std::copy(i_pbData,i_pbData + i_iDataSize, pbData);
            iDataSize=i_iDataSize;
        }
        iMsgID=i_iMsgID;
        pSender=i_pSender;
    }
    QueueMessage(int i_iDataSize) : iDataSize(i_iDataSize), pbData(new unsigned char[i_iDataSize]) {iMsgID=-1;pSender=NULL;}  
    // �������캯���������  
    QueueMessage(const QueueMessage& i_oOther) : iDataSize(i_oOther.iDataSize), pbData(new unsigned char[i_oOther.iDataSize]) 
    {  
        std::copy(i_oOther.pbData, i_oOther.pbData + i_oOther.iDataSize, pbData); 
        iMsgID=i_oOther.iMsgID;
        pSender=i_oOther.pSender;
    }  
    ~QueueMessage() // ��������
    {  
        if(NULL != pbData)
            delete[] pbData;  
    }  
    // ��ֵ������������  
    QueueMessage& operator=(const QueueMessage& i_oOther) 
    {  
        if (this != &i_oOther) 
        {  
            if(NULL != pbData)
                delete[] pbData;  
            pbData = new unsigned char[i_oOther.iDataSize];  
            std::copy(i_oOther.pbData, i_oOther.pbData + i_oOther.iDataSize, pbData);
            iDataSize = i_oOther.iDataSize;
            iMsgID=i_oOther.iMsgID;
            pSender=i_oOther.pSender;
        }  
        return *this;  
    }  
    unsigned char * pbData;//�Լ�����,�Լ��ͷ�
    int iDataSize;
    int iMsgID;
    void * pSender;//�ⲿ���룬�ⲿ���� ���� �ͷ�
};  


// �̰߳�ȫ����Ϣ����ģ����  
template <typename T>  
class ThreadSafeQueue 
{  
public:  
    int Push(const T& value) // ��Ӳ����������
    {  
        if(m_queue.size() >= QUEUE_MSG_MAX_NUM)
        {
            return -1;
        }
        /*
        ʹ��unique_lock������lock_guard ��ԭ��:
        unique_lock����������;����
        wait()������Ҫ��ʱ�ͷ��������ڷ���ǰ���»�ȡ
        lock_guardû�н����ӿڣ�����������������
        */
        std::unique_lock<std::mutex> lock(m_mutex);//m_queue.push(value) �����Ԫ�صĿ������캯���ڶ����ڲ�����һ���¶���  
        m_queue.push(value);// ��ʽ���ÿ������캯��  //m_queue.push(T(value)); // ��ʽ���ÿ������캯�� //push(const T& value)
        /*
        ����һ�����ڵȴ��������������߳�
        ����ж���߳��ڵȴ������廽���ĸ��ǲ�ȷ����(��ϵͳ���Ⱦ���)
        �����ѵ��᳢̻߳�����»�ȡ�����Ļ�����
        notify_one()����Ч��ֻ����һ����Ҫ���߳�
        notify_all()�����ڶ���߳̿��ܶ���Ҫ��Ӧ�ĳ���*/
        m_cond.notify_one(); 
        return 0;
    }  

    int Pop(T& value) // ���Ӳ����������  
    {  
        std::unique_lock<std::mutex> lock(m_mutex);  
        if (m_queue.empty()) {  
            return -1;//false;  
        }  
        value = m_queue.front();// ���ÿ�����ֵ�����  //T(m_queue.front());  // ��ʽ���ÿ������캯�� ,��࿽��һ��
        m_queue.pop();  // ԭ��������  
        return 0;//true;  
    }  

    int WaitAndPop(T& value, int timeout_ms = 0) 
    {  
        std::unique_lock<std::mutex> lock(m_mutex);//unique_lock������������������ʱ�Զ��ͷŻ�����  
        
        if (timeout_ms <= 0) 
        {  
            /*
            �ȴ��׶Σ�
            �߳����Ȼ�ȡ������lock,���ν��(predicate)���� !m_queue.empty()
            �������Ϊfalse���̻߳�ԭ���Ե��ͷ���������ȴ�״̬
            �̱߳����𣬲�������CPU��Դ
            ���ѽ׶Σ�
            �������̵߳���notify_one()��notify_all()ʱ,�ȴ��̱߳����Ѳ����»�ȡ��
            �ٴμ��ν������,ֻ�е�����Ϊtrueʱ��wait()�Ż᷵��
            */        
            m_cond.wait(lock, [this] { return !m_queue.empty(); });  
        } 
        else 
        {  
            if (!m_cond.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] { return !m_queue.empty(); })) 
            {  
                return -1;//false; // ��ʱ  
            }  
        }  
        
        value = m_queue.front();  // ���ÿ�����ֵ�����
        m_queue.pop();  // ԭ��������  
        return 0;//true;  
    }  

    bool IsEmpty() const
    {  
        std::unique_lock<std::mutex> lock(m_mutex);  
        return m_queue.empty();  
    }  

private:  
    mutable std::mutex m_mutex;  
    std::queue<T> m_queue; //std::queue<std::shared_ptr<T>> m_queue;  ����ָ��
    std::condition_variable m_cond;//ʹ����������(std::condition_variable)ʵ�ָ�Ч�ȴ�  
};  





#endif
