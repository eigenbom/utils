#include <iostream>
#include <typeindex>

#define BSP_ARRAY2D_ALLOCATION(array, bytes) {\
    /* if (bytes > 1024 * 1024){ */ \
    std::cout << "resizing array2d<" << typeid((array)(0,0)).name() << ">" \
    << "(" << (array).width() << "x" << (array).height() << ") " \
    << bytes << "B\n";} \
    /* } */

#include "../include/array2d.h"

#include "catch.hpp"
#include "container_matcher.h"

using bsp::array2d;
using Catch::Equals;

TEST_CASE("array2d basics", "[array2d]") {
    array2d<int> arr { 2, 2, 42 };
    CHECK(arr.width() == 2);
    CHECK(arr.height() == 2);
    CHECK(arr(0,0) == 42);
    CHECK(arr(1,0) == 42);
    CHECK(arr(0,1) == 42);
    CHECK(arr(1,1) == 42);
    CHECK(arr(-1,0) == 0);

    arr.resize(4, 2, 42);
    CHECK(arr.width() == 4);
    CHECK(arr.height() == 2);    
    CHECK(arr(0,0) == 42);
}

TEST_CASE("array2d construction", "[array2d]") {
    SECTION("default construction"){
        array2d<int> arr;
        CHECK(arr.width() == 0);
        CHECK(arr.height() == 0);
    }

    SECTION("copy construction"){
        array2d<int> arr {8, 8, 0};
        array2d<int> arr2 {arr};
        CHECK(arr2.width() == 8);
        CHECK(arr2.height() == 8);
    }

    SECTION("copy assignment"){
        array2d<int> arr {8, 8, 0};
        array2d<int> arr2;
        arr2 = arr;
        CHECK(arr2.width() == 8);
        CHECK(arr2.height() == 8);
    }

    SECTION("move construction"){
        array2d<int> arr {8, 8, 0};
        array2d<int> arr2 {std::move(arr)};
        CHECK(arr2.width() == 8);
        CHECK(arr2.height() == 8);
        CHECK(arr.width() == 0);
        CHECK(arr.height() == 0);
    }

    SECTION("move assignment"){
        array2d<int> arr {8, 8, 0};
        array2d<int> arr2;
        arr2 = std::move(arr);
        CHECK(arr2.width() == 8);
        CHECK(arr2.height() == 8);
        CHECK(arr.width() == 0);
        CHECK(arr.height() == 0);
    }
}


TEST_CASE("array2d operator<<", "[array2d]") {
    array2d<int> identity { 2, 2 };
    identity(0, 0) = 1;
    identity(1, 1) = 1;
    std::cout << "Should print the identity array2d...\n";
    std::cout << identity << "\n";
}