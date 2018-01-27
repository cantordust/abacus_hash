cmake_minimum_required(VERSION 2.8)

project(abacus_hash)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(bin_name "abacus_hash")

set(src_list
	src/main.cpp
	src/AbacusHash.hpp
	)

add_executable(${bin_name} ${src_list})
