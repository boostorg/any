// Copyright 2006 Alexander Nasonov.
// Copyright Antony Polukhin, 2013-2019.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/basic_any.hpp>

int main() {
    boost::basic_any<> const a;
    boost::basic_any_cast<int&, 8, 8>(a);
}

