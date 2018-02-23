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
};

std::ostream& operator<<(std::ostream& out, const hero& h){
    return out << "hero {name: \"" << h.name << "\", hp: " << h.hp << ", mp: " << h.mp << "}";
}

TEST_CASE("object_pool (int)", "[object_pool]") {
    object_pool<int> pool {512};
    CHECK(pool.size() == 0);    
}

TEST_CASE("object_pool (hero)", "[object_pool]") {
    object_pool<hero> pool {512};
    CHECK(pool.size() == 0);
}