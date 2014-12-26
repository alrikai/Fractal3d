cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Wall -std=c++11 -Wno-reorder")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
find_package(OpenCV COMPONENTS core highgui REQUIRED)

add_subdirectory(fractal_gen)
add_subdirectory(visualize)

set (fractal_src fractal_main.cpp)
add_executable(oclogre_fractals ${fractal_src}) 
target_link_libraries(oclogre_fractals ocl_fractals ogrevis)
