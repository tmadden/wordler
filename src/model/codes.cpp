#include "codes.h"

#include <cctype>

#include <cppcodec/base64_default_url_unpadded.hpp>

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>

#include "puzzle.h"

std::string
generate_puzzle_code(puzzle_definition const& puzzle)
{
    std::ostringstream stream(std::ios_base::out | std::ios_base::binary);
    stream << char(1);
    stream << char(puzzle.the_word.length());
    std::string the_word = puzzle.the_word;
    for (char& letter : the_word)
        letter = std::tolower(letter);
    stream << puzzle.the_word;
    stream << char(puzzle.disable_dict_warning ? 1 : 0);

    return cppcodec::base64_url_unpadded::encode(stream.str());
}

puzzle_definition
parse_puzzle_code(std::string const& code)
{
    auto bits = cppcodec::base64_url_unpadded::decode(code);
    std::istringstream stream(
        reinterpret_cast<char const*>(bits.data()),
        std::ios_base::in | std::ios_base::binary);

    unsigned char version;
    stream >> version;

    unsigned char word_length;
    stream >> word_length;
    std::string the_word(size_t(word_length), ' ');
    stream.read(&the_word[0], size_t(word_length));
    // Sanitize.
    for (char& letter : the_word)
    {
        if (!std::isalpha(letter))
            letter = 'a';
        letter = std::tolower(letter);
    }

    char disable_dict_warning;
    stream >> disable_dict_warning;

    return puzzle_definition{the_word, disable_dict_warning != 0};
}
