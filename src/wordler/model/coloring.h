#pragma once

#include <array>
#include <compare>

#include <wordler/model/dictionaries.h>
#include <wordler/model/puzzle.h>

// This file provides functions for coloring the puzzle boxes.

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
score_guess(std::string const& the_word, std::string const& guesses);

// Score multiple guesses.
std::vector<colorful_text>
score_guesses(
    puzzle_definition const& puzzle, std::vector<std::string> const& guesses);

// Add the rows on the board that represent unfinished (or unstarted) guesses.
std::vector<colorful_text>
add_unfinished_rows(
    std::vector<colorful_text> const& guesses,
    puzzle_definition const& puzzle,
    puzzle_state const& state);

// Use the information from the scored guesses to determine what colors the
// letter keys should be on the keyboard.
std::array<letter_color, 26>
extract_key_colors(std::vector<colorful_text> const& scored_guesses);

// Determine what guess words could've generated a given score.
std::vector<std::string>
decode_score(
    std::string const& the_word,
    dictionary const& dict,
    std::vector<letter_color> const& score);
