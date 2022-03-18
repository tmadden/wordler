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
    div(ctx, "col-6", [&] {
        p(ctx, [&] {
            text(ctx, "Create your own ");
            link(ctx, "Wordle", "https://www.nytimes.com/games/wordle/")
                .class_("text-primary");
            text(ctx, "-style puzzles!");
        }).classes("mt-5");

        auto the_word = get_state(ctx, "");

        auto word_is_letters = apply(ctx, is_alpha_only, the_word);
        auto word_is_valid = word_is_letters && !is_empty(the_word);

        element(ctx, "form").content([&]() {
            element(ctx, "fieldset").content([&] {
                div(ctx, "form-group", [&]() {
                    element(ctx, "fieldset").content([&] {
                        label(ctx, "What's the word?")
                            .classes("form-label mt-4")
                            .attr("for", "the-word-input");
                        input(ctx, the_word)
                            .classes("form-control")
                            .attr("id", "the-word-input")
                            .attr("type", "text")
                            .class_(mask("is-invalid", !word_is_letters));
                        alia_if(!word_is_letters)
                        {
                            div(ctx, "invalid-feedback", [&] {
                                text(ctx, "Letters only, please!");
                            });
                        }
                        alia_end
                    });
                }).class_(mask("has-danger", !word_is_letters));

                {
                    auto modal = bs::modal(ctx, [&](auto& modal) {
                        auto code = apply(
                            ctx,
                            generate_puzzle_code,
                            apply(
                                ctx,
                                ALIA_AGGREGATOR(puzzle_definition),
                                the_word,
                                value(false)));
                        puzzle_code_modal(ctx, modal, code);
                    });
                    modal.class_("fade");
                    bs::primary_button(
                        ctx,
                        "Create",
                        mask_action(actions::activate(modal), word_is_valid));
                }
            });
        });
    });
}
