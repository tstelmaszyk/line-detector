cmake_minimum_required(VERSION 3.18)
project( line_detector )
set(CMAKE_CXX_STANDARD 17)

find_package( OpenCV REQUIRED )
message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable( line_detector main.cpp DetectLines.cpp RegionOfInterest.cpp)
target_link_libraries( line_detector ${OpenCV_LIBS} )
