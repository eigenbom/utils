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

// #define BSP_FIXED_STRING_ZERO_CONTENTS
// #define BSP_FIXED_STRING_THROWS
#define BSP_FIXED_STRING_LOG_ERROR(message) std::cerr << message << "\n"
#include "../include/fixed_string.h"

#include "catch.hpp"
#include "container_matcher.h"

using bsp::fixed_string;
using Catch::Equals;

TEST_CASE("fixed_string basic conversion needed for tests", "[fixed_string]") {
    fixed_string<8> str { "Hello" };
    REQUIRE_THAT(str.str(), Equals("Hello"));
    REQUIRE_THAT(str.c_str(), Equals("Hello"));
}

TEST_CASE("fixed_string basics", "[fixed_string]") {

    SECTION("default constructed"){
        fixed_string<8> str;
        CHECK(str.max_size() == 8);    
        CHECK(str.size() == 0);
        CHECK(str.empty());
    }

    SECTION("constructed"){
        fixed_string<8> str { "Hello" };
        CHECK(str.max_size() == 8);    
        CHECK(str.size() == 5);
        CHECK(!str.empty());
        CHECK(str[0] == 'H');
        CHECK(str[1] == 'e');
        CHECK(str[4] == 'o');
        CHECK(str[5] == '\0');
        CHECK_THAT(str.str(), Equals("Hello"));
    }

    SECTION("copied"){
        fixed_string<8> str { "Hello" };
        str = "Goodbye";
        CHECK(str.size() == 7);
        CHECK_THAT(str.str(), Equals("Goodbye"));
    }

    SECTION("iterators"){
        fixed_string<8> str { "Hello" };
        CHECK(&*str.begin() == &str[0]);
        CHECK(std::distance(str.begin(), str.end()) == 5);
    }
}

TEST_CASE("fixed_string basic operations", "[fixed_string]") {
    SECTION ("construction"){
        fixed_string<8> str { "Hello" };
        CHECK_THAT(str.str(), Equals("Hello"));
    }

    SECTION ("construction"){
        std::string hello = "Hello";
        fixed_string<8> str { hello };
        CHECK_THAT(str.str(), Equals("Hello"));
    }

    SECTION ("construction"){
        fixed_string<8> hello = "Hello";
        fixed_string<8> str { hello };
        CHECK_THAT(str.str(), Equals("Hello"));
    }

    SECTION ("construction"){
        fixed_string<8> hello = "Hello";
        fixed_string<16> str { hello };
        CHECK_THAT(str.str(), Equals("Hello"));
    }

    SECTION ("copy construction"){
        fixed_string<8> hello = "Hello";
        fixed_string<8> str = hello;
        CHECK_THAT(str.str(), Equals("Hello"));
    }

    SECTION ("copy assignment"){
        fixed_string<8> hello = "Hello";
        fixed_string<8> str;
        str = hello;
        CHECK(str.size() == 5);
        CHECK_THAT(str.str(), Equals("Hello"));
    }

    SECTION ("move construction"){
        fixed_string<8> hello = "Hello";
        fixed_string<8> str { std::move(hello) };
        CHECK_THAT(str.str(), Equals("Hello"));
    }

    SECTION ("move assignment"){
        fixed_string<16> hello = "Hello";
        fixed_string<8> str {false};
        str = std::move(hello);
        CHECK_THAT(str.str(), Equals("Hello"));
    }

    SECTION ("move assignment"){
        fixed_string<16> hello = "Hello";
        fixed_string<4> str {true};
        str = std::move(hello);
        CHECK_THAT(str.str(), Equals("Hell"));
    }

     SECTION("assign_to"){
        fixed_string<8> str { "Hello" };
        std::string output;
        str.assign_to(output);
        CHECK_THAT(output, Equals("Hello"));
     }
}

TEST_CASE("fixed_string truncates", "[fixed_string]") {
    SECTION ("construction"){
        fixed_string<8> str { "Hello World", true };
        CHECK(str.size() == 8);
        CHECK_THAT(str.str(), Equals("Hello Wo"));
    }

    SECTION ("copy assignment"){
        fixed_string<8> str { true };
        str = "Hello World";
        CHECK(str.size() == 8);
        CHECK_THAT(str.str(), Equals("Hello Wo"));
    }
}

#ifdef BSP_FIXED_STRING_THROWS

TEST_CASE("fixed_string too many characters", "[fixed_string]") {
    SECTION ("construction"){
        std::cout << "Should print an error...\n";
        CHECK_THROWS(fixed_string<8> { "Hello World" });
    }

     SECTION ("construction"){
        fixed_string<16> str1 { "Hello World" };
        std::cout << "Should print an error...\n";
        CHECK_THROWS(fixed_string<8> {str1});
    }

    SECTION ("copy assignment"){
        fixed_string<8> str;
        std::cout << "Should print an error...\n";
        CHECK_THROWS(str = "Hello World");
    }
}

#else 

TEST_CASE("fixed_string too many characters", "[fixed_string]") {
    SECTION ("construction"){
        std::cout << "Should print an error...\n";
        fixed_string<8> str { "Hello World" };
        CHECK(str.size() == 8);
        CHECK_THAT(str.str(), Equals("Hello Wo"));
    }

     SECTION ("construction"){
        fixed_string<16> str1 { "Hello World" };
        std::cout << "Should print an error...\n";
        fixed_string<8> str2 {str1};
        CHECK(str2.size() == 8);
        CHECK_THAT(str2.str(), Equals("Hello Wo"));
    }

    SECTION ("copy assignment"){
        fixed_string<8> str;
        std::cout << "Should print an error...\n";
        str = "Hello World";
        CHECK(str.size() == 8);
        CHECK_THAT(str.str(), Equals("Hello Wo"));
    }
}

#endif