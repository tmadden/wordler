#include <wordler/model/coloring.h>

#include <iostream>

colorful_text
score_guess(std::string const& the_word, std::string const& guess)
{
    // Do a histogram of all the letters in 'the word'.
    int letter_counts[26] = {0};
    for (char letter : the_word)
        ++letter_counts[letter - 'a'];

    // Now go through and mark all the correct letters in the guess.
    // (And, importantly, also remove them from the histogram.)
    colorful_text text;
    size_t word_length = the_word.length();
    text.reserve(word_length);
    for (size_t i = 0; i != word_length; ++i)
    {
        char letter = guess[i];
        letter_color color = INCORRECT;
        if (letter == the_word[i])
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

std::vector<colorful_text>
score_guesses(
    puzzle_definition const& puzzle, std::vector<std::string> const& guesses)
{
    std::vector<colorful_text> rows;
    for (auto const& guess : guesses)
        rows.push_back(score_guess(puzzle.the_word, guess));
    return rows;
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
add_unfinished_rows(
    std::vector<colorful_text> const& guesses,
    puzzle_definition const& puzzle,
    puzzle_state const& state)
{
    std::vector<colorful_text> rows = guesses;
    if (!puzzle_is_solved(puzzle, state))
    {
        if (rows.size() < puzzle.max_guesses)
            rows.push_back(color_active_guess(puzzle, state));
        while (rows.size() < puzzle.max_guesses)
            rows.push_back(empty_row(puzzle));
    }
    return rows;
}

std::array<letter_color, 26>
extract_key_colors(std::vector<colorful_text> const& scored_guesses)
{
    std::array<letter_color, 26> colors;
    colors.fill(NEUTRAL);
    for (auto const& guess : scored_guesses)
    {
        for (auto const& cl : guess)
        {
            auto& color = colors[cl.letter - 'a'];
            if (cl.color > color)
                color = cl.color;
        }
    }
    return colors;
}

std::vector<std::string>
decode_score(
    std::string const& the_word,
    dictionary const& dict,
    std::vector<letter_color> const& score)
{
    size_t const word_size = the_word.size();
    if (score.size() != word_size)
        return std::vector<std::string>();

    std::vector<std::string> possible_guesses;
    for (auto const& guess : dict)
    {
        auto const full_score = score_guess(the_word, guess);
        bool match = true;
        for (size_t i = 0; i != word_size; ++i)
        {
            if (full_score[i].color != score[i])
                match = false;
        }
        if (match)
            possible_guesses.push_back(guess);
    }
    return possible_guesses;
}
