# Copyright Antony Polukhin, 2016-2026
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.21...4.3)

project(boost_any LANGUAGES CXX VERSION 1.90.0)

add_subdirectory(../../../assert boostorg/assert)
add_subdirectory(../../../core boostorg/core)
add_subdirectory(../../../config boostorg/config)
add_subdirectory(../../../container_hash boostorg/container_hash)
add_subdirectory(../../../describe boostorg/describe)
add_subdirectory(../../../detail boostorg/detail)
add_subdirectory(../../../integer boostorg/integer)
add_subdirectory(../../../mp11 boostorg/mp11)
add_subdirectory(../../../preprocessor boostorg/preprocessor)
add_subdirectory(../../../throw_exception boostorg/throw_exception)
add_subdirectory(../../../type_index boostorg/type_index)
add_subdirectory(../../../type_traits boostorg/type_traits)

# ../../CMakeLists.txt
add_subdirectory(../../ boostorg/any)

include(cmake/boost-install-library.cmake)
boost_install_library(boost_config TARGETS boost_config)
boost_install_library(boost_assert TARGETS boost_assert)
boost_install_library(boost_mp11 TARGETS boost_mp11 DEPENDENCIES Boost::core)
boost_install_library(boost_describe TARGETS boost_describe DEPENDENCIES Boost::mp11)
boost_install_library(boost_container_hash TARGETS boost_container_hash DEPENDENCIES Boost::describe)
boost_install_library(boost_throw_exception TARGETS boost_throw_exception DEPENDENCIES Boost::assert)
boost_install_library(boost_type_index TARGETS boost_type_index DEPENDENCIES Boost::boost_container_hash)
boost_install_library(boost_any TARGETS boost_any DEPENDENCIES Boost::type_index Boost::config Boost::throw_exception)

enable_testing()

if(BOOST_USE_MODULES)
    add_executable(boost_any_module_usage ../../modules/usage_sample.cpp)
    target_link_libraries(boost_any_module_usage PRIVATE Boost::any)
    add_test(NAME boost_any_module_usage COMMAND boost_any_module_usage)
else()
    set(RUN_TESTS_SOURCES
        any_test_mplif.cpp
        any_test_rv.cpp
        basic_any_test.cpp
        basic_any_test_large_object.cpp
        basic_any_test_mplif.cpp
        basic_any_test_rv.cpp
        basic_any_test_small_object.cpp
        unique_any/base.cpp
        unique_any/emplace.cpp
        unique_any/from_any.cpp
        unique_any/move.cpp
        # any_test.cpp  # Ambiguous with modules, because all the anys now available
    )
endif()

foreach(testsourcefile ${RUN_TESTS_SOURCES})
    if(NOT EXISTS ../${testsourcefile})
        message(WARNING "Missing ../${testsourcefile}")
        continue()
    endif()

    get_filename_component(testname ../${testsourcefile} NAME_WLE)
    add_executable(${PROJECT_NAME}_${testname} ../${testsourcefile})
    target_link_libraries(${PROJECT_NAME}_${testname} Boost::any Boost::core)
    add_test(NAME ${PROJECT_NAME}_${testname} COMMAND ${PROJECT_NAME}_${testname})
endforeach()

