cmake_minimum_required(VERSION 3.18)
project( line_detector )
set(CMAKE_CXX_STANDARD 17)

find_package( OpenCV REQUIRED )
message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")
include_directories( ${OpenCV_INCLUDE_DIRS} )

# Sources partagees entre l'executable et les tests (hors main.cpp).
# On y ajoute chaque nouveau .cpp au fil des tasks.
set(LANE_SOURCES
    DetectLines.cpp
    RegionOfInterest.cpp
    DiskImageSink.cpp
)

add_executable( line_detector main.cpp ${LANE_SOURCES} )
target_link_libraries( line_detector ${OpenCV_LIBS} )

# --- Tests unitaires (doctest) ---
enable_testing()
file(GLOB TEST_SOURCES tests/test_*.cpp)
add_executable( line_detector_tests ${TEST_SOURCES} ${LANE_SOURCES} )
target_include_directories( line_detector_tests PRIVATE ${CMAKE_SOURCE_DIR} tests )
target_link_libraries( line_detector_tests ${OpenCV_LIBS} )
