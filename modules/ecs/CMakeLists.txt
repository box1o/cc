cmake_minimum_required(VERSION 4.2.0)

project(cc_ecs
    VERSION 0.1.0
    DESCRIPTION "Entity Component System module for cc"
    LANGUAGES CXX
)

file(GLOB_RECURSE ECS_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

file(GLOB_RECURSE ECS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_module(ecs
    SOURCES
        ${ECS_SOURCES}
    HEADERS
        ${ECS_HEADERS}
    DEPENDENCIES
        cc::core
)

target_compile_definitions(cc_ecs
    PRIVATE
        CC_ECS_VERSION="${PROJECT_VERSION}"
)
