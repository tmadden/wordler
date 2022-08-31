#include "codes.h"

#include <cctype>

#include <cppcodec/base64_default_url_unpadded.hpp>

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>

#include "puzzle.h"

static void
write_short_string(std::ostream& stream, std::string const& string)
{
    stream << char(string.length());
    stream << string;
}

std::string
generate_puzzle_code(puzzle_definition const& puzzle)
{
    std::ostringstream stream(std::ios_base::out | std::ios_base::binary);

    stream << char(2); // version

    // Convert the word to lowecase.
    std::string the_word = puzzle.the_word;
    for (char& letter : the_word)
        letter = std::tolower(letter);

    write_short_string(stream, the_word);

    stream << char(puzzle.max_guesses);

    stream << char(puzzle.disable_dict_warning ? 1 : 0);

    write_short_string(stream, puzzle.author);

    return cppcodec::base64_url_unpadded::encode(stream.str());
}

static std::string
read_short_string(std::istream& stream)
{
    char length;
    stream.read(&length, 1);
    std::string string(size_t(length), '\0');
    stream.read(&string[0], size_t(length));
    return string;
}

puzzle_definition
parse_puzzle_code(std::string const& code)
{
    auto bits = cppcodec::base64_url_unpadded::decode(code);
    std::istringstream stream(
        std::string(reinterpret_cast<char const*>(bits.data()), bits.size()),
        std::ios_base::in | std::ios_base::binary);

    char version;
    stream.read(&version, 1);

    auto the_word = read_short_string(stream);
    // Sanitize.
    for (char& letter : the_word)
    {
        if (!std::isalpha(letter))
            letter = 'a';
        letter = std::tolower(letter);
    }

    char max_guesses;
    stream.read(&max_guesses, 1);

    char disable_dict_warning;
    stream.read(&disable_dict_warning, 1);

    std::string author;
    if (version > 1)
        author = read_short_string(stream);

    return puzzle_definition{
        the_word, max_guesses, disable_dict_warning != 0, author};
}
