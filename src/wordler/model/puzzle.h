#pragma once

#include <string>
#include <vector>

// TYPES

struct puzzle_definition
{
    std::string the_word;
    int max_guesses;
    bool disable_dict_warning;
    std::string author;

    bool
    operator==(puzzle_definition const&) const = default;
};

struct puzzle_state
{
    std::vector<std::string> guesses;
    std::string active_guess;
    bool tried_an_invalid_word = false;
};

// QUERY FUNCTIONS

bool
puzzle_is_solved(puzzle_definition const& puzzle, puzzle_state const& state);

bool
out_of_guesses(puzzle_definition const& puzzle, puzzle_state const& state);

// JSON I/O

std::string
puzzle_state_to_json(puzzle_state const& state);

puzzle_state
json_to_puzzle_state(std::string json);
