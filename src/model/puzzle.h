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
