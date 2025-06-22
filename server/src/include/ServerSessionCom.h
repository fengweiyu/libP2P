/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :    ServerCom.h
* Description           :    ģ���ڲ����ⲿ�����߹�ͬ���������ŵ������include��
* Created               :    2020.01.13.
* Author                :    Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef HTTP_FLV_SERVER_COM_H
#define HTTP_FLV_SERVER_COM_H

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

#define REPORT_NAT_INFO_MSG_ID 6701
#define GET_NAT_INFO_MSG_ID 6702
#define REQ_PEER_SEND_MSG_MSG_ID 6703
#define REPORT_P2P_RESULT_MSG_ID 6704
#define GET_P2P_RESULT_MSG_ID 6705

// ��Ϣ�ṹ��  //��ϢID���ݣ����ݸ���ID���Զ��壬���ݿ��Ը���ID��������
class QueueMessage 
{  
public:  
    // ���캯��  
    QueueMessage(int i_iDataSize) : iDataSize(i_iDataSize), pbData(new unsigned char[i_iDataSize]) {iMsgID=-1;}  
    // �������캯���������  
    QueueMessage(const QueueMessage& i_oOther) : iDataSize(i_oOther.iDataSize), pbData(new unsigned char[i_oOther.iDataSize]) 
    {  
        std::copy(i_oOther.pbData, i_oOther.pbData + i_oOther.iDataSize, pbData); 
        iMsgID=i_oOther.iMsgID;
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
        }  
        return *this;  
    }  
    int iMsgID;
    unsigned char * pbData;
    int iDataSize;
};  


// �̰߳�ȫ����Ϣ����ģ����  
template <typename T>  
class ThreadSafeQueue 
{  
public:  
    void Push(const T& value) // ��Ӳ����������
    {  
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


// �̹߳�������  
void workerThread(int thread_id,   
                 ThreadSafeQueue<QueueMessage>& in_queue,  
                 ThreadSafeQueue<QueueMessage>& out_queue,  
                 bool& running) 
{  
    std::cout << "Thread " << thread_id << " started\n";  
    
    while (running) 
    {  
    }  
    
    std::cout << "Thread " << thread_id << " exiting\n";  
}












#endif
