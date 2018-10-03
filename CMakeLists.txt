make_minimum_required(VERSION 3.6)

project(abacus_hash)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(bin_name "abacus_hash")

set(src_list
	src/example.cpp
	include/abacus_hash.hpp
	)

add_executable(${bin_name} ${src_list})
