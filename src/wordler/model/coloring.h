#pragma once

#include <array>
#include <compare>

#include <wordler/model/puzzle.h>

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

// Score a guess.
colorful_text
score_guess(puzzle_definition const& puzzle, std::string const& guesses);

// Score multiple guesses.
std::vector<colorful_text>
score_guesses(
    puzzle_definition const& puzzle, std::vector<std::string> const& guesses);

// Use the information from the scored guesses to determine what colors the
// letter keys should be on the keyboard.
std::array<letter_color, 26>
extract_key_colors(std::vector<colorful_text> const& scored_guesses);

// Add the rows on the board that represent unfinished (or unstarted) guesses.
std::vector<colorful_text>
add_unfinished_rows(
    std::vector<colorful_text> const& guesses,
    puzzle_definition const& puzzle,
    puzzle_state const& state);