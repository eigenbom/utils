#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#define BSP_INLINED_VECTOR_THROWS
// #define BSP_INLINED_VECTOR_LOG_ERROR(message) std::cerr << message << "\n"
#include "../include/inlined_vector.h"

#include "catch.hpp"
#include "container_matcher.h"

using bsp::detail::static_vector;
using bsp::inlined_vector;

TEST_CASE("inlined_vector basics", "[static_vector]") {
    static_vector<int, 8> v;
    CHECK(v.size() == 0);
    CHECK(v.max_size() == 8);

    v.push_back(42);
    v.push_back(3);

    CHECK(*v.begin() == 42);
    CHECK(*std::next(v.begin(), 1) == 3);
}

struct Counter {
    int* counter = nullptr;
    Counter() = default;
    Counter(int* i):counter(i){ if (counter) (*counter)++; }
    Counter(const Counter& c){ counter = c.counter; if (counter) (*counter)++; };
    Counter(Counter&& c){ counter = c.counter; c.counter = nullptr; };
    ~Counter(){ if (counter) (*counter)--; }
};

TEST_CASE("inlined_vector pod", "[static_vector]") {

    SECTION("destruction"){
        std::cout << "Expect: 1 non-trivial destruct and 1 trivial destruct" << "\n";
        static_vector<int, 1> v1;
        static_vector<std::string, 1> v2;
        CHECK(true);
    }

    SECTION("more destruction"){
        int counter = 0;
        {
            static_vector<Counter, 3> v;
            v.emplace_back(&counter);
            v.emplace_back(&counter);
            v.emplace_back(&counter);
            CHECK(counter == 3);
        }
        CHECK(counter == 0);

        {
            static_vector<Counter, 3> v;
            v.emplace_back(&counter);
            v.emplace_back(&counter);
            v.emplace_back(&counter);
            static_vector<Counter, 3> v2 = v;
            CHECK(counter == 6);
        }
        CHECK(counter == 0);

        {
            static_vector<Counter, 3> v;
            v.emplace_back(&counter);
            v.emplace_back(&counter);
            v.emplace_back(&counter);
            static_vector<Counter, 3> v2 = std::move(v);
            CHECK(counter == 3);
        }
        CHECK(counter == 0);
    }

    SECTION("copy"){
        std::cout << "Expect: 1 trivial copy and 1 non-trivial copy" << "\n";
        static_vector<int, 8> v1 (4, 42);
		CHECK(v1.size() == 4);
        static_vector<int, 8> v2 = v1;
		CHECK(v1.size() == 4);
		CHECK(v2.size() == 4);
        static_vector<std::string, 8> v3;
        v3.push_back(std::string("Hello"));
        static_vector<std::string, 8> v4 = v3;
        CHECK(true);
    }
}

// inlined_vector<int, -1, false> gNegativeSizedVectorWillStaticAssert;

TEST_CASE("inlined_vector sizeof", "[inlined_vector]") {
    inlined_vector<int, 16, false> v1;
    inlined_vector<int, 16, true> v2;
    CHECK(sizeof(v1) < sizeof(v2));
}

TEST_CASE("inlined_vector operator<<", "[inlined_vector]") {
    std::cout << "Testing output...\n";
    inlined_vector<int, 16, false> v1 { 1, 2, 3, 4, 5 };
    inlined_vector<int, 16, false> v2 { 6, 7, 8, 9, 10 };
    std::cout << v1 << "\n";
    std::cout << v2 << "\n";
    CHECK(true);
}

TEST_CASE("inlined_vector basic construction", "[inlined_vector]") {
    inlined_vector<int, 16, false> v1;
    CHECK(v1.size() == 0);
    CHECK(v1.max_size() == 16);
    CHECK(!v1.expanded());

    inlined_vector<int, 16, true> v2;
    CHECK(v2.size() == 0);
    CHECK(v2.max_size() == 16);
    CHECK(!v2.expanded());
}

TEST_CASE("inlined_vector basic operations 1", "[inlined_vector]") {
    inlined_vector<int, 16, false> v1 { 3 };
    v1.push_back(42);
    CHECK(v1.size() == 2);
    CHECK(v1.front() == 3);
    CHECK(v1.back() == 42);
}

