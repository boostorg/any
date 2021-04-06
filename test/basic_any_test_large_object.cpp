// Copyright Ruslan Arutyunyan, 2019-2021.
// Copyright Antony Polukhin, 2021.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/anys/basic_any.hpp>

#include <boost/core/lightweight_test.hpp>

#include <cassert>
#include <iostream>

static int move_ctors_count = 0;
static int destructors_count = 0;

struct A {
    char a[32];

    A() {}
    A(const A&) {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    A(A&&) BOOST_NOEXCEPT {
        ++move_ctors_count;
    }
#endif

    ~A() {
        ++destructors_count;
    }
};

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    template <class T>
    T&& portable_move(T& value) {
        return std::move(value);
    }
#else
    template <class T>
    T& portable_move(T& value) {
        return value;
    }
#endif

int main() {
    {
        A a;
        boost::anys::basic_any<24, 8> any1(a);
        // to the same memory object
        boost::anys::basic_any<24, 8> any2(portable_move(any1));
        boost::anys::basic_any<24, 8> any3(portable_move(any2));
        BOOST_TEST_EQ(move_ctors_count, 0);
    }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    BOOST_TEST_EQ(destructors_count, 2);
#else
    BOOST_TEST_EQ(destructors_count, 4);
#endif

    destructors_count = 0;

    {
        A a;
        boost::anys::basic_any<32, 8> any1(a);
        // to smaller object
        boost::anys::basic_any<16, 8> any2(portable_move(any1));
        boost::anys::basic_any<16, 8> any3(portable_move(any2));

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
        BOOST_TEST_EQ(move_ctors_count, 1);
#endif
    }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    BOOST_TEST_EQ(destructors_count, 3);
#endif
    return boost::report_errors();
}
