#pragma once

#include <string>
#include <vector>

struct puzzle_definition
{
    std::string the_word;
    bool disable_dict_warning;
};

struct puzzle_state
{
    std::vector<std::string> guesses;
    std::string active_guess;
};

std::string
puzzle_state_to_json(puzzle_state const& state);

puzzle_state
json_to_puzzle_state(std::string json);
