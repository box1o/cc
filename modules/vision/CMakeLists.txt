cmake_minimum_required(VERSION 4.2.0)

project(cc_vision
    VERSION 1.0.0
    DESCRIPTION "computer vision module - camera calibration and tracking"
    LANGUAGES CXX
)

file(GLOB_RECURSE VISION_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

file(GLOB_RECURSE VISION_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)


find_package(OpenCV REQUIRED)

add_module(vision
    SOURCES
        ${VISION_SOURCES}
    HEADERS
        ${VISION_HEADERS}
    DEPENDENCIES
        cc::core
        cc::math
        ${OpenCV_LIBS}
)


target_include_directories(cc_vision
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
)

target_compile_definitions(cc_vision
    PRIVATE
        CC_VISION_VERSION="${PROJECT_VERSION}"
)
