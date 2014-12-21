cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Wall -std=c++11 -Wno-reorder")
add_executable(ocl_fractals fractals.cpp)

#add_executable(ocl_fractals3dmesh fractals.cpp) 
#add_library(meshvis visualize/mesh_vis.cpp)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})
if( OPENCL_HAS_CPP_BINDINGS )
    message( "OpenCL has CPP bindings. Full include is: " ${OPENCL_INCLUDE_DIRS} )
else( OPENCL_HAS_CPP_BINDINGS )
    message( "No OpenCL CPP bindings found" )
endif( OPENCL_HAS_CPP_BINDINGS )

find_package(OpenCV COMPONENTS core highgui REQUIRED)

target_link_libraries(ocl_fractals ${OPENCL_LIBRARIES} ${OpenCV_LIBS})

find_package(PCL COMPONENTS common surface visualization features)
if (NOT PCL_COMMON_FOUND OR NOT PCL_SURFACE_FOUND OR NOT PCL_VISUALIZATION_FOUND OR NOT PCL_FEATURES_FOUND)
	message(status "PCL not found!")
else ()
  include_directories(${PCL_INCLUDE_DIRS})
  add_definitions(${PCL_DEFINITIONS})
#  link_directories(${PCL_LIBRARY_DIRS})

	#SET(VTK_LIB_D "/usr/local/lib/vtk-5.10")
	#link_directories(${VTK_LIB_D})
	#MESSAGE(STATUS "VTK LIBS: ${VTK_LIBS}")
	#target_link_libraries(ocl_fractals3d 
	#          ${OPENCL_LIBRARIES} 
	#          ${OpenCV_LIBS}
	#					  ${PCL_COMMON_LIBRARIES}
	#          ${PCL_SURFACE_LIBRARIES}
	#			      ${PCL_VISUALIZATION_LIBRARIES}
	#			      ${PCL_FEATURES_LIBRARIES}
	#			      ${VTK_LIBS})
endif()					


