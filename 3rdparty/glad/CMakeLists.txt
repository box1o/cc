cmake_minimum_required(VERSION 4.2.0)

project(glad
    VERSION 0.1.6
    DESCRIPTION "Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator"
    LANGUAGES C
)

add_library(glad STATIC
    src/glad.c
)

add_library(glad::glad ALIAS glad)

target_include_directories(glad
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(glad PUBLIC c_std_99)

set_target_properties(glad PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME glad
    EXPORT_NAME glad
    POSITION_INDEPENDENT_CODE ON
)

if(UNIX AND NOT APPLE)
    target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})
endif()
