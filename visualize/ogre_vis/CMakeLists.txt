cmake_minimum_required(VERSION 2.8)

add_library(ogrevis SHARED ogre_vis.cpp)

if(EXISTS "/usr/local/lib/OGRE/cmake")
  set(CMAKE_MODULE_PATH "/usr/local/lib/OGRE/cmake/;${CMAKE_MODULE_PATH}")
  set(OGRE_SAMPLES_INCLUDEPATH "/usr/local/share/OGRE/samples/Common/include/") # We could just *assume* that developers uses this basepath : /usr/local
else ()
  message(SEND_ERROR "Failed to find module path.")
endif()
set(CMAKE_DEBUG_POSTFIX "_d")
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dist")

find_package(OGRE REQUIRED)
if(NOT OGRE_FOUND)
	message(SEND_ERROR "Failed to find OGRE")
else()
    message(STATUS "Found OGRE Libs: ${OGRE_LIBRARIES}")
endif()
find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} REQUIRED)

find_package(OIS REQUIRED)
if(NOT OIS_FOUND)
	message(SEND_ERROR "Failed to find OIS.")
endif()

include_directories(${OIS_INCLUDE_DIRS})
include_directories(${OGRE_INCLUDE_DIRS})
include_directories(${OGRE_SAMPLES_INCLUDEPATH})
include_directories(${OGRE_Overlay_INCLUDE_DIRS})

include_directories("/home/alrik/TowerDefense")
set (controller_lib "/home/alrik/TowerDefense/build/lib/libController.so")
add_executable(ogrevis ${HDRS} ${SRCS})
set_target_properties(ogrevis PROPERTIES DEBUG_POSTFIX _d)
target_link_libraries(ogrevis 
	${controller_lib}
	${OpenCV_LIBS}
 	${PCL_COMMON_LIBRARIES}
	${PCL_SURFACE_LIBRARIES}
	${OGRE_LIBRARIES} 
	${OIS_LIBRARIES} 
	${OGRE_Overlay_LIBRARIES})
 
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/bin)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/media)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/dist/bin)
