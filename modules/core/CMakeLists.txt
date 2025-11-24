cmake_minimum_required(VERSION 4.2.0)

project(cc_core
    VERSION 1.0.0
    DESCRIPTION "core functionality: types, logging, error handling"
    LANGUAGES CXX
)

file(GLOB_RECURSE CORE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

file(GLOB_RECURSE CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_module(core
    HEADERS
        ${CORE_HEADERS}
    SOURCES
        ${CORE_SOURCES}
    DEPENDENCIES
        fmt
        spdlog
)

target_compile_features(cc_core PUBLIC cxx_std_23)

target_compile_definitions(cc_core
    PRIVATE
        CC_CORE_VERSION="${PROJECT_VERSION}"
)
