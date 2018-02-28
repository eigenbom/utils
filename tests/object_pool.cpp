#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <random>
#include <sstream>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include "../include/object_pool.h"
#include "catch.hpp"
#include "container_matcher.h"

using bsp::object_pool;
using bsp::detail::storage_pool;
using Catch::Equals;

static bool s_debug_log_allocations = false;

namespace bsp {

    template <typename T>
    void log_error(const storage_pool<T>& pool, const char* message){
        std::cerr << "Error: storage_pool<" << type_name<T>::get() << ">:" << message << "\n";
    }

    template <typename T>
    void log_error(const object_pool<T>& pool, const char* message){
        std::cerr << "Error: object_pool<" << type_name<T>::get() << ">:" << message << "\n";
    }

    template <typename T>
    void log_allocation(const storage_pool<T>& pool, int count, int bytes){
        if (s_debug_log_allocations){
            if (bytes>0){
                std::cout << "Memory: storage_pool<" << type_name<T>::get() << "> allocated " << (bytes / 1024) << "kB (" << count << " objects)\n";
            }
            else {
                std::cout << "Memory: storage_pool<" << type_name<T>::get() << "> deallocated " << (-bytes / 1024) << "kB (" << -count << " objects)\n";
            }
        }
    }

    template <typename T>
    struct type_name<std::vector<T>> {
        static std::string get(){
            std::ostringstream oss;
            oss << "vector<" << type_name<T>::get() << ">";
            return oss.str();
        }
    };

    template <>
    struct type_name<std::string> {
        static std::string get(){
            return "string";
        }
    };
}

#define HEADER() std::cout << "## "

TEST_CASE("storage_pool print allocations", "[storage_pool]"){
    s_debug_log_allocations = true;

    SECTION("default construction"){
        storage_pool<int> arr {512};
    }

    SECTION("default construction"){
        // Just testing type_name
        storage_pool<std::vector<std::string>> arr {512};
    }

    SECTION("default construction"){
        storage_pool<int> arr {512};
        CHECK(arr.storage_count() == 1);
        arr.allocate(256);
        CHECK(arr.storage_count() == 2);
        arr.allocate(128);
        CHECK(arr.storage_count() == 3);
        arr.deallocate();
        CHECK(arr.storage_count() == 2);
        arr.deallocate();
        CHECK(arr.storage_count() == 1);
        arr.deallocate();
        CHECK(arr.storage_count() == 0);
    }

    s_debug_log_allocations = false;
}

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
        bool success = arr.allocate(256);
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
        bool success = arr.allocate(256);
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

     SECTION("construction and destruction (ints)"){
        storage_pool<int> arr;
        CHECK(arr.storage_count() == 0);
        CHECK(arr.size() == 0);
        arr.allocate(512);
        CHECK(arr.storage_count() == 1);
        CHECK(arr.size() == 512);
        arr.allocate(512);
        CHECK(arr.storage_count() == 2);
        CHECK(arr.size() == 1024);
        arr.deallocate();
        CHECK(arr.storage_count() == 1);
        CHECK(arr.size() == 512);
        arr.deallocate();
        CHECK(arr.storage_count() == 0);
        CHECK(arr.size() == 0);
    }

    SECTION("allocation error"){
        HEADER() << "Should cause a bad_array_new_length exception or print an allocation error...\n";
        try {
            storage_pool<int_vector> vec;
            int shrink_factor = 0;
            for (int i=0; i<10; i++){
                bool res = vec.allocate((1 << (18 + i)) >> shrink_factor);
                if (!res) shrink_factor+=4; // make next allocation smaller
            }
        }
        catch (const std::bad_array_new_length& e){
            std::cout << "Exception caught: " << e.what() << "\n";
        }
    }
}

struct hero {
    const char* name = nullptr;
    int hp = 0;
    int mp = 0;
    hero() = default;
    hero(const char* name, int hp, int mp):name{name}, hp{hp}, mp{mp}{}
};

struct hero_policy {
    static const bool store_id_in_object = false;
	static const bool shrink_after_clear = false;
	static bool is_object_iterable(const hero& value){ return value.hp != 0; }
	static void set_object_id(hero&, const uint32_t&){}
	static uint32_t get_object_id(const hero&){return 0;}
};

struct hero_policy_shrink {
    static const bool store_id_in_object = false;
	static const bool shrink_after_clear = true;
	static bool is_object_iterable(const hero& value){ return value.hp != 0; }
	static void set_object_id(hero&, const uint32_t&){}
	static uint32_t get_object_id(const hero&){return 0;}
};

std::ostream& operator<<(std::ostream& out, const hero& h){
    return out << "hero {name: \"" << h.name << "\", hp: " << h.hp << ", mp: " << h.mp << "}";
}

TEST_CASE("object_pool msvc stack overflow??", "[object_pool]") {
	SECTION("test 1") {
		object_pool<hero> heroes{ 64 };
	}

	SECTION("test 2") {
		object_pool<hero> heroes{ 64 };
	}
}

