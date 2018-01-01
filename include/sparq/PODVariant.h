//
// Created by Samit Basu on 12/30/17.
//

#ifndef OLYMPUS_MC_PODVARIANT_H
#define OLYMPUS_MC_PODVARIANT_H

namespace sparq {

#include <cassert>

    /**
     * What we want is the std::variant type.  But this comes at some significant cost, either
     * in the form of a cutting edge (as yet released) c++ compiler, or a large footprint 3rd
     * party library.  The need is the ability to absorb a number of types (unique) into a
     * union, such as:
     *
     *   using Event = PODVariant<Event1, Event2, Event3>;
     *
     * And then have a type-safe dispatch in the back end based on which type is selected.
     * A full blown variant implementation is enormous and complex and stuffed full of templates
     * (see mpark's version or Boost::Variant for examples).  As we don't need all that, we
     * can live with a significant and substantial limitation: only PODs can be stored in the
     * union.  That means that each Event type must satisfy the std::is_pod<T> test.  It is
     * a tough test, but it means that interoperability with C is likely, and that the different
     * types are simply casts of each other.  Our more modest PODVariant type will only accept POD
     * arguments.
     */

    namespace PODVariantDetails {

        template<class F, class...Args>
        struct is_callable
        {
            template<class U> static auto test(U* p) -> decltype((*p)(std::declval<Args>()...), void(), std::true_type());
            template<class U> static auto test(...) -> decltype(std::false_type());

            static constexpr bool value = decltype(test<F>(0))::value;
        };


        template <typename ...>
        union PODVariantUnion;

        template <typename T>
        union PODVariantUnion<T> {
            static_assert(std::is_pod<T>::value, "non-pod types are not supported in PODVariant");
            T t;

            template <unsigned int i, typename U>
            void set(const U& u){
                assert(i == 0);
                assert((std::is_same<T,U>::value));
                t = ((T*)(&u))[0];
            }

            template <typename V>
            void exec(unsigned int i, const V& func) const {
                assert(i == 0);
                static_assert(is_callable<V, T>::value, "Not all possibilities are handled");
                func(t);
            }

            template <typename V>
            void exec(unsigned int i, V& func) const {
                assert(i == 0);
                static_assert(is_callable<V, T>::value, "Not all possibilities are handled");
                func(t);
            }
        };

        template <unsigned int, typename, typename ...>
        struct GetIndex;

        template <unsigned int i, typename U, typename T>
        struct GetIndex<i,U,T> {
            static constexpr bool found = std::is_same<T,U>::value;
            static constexpr unsigned int index = i;
        };

        template <unsigned int i, typename U, typename T, typename ... Ts>
        struct GetIndex<i,U,T,Ts ...> {
            static constexpr bool found =
                    std::is_same<T,U>::value ? true : GetIndex<i+1,U,Ts ...>::found;
            static constexpr unsigned int index =
                    std::is_same<T,U>::value ? i : GetIndex<i+1,U,Ts ...>::index;
        };


        template <typename T, typename ... Ts>
        union PODVariantUnion<T, Ts ...> {
            static_assert(std::is_pod<T>::value, "non-pod types are not supported in PODVariant");
            T t;
            PODVariantUnion<Ts ...> ts;

            template<unsigned int i, typename U>
            void set(const U &u) {
                if (i == 0) {
                    assert((std::is_same<U,T>::value));
                    t = ((T*)(&u))[0];
                } else {
                    return this->ts.template set<i-1, U>(u);
                }
            }

            template<typename V>
            void exec(unsigned int i, const V& func) const {
                if (i == 0) {
                    static_assert(is_callable<V, T>::value, "Not all possibilities are handled");
                    func(t);
                } else {
                    return this->ts.template exec<V>(i-1,func);
                }
            }

            template<typename V>
            void exec(unsigned int i, V& func) const {
                if (i == 0) {
                    static_assert(is_callable<V, T>::value, "Not all possibilities are handled");
                    func(t);
                } else {
                    return this->ts.template exec<V>(i-1,func);
                }
            }


        };

    }

    template <typename ... Ts>
    struct PODVariant {
        PODVariant() : _varUnion(), _isSet(), _setTo() {}

        template <typename U>
        PODVariant(const U &other) {
            constexpr bool found = PODVariantDetails::GetIndex<0,U, Ts ...>::found;
            constexpr unsigned int index = PODVariantDetails::GetIndex<0,U, Ts ...>::index;
            static_assert(found, "variant type not found");
            this->_varUnion.template set <index, U>(other);
            _isSet = true;
            _setTo = index;
        }

        template <typename U>
        const PODVariant& operator=(const U& other) {
            constexpr bool found = PODVariantDetails::GetIndex<0,U, Ts ...>::found;
            constexpr unsigned int index = PODVariantDetails::GetIndex<0,U, Ts ...>::index;
            static_assert(found, "variant type not found");
            this->_varUnion.template set <index, U>(other);
            _isSet = true;
            _setTo = index;
            return *this;
        }

        template <typename U>
        bool contains() const {
            constexpr bool found = PODVariantDetails::GetIndex<0,U, Ts ...>::found;
            constexpr unsigned int index = PODVariantDetails::GetIndex<0,U, Ts ...>::index;
            static_assert(found, "variant type not found");
            return _isSet && (index == _setTo);
        }

        template <typename V>
        void visit(const V& func) const {
            if (_isSet)
                this->_varUnion.template exec(_setTo,func);
        }

        template <typename V>
        void visit(V& func) const {
            if (_isSet)
                this->_varUnion.template exec(_setTo,func);
        }

    private:
        PODVariantDetails::PODVariantUnion<Ts ...> _varUnion;
        bool _isSet;
        unsigned int _setTo;
    };

}

#endif //OLYMPUS_MC_TUNION_H
