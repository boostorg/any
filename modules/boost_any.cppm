// Copyright Antony Polukhin, 2016-2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// To compile manually use a command like the following:
// clang++ -I ../include -std=c++20 --precompile -x c++-module any.cppm

// Global module fragment.
module;

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/throw_exception.hpp>

#include <memory> // for std::addressof

#ifndef BOOST_ANY_USE_STD_MODULE
#include <stdexcept>
#include <typeinfo>
#include <type_traits>
#include <utility>
#endif

// Begins the module purview
export module boost.any;

import boost.type_index;

#ifdef BOOST_ANY_USE_STD_MODULE
import std;
#endif

// We will use these macros when we get to compatibility headers
#define BOOST_ANY_INTERFACE_UNIT
#define BOOST_IN_MODULE_PURVIEW

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

// Include the entire library
#include <boost/any.hpp>
#include <boost/any/basic_any.hpp>
#include <boost/any/unique_any.hpp>

