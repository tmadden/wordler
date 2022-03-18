#include "puzzle.h"

bool
puzzle_is_solved(puzzle_definition const& puzzle, puzzle_state const& state)
{
    return std::any_of(
        state.guesses.begin(), state.guesses.end(), [&](auto const& guess) {
            return guess == puzzle.the_word;
        });
}

bool
out_of_guesses(puzzle_definition const& puzzle, puzzle_state const& state)
{
    return state.guesses.size() >= 6; // puzzle.max_guesses;
}
