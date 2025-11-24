cmake_minimum_required(VERSION 4.2.0)

set(STUDIO_NAME studio)
set(STUDIO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(STUDIO_RES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(STUDIO_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(STUDIO_RES_OUT_DIR ${STUDIO_BIN_DIR}/resources)

project(${STUDIO_NAME}
    VERSION ${TARGET_VERSION}
    DESCRIPTION "studio application"
    LANGUAGES CXX
)

file(GLOB_RECURSE STUDIO_HEADERS
    "${STUDIO_SRC_DIR}/*.h"
    "${STUDIO_SRC_DIR}/*.hpp"
)

file(GLOB_RECURSE STUDIO_SOURCES
    "${STUDIO_SRC_DIR}/*.cpp"
)

add_executable(${STUDIO_NAME}
    ${STUDIO_HEADERS}
    ${STUDIO_SOURCES}
)


target_link_libraries(${STUDIO_NAME}
    PRIVATE 
        cc::core
        cc::math
        cc::gfx
)

target_include_directories(${STUDIO_NAME}
    PRIVATE
        ${STUDIO_SRC_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OpenCV_INCLUDE_DIRS}
)

target_compile_features(${STUDIO_NAME} PRIVATE cxx_std_23)

target_compile_definitions(${STUDIO_NAME} PRIVATE
    STUDIO_VERSION="${PROJECT_VERSION}"
    ROOT_PATH="${ROOT_DIR}"
)

set_target_properties(${STUDIO_NAME} PROPERTIES
    OUTPUT_NAME "studio"
    RUNTIME_OUTPUT_DIRECTORY ${STUDIO_BIN_DIR}
)

if(WIN32)
    set_target_properties(${STUDIO_NAME} PROPERTIES WIN32_EXECUTABLE ON)
elseif(APPLE)
    set_target_properties(${STUDIO_NAME} PROPERTIES MACOSX_BUNDLE ON)
endif()

install(TARGETS ${STUDIO_NAME} 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(EXISTS ${STUDIO_RES_DIR})
    install(DIRECTORY ${STUDIO_RES_DIR}
        DESTINATION ${CMAKE_INSTALL_DATADIR}/cc
    )

    add_custom_command(
        TARGET ${STUDIO_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${STUDIO_RES_DIR}
            ${STUDIO_RES_OUT_DIR}
        COMMENT "Copying resources to ${STUDIO_RES_OUT_DIR}"
    )
endif()
