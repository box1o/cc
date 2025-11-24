cmake_minimum_required(VERSION 4.2.0)

project(cc_io
    VERSION 1.0.0
    DESCRIPTION "io module"
    LANGUAGES CXX
)

file(GLOB_RECURSE IO_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

file(GLOB_RECURSE IO_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_module(io
    SOURCES
        ${IO_SOURCES}
    HEADERS
        ${IO_HEADERS}
    DEPENDENCIES
        cc::core
        cc::math
)

target_compile_definitions(cc_io
    PRIVATE
        CC_IO_VERSION="${PROJECT_VERSION}"
)
