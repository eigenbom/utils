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

// #define BSP_FIXED_MAP_THROWS
#define BSP_FIXED_MAP_LOG_ERROR(message) std::cerr << message << "\n"
#include "../include/fixed_map.h"

#include "catch.hpp"
#include "container_matcher.h"

using bsp::fixed_map;
using Catch::Equals;

TEST_CASE("fixed_map basics", "[fixed_map]") {
    fixed_map<int, int, 8> map;
    CHECK(map.size() == 0);
    CHECK(map.empty());

    map.insert(0, 0);
    CHECK(map.size() == 1);
    CHECK(map[0] == 0);

    map.insert(1, 42);
    CHECK(map.size() == 2);
    CHECK(map[1] == 42);

    map.clear();
    CHECK(map.size() == 0);
}

TEST_CASE("fixed_map construction", "[fixed_map]") {
    SECTION("default construction"){
        fixed_map<int, int, 8> map;
    }

    SECTION("initialiser list construction"){
        fixed_map<int, int, 8> map { {0, 0}, {1, 42} };
        CHECK(map[0] == 0);
        CHECK(map[1] == 42);
    }

    SECTION("container construction"){
        std::vector<std::pair<int, int>> els { {0, 0}, {1, 42} };
        fixed_map<int, int, 8> map { els };
        CHECK(map[0] == 0);
        CHECK(map[1] == 42);
    }

    SECTION("initialiser list construction"){
        fixed_map<std::string, int, 8> map { {"hp", 16}, {"mp", 7}, {"int", 3} };
        CHECK(map["hp"] == 16);
    }

    SECTION("container construction"){
        std::vector<std::pair<std::string, int>> els { {"hp", 16}, {"mp", 7}, {"int", 3}  };
        fixed_map<std::string, int, 8> map { els };
        CHECK(map["hp"] == 16);
    }

    SECTION("copy construction"){
        fixed_map<int, int, 8> map1 { {0, 0}, {1, 42} };
        fixed_map<int, int, 8> map2 {map1};
        CHECK(map2[0] == 0);
        CHECK(map2[1] == 42);
    }

    SECTION("move construction"){
        fixed_map<int, int, 8> map1 { {0, 0}, {1, 42} };
        fixed_map<int, int, 8> map2 {std::move(map1)};
        CHECK(map2[0] == 0);
        CHECK(map2[1] == 42);
    }
}

TEST_CASE("fixed_map operator<<", "[fixed_map]") {
    std::cout << "testing operator<<...\n";
    fixed_map<int, int, 8> m1 { {0, 0}, {1, 42}, {6, 70} };
    std::cout << m1 << "\n";

    fixed_map<std::string, int, 8> m2 { {"hp", 16}, {"mp", 7}, {"int", 3} };
    std::cout << m2 << "\n";
}

TEST_CASE("fixed_map exceptions", "[fixed_map]") {
    CHECK_THROWS(fixed_map<int, int, 2>({ {0, 0}, {1, 42}, {6, 70} }));

    std::vector<std::pair<int, int>> els { {0, 0}, {1, 42}, {6, 70} };
    CHECK_THROWS(fixed_map<int, int, 2>(els));
}

#ifdef BSP_FIXED_MAP_THROWS

TEST_CASE("fixed_map exception in insertion", "[fixed_map]") {
    fixed_map<int, int, 2> map;
    map.insert(0, 0);
    map.insert(1, 1);
    CHECK_THROWS(map.insert(2, 1));
}

#endif

#ifndef BSP_FIXED_MAP_THROWS
#ifdef BSP_FIXED_MAP_LOG_ERROR

TEST_CASE("fixed_map error on insertion", "[fixed_map]") {
    fixed_map<int, int, 2> map;
    map.insert(0, 0);
    map.insert(1, 1);
    std::cout << "Should print an error...\n";
    map.insert(2, 1);
}

#endif
#endif