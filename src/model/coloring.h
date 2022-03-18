#pragma once

#include "model/puzzle.h"

enum letter_color
{
    NEUTRAL,
    CURSOR,
    ERROR,
    INCORRECT,
    MISPLACED,
    CORRECT
};

struct colorful_letter
{
    letter_color color;
    char letter;
};

typedef std::vector<colorful_letter> colorful_text;

std::vector<colorful_text>
make_letter_rows(puzzle_definition const& puzzle, puzzle_state const& state);
