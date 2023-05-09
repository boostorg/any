// Copyright Antony Polukhin, 2020-2023.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org/libs/any for Documentation.

#ifndef BOOST_ANYS_UNIQUE_ANY_HPP_INCLUDED
#define BOOST_ANYS_UNIQUE_ANY_HPP_INCLUDED

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
#error Header <boost/any/unique_any.hpp> requires C++11 compatible compiler with move semantics
#endif

#ifdef BOOST_NO_CXX11_DEFAULTED_FUNCTIONS
#error Header <boost/any/unique_any.hpp> requires C++11 compatible compiler with defaulted functions
#endif

#ifdef BOOST_NO_CXX11_SMART_PTR
#error Header <boost/any/unique_any.hpp> requires C++11 compatible standard library with std::unique_ptr
#endif
#include <memory>

#ifdef BOOST_NO_CXX11_HDR_INITIALIZER_LIST
#error Header <boost/any/unique_any.hpp> requires C++11 compatible standard library with std::initializer_list
#endif

#include <utility>
#include <type_traits>

#include <boost/any/fwd.hpp>
#include <boost/any/bad_any_cast.hpp>

#include <boost/type_index.hpp>

namespace boost { namespace anys {

template <class T>
struct in_place_type_t
{
};

#if !defined(BOOST_NO_CXX14_VARIABLE_TEMPLATES)
template <class T>
constexpr in_place_type_t<T> in_place_type{};
#endif

/// \brief A class whose instances can hold instances of any
/// type (including non-copyable and non-movable types).
///
/// \pre C++11 compatible compiler.
class unique_any {
public:
    /// \post this->empty() is true.
    constexpr unique_any() noexcept = default;

    /// Move constructor that moves content of
    /// `other` into new instance and leaves `other` empty.
    ///
    /// \post other->empty() is true
    /// \throws Nothing.
    unique_any(unique_any&& other) noexcept = default;

    /// Forwards <code>value</code>, so
    /// that the initial content of the new instance is equivalent
    /// in both type and value to `value` before the forward.
    ///
    /// \throws std::bad_alloc or any exceptions arising from the move or
    /// copy constructor of the contained type.
    template<typename T>
    unique_any(T&& value)
      : content(new holder< typename std::decay<T>::type >(std::forward<T>(value)))
    {
        static_assert(
            !boost::anys::detail::is_basic_any< typename std::decay<T>::type >::value,
            "boost::anys::unique_any could not be constructed from boost::anys::basic_any."
        );

        static_assert(
            !boost::anys::detail::is_some_any< typename std::decay<T>::type >::value,
            "unique_any could be only moved and could not be constructoed from "
            "other types of any."
        );
    }

    template<class T, class... Args>
    explicit unique_any(in_place_type_t<T>, Args&&... args)
      : content(new holder<typename std::decay<T>::type>(std::forward<Args>(args)...))
    {
    }

    template <class T, class U, class... Args>
    explicit unique_any(in_place_type_t<T>, std::initializer_list<U> il, Args&&... args)
      : content(new holder<typename std::decay<T>::type>(il, std::forward<Args>(args)...))
    {
    }

    /// Releases any and all resources used in management of instance.
    ///
    /// \throws Nothing.
    ~unique_any() noexcept = default;

    /// Moves content of `rhs` into
    /// current instance, discarding previous content, so that the
    /// new content is equivalent in both type and value to the
    /// content of <code>rhs</code> before move, or empty if `rhs.empty()`.
    ///
    /// \post `rhs->empty()` is true
    /// \throws Nothing.
    unique_any & operator=(unique_any&& rhs) noexcept = default;

    /// Forwards `rhs`,
    /// discarding previous content, so that the new content of is
    /// equivalent in both type and value to `rhs` before forward.
    ///
    /// \throws std::bad_alloc
    /// or any exceptions arising from the move or copy constructor of the
    /// contained type. Assignment satisfies the strong guarantee
    /// of exception safety.
    template <class T>
    unique_any & operator=(T&& rhs)
    {
        unique_any(std::forward<T>(rhs)).swap(*this);
        return *this;
    }

    template<class T, class... Args>
    typename std::decay<T>::type& emplace(Args&&... args) {
        auto* raw_ptr = new holder<typename std::decay<T>::type>(std::forward<Args>(args)...);
        content = std::unique_ptr<placeholder>(raw_ptr);
        return raw_ptr->held;
    }

    template<class T, class U, class... Args>
    typename std::decay<T>::type& emplace(std::initializer_list<U> il, Args&&... args) {
        auto* raw_ptr = new holder<typename std::decay<T>::type>(il, std::forward<Args>(args)...);
        content = std::unique_ptr<placeholder>(raw_ptr);
        return raw_ptr->held;
    }

    /// \post this->empty() is true
    void reset() noexcept
    {
        content.reset();
    }

    /// Exchange of the contents of `*this` and `rhs`.
    ///
    /// \returns `*this`
    /// \throws Nothing.
    void swap(unique_any& rhs) noexcept
    {
        content.swap(rhs.content);
    }

