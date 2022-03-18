#pragma once

#include <emscripten/html5.h>
#include <emscripten/val.h>

#include <alia/html.hpp>

using namespace alia;

void
copy_to_clipboard(std::string const& text);

struct targeted_window_callback : html::detail::window_callback
{
    component_identity identity;
};

template<class Handler>
void
window_event_handler(
    html::context ctx, char const* event_type, Handler&& handler)
{
    targeted_window_callback* data;
    if (get_cached_data(ctx, &data))
    {
        auto external_id = externalize(&data->identity);
        auto* system = &get<alia::system_tag>(ctx);
        auto callback = [=](emscripten::val v) {
            html::detail::dom_event event(v);
            dispatch_targeted_event(*system, event, external_id);
            return true;
        };
        html::detail::install_window_callback(*data, event_type, callback);
    }
    refresh_component_identity(ctx, data->identity);
    targeted_event_handler<html::detail::dom_event>(
        ctx, &data->identity, [&](auto ctx, auto& e) {
            std::forward<Handler>(handler)(e.event);
        });
}
