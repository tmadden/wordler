#include <alia/html.hpp>
#include <alia/html/bootstrap.hpp>

#include <cctype>

#include "model/codes.h"
#include "model/coloring.h"
#include "model/puzzle.h"
#include "ui/rgb.h"
#include "ui/utilities.h"

using namespace alia;
using namespace html;
namespace bs = alia::html::bootstrap;

void
letter_display(
    html::context ctx, readable<rgb8> color, readable<char> character)
{
    element(ctx, "div")
        .classes("letter")
        .attr(
            "style",
            printf(
                ctx,
                "background-color: #%02x%02x%02x",
                alia_field(color, r),
                alia_field(color, g),
                alia_field(color, b)))
        .content([&]() {
            span(ctx)
                .text(lazy_apply(
                    [](char c) { return std::string(1, std::toupper(c)); },
                    character))
                .classes("user-select-none");
        });
}

puzzle_state
process_key_press(
    puzzle_definition const& puzzle,
    puzzle_state state,
    std::string const& key)
{
    if (key.length() == 1 && isalpha(key[0]))
    {
        if (state.active_guess.length() < puzzle.the_word.length())
            state.active_guess += key;
    }
    else if (key == "Enter")
    {
        if (state.active_guess.length() == puzzle.the_word.length())
        {
            state.guesses.push_back(state.active_guess);
            state.active_guess.clear();
        }
    }
    else if (key == "Backspace")
    {
        if (!state.active_guess.empty())
            state.active_guess.pop_back();
    }
    return state;
}

void
letter_row(
    html::context ctx,
    readable<puzzle_definition> puzzle,
    readable<colorful_text> row)
{
    static rgb8 const palette[]
        = {rgb8(0xec, 0xf0, 0xf1),
           rgb8(0x4b, 0xdf, 0x8b),
           rgb8(0xff, 0xe3, 0x56),
           rgb8(0xec, 0xf0, 0xf1),
           rgb8(0xba, 0xc5, 0xc5)};

    div(ctx, "d-flex flex-row", [&] {
        for_each(ctx, row, [&](auto letter) {
            letter_display(
                ctx,
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

    p(ctx, [&] {
        text(ctx, "Someone has sent you a bespoke ");
        link(ctx, "Wordle", "https://www.nytimes.com/games/wordle/")
            .class_("text-primary");
        text(ctx, "-style puzzle!");
    }).classes("mt-5");

    window_event_handler(ctx, "keydown", [&](emscripten::val v) {
        write_signal(
            state,
            process_key_press(
                read_signal(puzzle),
                read_signal(state),
                v["key"].as<std::string>()));
    });

    div(ctx, "d-flex flex-column", [&] {
        auto letter_rows = apply(ctx, make_letter_rows, puzzle, state);
        for_each(
            ctx, letter_rows, [&](auto row) { letter_row(ctx, puzzle, row); });
    });
}