TEST_CASE("inlined_vector basic operations 2", "[inlined_vector]") {
	inlined_vector<int, 16, false> v { 1, 2, 3, 4, 5 };
	REQUIRE(v.max_size() == 16);
	REQUIRE(v.size() == 5);

	SECTION("operator[], back, and front works") {
		CHECK(v[0] == 1);
		CHECK(v[1] == 2);
		CHECK(v[2] == 3);
		CHECK(v[3] == 4);
		CHECK(v[4] == 5);
		CHECK(v.front() == 1);
		CHECK(v.back() == 5);
	}

	SECTION("can push back and pop") {
		v.push_back(13);
		REQUIRE(v.size() == 6);
		REQUIRE(v.back() == 13);
		int popResult = v.back();
		v.pop_back();
		CHECK(popResult == 13);
		CHECK(v.size() == 5);
	}

	SECTION("can erase") {
		int valueToErase = 3;
		CHECK(v.contains(valueToErase));
		auto it = v.begin();
		while (it != v.end()) {
			if (*it == valueToErase) {
				it = v.erase(it);
			}
			else it++;
		}
		CHECK(!v.contains(valueToErase));
	}

    SECTION("can insert") {
        auto it = v.insert(std::next(v.begin(), 3), 42);
        CHECK(v.contains(42));
        CHECK(v[3] == 42);
        CHECK(v.size() == 6);
        v.erase(it);
    }

	SECTION("iteration") {
        for (auto& p: v){
            p += 1;            
        }
        CHECK(v[4] == 6);
        for (auto& p: v){
            p -= 1;
        }

        int i = 5;
        for (auto it = v.rbegin(); it != v.rend(); ++it){
            CHECK(*it == i);
            i--;
        }
    }

    SECTION("can clear") {
        v.clear();
        CHECK(v.empty());
        CHECK(v.size() == 0);
    }
}

TEST_CASE("inlined_vector basic operations 1 (expandable)", "[inlined_vector]") {
    inlined_vector<int, 16, true> v1 { 3 };
    CHECK(v1.front() == 3);
    CHECK(v1.size() == 1);
    v1.push_back(42);
    CHECK(v1.size() == 2);
    CHECK(v1.front() == 3);
    CHECK(v1.back() == 42);
}

struct simple_pair { int a, b; };
TEST_CASE("inlined_vector aggregate push_back", "[inlined_vector]") {
    inlined_vector<simple_pair, 16, true> v;
    v.push_back({4, 4});
    CHECK(v.size() == 1);
    CHECK(v.front().a == 4);
    CHECK(v.front().b == 4);
}

TEST_CASE("inlined_vector basic operations 2 (expandable)", "[inlined_vector]") {
	inlined_vector<int, 16, true> v { 1, 2, 3, 4, 5 };
	REQUIRE(v.max_size() == 16);
	REQUIRE(v.size() == 5);

	SECTION("operator[], back, and front works") {
		CHECK(v[0] == 1);
		CHECK(v[1] == 2);
		CHECK(v[2] == 3);
		CHECK(v[3] == 4);
		CHECK(v[4] == 5);
		CHECK(v.front() == 1);
		CHECK(v.back() == 5);
	}

	SECTION("can push back and pop") {
        v.push_back(13);
		REQUIRE(v.size() == 6);
		REQUIRE(v.back() == 13);
		int popResult = v.back();
		v.pop_back();
		CHECK(popResult == 13);
		CHECK(v.size() == 5);
	}

    SECTION("can push back and pop beyond capacity") {
        CHECK(!v.expanded());
        for (int i=0; i<100; i++) v.push_back(i);
        CHECK(v.size() == 105);
        CHECK(v.expanded());
        for (int i=0; i<100; i++) v.pop_back();
        CHECK(v.size() == 5);
	}

	SECTION("can erase") {
		int valueToErase = 3;
		CHECK(v.contains(valueToErase));
		auto it = v.begin();
		while (it != v.end()) {
			if (*it == valueToErase) {
				it = v.erase(it);
			}
			else it++;
		}
		CHECK(!v.contains(valueToErase));
	}

    SECTION("can insert") {
        auto it = v.insert(std::next(v.begin(), 3), 42);
        CHECK(v.contains(42));
        CHECK(v[3] == 42);
        CHECK(v.size() == 6);
        v.erase(it);
    }

    SECTION("iteration") {
        for (auto& p: v){
            p += 1;            
        }
        CHECK(v[4] == 6);
        for (auto& p: v){
            p -= 1;
        }

        int i = 5;
        for (auto it = v.rbegin(); it != v.rend(); ++it){
            CHECK(*it == i);
            i--;
        }
    }
    
    SECTION("can clear") {
        v.clear();
        CHECK(v.empty());
        CHECK(v.size() == 0);
    }
}

