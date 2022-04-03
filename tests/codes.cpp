#include <wordler/model/codes.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("puzzle codes", "[model]")
{
    auto test_code = [](puzzle_definition const& puzzle) {
        REQUIRE(parse_puzzle_code(generate_puzzle_code(puzzle)) == puzzle);
    };

    test_code(puzzle_definition{"trivial", 6, false});
    test_code(puzzle_definition{"tabernacle", 7, true});
    test_code(puzzle_definition{"hazel", 1, false});
}

TEST_CASE("legacy puzzle codes", "[model]")
{
    // v1
    REQUIRE(
        parse_puzzle_code("AQdUcml2aWFsBgA")
        == puzzle_definition{"trivial", 6, false});
    REQUIRE(
        parse_puzzle_code("AQp0YWJlcm5hY2xlBgA")
        == puzzle_definition{"tabernacle", 6, false});
    REQUIRE(
        parse_puzzle_code("AQp0YWJlcm5hY2xlBwA")
        == puzzle_definition{"tabernacle", 7, false});
}
