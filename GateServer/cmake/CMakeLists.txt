cmake_minimum_required(VERSION 3.16)
project(GateServer)
include(/root/grpc/examples/cpp/cmake/common.cmake)
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED True)
aux_source_directory("/home/xxxten/ChatProj/CharServerProj/GateServer/src" SRC)
set(EXECUTABLE_OUTPUT_PATH "/home/xxxten/ChatProj/CharServerProj/GateServer/bin") 
include_directories("/usr/local/include")
link_directories("/usr/local/lib" "/usr/local/lib64")
# include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories(" /home/xxxten/ChatProj/CharServerProj/GateServer/include")
# hw_grpc_proto
add_library(gate_server_proto
    "/home/xxxten/ChatProj/CharServerProj/GateServer/include/GateServer.Varify.grpc.pb.cc" 
    "/home/xxxten/ChatProj/CharServerProj/GateServer/include/GateServer.Varify.grpc.pb.h"
    "/home/xxxten/ChatProj/CharServerProj/GateServer/include/GateServer.Varify.pb.cc" 
    "/home/xxxten/ChatProj/CharServerProj/GateServer/include/GateServer.Varify.pb.h"
    )
target_link_libraries(gate_server_proto
  absl::check
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

add_executable(GateServer ${SRC})
target_link_libraries(GateServer  boost_system   pthread  hiredis redis++
    gate_server_proto
    absl::check
    absl::flags
    absl::flags_parse
    absl::log
    absl::log_initialize
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
