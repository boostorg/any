// Copyright Ruslan Arutyunyan, 2019-2021.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/basic_any.hpp>

#include <cassert>
#include <iostream>

static bool test_flag = false;
static int move_ctors_count{};
static int destructors_count{};

struct A {
    char a[32];
    A() = default;
    A(const A&) {}
    A(A&&) noexcept {
        ++move_ctors_count;
    }
    ~A() {
        ++destructors_count;
    }
};

int main() {
    {
        A a;
        boost::basic_any<24, 8> any1(a);
        // to the same memory object
        boost::basic_any<24, 8> any2(std::move(any1));
        boost::basic_any<24, 8> any3(std::move(any2));
        assert(move_ctors_count == 0);
    }

    assert(destructors_count == 2);

    destructors_count = 0;

    {
        A a;
        boost::basic_any<32, 8> any1(a);
        // to smaller object
        boost::basic_any<16, 8> any2(std::move(any1));
        boost::basic_any<16, 8> any3(std::move(any2));
        assert(move_ctors_count == 1);
    }

    assert(destructors_count == 3);
}
