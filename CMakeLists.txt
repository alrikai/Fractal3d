cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Wall -std=c++11 -fPIC -Wno-reorder")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
find_package(OpenCV COMPONENTS core highgui REQUIRED)

find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})
if(OPENCL_HAS_CPP_BINDINGS)
    message("OpenCL has CPP bindings. Full include is: " ${OPENCL_INCLUDE_DIRS})
else(OPENCL_HAS_CPP_BINDINGS)
    message("No OpenCL CPP bindings found")
endif(OPENCL_HAS_CPP_BINDINGS)


add_subdirectory(fractal_gen)
add_subdirectory(visualize)

set (fractal_src fractal_main.cpp)
add_executable(oclogre_fractals ${fractal_src}) 
target_link_libraries(oclogre_fractals ${OPENCL_LIBRARIES} ogrevis)
