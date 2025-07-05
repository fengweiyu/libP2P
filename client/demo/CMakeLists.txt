cmake_minimum_required(VERSION 3.15) 


project (libP2PClientDemo)
 

#SET(CMAKE_C_FLAGS "-g")
SET(CMAKE_CXX_FLAGS "-g -std=c++11 -pthread ")
# 设置C++标准为C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("CMAKE_SOURCE_DIR is: ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR}")


add_subdirectory(net) 
add_subdirectory(cJSON-1.7.12) 


# 查找线程库(跨平台)  会查找在当前平台上可用的线程库，并定义 Threads::Threads 目标。如果没有找到库，REQUIRED 会导致配置失败
if (UNIX AND NOT APPLE)  
    find_package(Threads REQUIRED) 
endif() 


# 收集所有源文件  
file(GLOB src_list "./*.cpp")  # 可以根据你的目录结构调整 

#set(src_list
#    src/xxx.cpp
#)

# 定义库  
add_executable(P2PClientDemo ${src_list}) 

target_include_directories(P2PClientDemo PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/net/include
    ${PROJECT_SOURCE_DIR}/cJSON-1.7.12/include
    ${PROJECT_SOURCE_DIR}/../src/include
#    ${PROJECT_SOURCE_DIR}/../src/third/libSTUN/src/include
#    ${PROJECT_SOURCE_DIR}/../src/third/libSTUN/app/NatDetect/include
)
# P2PClient 依赖 NatDetect，linux 编译就需要 P2PClient NatDetect 这样的顺序放入，否则链接失败
target_link_libraries(P2PClientDemo PUBLIC P2PClient NatDetect STUN cJSON net
#    ${CMAKE_BINARY_DIR}/demo/net/libnet.a  ${SYSTEM_PLAT_LIB_PATH}
#    ${CMAKE_BINARY_DIR}/src/libHttpFlvClient.a
#    ${CMAKE_BINARY_DIR}/src/ThirdSrc/http/libhttp.a
#    ${CMAKE_BINARY_DIR}/src/ThirdSrc/media/libmedia.a
) 


if(WIN32)  
    # Windows平台上链接WinSock库  iphlpapi获取本机ip地址需要
    target_link_libraries(P2PClientDemo PUBLIC ws2_32 iphlpapi)
endif()


#target_link_libraries(P2PClientDemo PRIVATE Threads::Threads)  
#TARGET_LINK_LIBRARIES(webrtcServer -pthread -lm -lrt -ldl -lresolv -static)

# 安装库文件  
install(TARGETS P2PClientDemo  
        DESTINATION P2PClientDemo) #安装目录 目录StunDemo换成目录. 也可以，这样就在当前目录生成不会生成在HttpFlvClientDemo目录下





#SET_TARGET_PROPERTIES(base64_static PROPERTIES OUTPUT_NAME base64)
#SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)

#TARGET_LINK_LIBRARIES(webrtc pthread -lm)

#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)





















