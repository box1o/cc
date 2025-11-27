cmake_minimum_required(VERSION 4.2.0)

project(cc_gfx
    VERSION 1.0.0
    DESCRIPTION "Graphics rendering and visualization"
    LANGUAGES C CXX
)

file(GLOB_RECURSE GFX_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/backends/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/backends/*.hpp"
)

file(GLOB_RECURSE GFX_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/backends/*.cpp"
)

add_module(gfx
    SOURCES
        ${GFX_SOURCES}
    HEADERS
        ${GFX_HEADERS}
    DEPENDENCIES
        cc::core
        cc::math
        glfw
        glad
        stb::stb
        shaderc_combined
        glslang
        SPIRV-Tools
        SPIRV-Tools-opt
        spirv-cross-cpp 
        spirv-cross-glsl 
        spirv-cross-hlsl 
        spirv-cross-msl 
        spirv-cross-reflect
)

target_include_directories(cc_gfx
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(cc_gfx
    PRIVATE
        CC_GFX_VERSION="${PROJECT_VERSION}"
        CC_GFX_BACKEND_GL=1
        CC_GFX_BACKEND_VK=0
        CC_GFX_BACKEND_MTL=0
)
