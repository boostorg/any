#ifndef BOOST_ANY_INCLUDED
#define BOOST_ANY_INCLUDED

// what:  variant type boost::any
// who:   contributed by Kevlin Henney,
//        with features contributed and bugs found by
//        Ed Brey, Mark Rodgers, Peter Dimov, and James Curran;
//        visitation, move added by Eric Friedman 
// when:  original July 2001; visitation December 2002
// where: tested with BCC 5.5, MSVC 6.0, and g++ 2.95;
//        visitation, move tested with g++ 3.2

#include <algorithm>
#include <typeinfo>

#include "boost/config.hpp"
#include "boost/move/moveable.hpp"
#include "boost/visitor/bad_visit.hpp"
#include "boost/visitor/dynamic_visitable_base.hpp"
#include "boost/visitor/try_dynamic_visit.hpp"

namespace boost
{
    class any
      : public moveable< any >
      , public dynamic_visitable_base
    {
    public: // structors

        any()
          : content(0)
        {
        }

        template<typename ValueType>
        any(const ValueType & value)
          : content(new holder<ValueType>(value))
        {
        }

        any(const any & other)
          : content(other.content ? other.content->clone() : 0)
        {
        }

        any(move_source< any > source)
          : content(source.get().content)
        {
            source.get().content = 0;
        }

        ~any()
        {
            delete content;
        }

    public: // modifiers

        any & swap(any & rhs)
        {
            std::swap(content, rhs.content);
            return *this;
        }

        template<typename ValueType>
        any & operator=(const ValueType & rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }

        any & operator=(const any & rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }

        any & operator=(move_source< any > source)
        {
            delete content;
            content = source.get().content;
            source.get().content = 0;

            return *this;
        }

    public: // queries

        bool empty() const
        {
            return !content;
        }

        const std::type_info & type() const
        {
            return content ? content->type() : typeid(void);
        }

    private: // visitation support

        virtual void apply_visitor(dynamic_visitor_base & visitor)
        {
            if (!content)
                throw bad_visit();

            content->apply_visitor(visitor);
        }

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
    private: // types
#else
    public: // types (public so any_cast can be non-friend)
#endif

        class placeholder : public dynamic_visitable_base
        {
        public: // structors
    
            virtual ~placeholder()
            {
            }

        public: // queries

            virtual const std::type_info & type() const = 0;

            virtual placeholder * clone() const = 0;
    
        };

        template<typename ValueType>
        class holder : public placeholder
        {
        public: // structors

            holder(const ValueType & value)
              : held(value)
            {
            }

        public: // queries

            virtual const std::type_info & type() const
            {
                return typeid(ValueType);
            }

            virtual placeholder * clone() const
            {
                return new holder(held);
            }

        public: // visitation support

            virtual void apply_visitor(dynamic_visitor_base & visitor)
            {
                if (!try_dynamic_visit(visitor, held))
                    throw bad_visit();
            }

        public: // representation

            ValueType held;

        };

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS

    private: // representation

        template<typename ValueType>
        friend ValueType * any_cast(any *);

#else

    public: // representation (public so any_cast can be non-friend)

#endif

        placeholder * content;

    };

    class bad_any_cast : public std::bad_cast
    {
    public:
        virtual const char * what() const throw()
        {
            return "boost::bad_any_cast: "
                   "failed conversion using boost::any_cast";
        }
    };

    template<typename ValueType>
    ValueType * any_cast(any * operand)
    {
        return operand && operand->type() == typeid(ValueType)
                    ? &static_cast<any::holder<ValueType> *>(operand->content)->held
                    : 0;
    }

    template<typename ValueType>
    const ValueType * any_cast(const any * operand)
    {
        return any_cast<ValueType>(const_cast<any *>(operand));
    }

    template<typename ValueType>
    ValueType any_cast(const any & operand)
    {
        const ValueType * result = any_cast<ValueType>(&operand);
        if(!result)
            throw bad_any_cast();
        return *result;
    }

}

// Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#endif
