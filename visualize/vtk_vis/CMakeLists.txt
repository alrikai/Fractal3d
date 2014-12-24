cmake_minimum_required(VERSION 2.8)

add_library(meshvis EXCLUDE_FROM_ALL SHARED mesh_vis.cpp)

set(VTK_DIR "/usr/local/lib/vtk-5.10")
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
if(VTK_FOUND)
  message("VTK. Version:" ${VTK_VERSION}. VTK_DIR: ${VTK_DIR})
else(VTK_FOUND)
  message(FATAL_ERROR "CANNOT FIND VTK")
endif(VTK_FOUND)
target_link_libraries(meshvis 
	          ${OPENCL_LIBRARIES} 
	          ${OpenCV_LIBS}
						${PCL_COMMON_LIBRARIES}
	          ${PCL_SURFACE_LIBRARIES}
	 		      ${PCL_VISUALIZATION_LIBRARIES}
	 		      ${PCL_FEATURES_LIBRARIES}
				    ${VTK_LIBRARIES})
