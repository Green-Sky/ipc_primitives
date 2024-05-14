cmake_minimum_required(VERSION 3.9...3.24 FATAL_ERROR)

project(OffsetAllocator)

add_library(offset_allocator
	./OffsetAllocator/offsetAllocator.hpp
	./OffsetAllocator/offsetAllocator.cpp
)

target_include_directories(offset_allocator PUBLIC .)
target_compile_features(offset_allocator PUBLIC cxx_std_11)

