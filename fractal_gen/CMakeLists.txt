cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/cpu_fractals")

add_subdirectory(ocl_fractals)
add_subdirectory(cuda_fractals)
