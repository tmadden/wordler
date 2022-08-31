#include <alia/html.hpp>
#include <alia/html/bootstrap.hpp>

#include <algorithm>
#include <cctype>

#include <wordler/model/codes.h>
#include <wordler/model/coloring.h>
#include <wordler/model/dictionaries.h>
#include <wordler/model/puzzle.h>
#include <wordler/ui/rgb.h>
#include <wordler/ui/utilities.h>

using namespace alia;
using namespace html;
namespace bs = alia::html::bootstrap;

std::string
construct_outcome_text(
    std::string const& code,
    puzzle_definition const& puzzle,
    puzzle_state const& state)
{
    std::stringstream out;

    if (!puzzle.author.empty())
        out << "A Wordle by " << puzzle.author << "\n";
    out << "https://tmadden.github.io/wordler/#/puzzle/" << code << "\n\n";

    if (puzzle_is_solved(puzzle, state))
        out << state.guesses.size();
    else
        out << "X";
    out << " / " << puzzle.max_guesses;
    out << "\n\n";

    for (auto const& guess : score_guesses(puzzle, state.guesses))
    {
        for (auto const& letter : guess)
        {
            switch (letter.color)
            {
                case MISPLACED:
                    out << "\xF0\x9F\x9F\xA8";
                    break;
                case CORRECT:
                    out << "\xF0\x9F\x9F\xA9";
                    break;
                default:
                    out << "\xE2\xAC\x9C";
                    break;
            }
        }
        out << "\n";
    }

    return out.str();
}

template<class Modal>
void
outcome_modal(
    html::context ctx,
    Modal& modal,
    readable<std::string> code,
    readable<puzzle_definition> puzzle,
    readable<puzzle_state> state)
{
    auto is_victory = alia::apply(ctx, puzzle_is_solved, puzzle, state);

    auto outcome_text
        = apply(ctx, construct_outcome_text, code, puzzle, state);

    modal.title(alia::conditional(is_victory, "Congratulations!", "Sorry!"));

    modal.body([&] {
        p(ctx, [&] {
            alia_if(is_victory)
            {
                text(ctx, "You got it! ");
            }
            alia_end
            text(ctx, "It was ");
            b(ctx, alia_field(puzzle, the_word))
                .classes("text-weight-bold text-monospace text-uppercase");
            text(ctx, ".");
        });

        p(ctx, "Share your results below.");

        p(ctx, [&] {
            text(ctx, "Or... ");
            auto url = value("https://tmadden.github.io/wordler/#/");
            element(ctx, "a")
                .attr("href", url)
                .text("Create your own puzzle!")
                .on("click", (actions::close(modal), callback([&]() {
                                  emscripten::val::global("window").set(
                                      "location", read_signal(url));
                              })))
                .classes("text-decoration-none");
        });
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
            (callback([](std::string const& text) { copy_to_clipboard(text); })
                 << outcome_text,
             copied <<= true));

        bs::primary_button(ctx, "Share", callback([](std::string const& text) {
                                             share_text(
                                                 "Wordler Results", text);
                                         }) << outcome_text);
    });
}

static rgb8 const palette[]
    = {rgb8(0xaf, 0xd4, 0xec),
       rgb8(0xef, 0x8b, 0x81),
       rgb8(0xec, 0xf0, 0xf1),
       rgb8(0x90, 0x9D, 0x9E),
       rgb8(0xff, 0xe3, 0x56),
       rgb8(0x4b, 0xdf, 0x8b)};

void
letter_display(
    html::context ctx,
    readable<double> scale,
    readable<rgb8> color,
    readable<char> character)
{
    auto style = printf(
        ctx,
        "font-size: min(%fvw, 42px); "
        "width: %f%%; "
        "padding-bottom: %f%%; "
        "background-color: #%02x%02x%02x",
        scale * 6,
        scale * 15,
        scale * 15,
        alia_field(color, r),
        alia_field(color, g),
        alia_field(color, b));

    element(ctx, "div").classes("letter").attr("style", style).content([&]() {
        div(ctx, "letter-content", [&] {
            span(ctx)
                .text(lazy_apply(
                    [](char c) { return std::string(1, std::toupper(c)); },
                    character))
                .classes("user-select-none");
        });
    });
}

