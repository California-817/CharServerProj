cmake_minimum_required(VERSION 3.16)
project(StatusServer)
#include(/root/grpc/examples/cpp/cmake/common.cmake)
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Threads REQUIRED)
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)

aux_source_directory("/home/xxxten/ChatProj/CharServerProj/StatusServer/src" SRC)
set(EXECUTABLE_OUTPUT_PATH "/home/xxxten/ChatProj/CharServerProj/StatusServer/bin") 
include_directories("/usr/local/include")
link_directories("/usr/local/lib" "/usr/local/lib64")
# include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories(" /home/xxxten/ChatProj/CharServerProj/StatusServer/include")
# hw_grpc_proto

add_library(gate_server_proto_status
    "/home/xxxten/ChatProj/CharServerProj/StatusServer/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc" 
    "/home/xxxten/ChatProj/CharServerProj/StatusServer/include/grpc_cpp_out/GateServer.Status.grpc.pb.h"
    "/home/xxxten/ChatProj/CharServerProj/StatusServer/include/grpc_cpp_out/GateServer.Status.pb.cc" 
    "/home/xxxten/ChatProj/CharServerProj/StatusServer/include/grpc_cpp_out/GateServer.Status.pb.h"
    )

target_link_libraries(gate_server_proto_status
  absl::check
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

add_executable(StatusServer ${SRC})
target_link_libraries(StatusServer  boost_system   pthread 
    gate_server_proto_status
    absl::check
    absl::flags
    absl::flags_parse
    absl::log
    absl::log_initialize
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})