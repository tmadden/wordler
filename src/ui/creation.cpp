#include <alia/html.hpp>
#include <alia/html/bootstrap.hpp>

#include <algorithm>
#include <cctype>

#include "model/codes.h"
#include "model/dictionaries.h"
#include "model/puzzle.h"
#include "ui/utilities.h"

using namespace alia;
using namespace html;
namespace bs = alia::html::bootstrap;

namespace {

bool
is_alpha_only(std::string const& word)
{
    return std::all_of(word.begin(), word.end(), ALIA_LAMBDIFY(std::isalpha));
}

template<class Modal>
void
puzzle_code_modal(html::context ctx, Modal& modal, readable<std::string> code)
{
    modal.title("Your Puzzle");

    auto url = "https://tmadden.github.io/wordler/#/puzzle/" + code;

    modal.body(
        [&] { link(ctx, url, url).attr("style", "word-break: break-all;"); });

    modal.footer([&] {
        auto copied = get_transient_state(ctx, false);

        alia_if(copied)
        {
            p(ctx, "Copied!").classes("text-muted user-select-none");
        }
        alia_end

        bs::primary_button(
            ctx,
            "Copy",
            (callback([](std::string const& url) { copy_to_clipboard(url); })
                 << url,
             copied <<= true));

        bs::secondary_button(ctx, "Close", actions::close(modal));
    });
}

} // namespace

void
creation_ui(html::context ctx)
{
    p(ctx, [&] {
        text(ctx, "Create your own ");
        link(ctx, "Wordle", "https://www.nytimes.com/games/wordle/")
            .class_("text-primary");
        text(ctx, "-style puzzles!");
    }).classes("mt-5");

    auto word = get_state(ctx, "");
    input(ctx, word);

    auto words = fetch_dictionary(ctx, size(word));
    p(ctx, printf(ctx, "%i words of that length %i", size(words), size(word)));
    alia_if(lazy_apply(dictionary_contains, words, word))
    {
        p(ctx, "That word is in the dictionary!");
    }
    alia_end

    alia_if(!apply(ctx, is_alpha_only, word))
    {
        p(ctx, "Letters only, please!");
    }
    alia_else_if(!is_empty(word))
    {
        auto modal = bs::modal(ctx, [&](auto& modal) {
            auto code = apply(
                ctx,
                generate_puzzle_code,
                apply(
                    ctx,
                    ALIA_AGGREGATOR(puzzle_definition),
                    word,
                    value(false)));
            puzzle_code_modal(ctx, modal, code);
        });
        modal.class_("fade");
        bs::primary_button(ctx, "Create", actions::activate(modal));
    }
    alia_end
}
