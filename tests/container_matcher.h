#ifndef INLINED_VECTOR_TESTS_CONTAINER_MATCHER_H
#define INLINED_VECTOR_TESTS_CONTAINER_MATCHER_H

#include "catch.hpp"

// A generic container comparator
// Why doesn't Catch2 have this already?
template<typename Container1, typename Container2>
struct EqualsMatcher : Catch::MatcherBase<Container1, Container2> {
    EqualsMatcher(Container2 const &comparator) : m_comparator( comparator ) {}

    bool match(Container1 const &v) const override {
        auto it = v.begin();
        auto it2 = m_comparator.begin();
        while (true){
            if (it == v.end() && it2 == m_comparator.end()) return true;
            if (it == v.end() || it2 == m_comparator.end()) return false;
            if (*it != *it2) return false;
            it++, it2++;
        }
        return true;
    }
    std::string describe() const override {
        return "Equals: " + ::Catch::Detail::stringify( m_comparator );
    }
    Container2 const& m_comparator;
};

template<typename C1, typename C2>
EqualsMatcher<C1, C2> Equals(C1 const&, C2 const& comparator ) {
    return EqualsMatcher<C1, C2>( comparator );
}

#endif