TEST_CASE("object_pool (int)", "[object_pool]") {
    object_pool<int> pool {512};
    CHECK(pool.size() == 0);    
	
    SECTION("can construct empty"){
        pool.construct();
        CHECK(pool.size() == 1);
    }

    SECTION("can construct rvalue"){
        pool.construct(42);
        CHECK(pool.size() == 1);
    }

    SECTION("id starts at 0 and increments"){
        auto id0 = pool.construct().first;
        auto id1 = pool.construct().first;
        auto id2 = pool.construct().first;
        auto id3 = pool.construct().first;
        CHECK(id0 == 0);
        CHECK(id1 == 1);
        CHECK(id2 == 2);
        CHECK(id3 == 3);
    }

    SECTION("can remove"){
        uint32_t ids[10];
        for (int i=0; i<10; i++){
            ids[i] = pool.construct((int) std::pow(2, i)).first;
        }
        CHECK(pool.size() == 10);
        std::vector<int> powers { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 };
        CHECK_THAT(pool, Equals(pool, powers));

        for (int i=0; i<10; i+=2){
            pool.remove(i);
        }
        CHECK(pool.size() == 5);
        std::vector<int> otherPowers { 512, 2, 32, 8, 128 };
        CHECK_THAT(pool, Equals(pool, otherPowers));        
    }
}

TEST_CASE("object_pool (std::string)", "[object_pool]") {
    object_pool<std::string> pool {512};
    CHECK(pool.size() == 0);

    SECTION("can construct empty"){
        pool.construct();
        CHECK(pool.size() == 1);
    }

    SECTION("can construct rvalue"){
        auto res = pool.construct(std::string("Hello"));
        CHECK(pool.size() == 1);
        CHECK(res.first == 0);
        CHECK(*res.second == "Hello");
    }

    SECTION("can construct move"){
        std::string s {"Hello"};
        auto res = pool.construct(std::move(s));
        CHECK(pool.size() == 1);
        CHECK(res.first == 0);
        CHECK(*res.second == "Hello");
    }

    SECTION("can construct args"){
        pool.construct("Hello");
        CHECK(pool.size() == 1);
    }    
}

TEST_CASE("object_pool (hero)", "[object_pool]") {
    object_pool<hero> pool {512};
    CHECK(pool.size() == 0);

    SECTION("can construct empty"){
        pool.construct();
        CHECK(pool.size() == 1);
    }

    SECTION("can construct copy"){
        hero batman {"batman", 5, 3};
        pool.construct(batman);
        CHECK(pool.size() == 1);
    }

    SECTION("can construct rvalue"){
        pool.construct({"spiderman", 6, 3});
        CHECK(pool.size() == 1);
    }

    SECTION("can construct back"){
        pool.construct("flash", 3, 4);
        CHECK(pool.size() == 1);
    }
}

TEST_CASE("object_pool object_is_valid (hero)", "[object_pool]") {
    object_pool<hero, uint32_t, hero_policy> heroes {32};
    CHECK(heroes.size() == 0);

    heroes.construct("batman", 5, 3);
    heroes.construct("superman", 0, 2);
    heroes.construct("spiderman", 6, 3);
    heroes.construct("flash", 3, 4);

    int num_iters = 0;
    for (auto it = heroes.begin(); it != heroes.end(); ++it){
        num_iters++;
    }
    CHECK(num_iters == 3); // Should skip superman because hp = 0
}

TEST_CASE("object_pool (grow and clear)", "[object_pool]") {
    object_pool<hero, uint32_t, hero_policy_shrink> pool {512};
    CHECK(pool.capacity() == 512);
    for (int i=0; i<513; ++i) pool.construct("batman", 5, 5);
    CHECK(pool.capacity() == 1024);
    pool.clear();
    CHECK(pool.capacity() == 512);
}

TEST_CASE("object_pool (grow to max size)", "[object_pool]") {
    object_pool<int> pool {512};
    for (int i=0; i<pool.max_size(); ++i) pool.construct();
    CHECK(pool.size() == pool.max_size());
    CHECK(pool.capacity() == pool.max_size());
    CHECK_THROWS_AS(pool.construct(), std::length_error);        
}

