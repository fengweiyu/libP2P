cmake_minimum_required(VERSION 3.10) 


project (libSTUN)
 


if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    #子目录也会设置CMAKE_INSTALL_PREFIX，但是子目录退出后不生效，所以这里设置全局的，
    #这样这里生成的Makefile可以执行install到下面这个目录，缺点是子目录相关的库文件头文件也会拷贝过来
    set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/build/wasm/x64)
    set(HOST_PATH_NAME wasm)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows") #WIN32
    #子目录也会设置CMAKE_INSTALL_PREFIX，但是子目录退出后不生效，所以这里设置全局的，
    #这样这里生成的Makefile可以执行install到下面这个目录，缺点是子目录相关的库文件头文件也会拷贝过来
    set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/build/win/x64)
    #set(SYSTEM_PLAT_LIB_PATH Ws2_32.lib)
    set(HOST_PATH_NAME win)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    #子目录也会设置CMAKE_INSTALL_PREFIX，但是子目录退出后不生效，所以这里设置全局的，
    #这样这里生成的Makefile可以执行install到下面这个目录，缺点是子目录相关的库文件头文件也会拷贝过来
    set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/build/linux/x64)
    set(HOST_PATH_NAME linux)
    set(SYSTEM_PLAT_LIB_PATH pthread)
else()  
    message("CMAKE_SYSTEM_NAME err is: ${CMAKE_SYSTEM_NAME}")
endif() 


#-s WASM=1 -s USE_SDL=2 -s TOTAL_MEMORY=268435456 通过设置 WASM=1，编译器会生成适用于 WebAssembly 的输出 定义 WebAssembly 模块的总内存大小256 MB 用于_mallo等内存拷贝
#-s EXPORTED_FUNCTIONS="['_fnInitDecoder', '_fnUninitDecoder', '_main', '_malloc', '_free']" js中无用定义和使用数组以及指针，必须要_malloc
#-s EXTRA_EXPORTED_RUNTIME_METHODS="['addFunction']" 这些都是链接选项，不是编译选项
#-s RESERVED_FUNCTION_POINTERS=15 指定要保留的函数指针（Function Pointers）数量 Emscripten 将为最多 15 个函数指针分配存储空间
#-s FORCE_FILESYSTEM=1 参数强制 Emscripten 在 WebAssembly 模块中启用文件系统支持
# 为目标添加编译选项  
#target_compile_options(MyExecutable PRIVATE -Wall -Wextra)  # 例如，添加警告选项  
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1 -s TOTAL_MEMORY=268435456 -s RESERVED_FUNCTION_POINTERS=15 -s FORCE_FILESYSTEM=1 -s EXPORTED_FUNCTIONS=[_malloc,_free]")  
#target_link_options(MyExecutable PRIVATE ) 
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")  
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s WASM=1 -s TOTAL_MEMORY=314572800 -s RESERVED_FUNCTION_POINTERS=20 -s FORCE_FILESYSTEM=1 -s EXPORTED_FUNCTIONS=[_malloc,_free] -s EXPORTED_RUNTIME_METHODS=[FS,ccall,cwrap]") 
endif() 



# 添加 src 子目录   src和app统一用一个cmakelist还是分开？理论上app和src是强关联可以放一起，但是实际放一起如果要选择某一个app则要修改编译相关的命令,为了方便则分开
add_subdirectory(src)  

# 添加 app 子目录  可选择具体哪个stun应用
add_subdirectory(app/NatDetect)  
#add_subdirectory(app/ICE) 

# 添加 demo 子目录  
add_subdirectory(demo/net) 
add_subdirectory(demo) 
