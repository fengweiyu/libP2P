cmake_minimum_required(VERSION 3.15) 


project (libP2PClient)
 


if(CMAKE_SYSTEM_NAME STREQUAL "Windows") #WIN32
    #子目录也会设置CMAKE_INSTALL_PREFIX，但是子目录退出后不生效，所以这里设置全局的，
    #这样这里生成的Makefile可以执行install到下面这个目录，缺点是子目录相关的库文件头文件也会拷贝过来
    set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/build/win/x64)
    #set(SYSTEM_PLAT_LIB_PATH Ws2_32.lib)
    #set(HOST_PATH_NAME win)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    #子目录也会设置CMAKE_INSTALL_PREFIX，但是子目录退出后不生效，所以这里设置全局的，
    #这样这里生成的Makefile可以执行install到下面这个目录，缺点是子目录相关的库文件头文件也会拷贝过来
    set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/build/linux/x64)
    #set(HOST_PATH_NAME linux)
    #set(SYSTEM_PLAT_LIB_PATH pthread)
else()  
    message("CMAKE_SYSTEM_NAME err is: ${CMAKE_SYSTEM_NAME}")
endif() 






# 添加 src 子目录   
add_subdirectory(src)  


# 添加 demo 子目录  
add_subdirectory(demo) 
