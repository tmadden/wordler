#pragma once

#include <string>
#include <vector>

struct puzzle_definition
{
    std::string the_word;
    int max_guesses;
    bool disable_dict_warning;

    bool
    operator==(puzzle_definition const&) const = default;
};

struct puzzle_state
{
    std::vector<std::string> guesses;
    std::string active_guess;
    bool tried_an_invalid_word = false;
};

bool
puzzle_is_solved(puzzle_definition const& puzzle, puzzle_state const& state);

bool
out_of_guesses(puzzle_definition const& puzzle, puzzle_state const& state);

std::string
puzzle_state_to_json(puzzle_state const& state);

puzzle_state
json_to_puzzle_state(std::string json);
