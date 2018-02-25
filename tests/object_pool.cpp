#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define BSP_STORAGE_POOL_LOG_ERROR(message) std::cerr << "Error! " << message << "\n";
#define BSP_OBJECT_POOL_LOG_ERROR(message) std::cerr << "Error! " << message << "\n";

const bool DebugLogAllocations = true;
#define BSP_STORAGE_POOL_ALLOCATION(message, bytes) do { \
        if (DebugLogAllocations) \
			std::cout << "Memory: Allocated " << (bytes / 1024) << "kB \"" << message << "\"\n"; \
		} while(false)

#include <typeinfo>

namespace {
    template <typename T, typename U>
    using type_check = typename std::enable_if<std::is_same<typename std::decay<T>::type, U>::value, void>::type*;

    template <typename T> 
    std::string type_name(T* = 0){
        return typeid(T).name();
    }

    template <typename T, typename U = typename T::value_type> 
    std::string type_name(type_check<T, std::vector<U>> = 0){
        std::ostringstream oss;
        oss << "vector<" << type_name<U>() << ">";
        return oss.str();
    }
}

#define BSP_TYPE_NAME(type) type_name<type>()

#include "../include/object_pool.h"

#include "catch.hpp"
#include "container_matcher.h"

using bsp::object_pool;
using Catch::Equals;

struct hero {
    const char* name = nullptr;
    int hp = 0;
    int mp = 0;
    hero() = default;
    hero(const char* name, int hp, int mp):name{name}, hp{hp}, mp{mp}{}
};

namespace bsp {
    template <>
    struct object_is_valid<hero> {
        static bool get(const hero& value) {
            return value.hp != 0;
        }
    };
}

std::ostream& operator<<(std::ostream& out, const hero& h){
    return out << "hero {name: \"" << h.name << "\", hp: " << h.hp << ", mp: " << h.mp << "}";
}

TEST_CASE("object_pool (int)", "[object_pool]") {
    object_pool<int> pool {512};
    CHECK(pool.size() == 0);    

    SECTION("can push back empty"){
        pool.push_back();
        CHECK(pool.size() == 1);
    }

    SECTION("can push back rvalue"){
        pool.push_back(42);
        CHECK(pool.size() == 1);
    }

    SECTION("can emplace back"){
        pool.emplace_back(42);
        CHECK(pool.size() == 1);
    }
}

TEST_CASE("object_pool (std::string)", "[object_pool]") {
    object_pool<std::string> pool {512};
    CHECK(pool.size() == 0);

    SECTION("can push back empty"){
        pool.push_back();
        CHECK(pool.size() == 1);
    }

    SECTION("can push back rvalue"){
        pool.push_back(std::string("Hello"));
        CHECK(pool.size() == 1);
    }

    SECTION("can emplace back"){
        pool.emplace_back("Hello");
        CHECK(pool.size() == 1);
    }    
}

TEST_CASE("object_pool (hero)", "[object_pool]") {
    object_pool<hero> pool {512};
    CHECK(pool.size() == 0);

    SECTION("can push back empty"){
        pool.push_back();
        CHECK(pool.size() == 1);
    }

    SECTION("can push back copy"){
        hero batman {"batman", 5, 3};
        pool.push_back(batman);
        CHECK(pool.size() == 1);
    }

    SECTION("can push back rvalue"){
        pool.push_back({"spiderman", 6, 3});
        CHECK(pool.size() == 1);
    }

    SECTION("can emplace back"){
        pool.emplace_back("flash", 3, 4);
        CHECK(pool.size() == 1);
    }
}

TEST_CASE("object_pool object_is_valid (hero)", "[object_pool]") {
    object_pool<hero> heroes {32};
    CHECK(heroes.size() == 0);

    heroes.emplace_back("batman", 5, 3);
    heroes.emplace_back("superman", 0, 2);
    heroes.emplace_back("spiderman", 6, 3);
    heroes.emplace_back("flash", 3, 4);

    int num_iters = 0;
    for (auto it = heroes.begin(); it != heroes.end(); ++it){
        num_iters++;
    }
    CHECK(num_iters == 3); // Should skip superman because hp = 0
}

TEST_CASE("object_pool operator<<", "[object_pool]") {
    std::cout << "Should print object_pool...\n";

    object_pool<hero> heroes {64};
    heroes.emplace_back("batman", 5, 3);
    std::cout << heroes << "\n";

    heroes.emplace_back("spiderman", 6, 3);
    std::cout << heroes << "\n";

    heroes.emplace_back("flash", 3, 4);
    std::cout << heroes << "\n";
}