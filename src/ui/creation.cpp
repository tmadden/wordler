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

std::string
trim(std::string const& str)
{
    auto const begin = str.find_first_not_of(" ");
    if (begin == std::string::npos)
        return "";

    auto const end = str.find_last_not_of(" ");

    return str.substr(begin, end - begin + 1);
}

template<class Modal>
void
puzzle_code_modal(html::context ctx, Modal& modal, readable<std::string> code)
{
    modal.title("Your Puzzle");

    auto url = "https://tmadden.github.io/wordler/#/puzzle/" + code;

    modal.body([&] {
        element(ctx, "a")
            .attr("href", url)
            .text(url)
            .on("click", (actions::close(modal), callback([&]() {
                              emscripten::val::global("window").set(
                                  "location", read_signal(url));
                          })))
            .attr("style", "word-wrap: break-word;");
    });

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
creation_form(html::context ctx)
{
    element(ctx, "form").content([&]() {
        auto the_word = get_state(ctx, "");

        auto trimmed_word = apply(ctx, trim, the_word);

        auto word_is_letters = apply(ctx, is_alpha_only, trimmed_word);
        auto word_is_valid = word_is_letters && !is_empty(trimmed_word);

        div(ctx, "form-group", [&]() {
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

        auto max_guesses = get_state(ctx, 6);

        auto max_guesses_is_valid = max_guesses > 0 && max_guesses <= 100;

        div(ctx, "form-group", [&]() {
            label(ctx, "How many guesses do they get?")
                .classes("form-label mt-4")
                .attr("for", "the-word-input");
            input(ctx, max_guesses)
                .classes("form-control")
                .attr("id", "the-word-input")
                .attr("type", "number")
                .class_(mask("is-invalid", !max_guesses_is_valid));
            alia_if(max_guesses < 1)
            {
                div(ctx, "invalid-feedback", [&] {
                    text(ctx, "You need to allow at least one guess!");
                });
            }
            alia_else_if(max_guesses > 100)
            {
                div(ctx, "invalid-feedback", [&] {
                    text(ctx, "That's too many guesses!");
                });
            }
            alia_end
        });

        {
            auto modal = bs::modal(ctx, [&](auto& modal) {
                auto code = apply(
                    ctx,
                    generate_puzzle_code,
                    apply(
                        ctx,
                        ALIA_AGGREGATOR(puzzle_definition),
                        trimmed_word,
                        max_guesses,
                        value(false)));
                puzzle_code_modal(ctx, modal, code);
            });
            modal.class_("fade");
            bs::primary_button(
                ctx,
                "Create",
                mask_action(
                    actions::activate(modal),
                    word_is_valid && max_guesses_is_valid))
                .class_("mt-3");
        }
    });
}

void
creation_ui(html::context ctx)
{
    div(ctx, "container", [&] {
        div(ctx, "row", [&] {
            div(ctx, "col-12", [&] {
                p(ctx, [&] {
                    text(ctx, "Create your own ");
                    link(
                        ctx, "Wordle", "https://www.nytimes.com/games/wordle/")
                        .class_("text-primary");
                    text(ctx, " puzzles!");
                }).classes("mt-5");

                creation_form(ctx);
            });
        });
    });
}
