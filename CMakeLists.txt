# Copyright 2019 Mike Dev
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.8...4.4)
project( boost_any VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX )

if(BOOST_USE_MODULES)
    add_library(boost_any STATIC)
    target_sources(boost_any PUBLIC
        FILE_SET CXX_MODULES
            BASE_DIRS modules
            FILES modules/boost_any.cppm
    )

    target_compile_features(boost_any PUBLIC cxx_std_20)
    target_compile_definitions(boost_any PUBLIC BOOST_USE_MODULES)
    if(NOT CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 20)
    endif()
    if(CMAKE_CXX_MODULE_STD AND CMAKE_CXX_STANDARD IN_LIST CMAKE_CXX_COMPILER_IMPORT_STD)
        target_compile_definitions(boost_any PUBLIC BOOST_ANY_USE_STD_MODULE)
        target_compile_features(boost_any PUBLIC cxx_std_23)
        message(STATUS "Using `import std;`")
    else()
        message(STATUS "`import std;` is not available")
    endif()
    set(__scope PUBLIC)
else()
    add_library(boost_any INTERFACE)
    set(__scope INTERFACE)
endif()

target_include_directories(boost_any ${__scope} include)
target_link_libraries( boost_any
    ${__scope}
        Boost::config
        Boost::throw_exception
        Boost::type_index
)

add_library( Boost::any ALIAS boost_any )

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
