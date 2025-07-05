cmake_minimum_required(VERSION 3.15) 


project (libP2PServer)
 

# 设置C++标准为C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("PROJECT_SOURCE_DIR is: ${PROJECT_SOURCE_DIR} ${PROJECT_ROOT_DIR}")

add_subdirectory(third/cJSON-1.7.12) 


# 收集所有源文件  
file(GLOB src_list "./*.cpp")  # 可以根据你的目录结构调整 

#set(src_list
#    src/xxx.cpp
#)

# 定义库  
add_library(P2PServer STATIC ${src_list}) 

target_include_directories(P2PServer PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third/cJSON-1.7.12
)


message("PROJECT_SOURCE_DIR is: ${PROJECT_SOURCE_DIR}")


# 安装库文件  
install(TARGETS P2PServer  
        DESTINATION lib)  # 库文件的安装目录  

# 安装头文件  
file(GLOB PUBLIC_HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")
install(FILES ${PUBLIC_HEADERS}  
        DESTINATION include)  # 头文件的安装目录




#SET_TARGET_PROPERTIES(base64_static PROPERTIES OUTPUT_NAME base64)
#SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)

#TARGET_LINK_LIBRARIES(webrtc pthread -lm)

#TARGET_LINK_LIBRARIES(crnode log4cpp thrift)










