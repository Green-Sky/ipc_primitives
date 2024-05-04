cmake_minimum_required(VERSION 3.9...3.24 FATAL_ERROR)

project(ipc_primitives)

add_library(ipc_shared_memory
	./ipc/shared_memory.hpp
	./ipc/shared_memory.cpp
)

target_include_directories(ipc_shared_memory PUBLIC .)
target_compile_features(ipc_shared_memory PUBLIC cxx_std_14)

#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)
#target_link_libraries(ipc_shared_memory Threads::Threads)

if(UNIX AND NOT APPLE)
	target_link_libraries(ipc_shared_memory rt)
endif()

########################################

add_library(ipc_semaphore
	./ipc/semaphore.hpp
	./ipc/semaphore.cpp
)

target_include_directories(ipc_semaphore PUBLIC .)
target_compile_features(ipc_semaphore PUBLIC cxx_std_14)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(ipc_semaphore Threads::Threads)

#if(UNIX AND NOT APPLE)
	#target_link_libraries(ipc_shared_memory rt)
#endif()

########################################

