#pragma once

#include <string>
#include <unordered_set>

#include <alia/html.hpp>

typedef std::unordered_set<std::string> dictionary;

using namespace alia;

readable<dictionary>
fetch_dictionary(html::context ctx, readable<size_t> word_length);

inline bool
dictionary_contains(dictionary const& dict, std::string const& word)
{
    return dict.find(word) != dict.end();
}
