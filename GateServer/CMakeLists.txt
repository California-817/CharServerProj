cmake_minimum_required(VERSION 3.10) #cmake最低版本
project(GateServer) #项目名称
set(CMAKE_CXX_STANDARD 14) #C++版本
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC) #源文件
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) #exe输出路径
include_directories("/usr/local/include") #头文件路径
link_directories("/usr/local/lib" "/usr/local/lib64") #库文件路径
add_executable(GateServer ${SRC}) #可执行程序名称
target_link_libraries(GateServer boost_system pthread jsoncpp) #连接的动态库
