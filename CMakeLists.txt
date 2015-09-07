cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Wall -std=c++11 -fPIC -Wno-reorder")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
find_package(OpenCV COMPONENTS core highgui REQUIRED)

#--------------------------------------------------------------------------------
#TODO: do compile-time branching based on what the system supports -- e.g. try
#      CUDA, then OpenCL, then CPU as last resort
#--------------------------------------------------------------------------------
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})
if(OPENCL_HAS_CPP_BINDINGS)
    message("OpenCL has CPP bindings. Full include is: " ${OPENCL_INCLUDE_DIRS})
else(OPENCL_HAS_CPP_BINDINGS)
    message("No OpenCL CPP bindings found")
endif(OPENCL_HAS_CPP_BINDINGS)

set(OCLKERNEL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/fractal_gen/ocl_fractals/kernels/")
add_definitions(-DOCLKERNEL_FILEPATH=${OCLKERNEL_PATH})

#--------------------------------------------------------------------------------

find_package(CUDA REQUIRED)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -std=c++11 -O3 -gencode arch=compute_30,code=sm_30)

#--------------------------------------------------------------------------------

add_subdirectory(fractal_gen)
add_subdirectory(visualize)

set (cpufractal_src cpufractal_main.cpp)
add_executable(cpuogre_fractals ${cpufractal_src}) 
target_link_libraries(cpuogre_fractals ogrevis)


set (oclfractal_src oclfractal_main.cpp)
add_executable(oclogre_fractals ${oclfractal_src}) 
target_link_libraries(oclogre_fractals ${OPENCL_LIBRARIES} ogrevis)

set (cudafractal_src cudafractal_main.cpp)
add_executable(cudaogre_fractals ${cudafractal_src}) 
target_link_libraries(cudaogre_fractals cuda_fractals ${CUDA_LIBRARIES} ogrevis)

