cmake_minimum_required(VERSION 3.18)
project( line_detector )
set(CMAKE_CXX_STANDARD 17)

find_package( OpenCV REQUIRED )
message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")

# --- Bibliotheque : pipeline de detection ---
# Rien ici ne lit argv, n'ecrit sur stdout, ni ne possede de boucle de capture.
set(LANE_SOURCES
    DetectLines.cpp
    DiskImageSink.cpp
    LanePolynomial.cpp
    LaneGeometry.cpp
    LaneMask.cpp
    PerspectiveView.cpp
    SlidingWindowSearch.cpp
    LaneOverlay.cpp
)

add_library( line_detector_lib STATIC ${LANE_SOURCES} )
target_include_directories( line_detector_lib PUBLIC ${CMAKE_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS} )
target_link_libraries( line_detector_lib PUBLIC ${OpenCV_LIBS} )

# --- Bibliotheque applicative : capture, boucle, sorties dev/test ---
set(APP_SOURCES
    AnnotatedVideoWriter.cpp
    CaptureFrameSource.cpp
    CliOptions.cpp
    LaneModelLogger.cpp
    ResultImageWriter.cpp
    StillImageFrameSource.cpp
)

add_library( line_detector_app STATIC ${APP_SOURCES} )
target_include_directories( line_detector_app PUBLIC ${CMAKE_SOURCE_DIR} )
target_link_libraries( line_detector_app PUBLIC line_detector_lib )
target_compile_options( line_detector_app PRIVATE -Wall -Wextra )

# --- Executable ---
add_executable( line_detector main.cpp )
target_link_libraries( line_detector PRIVATE line_detector_app )

# --- Tests unitaires (doctest) ---
enable_testing()
file(GLOB TEST_SOURCES tests/test_*.cpp)
add_executable( line_detector_tests ${TEST_SOURCES} )
target_include_directories( line_detector_tests PRIVATE ${CMAKE_SOURCE_DIR} tests )
target_link_libraries( line_detector_tests PRIVATE line_detector_app )
