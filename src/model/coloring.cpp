#include "model/coloring.h"

colorful_text
color_guess(puzzle_definition const& puzzle, std::string const& guess)
{
    // Do a histogram of all the letters in 'the word'.
    int letter_counts[26] = {0};
    for (char letter : puzzle.the_word)
        ++letter_counts[letter - 'a'];

    // Now go through and mark all the correct letters in the guess.
    // (And, importantly, also remove them from the histogram.)
    colorful_text text;
    size_t word_length = puzzle.the_word.length();
    text.reserve(word_length);
    for (size_t i = 0; i != word_length; ++i)
    {
        char letter = guess[i];
        letter_color color = INCORRECT;
        if (letter == puzzle.the_word[i])
        {
            color = CORRECT;
            --letter_counts[letter - 'a'];
        }
        text.push_back(colorful_letter{color, letter});
    }

    // Now that all the correct letters are accounted for, mark any misplaced
    // letters.
    for (size_t i = 0; i != word_length; ++i)
    {
        char letter = guess[i];
        if (text[i].color == INCORRECT && letter_counts[letter - 'a'] != 0)
        {
            --letter_counts[letter - 'a'];
            text[i].color = MISPLACED;
        }
    }
    return text;
}

colorful_text
empty_row(puzzle_definition const& puzzle)
{
    colorful_text row;
    for (size_t i = 0; i != puzzle.the_word.size(); ++i)
        row.push_back(colorful_letter{NEUTRAL, ' '});
    return row;
}

colorful_text
color_active_guess(puzzle_definition const& puzzle, puzzle_state const& state)
{
    colorful_text row;
    for (char letter : state.active_guess)
        row.push_back(colorful_letter{
            state.tried_an_invalid_word ? ERROR : NEUTRAL, letter});
    if (row.size() < puzzle.the_word.size())
        row.push_back(colorful_letter{CURSOR, ' '});
    while (row.size() < puzzle.the_word.size())
        row.push_back(colorful_letter{NEUTRAL, ' '});
    return row;
}

std::vector<colorful_text>
make_letter_rows(puzzle_definition const& puzzle, puzzle_state const& state)
{
    std::vector<colorful_text> rows;
    for (auto const& guess : state.guesses)
        rows.push_back(color_guess(puzzle, guess));
    if (rows.size() < puzzle.max_guesses)
        rows.push_back(color_active_guess(puzzle, state));
    while (rows.size() < puzzle.max_guesses)
        rows.push_back(empty_row(puzzle));
    return rows;
}
