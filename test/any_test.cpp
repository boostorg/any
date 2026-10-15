// Copyright Kevlin Henney, 2000, 2001. All rights reserved.
// Copyright Antony Polukhin, 2013-2026.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// what:  unit tests for variant type boost::any
// who:   contributed by Kevlin Henney
// when:  July 2001, 2013, 2014
// where: tested with BCC 5.5, MSVC 6.0, and g++ 2.95

#include <boost/any.hpp>

#include "basic_test.hpp"

static const std::string& returning_string1()
{
    static const std::string ret("foo");
    return ret;
}

static std::string returning_string2()
{
    static const std::string ret("foo");
    return ret;
}

static void test_with_func()
{
    std::string s;
    s = boost::any_cast<std::string>(returning_string1());
    s = boost::any_cast<const std::string&>(returning_string1());

    s = boost::any_cast<std::string>(returning_string2());
    s = boost::any_cast<const std::string&>(returning_string2());

#if !defined(__INTEL_COMPILER) && !defined(__ICL) && (!defined(_MSC_VER) || _MSC_VER != 1600)
    // Intel compiler thinks that it must choose the `any_cast(const any&)` function
    // instead of the `any_cast(const any&&)`.
    // Bug was not reported because of missing premier support account + annoying
    // registrations requirements.

    // MSVC-10 had a bug:
    //
    // any.hpp(291) : error C2440: 'return' : cannot convert.
    // Conversion loses qualifiers
    // any_test.cpp(304) : see reference to function template instantiation
    //
    // This issue was fixed in MSVC-11.

    s = boost::any_cast<std::string&&>(returning_string1());
#endif

    s = boost::any_cast<std::string&&>(returning_string2());
}



#ifdef __cpp_lib_constexpr_new

#define BOOST_ANY_IMPL_CONSTEXPR_ASSERT(X) \
    if (!(X)) throw std::runtime_error(#X " is false at " + std::to_string(__LINE__))

constexpr int test_constexpr() {
    boost::any a;

    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(a.empty());

    a = 42;
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(!a.empty());

    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(boost::any_cast<int>(a) == 42);
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(*boost::unsafe_any_cast<int>(&a) == 42);

    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(boost::any_cast<short>(&a) == nullptr);
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(boost::any_cast<int&>(a) == 42);

    boost::any b = std::move(a);
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(a.empty());
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(!b.empty());
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(boost::any_cast<int&>(b) == 42);

    a = std::move(b);
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(!a.empty());
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(b.empty());

    a.clear();
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(a.empty());
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(b.empty());

    a = 43;
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(!a.empty());
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(boost::any_cast<int>(a) == 43);

    a.swap(b);
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(a.empty());
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(!b.empty());
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(boost::any_cast<int>(b) == 43);

    a.swap(b);
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(!a.empty());
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(b.empty());
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(boost::any_cast<int>(a) == 43);

    b = a;
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(!a.empty());
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(!b.empty());
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(boost::any_cast<int>(a) == 43);
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(boost::any_cast<int>(b) == 43);

    const boost::any ca = 44;
    BOOST_ANY_IMPL_CONSTEXPR_ASSERT(boost::any_cast<int>(ca) == 44);

    return 0;
}

#undef BOOST_ANY_IMPL_CONSTEXPR_ASSERT

#endif

int main() {
    test_with_func();

#ifdef __cpp_lib_constexpr_new
    static_assert(test_constexpr() == 0);
#endif

    return any_tests::basic_tests<boost::any>::run_tests();
}

