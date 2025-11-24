cmake_minimum_required(VERSION 4.2.0)

project(cc_math
    VERSION 1.0.0
    DESCRIPTION "Mathematics and linear algebra module for cc"
    LANGUAGES CXX
)


file(GLOB_RECURSE MATH_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

file(GLOB_RECURSE MATH_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)


add_module(math
    SOURCES
        ${MATH_SOURCES}
    HEADERS
        ${MATH_HEADERS}
    DEPENDENCIES
    
)


target_compile_definitions(cc_math
    PRIVATE
        CC_MATH_VERSION="${PROJECT_VERSION}"
)


