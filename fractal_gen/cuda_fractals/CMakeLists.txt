cmake_minimum_required(VERSION 2.8)

set(BUILD_SHARED_LIBS OFF)
cuda_add_library(cuda_fractals fractal3d.cu fractalgen3d.hpp)
target_link_libraries(cuda_fractals ${CUDA_LIBRARIES})

#add_library(ocl_fractals SHARED fractals.cpp)

#find_package(OpenCL REQUIRED)
#include_directories(${OPENCL_INCLUDE_DIRS})
#if(OPENCL_HAS_CPP_BINDINGS)
#    message("OpenCL has CPP bindings. Full include is: " ${OPENCL_INCLUDE_DIRS})
#else(OPENCL_HAS_CPP_BINDINGS)
#    message("No OpenCL CPP bindings found")
#endif(OPENCL_HAS_CPP_BINDINGS)

#target_link_libraries(ocl_fractals ${OPENCL_LIBRARIES} ${OpenCV_LIBS})
