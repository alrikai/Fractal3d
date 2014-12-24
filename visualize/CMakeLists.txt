cmake_minimum_required(VERSION 2.8)

find_package(PCL REQUIRED COMPONENTS common surface visualization features)
include_directories(${PCL_INCLUDE_DIRS})
add_definitions(${PCL_DEFINITIONS})
link_directories(${PCL_LIBRARY_DIRS})

add_subdirectory(ogre_vis)
add_subdirectory(vtk_vis)