struct MoveOnly {
    MoveOnly(int value = 0):value(value){}
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&& other):value(std::move(other.value)){ other.value = 0; }
    MoveOnly& operator=(MoveOnly&& other){ value = std::move(other.value); other.value = 0; return *this;}
    int value = 0;
};

TEST_CASE("inlined_vector construction", "[inlined_vector]"){
    SECTION("construct with duplicates"){     
        inlined_vector<int, 8, false> fv (6, 42);
        std::vector<int> result {42, 42, 42, 42, 42, 42 };
        CHECK(fv.size() == 6);
        CHECK_THAT(fv, Equals(fv, result));
    }

    SECTION("construct with duplicates"){     
        inlined_vector<int, 8, true> fv (6, 42);
        std::vector<int> result {42, 42, 42, 42, 42, 42 };
        CHECK(fv.size() == 6);
        CHECK_THAT(fv, Equals(fv, result));
    }

    SECTION("construct with duplicates"){     
        inlined_vector<int, 8, true> fv (100, 42);
        CHECK(fv.size() == 100);
        for (int i=0; i<100; i++){
            CHECK(fv[i] == 42);
        }
    }

    SECTION("construct from initialiser_list"){     
        inlined_vector<int, 8, false> fv { 1, 2, 3, 4, 5 };
        std::vector<int> v {1, 2, 3, 4, 5};
        CHECK_THAT(fv, Equals(fv, v));
    }

    SECTION("construct from initialiser_list"){     
        inlined_vector<int, 8, true> fv { 1, 2, 3, 4, 5 };
        std::vector<int> v {1, 2, 3, 4, 5};
        CHECK_THAT(fv, Equals(fv, v));
    }

    SECTION("construct from std::vector"){
        std::vector<int> v {1, 2, 3, 4, 5};
        inlined_vector<int, 8, false> fv = v;
        CHECK_THAT(fv, Equals(fv, v));
    }

    SECTION("construct from std::vector"){
        std::vector<int> v {1, 2, 3, 4, 5};
        inlined_vector<int, 8, false> fv = v;
        CHECK_THAT(fv, Equals(fv, v));
    }

    SECTION("copy construct"){
        inlined_vector<int, 8, false> v1 { 1, 2, 3, 4, 5 };
        inlined_vector<int, 8, false> v2 { v1 };
        CHECK_THAT(v1, Equals(v1, v2));
    }

    SECTION("copy construct"){
        inlined_vector<int, 8, true> v1 { 1, 2, 3, 4, 5 };
        inlined_vector<int, 8, true> v2 { v1 };
        CHECK_THAT(v1, Equals(v1, v2));
    }

    SECTION("move construct"){
        auto res = { 1, 2, 3, 4, 5 };
        inlined_vector<int, 8, false> v1 { res };
        inlined_vector<int, 8, false> v2 { std::move(v1) };
        CHECK_THAT(v2, Equals(v2, res));
    }

    SECTION("move construct"){
        auto res = { 1, 2, 3, 4, 5 };
        inlined_vector<int, 8, true> v1 { res };
        inlined_vector<int, 8, true> v2 { std::move(v1) };
        CHECK_THAT(v2, Equals(v2, res));
    }

	SECTION("move construct") {
		auto res = { 1, 2, 3, 4, 5 };
		inlined_vector<int, 2, true> v1{ res };
		inlined_vector<int, 2, true> v2{ std::move(v1) };
		CHECK_THAT(v2, Equals(v2, res));
	}

    SECTION("move construct"){
        inlined_vector<MoveOnly, 8, false> v1;
        v1.emplace_back();
        v1.emplace_back();
        v1.emplace_back();
        v1.emplace_back();

        inlined_vector<MoveOnly, 8, false> v2 { std::move(v1) };
        CHECK(v2.size() == 4);
    }

    SECTION("move construct"){
        inlined_vector<MoveOnly, 8, true> v1;
        v1.emplace_back();
        v1.emplace_back();
        v1.emplace_back();
        v1.emplace_back();

        inlined_vector<MoveOnly, 8, true> v2 { std::move(v1) };
        CHECK(v2.size() == 4);
    }

    SECTION("copy assignment"){
        auto res = { 1, 2, 3, 4, 5 };
        inlined_vector<int, 8, false> v1 { res };
        inlined_vector<int, 8, false> v2;
        v2 = v1;
        CHECK_THAT(v2, Equals(v2, res));
    }

    SECTION("copy assignment"){
        auto res = { 1, 2, 3, 4, 5 };
        inlined_vector<int, 8, true> v1 { res };
        inlined_vector<int, 8, true> v2;
        v2 = v1;
        CHECK_THAT(v2, Equals(v2, res));
    }

    SECTION("move assignment"){
        auto res = { 1, 2, 3, 4, 5 };
        inlined_vector<int, 8, false> v1 { res };
        inlined_vector<int, 8, false> v2;
        v2 = std::move(v1);
        CHECK_THAT(v2, Equals(v2, res));
    }

    SECTION("move assignment"){
        auto res = { 1, 2, 3, 4, 5 };
        inlined_vector<int, 8, true> v1 { res };
        inlined_vector<int, 8, true> v2;
        v2 = std::move(v1);
        CHECK_THAT(v2, Equals(v2, res));
    }

    SECTION("move assignment"){
        auto res = { 1, 2, 3, 4, 5 };
        inlined_vector<int, 8, true> v1 { res };
        inlined_vector<int, 2, true> v2;
        v2 = std::move(v1);
        CHECK_THAT(v2, Equals(v2, res));
    }

    SECTION("extend"){
        auto res1 = { 1, 2, 3, 4, 5 };
        auto res2 = { 1, 2, 3, 4, 5 };
        inlined_vector<int, 8, true> v1 { res1 };
        v1.extend(res2);
        CHECK(v1.size() == 10);
    }
}

