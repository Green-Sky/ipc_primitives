cmake_minimum_required(VERSION 3.9...3.24 FATAL_ERROR)

project(ipc_primitives)

add_executable(shared_memory_test1
	test1.cpp
)
target_compile_features(shared_memory_test1 PUBLIC cxx_std_14)
target_link_libraries(shared_memory_test1 PRIVATE shared_memory)

add_test(NAME shared_memory_test1 COMMAND shared_memory_test1)

########################################

