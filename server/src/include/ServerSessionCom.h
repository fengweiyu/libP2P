/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :    ServerCom.h
* Description           :    模块内部与外部调用者共同的依赖，放到对外的include里
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

// 消息结构体  //消息ID数据，数据根据ID可自定义，数据可以根据ID解析处理
class QueueMessage 
{  
public:  
    // 构造函数  
    QueueMessage(int i_iDataSize) : iDataSize(i_iDataSize), pbData(new unsigned char[i_iDataSize]) {iMsgID=-1;}  
    // 拷贝构造函数（深拷贝）  
    QueueMessage(const QueueMessage& i_oOther) : iDataSize(i_oOther.iDataSize), pbData(new unsigned char[i_oOther.iDataSize]) 
    {  
        std::copy(i_oOther.pbData, i_oOther.pbData + i_oOther.iDataSize, pbData); 
        iMsgID=i_oOther.iMsgID;
    }  
    ~QueueMessage() // 析构函数
    {  
        if(NULL != pbData)
            delete[] pbData;  
    }  
    // 赋值运算符（深拷贝）  
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


// 线程安全的消息队列模板类  
template <typename T>  
class ThreadSafeQueue 
{  
public:  
    void Push(const T& value) // 入队操作（深拷贝）
    {  
        /*
        使用unique_lock而不是lock_guard 的原因:
        unique_lock更灵活，可以中途解锁
        wait()函数需要暂时释放锁，并在返回前重新获取
        lock_guard没有解锁接口，不能用于条件变量
        */
        std::unique_lock<std::mutex> lock(m_mutex);//m_queue.push(value) 会调用元素的拷贝构造函数在队列内部创建一个新对象  
        m_queue.push(value);// 隐式调用拷贝构造函数  //m_queue.push(T(value)); // 显式调用拷贝构造函数 //push(const T& value)
        /*
        唤醒一个正在等待该条件变量的线程
        如果有多个线程在等待，具体唤醒哪个是不确定的(由系统调度决定)
        被唤醒的线程会尝试重新获取关联的互斥锁
        notify_one()更高效，只唤醒一个需要的线程
        notify_all()适用于多个线程可能都需要响应的场景*/
        m_cond.notify_one();  
    }  

    int Pop(T& value) // 出队操作（深拷贝）  
    {  
        std::unique_lock<std::mutex> lock(m_mutex);  
        if (m_queue.empty()) {  
            return -1;//false;  
        }  
        value = m_queue.front();// 调用拷贝赋值运算符  //T(m_queue.front());  // 显式调用拷贝构造函数 ,会多拷贝一次
        m_queue.pop();  // 原对象被销毁  
        return 0;//true;  
    }  

    int WaitAndPop(T& value, int timeout_ms = 0) 
    {  
        std::unique_lock<std::mutex> lock(m_mutex);//unique_lock对象会在其作用域结束时自动释放互斥量  
        
        if (timeout_ms <= 0) 
        {  
            /*
            等待阶段：
            线程首先获取互斥锁lock,检查谓词(predicate)条件 !m_queue.empty()
            如果条件为false，线程会原子性地释放锁并进入等待状态
            线程被挂起，不会消耗CPU资源
            唤醒阶段：
            当其他线程调用notify_one()或notify_all()时,等待线程被唤醒并重新获取锁
            再次检查谓词条件,只有当条件为true时，wait()才会返回
            */        
            m_cond.wait(lock, [this] { return !m_queue.empty(); });  
        } 
        else 
        {  
            if (!m_cond.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] { return !m_queue.empty(); })) 
            {  
                return -1;//false; // 超时  
            }  
        }  
        
        value = m_queue.front();  // 调用拷贝赋值运算符
        m_queue.pop();  // 原对象被销毁  
        return 0;//true;  
    }  

    bool IsEmpty() const
    {  
        std::unique_lock<std::mutex> lock(m_mutex);  
        return m_queue.empty();  
    }  

private:  
    mutable std::mutex m_mutex;  
    std::queue<T> m_queue; //std::queue<std::shared_ptr<T>> m_queue;  智能指针
    std::condition_variable m_cond;//使用条件变量(std::condition_variable)实现高效等待  
};  


// 线程工作函数  
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
