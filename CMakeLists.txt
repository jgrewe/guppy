CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

PROJECT( guppy-gui )

set(EXECUTABLE_NAME guppy-gui)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O0 -g3 -Wall")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(SRC_FILES  src/guppy.cpp)
set(SRC_FILES  src/movieWriter.cpp)
set(SRC_FILES  src/guppy-gui.cpp)

include_directories( include )

########## OpenCV  ##########
find_package( OpenCV REQUIRED opencv_highgui opencv_core)
#INCLUDE( ${OpenCV_USE_FILE} )
set (LINK_LIBS ${LINK_LIBS} ${OpenCV_LIBRARIES})

##########  BOOST  ##########
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.49.0 REQUIRED date_time program_options)
include_directories(${Boost_INCLUDE_DIR})
set (LINK_LIBS ${LINK_LIBS} ${Boost_LIBRARIES})


##nix
find_package(NIX REQUIRED)
set (LINK_LIBS ${LINK_LIBS} ${NIX_LIBRARIES})

add_executable( guppy-gui src/guppy-gui.cpp src/movieWriter.cpp src/guppy.cpp )

target_link_libraries( guppy-gui ${LINK_LIBS})