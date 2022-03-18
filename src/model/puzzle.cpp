#include "puzzle.h"

#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

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

// CEREAL INTERFACES - We use Cereal to convert our puzzle state to and from
// JSON for external storage in the browser.

template<class Archive>
void
serialize(Archive& archive, puzzle_state& state)
{
    archive(state.guesses, state.active_guess);
}

std::string
puzzle_state_to_json(puzzle_state const& state)
{
    std::ostringstream stream;
    {
        cereal::JSONOutputArchive archive(stream);
        archive(state);
    }
    return stream.str();
}

puzzle_state
json_to_puzzle_state(std::string json)
{
    try
    {
        std::istringstream stream(std::move(json));
        puzzle_state state;
        {
            cereal::JSONInputArchive archive(stream);
            archive(state);
        }
        return state;
    }
    catch (...)
    {
        // If anything goes wrong, just return a fresh state.
        return puzzle_state();
    }
}