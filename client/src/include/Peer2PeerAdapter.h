/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module       : 	Peer2PeerAdapter.h
* Description       : 	模块(对外)的依赖，比如一些系统相关的 定义 ，日志函数等，暂时先放到对外的include里
* Created           : 	2020.11.21.
* Author            : 	Yu Weifeng
* Function List     : 	
* Last Modified     : 	
* History           : 	
******************************************************************************/
#ifndef PEER_2_PEER_ADAPTER_H
#define PEER_2_PEER_ADAPTER_H


#ifdef MEDIA_SEVER_TYPE_WEBRTC //0
#define  P2P_LOGW(...)     logi(P2P) << lformat(P2P,__VA_ARGS__) << lend 
#define  P2P_LOGE(...)     loge(P2P) << lformat(P2P,__VA_ARGS__) << lend
#define  P2P_LOGD(...)     logd(P2P) << lformat(P2P,__VA_ARGS__) << lend
#define  P2P_LOGI(...)     logi(P2P) << lformat(P2P,__VA_ARGS__) << lend
#else
#define  P2P_LOGW(...)     printf(__VA_ARGS__)
#define  P2P_LOGE(...)     printf(__VA_ARGS__)
#define  P2P_LOGD(...)     printf(__VA_ARGS__)
#define  P2P_LOGI(...)     printf(__VA_ARGS__)
#endif

#endif //MEDIA_ADAPTER_H