struct EmplaceableStruct { 
    int a = 0, b = 0, c = 0; 
    EmplaceableStruct() = default; 
    EmplaceableStruct(int a, int b, int c):a{a},b{b},c{c}{} 
};

TEST_CASE("inlined_vector emplacement", "[inlined_vector]") {
    SECTION("vector"){
        inlined_vector<std::unique_ptr<int>, 2, false> v;
        for (int i=0; i<2; ++i) v.emplace_back(new int {i});
        CHECK(*v.back() == 1);
    }

    SECTION("vector (expandable)"){
        inlined_vector<std::unique_ptr<int>, 2, true> v;
        for (int i=0; i<10; ++i) v.emplace_back(new int {i});
        CHECK(*v.back() == 9);
    }

    SECTION("many params"){        
        inlined_vector<EmplaceableStruct, 2, false> v;
        for (int i=0; i<2; ++i) v.emplace_back(0, i, 1 + i);
    }

    SECTION("many params (expandable)"){
        inlined_vector<EmplaceableStruct, 2, true> v;
        for (int i=0; i<10; ++i) v.emplace_back(0, i, 1 + i);
    }
}

#ifdef BSP_INLINED_VECTOR_THROWS
TEST_CASE("inlined_vector exception reporting", "[inlined_vector]"){
    SECTION ("too many elements in std::vector"){
        std::vector<int> v (100, 0);
        CHECK_THROWS(inlined_vector<int, 8, false>(v)); //, std::exception);
    }

    SECTION ("too many elements in initializer_list"){
        CHECK_THROWS(inlined_vector<int, 8, false> {1, 2, 3, 4, 5, 6, 7, 8, 9}); //, std::exception);
    }

    SECTION ("too many elements in copy constructor"){
        inlined_vector<int, 4, false> v1 { 1, 2, 3, 4};
        CHECK_THROWS(inlined_vector<int, 2, false> {v1});
    }

    SECTION ("too many elements in copy constructor"){
        inlined_vector<int, 4, true> v1 { 1, 2, 3, 4};
        CHECK_THROWS(inlined_vector<int, 2, false> {v1});
    }

    SECTION ("too many elements in push_back"){
        inlined_vector<int, 4, false> v1 { 1, 2, 3 };
        CHECK_NOTHROW(v1.push_back(42));
        CHECK_THROWS(v1.push_back(666));
    }
}
#endif

#ifndef BSP_INLINED_VECTOR_THROWS
TEST_CASE("inlined_vector ignore extra elements", "[inlined_vector]"){
    SECTION ("too many elements in std::vector"){
        std::vector<int> v (100, 42);
        v[7] = 1;
        inlined_vector<int, 8, false> v2 = v;
        CHECK(v2.front() == 42);
        CHECK(v2.back() == 1);
    }
}
#endif