void
letter_row(
    html::context ctx,
    readable<puzzle_definition> puzzle,
    readable<colorful_text> row)
{
    div(ctx, "letter-row", [&] {
        for_each(ctx, row, [&](auto letter) {
            letter_display(
                ctx,
                apply(
                    ctx,
                    [](puzzle_definition const& puzzle) {
                        size_t word_length = puzzle.the_word.length();
                        return 7.0 / std::max(word_length, size_t(7));
                    },
                    puzzle),
                smooth(
                    ctx,
                    lazy_apply(
                        [](auto color) { return palette[color]; },
                        alia_field(letter, color)),
                    animated_transition{ease_in_out_curve, 400}),
                alia_field(letter, letter));
        });
    });
}

puzzle_state
process_key_press(
    puzzle_definition const& puzzle,
    dictionary const& dict,
    puzzle_state state,
    std::string const& key)
{
    state.tried_an_invalid_word = false;
    if (key.length() == 1 && isalpha(key[0]))
    {
        if (state.active_guess.length() < puzzle.the_word.length())
            state.active_guess += key;
    }
    else if (key == "Enter")
    {
        if (state.active_guess.length() == puzzle.the_word.length())
        {
            if (state.active_guess == puzzle.the_word
                || dictionary_contains(dict, state.active_guess))
            {
                state.guesses.push_back(state.active_guess);
                state.active_guess.clear();
            }
            else
            {
                state.tried_an_invalid_word = true;
            }
        }
    }
    else if (key == "Backspace")
    {
        if (!state.active_guess.empty())
            state.active_guess.pop_back();
    }
    return state;
}

template<class Content>
void
keyboard_key(
    html::context ctx,
    readable<puzzle_definition> puzzle,
    readable<dictionary> dict,
    duplex<puzzle_state> state,
    readable<rgb8> color,
    std::string const& code,
    Content&& content)
{
    element(ctx, "div")
        .classes("key")
        .attr(
            "style",
            printf(
                ctx,
                "background-color: #%02x%02x%02x",
                alia_field(color, r),
                alia_field(color, g),
                alia_field(color, b)))
        .content(
            [&]() { div(ctx, "key-content", std::forward<Content>(content)); })
        .handler("click", [&](emscripten::val v) {
            write_signal(
                state,
                process_key_press(
                    read_signal(puzzle),
                    read_signal(dict),
                    read_signal(state),
                    code));
        });
}

void
keyboard_ui(
    html::context ctx,
    readable<puzzle_definition> puzzle,
    readable<dictionary> dict,
    duplex<puzzle_state> state,
    readable<std::vector<colorful_text>> scored_guesses)
{
    auto colors = apply(ctx, extract_key_colors, scored_guesses);

    auto do_letter = [&](char letter) {
        keyboard_key(
            ctx,
            puzzle,
            dict,
            state,
            smooth(
                ctx,
                lazy_apply(
                    [](auto color) { return palette[color]; },
                    colors[letter - 'a']),
                animated_transition{ease_in_out_curve, 400}),
            std::string(1, letter),
            [&] {
                span(ctx)
                    .text(lazy_apply(
                        [](char c) { return std::string(1, std::toupper(c)); },
                        value(letter)))
                    .classes("user-select-none");
            });
    };

    div(ctx, "keyboard", [&] {
        div(ctx, "keyboard-row", [&] {
            do_letter('q');
            do_letter('w');
            do_letter('e');
            do_letter('r');
            do_letter('t');
            do_letter('y');
            do_letter('u');
            do_letter('i');
            do_letter('o');
            do_letter('p');
        });

        div(ctx, "keyboard-row", [&] {
            do_letter('a');
            do_letter('s');
            do_letter('d');
            do_letter('f');
            do_letter('g');
            do_letter('h');
            do_letter('j');
            do_letter('k');
            do_letter('l');
        });

        div(ctx, "keyboard-row", [&] {
            keyboard_key(
                ctx,
                puzzle,
                dict,
                state,
                value(palette[NEUTRAL]),
                "Enter",
                [&] {
                    span(ctx)
                        .classes("material-icons-outlined user-select-none")
                        .text("keyboard_return");
                });

            do_letter('z');
            do_letter('x');
            do_letter('c');
            do_letter('v');
            do_letter('b');
            do_letter('n');
            do_letter('m');

            keyboard_key(
                ctx,
                puzzle,
                dict,
                state,
                value(palette[NEUTRAL]),
                "Backspace",
                [&] {
                    span(ctx)
                        .classes("material-icons user-select-none")
                        .text("backspace");
                });
        });
    });
}

