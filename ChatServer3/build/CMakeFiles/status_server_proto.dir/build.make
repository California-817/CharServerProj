# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/xxxten/ChatProj/CharServerProj/ChatServer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/xxxten/ChatProj/CharServerProj/ChatServer/build

# Include any dependencies generated for this target.
include CMakeFiles/status_server_proto.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/status_server_proto.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/status_server_proto.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/status_server_proto.dir/flags.make

CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.o: CMakeFiles/status_server_proto.dir/flags.make
CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.o: /home/xxxten/ChatProj/CharServerProj/ChatServer/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc
CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.o: CMakeFiles/status_server_proto.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xxxten/ChatProj/CharServerProj/ChatServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.o -MF CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.o.d -o CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.o -c /home/xxxten/ChatProj/CharServerProj/ChatServer/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc

CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xxxten/ChatProj/CharServerProj/ChatServer/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc > CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.i

CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xxxten/ChatProj/CharServerProj/ChatServer/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc -o CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.s

CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.o: CMakeFiles/status_server_proto.dir/flags.make
CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.o: /home/xxxten/ChatProj/CharServerProj/ChatServer/include/grpc_cpp_out/GateServer.Status.pb.cc
CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.o: CMakeFiles/status_server_proto.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/xxxten/ChatProj/CharServerProj/ChatServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.o -MF CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.o.d -o CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.o -c /home/xxxten/ChatProj/CharServerProj/ChatServer/include/grpc_cpp_out/GateServer.Status.pb.cc

CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/xxxten/ChatProj/CharServerProj/ChatServer/include/grpc_cpp_out/GateServer.Status.pb.cc > CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.i

CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/xxxten/ChatProj/CharServerProj/ChatServer/include/grpc_cpp_out/GateServer.Status.pb.cc -o CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.s

# Object files for target status_server_proto
status_server_proto_OBJECTS = \
"CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.o" \
"CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.o"

# External object files for target status_server_proto
status_server_proto_EXTERNAL_OBJECTS =

libstatus_server_proto.a: CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.grpc.pb.cc.o
libstatus_server_proto.a: CMakeFiles/status_server_proto.dir/include/grpc_cpp_out/GateServer.Status.pb.cc.o
libstatus_server_proto.a: CMakeFiles/status_server_proto.dir/build.make
libstatus_server_proto.a: CMakeFiles/status_server_proto.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/xxxten/ChatProj/CharServerProj/ChatServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libstatus_server_proto.a"
	$(CMAKE_COMMAND) -P CMakeFiles/status_server_proto.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/status_server_proto.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/status_server_proto.dir/build: libstatus_server_proto.a
.PHONY : CMakeFiles/status_server_proto.dir/build

CMakeFiles/status_server_proto.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/status_server_proto.dir/cmake_clean.cmake
.PHONY : CMakeFiles/status_server_proto.dir/clean

CMakeFiles/status_server_proto.dir/depend:
	cd /home/xxxten/ChatProj/CharServerProj/ChatServer/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/xxxten/ChatProj/CharServerProj/ChatServer /home/xxxten/ChatProj/CharServerProj/ChatServer /home/xxxten/ChatProj/CharServerProj/ChatServer/build /home/xxxten/ChatProj/CharServerProj/ChatServer/build /home/xxxten/ChatProj/CharServerProj/ChatServer/build/CMakeFiles/status_server_proto.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/status_server_proto.dir/depend