TEST_CASE("inlined_vector moveability", "[inlined_vector]"){    
    SECTION("can move element into vector"){
        inlined_vector<MoveOnly, 8, false> v;
        MoveOnly p {3};
        v.push_back(std::move(p));
        auto& q = v.back();
        q.value = 6;
        CHECK(v.back().value == 6);
    }

    SECTION("can move vector"){
        inlined_vector<MoveOnly, 8, false> v1;
        MoveOnly p {3};
        v1.push_back(std::move(p));
        inlined_vector<MoveOnly, 8, false> v2 = std::move(v1);
        CHECK(v2.front().value == 3);
    }
}

TEST_CASE("inlined_vector moveability (expanded)", "[inlined_vector]"){    
    SECTION("can move element into vector"){
        inlined_vector<MoveOnly, 8, true> v;
        MoveOnly p {3};
        v.push_back(std::move(p));
        auto& q = v.back();
        q.value = 6;
        CHECK(v.back().value == 6);
    }

    SECTION("can move vector"){
        inlined_vector<MoveOnly, 8, true> v1;
        MoveOnly p {3};
        v1.push_back(std::move(p));
        inlined_vector<MoveOnly, 8, true> v2 = std::move(v1);
        CHECK(v2.front().value == 3);
    }

    SECTION("can expand vector with moveable elements"){
        inlined_vector<MoveOnly, 8, true> v;
        v.emplace_back(0);
        for (int i=0; i<10; i++){
            v.push_back(MoveOnly{42});
        }
        v.emplace_back(42);
        CHECK(v.back().value == 42);
    }
}

TEST_CASE("inlined_vector assignment", "[inlined_vector]"){
    inlined_vector<int, 4, false> v1 { 1, 2, 3, 4 };
    inlined_vector<int, 4, true> v2 { 1, 2, 4, 8 };
    inlined_vector<int, 8, false> v3 { 0, 1, 0, 1, 0, 1, 0, 1};
    inlined_vector<int, 8, true> v4 { 42, 42, 42, 42, 42, 42, 42, 42 };

    v1 = v2;
    CHECK_THAT(v1, Equals(v1, v2));

    v2 = v3;
    CHECK_THAT(v2, Equals(v2, v3));

    v3 = v4;
    CHECK_THAT(v3, Equals(v3, v4));
}

using Clock = std::chrono::high_resolution_clock;
struct Profile {
    decltype(Clock::now()) start = Clock::now();
    ~Profile(){
        auto end = Clock::now();
        std::cout << "- result: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << "ns\n";
    }
};

TEST_CASE("inlined_vector benchmark", "[inlined_vector]"){
    std::cout << "Performing basic benchmarks\n";

    constexpr int ArraySize = 128;
    constexpr int VecSize = 128;

    {
        std::cout << "inlined_vector\n";
        Profile profiler;        

        std::array<inlined_vector<int, VecSize, false>, ArraySize> vecs;
        for (auto& vec: vecs){
            for (int i=0; i<VecSize; i++){
                vec.push_back(i);
            }
        }
    }

    {
        std::cout << "inlined_vector forced to expand\n";
        Profile profiler;

        std::array<inlined_vector<int, VecSize/2, true>, ArraySize> vecs;
        for (auto& vec: vecs){
            for (int i=0; i<VecSize; i++){
                vec.push_back(i);
            }
        }
    }

    {
        std::cout << "std::vector\n";
        Profile profiler;

        std::array<std::vector<int>, ArraySize> vecs;
        for (auto& vec: vecs){
            for (int i=0; i<VecSize; i++){
                vec.push_back(i);
            }
        }
    }
}

TEST_CASE("inlined_vector move", "[inlined_vector]") {
    using vector = inlined_vector<int, 2, true>;
    vector v1 {1, 2, 3, 4, 5};
    vector v2 {1, 2, 3};
    CHECK(v1.size() == 5);
    CHECK(v2.size() == 3);
    v2 = std::move(v1);

	// in-place move
	std::aligned_storage<sizeof(vector), alignof(vector)>::type storage;
	new (&storage) vector(std::move(v2));
	vector* v3 = reinterpret_cast<vector*>(&storage);

	CHECK(v2.size() == 0);
	CHECK(v3->size() == 5);

	v3->~vector();
}
