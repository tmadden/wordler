#include "dictionaries.h"

#include <emscripten/val.h>

#include <alia/html/fetch.hpp>

using namespace alia;

namespace {

html::http_request
make_dictionary_request(size_t word_length)
{
    return html::http_request{
        html::http_method::GET,
        emscripten::val::global("window")["location"]["pathname"]
                .as<std::string>()
            + "words/" + std::to_string(word_length) + ".txt",
        html::http_headers(),
        html::blob()};
}

dictionary
parse_dictionary_file(html::http_response const& response)
{
    dictionary dict;
    if (response.status_code == 200)
    {
        std::stringstream ss(
            std::string(response.body.data, response.body.size));
        std::string word;
        while (std::getline(ss, word, '\n'))
            dict.insert(std::move(word));
    }
    return dict;
}

} // namespace

readable<dictionary>
fetch_dictionary(html::context ctx, readable<size_t> word_length)
{
    auto request = apply(ctx, make_dictionary_request, word_length);
    auto response = html::fetch(ctx, request);
    auto dict = apply(ctx, parse_dictionary_file, response);
    return make_returnable_ref(ctx, dict);
}
