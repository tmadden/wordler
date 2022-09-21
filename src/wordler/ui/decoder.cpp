#include <alia/html.hpp>
#include <alia/html/bootstrap.hpp>

#include <codecvt>
#include <locale>

#include <wordler/model/codes.h>
#include <wordler/model/coloring.h>
#include <wordler/model/dictionaries.h>
#include <wordler/model/puzzle.h>
#include <wordler/ui/utilities.h>

using namespace alia;
using namespace html;
namespace bs = alia::html::bootstrap;

namespace {

std::vector<std::vector<letter_color>>
parse_coded_guesses(std::string const& raw)
{
    std::vector<std::vector<letter_color>> guesses;

    std::vector<letter_color> guess;

    auto on_eol = [&]() {
        if (!guess.empty())
        {
            guesses.push_back(std::move(guess));
            guess = std::vector<letter_color>();
        }
    };

    for (char32_t const c :
         std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}
             .from_bytes(raw))
    {
        switch (c)
        {
            case '\n':
            case '\r':
                on_eol();
                break;
            case U'🟨':
                guess.push_back(MISPLACED);
                break;
            case U'🟩':
                guess.push_back(CORRECT);
                break;
            case U'⬜':
            case U'⬛':
                guess.push_back(INCORRECT);
                break;
        }
    }
    on_eol();

    return guesses;
}

std::string
score_to_text(std::vector<letter_color> score)
{
    std::stringstream out;
    for (auto const color : score)
    {
        switch (color)
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
    return out.str();
}

template<class Modal>
void
decoded_guess_modal(
    html::context ctx,
    Modal& modal,
    readable<std::string> the_word,
    readable<std::string> coded_guesses)
{
    modal.title("Decoded Guesses");

    auto dict = fetch_dictionary(ctx, size(the_word));

    modal.body([&] {
        for_each(
            ctx,
            apply(ctx, parse_coded_guesses, coded_guesses),
            [&](auto score) {
                p(ctx, apply(ctx, score_to_text, score));
                p(ctx, [&] {
                    for_each(
                        ctx,
                        apply(ctx, decode_score, the_word, dict, score),
                        [&](auto guess) {
                            text(ctx, guess);
                            text(ctx, " ");
                        });
                });
            });
    });

    modal.footer(
        [&] { bs::primary_button(ctx, "Close", actions::close(modal)); });
}

} // namespace

void
decoder_form(html::context ctx)
{
    element(ctx, "form").content([&]() {
        auto word_input = get_state(ctx, "");

        auto the_word = apply(
            ctx,
            [](std::string s) {
                for (char& c : s)
                    c = std::tolower(c);
                return s;
            },
            apply(ctx, trim, word_input));

        auto word_is_letters = apply(ctx, is_alpha_only, the_word);
        auto word_is_valid = word_is_letters && !is_empty(the_word);

        div(ctx, "form-group", [&]() {
            label(ctx, "What's the word?")
                .classes("form-label mt-4")
                .attr("for", "the-word-input");
            input(ctx, word_input)
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

        auto coded_guesses = get_state(ctx, std::string());

        div(ctx, "form-group", [&]() {
            label(ctx, "Paste in the colored blocks here.")
                .classes("form-label mt-4")
                .attr("for", "coded-guesses-input");
            textarea(ctx, coded_guesses)
                .classes("form-control")
                .attr("id", "coded-guesses-input")
                .attr("rows", "7");
        });

        {
            auto modal = scrollable_modal(ctx, [&](auto& modal) {
                decoded_guess_modal(ctx, modal, the_word, coded_guesses);
            });
            modal.class_("fade");
            bs::primary_button(
                ctx,
                "Decode",
                mask_action(actions::activate(modal), word_is_valid))
                .class_("mt-3");
        }
    });
}

void
decoder_ui(html::context ctx)
{
    div(ctx, "container", [&] {
        div(ctx, "row", [&] {
            div(ctx, "col-12", [&] {
                p(ctx, [&] {
                    text(ctx, "Decode your friends' ");
                    link(
                        ctx, "Wordle", "https://www.nytimes.com/games/wordle/")
                        .class_("text-primary");
                    text(ctx, " guesses!");
                }).classes("mt-5");

                decoder_form(ctx);
            });
        });
    });
}
