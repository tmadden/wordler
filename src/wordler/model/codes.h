#pragma once

#include "puzzle.h"

// This file provides functions for converting puzzle definitions to and from
// URL-friendly codes.

std::string
generate_puzzle_code(puzzle_definition const& puzzle);

puzzle_definition
parse_puzzle_code(std::string const& code);