void
solving_ui(html::context ctx, readable<std::string> code)
{
    auto puzzle = apply(ctx, parse_puzzle_code, code);

    // Construct the signal for our puzzle state...
    // First, create a binding to the raw, JSON state in local storage.
    auto json_state = get_local_state(ctx, code);
    // Pass that through a two-way serializer/deserializer to convert it to
    // our native C++ representation.
    auto native_state = duplex_apply(
        ctx, json_to_puzzle_state, puzzle_state_to_json, json_state);
    // And finally, add a default value (of default-initialized state) for
    // when the raw JSON doesn't exist yet.
    auto state
        = add_default(native_state, default_initialized<puzzle_state>());

    // Fetch the dictionary for this size.
    auto dict = fetch_dictionary(ctx, size(alia_field(puzzle, the_word)));

    // Score the guesses.
    auto scored_guesses = apply(
        ctx,
        score_guesses,
        puzzle,
        minimize_id_changes(ctx, alia_field(state, guesses)));

    // Handle keyboard events.
    window_event_handler(ctx, "keydown", [&](emscripten::val v) {
        write_signal(
            state,
            process_key_press(
                read_signal(puzzle),
                read_signal(dict),
                read_signal(state),
                v["key"].as<std::string>()));
    });

    // Do the main body of the app.
    div(ctx, "container-lg flexible p-2", [&] {
        div(ctx, "d-flex flex-column h-100 w-100", [&] {
            // Do the intro message.
            p(ctx, [&] {
                alia_if(is_empty(alia_field(puzzle, author)))
                {
                    text(ctx, "Someone has sent you a personalized ");
                    link(
                        ctx, "Wordle", "https://www.nytimes.com/games/wordle/")
                        .class_("text-primary");
                    text(ctx, " puzzle!");
                }
                alia_else
                {
                    text(ctx, "A ");
                    link(
                        ctx, "Wordle", "https://www.nytimes.com/games/wordle/")
                        .class_("text-primary");
                    text(ctx, " by ");
                    text(ctx, alia_field(puzzle, author));
                }
                alia_end
            }).classes("mt-3");

            // Do the guess rows.
            auto letter_rows = apply(
                ctx, add_unfinished_rows, scored_guesses, puzzle, state);
            div(ctx, "letter-panel", [&] {
                for_each(ctx, letter_rows, [&](auto row) {
                    invoke_pure_component(
                        ctx,
                        letter_row,
                        puzzle,
                        minimize_id_changes(ctx, row));
                });
            });

            // Do the keyboard.
            div(ctx, "footer", [&] {
                keyboard_ui(ctx, puzzle, dict, state, scored_guesses);
            });
        });
    });

    // If the game is over, do the outcome modal.
    alia_if(
        apply(ctx, puzzle_is_solved, puzzle, state)
        || apply(ctx, out_of_guesses, puzzle, state))
    {
        auto modal = bs::modal(ctx, [&](auto& modal) {
            outcome_modal(ctx, modal, code, puzzle, state);
        });
        modal.class_("fade");
        // Activate the modal as soon as this block is activated.
        on_activate(ctx, actions::activate(modal));
    }
    alia_end
}
