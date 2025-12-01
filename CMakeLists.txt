# Copyright 2019 Mike Dev
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.21...4.2)
project(boost_any VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)

if(BOOST_USE_MODULES)
    add_library(boost_any)
    target_sources(
        boost_any
        PUBLIC
            FILE_SET modules_public
                TYPE CXX_MODULES
                FILES ${CMAKE_CURRENT_LIST_DIR}/modules/boost_any.cppm
    )

    target_compile_features(boost_any PUBLIC cxx_std_20)
    target_compile_definitions(boost_any PUBLIC BOOST_USE_MODULES)
    if(${CMAKE_CXX_STANDARD} IN_LIST CMAKE_CXX_COMPILER_IMPORT_STD)
        set(CMAKE_CXX_MODULE_STD ON)
        target_compile_definitions(boost_any PRIVATE BOOST_ANY_USE_STD_MODULE)
        message(STATUS "Using `import std;`")
    else()
        message(WARNING "`import std;` is not available")
    endif()
    set(__scope PUBLIC)

    # TODO(CK):
    # add_executable(module_sample modules/usage_sample.cpp)
    # (module_sample PRIVATE boost_any)
else()
    set(CMAKE_VERIFY_INTERFACE_HEADER_SETS ON)
    add_library(boost_any INTERFACE)
    set(__scope INTERFACE)
endif()

if(CMAKE_SKIP_INSTALL_RULES OR PROJECT_IS_TOP_LEVEL)
    target_sources(
        boost_any
        ${__scope}
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

target_link_libraries(
    boost_any
    ${__scope}
    Boost::config
    Boost::throw_exception
    Boost::type_index
)

add_library(Boost::any ALIAS boost_any)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
