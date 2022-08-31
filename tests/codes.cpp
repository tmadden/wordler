#include <wordler/model/codes.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("puzzle codes", "[model]")
{
    // Check that various puzzle definitions successfully make a roundtrip to
    // code form and back.
    auto test_code = [](puzzle_definition const& puzzle) {
        REQUIRE(parse_puzzle_code(generate_puzzle_code(puzzle)) == puzzle);
    };
    test_code(puzzle_definition{"trivial", 6, false, "tmadden"});
    test_code(puzzle_definition{"tabernacle", 7, true, "Meryll"});
    test_code(puzzle_definition{"hazel", 1, false, "Gina"});
}

TEST_CASE("legacy puzzle codes", "[model]")
{
    // v1
    REQUIRE(
        parse_puzzle_code("AQdUcml2aWFsBgA")
        == puzzle_definition{"trivial", 6, false, ""});
    REQUIRE(
        parse_puzzle_code("AQp0YWJlcm5hY2xlBgA")
        == puzzle_definition{"tabernacle", 6, false, ""});
    REQUIRE(
        parse_puzzle_code("AQp0YWJlcm5hY2xlBwA")
        == puzzle_definition{"tabernacle", 7, false, ""});

    // v2
    REQUIRE(
        parse_puzzle_code("Agd0cml2aWFsBgAHdG1hZGRlbg")
        == puzzle_definition{"trivial", 6, false, "tmadden"});
    REQUIRE(
        parse_puzzle_code("Agp0YWJlcm5hY2xlBwEGTWVyeWxs")
        == puzzle_definition{"tabernacle", 7, true, "Meryll"});
    REQUIRE(
        parse_puzzle_code("AgVoYXplbAEABEdpbmE")
        == puzzle_definition{"hazel", 1, false, "Gina"});
}
