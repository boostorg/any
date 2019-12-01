//  Unit test for boost::any.
//
//  See http://www.boost.org for most recent version, including documentation.
//
//  Copyright Antony Polukhin, 2013-2019.
//
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).

#include <cstdlib>
#include <string>
#include <utility>

#include <boost/any.hpp>
#include "test.hpp"

#define ANY_CAST_OP any_cast // Must be defined before including move_test

#include "move_test.hpp"
#include <boost/move/move.hpp>

#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES

int main() 
{
    return EXIT_SUCCESS;
}

#else 

int main()
{
    using namespace any_tests;

    typedef any_test_type<boost::any> test_type;
    tester<test_type::test_case_iterator> test_suite(test_type::begin, test_type::end);
    return test_suite() ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

