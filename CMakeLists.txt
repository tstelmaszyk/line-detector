cmake_minimum_required(VERSION 3.18)
project( test_project )
set(CMAKE_CXX_STANDARD 17)

find_package( OpenCV REQUIRED )
message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")
include_directories( ${OpenCV_INCLUDE_DIRS} )


find_package(PkgConfig)
pkg_check_modules(LIBCAMERA REQUIRED IMPORTED_TARGET libcamera)
message(STATUS "libcamera library found:")
message(STATUS "    version: ${LIBCAMERA_VERSION}")
message(STATUS "    libraries: ${LIBCAMERA_LINK_LIBRARIES}")
message(STATUS "    include path: ${LIBCAMERA_INCLUDE_DIRS}")
include_directories( ${LIBCAMERA_INCLUDE_DIRS} )

add_executable( test main.cpp DetectLines.cpp RegionOfInterest.cpp)
target_link_libraries( test ${OpenCV_LIBS} ${LIBCAMERA_LINK_LIBRARIES} )
set_target_properties(test PROPERTIES 
VS_DEBUGGER_COMMAND_ARGUMENTS "lena.png")
