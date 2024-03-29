#include <alia/html.hpp>
#include <alia/html/bootstrap.hpp>

#include <algorithm>
#include <cctype>

#include <wordler/model/codes.h>
#include <wordler/model/dictionaries.h>
#include <wordler/model/puzzle.h>
#include <wordler/ui/utilities.h>

using namespace alia;
using namespace html;
namespace bs = alia::html::bootstrap;

namespace {

template<class Modal>
void
puzzle_code_modal(
    html::context ctx, Modal& modal, readable<puzzle_definition> puzzle)
{
    modal.title(conditional(
        is_empty(alia_field(puzzle, author)),
        value("Your Puzzle"),
        "A Wordle by " + alia_field(puzzle, author)));

    auto code = apply(ctx, generate_puzzle_code, puzzle);

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

        bs::primary_button(ctx, "Share", callback([](std::string const& url) {
                                             share_text("Wordler Puzzle", url);
                                         }) << url);

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

        auto author = get_local_state(ctx, "author");

        div(ctx, "form-group", [&]() {
            label(ctx, "Sign your work?")
                .classes("form-label mt-4")
                .attr("for", "author-input");
            input(ctx, author)
                .classes("form-control")
                .attr("id", "author-input")
                .attr("type", "text");
        });

        {
            auto modal = bs::modal(ctx, [&](auto& modal) {
                auto puzzle = apply(
                    ctx,
                    ALIA_AGGREGATOR(puzzle_definition),
                    trimmed_word,
                    max_guesses,
                    value(false),
                    author);
                puzzle_code_modal(ctx, modal, puzzle);
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
