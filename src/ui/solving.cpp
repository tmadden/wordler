#include <alia/html.hpp>
#include <alia/html/bootstrap.hpp>

#include <algorithm>
#include <cctype>

#include "model/codes.h"
#include "model/coloring.h"
#include "model/dictionaries.h"
#include "model/puzzle.h"
#include "ui/rgb.h"
#include "ui/utilities.h"

using namespace alia;
using namespace html;
namespace bs = alia::html::bootstrap;

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
        scale * 8,
        scale * 20,
        scale * 20,
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

static rgb8 const palette[]
    = {rgb8(0xaf, 0xd4, 0xec),
       rgb8(0xef, 0x8b, 0x81),
       rgb8(0xec, 0xf0, 0xf1),
       rgb8(0x90, 0x9D, 0x9E),
       rgb8(0xff, 0xe3, 0x56),
       rgb8(0x4b, 0xdf, 0x8b)};

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
                        return 5.0 / std::max(word_length, size_t(5));
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

letter_color
extract_key_color(std::vector<colorful_text> const& letter_rows, char letter)
{
    letter_color color = NEUTRAL;
    for (auto const& row : letter_rows)
    {
        for (auto const& cl : row)
        {
            if (cl.letter == letter && cl.color > color)
                color = cl.color;
        }
    }
    return color;
}

void
letter_key(
    html::context ctx,
    readable<puzzle_definition> puzzle,
    readable<dictionary> dict,
    duplex<puzzle_state> state,
    readable<std::vector<colorful_text>> letter_rows,
    char letter)
{
    auto color = lazy_apply(
        [](auto color) { return palette[color]; },
        apply(ctx, extract_key_color, letter_rows, value(letter)));

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
        .content([&]() {
            div(ctx, "key-content", [&] {
                span(ctx)
                    .text(lazy_apply(
                        [](char c) { return std::string(1, std::toupper(c)); },
                        value(letter)))
                    .classes("user-select-none");
            });
        })
        .handler("click", [&](emscripten::val v) {
            write_signal(
                state,
                process_key_press(
                    read_signal(puzzle),
                    read_signal(dict),
                    read_signal(state),
                    std::string(1, letter)));
        });
}

void
keyboard_ui(
    html::context ctx,
    readable<puzzle_definition> puzzle,
    readable<dictionary> dict,
    duplex<puzzle_state> state,
    readable<std::vector<colorful_text>> letter_rows)
{
    auto do_letter = [&](char letter) {
        letter_key(ctx, puzzle, dict, state, letter_rows, letter);
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
            element(ctx, "div")
                .classes("key")
                .attr(
                    "style",
                    printf(
                        ctx,
                        "background-color: #%02x%02x%02x",
                        value(palette[NEUTRAL].r),
                        value(palette[NEUTRAL].g),
                        value(palette[NEUTRAL].b)))
                .content([&]() {
                    div(ctx, "key-content", [&] {
                        span(ctx)
                            .classes(
                                "material-icons-outlined user-select-none")
                            .text("keyboard_return");
                    });
                })
                .handler("click", [&](emscripten::val v) {
                    write_signal(
                        state,
                        process_key_press(
                            read_signal(puzzle),
                            read_signal(dict),
                            read_signal(state),
                            "Enter"));
                });

            do_letter('z');
            do_letter('x');
            do_letter('c');
            do_letter('v');
            do_letter('b');
            do_letter('n');
            do_letter('m');

            element(ctx, "div")
                .classes("key")
                .attr(
                    "style",
                    printf(
                        ctx,
                        "background-color: #%02x%02x%02x",
                        value(palette[NEUTRAL].r),
                        value(palette[NEUTRAL].g),
                        value(palette[NEUTRAL].b)))
                .content([&]() {
                    div(ctx, "key-content", [&] {
                        span(ctx)
                            .classes("material-icons user-select-none")
                            .text("backspace");
                    });
                })
                .handler("click", [&](emscripten::val v) {
                    write_signal(
                        state,
                        process_key_press(
                            read_signal(puzzle),
                            read_signal(dict),
                            read_signal(state),
                            "Backspace"));
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
    // Pass that through a two-way serializer/deserializer to convert it to our
    // native C++ representation.
    auto native_state = duplex_apply(
        ctx, json_to_puzzle_state, puzzle_state_to_json, json_state);
    // And finally, add a default value (of default-initialized state) for when
    // the raw JSON doesn't exist yet.
    auto state
        = add_default(native_state, default_initialized<puzzle_state>());

    auto dict = fetch_dictionary(ctx, size(alia_field(puzzle, the_word)));

    auto letter_rows = apply(ctx, make_letter_rows, puzzle, state);


    div(ctx, "container-lg flexible", [&] {
        div(ctx, "d-flex flex-column h-100 w-100", [&] {
            window_event_handler(ctx, "keydown", [&](emscripten::val v) {
                write_signal(
                    state,
                    process_key_press(
                        read_signal(puzzle),
                        read_signal(dict),
                        read_signal(state),
                        v["key"].as<std::string>()));
            });

            div(ctx, "letter-panel", [&] {
                for_each(ctx, letter_rows, [&](auto row) {
                    std::cout << "letter_row_invoker" << std::endl;
                    invoke_pure_component(
                        ctx,
                        letter_row,
                        puzzle,
                        minimize_id_changes(ctx, row));
                });
            });

            div(ctx, "footer", [&] {
                div(ctx, "container", [&] {
                    keyboard_ui(ctx, puzzle, dict, state, letter_rows);
                });
            });
        });
    });

}