    /// \returns `true` if instance is not empty, otherwise `false`.
    /// \throws Nothing.
    bool has_value() const noexcept
    {
        return !!content;
    }

    /// \returns the `typeid` of the
    /// contained value if instance is non-empty, otherwise
    /// `typeid(void)`.
    ///
    /// Useful for querying against types known either at compile time or
    /// only at runtime.
    const boost::typeindex::type_info& type() const noexcept
    {
        return content ? content->type() : boost::typeindex::type_id<void>().type_info();
    }

private: // types
    class BOOST_SYMBOL_VISIBLE placeholder
    {
    public:
        virtual ~placeholder()
        {
        }

        virtual const boost::typeindex::type_info& type() const noexcept = 0;
    };

    template<typename T>
    class holder final: public placeholder
    {
    public:
        template <class... Args>
        holder(Args&&... args)
          : held(std::forward<Args>(args)...)
        {
        }

        template <class U, class... Args>
        holder(std::initializer_list<U> il, Args&&... args)
          : held(il, std::forward<Args>(args)...)
        {
        }

        const boost::typeindex::type_info& type() const noexcept override
        {
            return boost::typeindex::type_id<T>().type_info();
        }

    public:
        T held;
    };

private: // representation
    template<typename T>
    friend T * unsafe_any_cast(unique_any *) noexcept;

    std::unique_ptr<placeholder> content;
};

/// Exchange of the contents of `lhs` and `rhs`.
/// \throws Nothing.
inline void swap(unique_any & lhs, unique_any & rhs) noexcept
{
    lhs.swap(rhs);
}


// Note: The "unsafe" versions of any_cast are not part of the
// public interface and may be removed at any time. They are
// required where we know what type is stored in the any and can't
// use typeid() comparison, e.g., when our types may travel across
// different shared libraries.
template<typename T>
inline T * unsafe_any_cast(unique_any * operand) noexcept
{
    return std::addressof(
        static_cast<unique_any::holder<T>&>(*operand->content).held
    );
}

template<typename T>
inline const T * unsafe_any_cast(const unique_any * operand) noexcept
{
    return anys::unsafe_any_cast<T>(const_cast<unique_any *>(operand));
}

/// \returns Pointer to a ValueType stored in `operand`, nullptr if
/// `operand` does not contain specified ValueType.
template<typename T>
T * any_cast(unique_any * operand) noexcept
{
    return operand && operand->type() == boost::typeindex::type_id<T>()
        ? anys::unsafe_any_cast<typename std::remove_cv<T>::type>(operand)
        : nullptr;
}

/// \returns Const pointer to a ValueType stored in `operand`, nullptr if
/// `operand` does not contain specified ValueType.
template<typename T>
inline const T * any_cast(const unique_any * operand) noexcept
{
    return anys::any_cast<T>(const_cast<unique_any *>(operand));
}

/// \returns `T` stored in `operand`
/// \throws boost::bad_any_cast if `operand` does not contain specified `T`.
template<typename T>
T any_cast(unique_any & operand)
{
    typedef typename std::remove_reference<T>::type nonref;

    nonref * result = anys::any_cast<nonref>(std::addressof(operand));
    if(!result)
        boost::throw_exception(bad_any_cast());

    // Attempt to avoid construction of a temporary object in cases when
    // `T` is not a reference. Example:
    // `static_cast<std::string>(*result);`
    // which is equal to `std::string(*result);`
    typedef typename std::conditional<
        std::is_reference<T>::value,
        T,
        T&
    >::type ref_type;

#ifdef BOOST_MSVC
#   pragma warning(push)
#   pragma warning(disable: 4172) // "returning address of local variable or temporary" but *result is not local!
#endif
    return static_cast<ref_type>(*result);
#ifdef BOOST_MSVC
#   pragma warning(pop)
#endif
}

/// \returns `T` stored in `operand`
/// \throws boost::bad_any_cast if `operand` does not contain specified `T`.
template<typename T>
inline T any_cast(const unique_any & operand)
{
    typedef typename std::remove_reference<T>::type nonref;
    return anys::any_cast<const nonref &>(const_cast<unique_any &>(operand));
}

/// \returns `T` stored in `operand`
/// \throws boost::bad_any_cast if `operand` does not contain specified `T`.
template<typename T>
inline T any_cast(unique_any&& operand)
{
    static_assert(
        std::is_rvalue_reference<T&&>::value /*true if T is rvalue or just a value*/
        || std::is_const< typename std::remove_reference<T>::type >::value,
        "boost::any_cast shall not be used for getting nonconst references to temporary objects"
    );
    return std::move(anys::any_cast<T&>(operand));
}

} // namespace anys

using boost::anys::any_cast;
using boost::anys::unsafe_any_cast;

} // namespace boost


#endif // BOOST_ANYS_UNIQUE_ANY_HPP_INCLUDED
