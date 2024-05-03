cmake_minimum_required(VERSION 3.9...3.24 FATAL_ERROR)

project(ipc_primitives)

add_library(shared_memory
	./shared_memory.hpp
	./shared_memory.cpp
)

target_include_directories(shared_memory PUBLIC .)
target_compile_features(shared_memory PUBLIC cxx_std_14)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(shared_memory Threads::Threads)

if(UNIX AND NOT APPLE)
	target_link_libraries(shared_memory rt)
endif()

########################################

