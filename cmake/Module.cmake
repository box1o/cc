
function(add_module module_name)
    set(options)
    set(one_value_args)
    set(multi_value_args SOURCES HEADERS DEPENDENCIES)
    cmake_parse_arguments(ARG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    # Determine library type (static/shared)
    if(MODULE_LIB_TYPE)
        set(lib_type SHARED)
    else()
        set(lib_type STATIC)
    endif()

    # Define target
    set(target_name cc_${module_name})
    add_library(${target_name} ${lib_type} ${ARG_SOURCES} ${ARG_HEADERS})
    add_library(cc::${module_name} ALIAS ${target_name})

    # Register that a library target exists
    set_property(GLOBAL PROPERTY CC_HAS_LIBRARY_TARGETS TRUE)

    # Basic target metadata
    set_target_properties(${target_name} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME cc_${module_name}
        EXPORT_NAME ${module_name}
    )

    # Include directories
    target_include_directories(${target_name}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/cc/${module_name}>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    if(ARG_DEPENDENCIES)
        target_link_libraries(${target_name} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    target_compile_features(${target_name} PUBLIC cxx_std_23)
    # cc_set_target_options(${target_name})

endfunction()
