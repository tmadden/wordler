#pragma once

#include <compare>

#include "model/puzzle.h"

enum letter_color
{
    CURSOR,
    ERROR,
    NEUTRAL,
    INCORRECT,
    MISPLACED,
    CORRECT
};

struct colorful_letter
{
    letter_color color;
    char letter;

    auto
    operator<=>(colorful_letter const&) const = default;
};

typedef std::vector<colorful_letter> colorful_text;

std::vector<colorful_text>
make_letter_rows(puzzle_definition const& puzzle, puzzle_state const& state);
