#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define BSP_STORAGE_POOL_LOG_ERROR(message) std::cerr << "Error! " << message << "\n";

const bool DebugLogAllocations = true;
#define BSP_STORAGE_POOL_ALLOCATION(message, bytes) do { \
        if (DebugLogAllocations) \
			std::cout << "Memory: Allocated " << (bytes / 1024) << "kB \"" << message << "\"\n"; \
		} while(false)

#include <typeinfo>

namespace {
    template <typename T> std::string type_name(typename std::enable_if<std::is_same<T, int>::value, void>::type* v = 0){
        return typeid(T).name();
    }

    template <typename T, typename U = typename T::value_type> std::string type_name(typename std::enable_if<std::is_same<typename std::decay<T>::type, std::vector<U>>::value, void>::type* v = 0){
        std::ostringstream oss;
        oss << "vector<" << type_name<U>() << ">";
        return oss.str();
    }
}

#define BSP_TYPE_NAME(type) type_name<type>()

#include "../include/storage_pool.h"

#include "catch.hpp"
#include "container_matcher.h"

using bsp::storage_pool;
using Catch::Equals;

TEST_CASE("storage_pool", "[storage_pool]") {    
    SECTION("default construction (ints)"){
        storage_pool<int> arr;
        CHECK(arr.storage_count() == 0); 
    }

    SECTION("construction (ints)"){
        storage_pool<int> arr { 512 };
        CHECK(arr.storage_count() == 1); 
        CHECK(arr.size() == 512);
    }

    SECTION("adding storage (ints)"){
        storage_pool<int> arr { 512 };
        bool success = arr.append_storage(256);
        CHECK(success == true);
        CHECK(arr.storage_count() == 2); 
        CHECK(arr.size() == 512 + 256);
    }

    SECTION("creating and destroying ints"){
        storage_pool<int> arr { 512 };
        new (&arr[0]) int {42};
        CHECK(arr[0] == 42);
        // No need to destroy
    }

    using int_vector = std::vector<int>;

    SECTION("default construction (int_vector)"){
        storage_pool<int_vector> arr;
        CHECK(arr.storage_count() == 0); 
    }

    SECTION("construction (int_vector)"){
        storage_pool<int_vector> arr { 512 };
        CHECK(arr.storage_count() == 1); 
        CHECK(arr.size() == 512);
    }

    SECTION("adding storage (int_vector)"){
        storage_pool<int_vector> arr { 512 };
        bool success = arr.append_storage(256);
        CHECK(success == true);
        CHECK(arr.storage_count() == 2); 
        CHECK(arr.size() == 512 + 256);
    }

    SECTION("creating and destroying (int_vector)"){
        storage_pool<int_vector> arr { 512 };
        new (&arr[0]) int_vector(100, 42);
        CHECK(arr[0].size() == 100);
        CHECK(arr[0][0] == 42);
        (&arr[0])->~int_vector();
    }

#ifdef BSP_STORAGE_POOL_LOG_ERROR
     SECTION("allocation error"){
         std::cout << "Should print an error...\n";
         storage_pool<int_vector> arr { 1 << 31 };
     }
#endif
}

