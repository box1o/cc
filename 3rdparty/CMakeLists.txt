cmake_minimum_required(VERSION 4.2.0)

find_or_fetch_package(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.2.1
)

find_or_fetch_package(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)

find_or_fetch_package(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
)

add_subdirectory(glad)
add_subdirectory(stb)

find_or_fetch_package(spirv-cross
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
    GIT_TAG main 
)

find_or_fetch_package(shaderc
    GIT_REPOSITORY https://github.com/google/shaderc.git
    GIT_TAG main
    RECURSE_SUBMODULES
    DISABLE_INSTALL
    POST_FETCH_SCRIPT utils/git-sync-deps
    CMAKE_CACHE_VARS
        SHADERC_SKIP_TESTS=ON
        SHADERC_SKIP_INSTALL=ON
        SHADERC_SKIP_EXAMPLES=ON
        SHADERC_SKIP_COPYRIGHT_CHECK=ON
)
