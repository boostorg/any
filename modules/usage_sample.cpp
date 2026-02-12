// Copyright Antony Polukhin, 2016-2026
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// To compile manually use a command like the following:
// clang++ -std=c++20 -fmodule-file=type_index.pcm type_index.pcm usage_sample.cpp

//[any_module_example

#ifdef BOOST_ANY_USE_STD_MODULE
import std;
#else
# include <iostream>
# include <sstream>
# include <string>
#endif

import boost.any;

namespace {

    template <typename... Ts> auto any_to_string(const boost::any& a) -> std::string {
        std::ostringstream oss;

        auto try_cast = [&](auto* dummy) -> bool {
            using T = std::decay_t<decltype(*dummy)>;
            if (a.type() == typeid(T)) {
                oss << boost::any_cast<T>(a);
                return true;
            }
            return false;
        };

        // Expand over Ts...
        bool const success = (try_cast((Ts*)nullptr) || ...);

        if (!success) {
            oss << "<unknown type: " << a.type().name() << ">";
        }
        return oss.str();
    }

}  // namespace

// Usage:
auto main() -> int {
    boost::any const a = 42;
#ifdef BOOST_ANY_USE_STD_MODULE
    std::println("{}", any_to_string<int, double, std::string>(a));
#else
    std::cout << any_to_string<int, double, std::string>(a) << '\n';
#endif
}
//]