TEST_CASE("object_pool ostream", "[object_pool]") {
    using hero_pool = object_pool<hero, uint32_t, hero_policy>;
    SECTION("all valid"){
        hero_pool heroes {64};
        heroes.construct("batman", 5, 3);
        heroes.construct("spiderman", 6, 3);
        heroes.construct("flash", 3, 4);

        std::ostringstream oss;
        oss << heroes;
        auto result = R"(object_pool [hero {name: "batman", hp: 5, mp: 3}, hero {name: "spiderman", hp: 6, mp: 3}, hero {name: "flash", hp: 3, mp: 4}])";
        CHECK_THAT(oss.str(), Equals(result));
    }

    SECTION("start invalid"){
        hero_pool heroes {64};
        heroes.construct("superman", 0, 3);
        heroes.construct("batman", 5, 3);
        heroes.construct("spiderman", 6, 3);
        heroes.construct("flash", 3, 4);

        std::ostringstream oss;
        oss << heroes;
        auto result = R"(object_pool [hero {name: "batman", hp: 5, mp: 3}, hero {name: "spiderman", hp: 6, mp: 3}, hero {name: "flash", hp: 3, mp: 4}])";
        CHECK_THAT(oss.str(), Equals(result));
    }

    SECTION("middle invalid"){
        hero_pool heroes {64};
        heroes.construct("batman", 5, 3);
        heroes.construct("superman", 0, 3);
        heroes.construct("spiderman", 6, 3);
        heroes.construct("flash", 3, 4);
        
        std::ostringstream oss;
        oss << heroes;
        auto result = R"(object_pool [hero {name: "batman", hp: 5, mp: 3}, hero {name: "spiderman", hp: 6, mp: 3}, hero {name: "flash", hp: 3, mp: 4}])";
        CHECK_THAT(oss.str(), Equals(result));
    }

    SECTION("end invalid"){
        hero_pool heroes {64};
        heroes.construct("batman", 5, 3);
        heroes.construct("spiderman", 6, 3);
        heroes.construct("flash", 3, 4);
        heroes.construct("superman", 0, 3);

        std::ostringstream oss;
        oss << heroes;
        auto result = R"(object_pool [hero {name: "batman", hp: 5, mp: 3}, hero {name: "spiderman", hp: 6, mp: 3}, hero {name: "flash", hp: 3, mp: 4}])";
        CHECK_THAT(oss.str(), Equals(result));
    }

    SECTION("all invalid"){
        hero_pool heroes {64};
        heroes.construct("batman", 0, 3);
        heroes.construct("spiderman", 0, 3);
        heroes.construct("flash", 0, 4);
        heroes.construct("superman", 0, 3);
        
        std::ostringstream oss;
        oss << heroes;
        auto result = R"(object_pool [])";
        CHECK_THAT(oss.str(), Equals(result));
    }
}
struct custom_id {
    uint32_t id = 0;
    explicit custom_id(uint32_t id = 0): id(id){}
    explicit operator uint32_t() const { return id; }
};

TEST_CASE("object_pool (hero, custom id)", "[object_pool]") {
    object_pool<hero, custom_id> pool {512};
    auto id1 = pool.construct("batman", 5, 3);
    CHECK((uint32_t) id1.first == 0);
    auto id2 = pool.construct("superman", 999, 4);
    CHECK((uint32_t) id2.first == 1);
}

struct quote {
    uint32_t id = 0;
    std::string text {};
    quote() = default;
    quote(std::string text):text{text}{}
};

struct quote_policy {
    static const bool store_id_in_object = true;
	static const bool shrink_after_clear = true;
	static bool is_object_iterable(const quote& value){ return value.id != 0; }
	static void set_object_id(quote& value, const uint32_t& id){ value.id = id; }
	static uint32_t get_object_id(const quote& value) { return value.id; }
};

TEST_CASE("object_pool (object with id)", "[object_pool]") {
    object_pool<quote, uint32_t, quote_policy> pool {512};
    // This system has id==0 as invalid, so we make an invalid quote first
    pool.construct();
    CHECK(std::distance(pool.begin(), pool.end()) == 0);

    auto id1 = pool.construct("The unexamined life is not worth living.");
    auto id2 = pool.construct("The only true wisdom is in knowing you know nothing.");
    auto id3 = pool.construct("There is only one good, knowledge, and one evil, ignorance.");

    const auto& quote1 = pool[id1.first];
    const auto& quote2 = pool[id2.first];
    const auto& quote3 = pool[id3.first];

    CHECK(quote1.id == id1.first);
    CHECK(quote2.id == id2.first);
    CHECK(quote3.id == id3.first);
}

TEST_CASE("object_pool (internal consistency)", "[object_pool]") {
    object_pool<int> pool {512};
    std::default_random_engine engine {0};

    auto random_int = [&engine](int from, int to){
        return std::uniform_int_distribution<int>{from, to}(engine);
    };
    
    std::list<uint32_t> ids;
    for (int i=0; i<256; ++i){
        auto res = pool.construct(random_int(0, 100));
        ids.push_back(res.first);
    }

    // Randomly insert and remove things and then check consistency of freelist
    for (int i=0; i<128; ++i){
        if (random_int(0, 1) == 0){
            auto res = pool.construct(random_int(0, 100));
            ids.push_back(res.first);
        }
        else {
            auto it = std::next(ids.begin(), random_int(0, ids.size()-1));
            pool.remove(*it);
            ids.erase(it);
        }
    }

    pool.debug_check_internal_consistency();
}

TEST_CASE("object_pool (benchmarks)", "[object_pool]"){
    static const int size = 512;
    object_pool<int> pool {512};

    BENCHMARK("construct elements") {
        for(int i = 0; i < size; ++i)
            pool.construct(i);
    }

}