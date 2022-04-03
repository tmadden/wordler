#pragma once

#include "puzzle.h"

std::string
generate_puzzle_code(puzzle_definition const& puzzle);

puzzle_definition
parse_puzzle_code(std::string const& code);
