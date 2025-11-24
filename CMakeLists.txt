cmake_minimum_required(VERSION 4.2.0)

set(TARGET_VERSION 0.0.1)

project(cc
    VERSION ${TARGET_VERSION}
    DESCRIPTION "Computer vision and motion tracking library"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules)
set(BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})





include(Module)
include(FindOrAdd)


#╔════════════════════════════════════════════════════════════════════════╗
#║                              Options                                   ║
#╚════════════════════════════════════════════════════════════════════════╝

option(MODULE_LIB_TYPE "Build modules STATIC or SHARED ON-SHARED" ON)




message(STATUS "")
message(STATUS "╔═══════════════════════════════════════════════════════════════╗")
message(STATUS "║                cc Configuration Summary                       ║")
message(STATUS "╚═══════════════════════════════════════════════════════════════╝")
message(STATUS "  Version:              ${PROJECT_VERSION}")
message(STATUS "  Build type:           ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  C++ standard:         C++${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "  Build options:")
message(STATUS "    Shared libraries:   ${MODULE_LIB_TYPE}")
message(STATUS "")
message(STATUS "  Install prefix:       ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")


add_subdirectory(3rdparty)


# NOTE: Modules
set(MODULES_DIR "${CMAKE_SOURCE_DIR}/modules")

# Get all folders in that directory
file(GLOB CHILD_FOLDERS RELATIVE "${MODULES_DIR}" "${MODULES_DIR}/*")

# Iterate and add as subdirectory
foreach(FOLDER ${CHILD_FOLDERS})
    if(IS_DIRECTORY "${MODULES_DIR}/${FOLDER}")
        message("Mmodule ../${FOLDER}")
        add_subdirectory("${MODULES_DIR}/${FOLDER}" "${CMAKE_BINARY_DIR}/${FOLDER}")
    endif()
endforeach()




# NOTE: Main application
add_subdirectory(studio)




