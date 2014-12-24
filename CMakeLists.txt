cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Wall -std=c++11 -Wno-reorder")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(OpenCV COMPONENTS core highgui REQUIRED)

add_subdirectory(fractal_gen)
add_subdirectory(visualize)


