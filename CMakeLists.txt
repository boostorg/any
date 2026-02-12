# -----------------------------------------------------------------------------
# Boost.Any CMake
# Handles: no modules, modules, modules + import std;
# -----------------------------------------------------------------------------
# Copyright 2019 Mike Dev
# Distributed under the Boost Software License, Version 1.0.
# -----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.21...4.2)

project(boost_any VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)

if(PROJECT_IS_TOP_LEVEL)
    find_package(Boost 1.90.0 CONFIG)
endif()

# -----------------------------------------------------------------------------
# User option: enable C++ modules
# -----------------------------------------------------------------------------
option(BOOST_USE_MODULES "Build Boost using C++ modules" OFF)

# -----------------------------------------------------------------------------
# Determine target type and sources
# -----------------------------------------------------------------------------
if(BOOST_USE_MODULES)

    # Ensure CMAKE_CXX_STANDARD is set for module detection
    if(NOT DEFINED CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 20)
    endif()

    add_library(boost_any)

    target_sources(
        boost_any
        PUBLIC
            FILE_SET modules_public
                TYPE CXX_MODULES
                FILES modules/boost_any.cppm
    )

    # Require C++20 for modules
    target_compile_features(boost_any PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

    # Define macro indicating modules usage
    target_compile_definitions(boost_any PUBLIC BOOST_USE_MODULES)

    # Check if import std; is available for the current standard
    if(${CMAKE_CXX_STANDARD} IN_LIST CMAKE_CXX_COMPILER_IMPORT_STD)
        target_compile_definitions(boost_any PUBLIC BOOST_ANY_USE_STD_MODULE)
        set_propery(TARGET boost_any CXX_MODULE_STD ON)
        message(STATUS "Boost.Any: Using `import std;`")
    else()
        message(WARNING "Boost.Any: `import std;` is not available for C++${CMAKE_CXX_STANDARD}")
    endif()

    set(__scope PUBLIC)

else()

    # Modules disabled -> INTERFACE library
    add_library(boost_any INTERFACE)

    # If modules are disabled, require C++17 for headers
    target_compile_features(boost_any INTERFACE cxx_std_17)

    # Verify interface headers only at top level
    if(PROJECT_IS_TOP_LEVEL)
        set(CMAKE_VERIFY_INTERFACE_HEADER_SETS ON)
    endif()

    set(__scope INTERFACE)

endif()

# -----------------------------------------------------------------------------
# Include headers
# -----------------------------------------------------------------------------
if(BOOST_USE_MODULES)
    target_sources(
        boost_any
        PUBLIC
            FILE_SET headers_public
                TYPE HEADERS
                BASE_DIRS include
                FILES
                    include/boost/any/bad_any_cast.hpp
                    include/boost/any/basic_any.hpp
                    include/boost/any/fwd.hpp
                    include/boost/any/unique_any.hpp
                    include/boost/any/detail/config.hpp
                    include/boost/any/detail/placeholder.hpp
    )
else()
    target_include_directories(boost_any ${__scope} include)
endif()

# -----------------------------------------------------------------------------
# Link dependencies
# -----------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL)
    target_link_libraries(boost_any ${__scope} Boost::headers)
else()
    target_link_libraries(boost_any
       ${__scope}
        Boost::config
        Boost::throw_exception
        Boost::type_index
    )
endif()

# Alias for convenient import
add_library(Boost::any ALIAS boost_any)

# -----------------------------------------------------------------------------
# Testing
# -----------------------------------------------------------------------------
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
