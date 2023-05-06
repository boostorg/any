// Copyright Antony Polukhin, 2013-2023.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/any/unique_any.hpp>

#include <boost/core/lightweight_test.hpp>

#include <vector>

void test_basic() {
    boost::anys::unique_any a;
    BOOST_TEST(!a.has_value());

    auto& i = a.emplace<int>(42);
    BOOST_TEST_EQ(i, 42);
    BOOST_TEST(a.has_value());
    BOOST_TEST_EQ(boost::any_cast<int>(a), 42);
    BOOST_TEST_EQ(boost::anys::any_cast<int>(a), 42);
    BOOST_TEST_EQ(boost::any_cast<int&>(a), 42);
    BOOST_TEST_EQ(boost::anys::any_cast<int&>(a), 42);

    boost::anys::unique_any b = std::move(a);
    BOOST_TEST(!a.has_value());
    BOOST_TEST(b.has_value());
    BOOST_TEST_EQ(&boost::any_cast<int&>(b), &i);
    BOOST_TEST_EQ(i, 42);

    i = 43;
    BOOST_TEST_EQ(boost::any_cast<int>(b), 43);
    BOOST_TEST_EQ(boost::anys::any_cast<int>(b), 43);

    b.reset();
    BOOST_TEST(!b.has_value());
}

struct counting_destroy {
    static int destructor_called;

    ~counting_destroy() {
      ++destructor_called;
    }
};

int counting_destroy::destructor_called = 0;

void test_destructor() {
    {
        boost::anys::unique_any a;
        BOOST_TEST_EQ(counting_destroy::destructor_called, 0);

        a.emplace<counting_destroy>();
        BOOST_TEST_EQ(counting_destroy::destructor_called, 0);

        a.reset();
        BOOST_TEST_EQ(counting_destroy::destructor_called, 1);
    }
    BOOST_TEST_EQ(counting_destroy::destructor_called, 1);

    {
        boost::anys::unique_any a = counting_destroy{};
        BOOST_TEST_EQ(counting_destroy::destructor_called, 2);

        a.emplace<counting_destroy>();
        BOOST_TEST_EQ(counting_destroy::destructor_called, 3);

        boost::anys::unique_any b;
        b = std::move(a);
        BOOST_TEST_EQ(counting_destroy::destructor_called, 3);

        b.reset();
        BOOST_TEST_EQ(counting_destroy::destructor_called, 4);
    }
    BOOST_TEST_EQ(counting_destroy::destructor_called, 4);

    {
        boost::anys::unique_any a = counting_destroy{};
        BOOST_TEST_EQ(counting_destroy::destructor_called, 5);
    }
    BOOST_TEST_EQ(counting_destroy::destructor_called, 6);
}

void test_swap() {
    boost::anys::unique_any a;
    boost::anys::unique_any b;

    a = 42;
    std::vector<int> ethalon_vec{1, 2, 3, 4, 5};
    b = ethalon_vec;

    auto& int_ref = boost::any_cast<int&>(a);
    auto& vec_ref = boost::any_cast<std::vector<int>&>(b);

    swap(a, b);
    BOOST_TEST(a.has_value());
    BOOST_TEST(b.has_value());

    BOOST_TEST_EQ(boost::any_cast<int>(b), 42);
    BOOST_TEST(boost::any_cast<std::vector<int>>(a) == ethalon_vec);

    BOOST_TEST_EQ(&boost::any_cast<std::vector<int>&>(a), &vec_ref);
    BOOST_TEST_EQ(&boost::any_cast<int&>(b), &int_ref);
}

int main() {
    test_basic();
    test_destructor();
    test_swap();

    return boost::report_errors();
}

