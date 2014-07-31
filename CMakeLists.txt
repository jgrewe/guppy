CMAKE_MINIMUM_REQUIRED( VERSION 2.4 )

PROJECT( guppy-gui )

set(LIBRARY_NAME        gupppy)
set(EXECUTABLE_NAME     guppy-gui)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} -std=c++11") ## Optimize
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O0 -g3 -Wall -c")

set(SRC_FILES           ../src/guppy.cpp)
set(INCLUDE_FILES       ../include/guppy.hpp)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include )

SET( EXECUTABLE_OUTPUT_PATH bin )

SET( LIBRARY_OUTPUT_PATH lib )

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

########## OpenCV  ##########
FIND_PACKAGE( OpenCV REQUIRED )
#INCLUDE( ${OpenCV_USE_FILE} )

##########  BOOST  ##########
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.49.0 REQUIRED date_time)
include_directories(${Boost_INCLUDE_DIR})
set (LINK_LIBS ${LINK_LIBS} ${Boost_LIBRARIES})

add_executable(
  ${EXECUTABLE_NAME}
  ../src/guppy-gui.cpp
  )

add_library(
  ${LIBRARY_NAME}
  SHARED
  ${SRC_FILES}
  ${INCLUDE_FILES}
  )

target_link_libraries(
  ${EXECUTABLE_NAME}
  ${LIBRARY_NAME}
  )

target_link_libraries(
  ${LIBRARY_NAME}
  )

#SET( EXECUTABLE_OUTPUT_PATH bin )

#SET( LIBRARY_OUTPUT_PATH lib )


#
########### OpenCV  ##########
#FIND_PACKAGE( OpenCV REQUIRED )
#INCLUDE( ${OpenCV_USE_FILE} )

##########  BOOST  ##########
#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#find_package(Boost 1.49.0 REQUIRED date_time)
#include_directories(${Boost_INCLUDE_DIR})
#set (LINK_LIBS ${LINK_LIBS} ${Boost_LIBRARIES})

#INCLUDE_DIRECTORIES( include )

#SET( lib_includes
#  ../include/guppy.hpp
#)

#SET( lib_src
#  ../src/guppy.cpp
#)

#SET ( exe_src
#  ../src/guppy-gui.cpp
#)

#ADD_LIBRARY( guppy  STATIC ../src/guppy.cpp )
#install(TARGETS guppy DESTINATION lib)

#add_executable(guppy-gui ${exe_src})
#target_link_libraries(guppy-gui guppy)