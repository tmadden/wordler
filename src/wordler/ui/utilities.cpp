#include "utilities.h"

#include <emscripten/html5.h>
#include <emscripten/val.h>

std::string
trim(std::string const& str)
{
    auto const begin = str.find_first_not_of(" ");
    if (begin == std::string::npos)
        return "";

    auto const end = str.find_last_not_of(" ");

    return str.substr(begin, end - begin + 1);
}

void
copy_to_clipboard(std::string const& text)
{
    EM_ASM(
        { navigator.clipboard.writeText(Module['UTF8ToString']($0)); },
        text.c_str());
}

struct textarea_data
{
    captured_id value_id;
    std::string value;
};

html::element_handle
textarea(html::context ctx, duplex<std::string> value)
{
    textarea_data* data;
    get_cached_data(ctx, &data);

    refresh_handler(ctx, [&](auto ctx) {
        if (!value.is_invalidated())
        {
            refresh_signal_view(
                data->value_id,
                value,
                [&](std::string new_value) {
                    data->value = std::move(new_value);
                },
                [&]() { data->value.clear(); });
        }
    });

    return element(ctx, "textarea")
        .prop("value", data->value)
        .handler("input", [=](emscripten::val& e) {
            auto new_value = e["target"]["value"].as<std::string>();
            write_signal(value, new_value);
            data->value = new_value;
        });
}

void
share_text(std::string const& title, std::string const& text)
{
    EM_ASM(
        {
            if (navigator.share)
            {
                navigator.share({
                    title : Module['UTF8ToString']($0),
                    text : Module['UTF8ToString']($1)
                });
            }
        },
        title.c_str(),
        text.c_str());
}

struct dynamic_storage_signal_data
{
    captured_id key_id;
    html::storage_signal_data static_data;
};

html::storage_signal
get_storage_state(
    html::context ctx, char const* storage_name, readable<std::string> key)
{
    dynamic_storage_signal_data* data;
    if (get_cached_data(ctx, &data))
    {
        // Record the storage name.
        data->static_data.storage = storage_name;

        // Install a handler for the HTML window's 'storage' event to monitor
        // changes in the underlying value of the signal.
        if (!strcmp(storage_name, "localStorage"))
        {
            alia::system* sys = &get<alia::system_tag>(ctx);
            html::detail::install_window_callback(
                data->static_data.on_storage_event,
                "storage",
                [=](emscripten::val event) {
                    // The event will fire for any storage activity related to
                    // our domain, so we have to check that the key matches.
                    if (event["key"].as<std::string>()
                        == data->static_data.key)
                    {
                        data->static_data.value.set(
                            event["newValue"].as<std::string>());
                        refresh_system(*sys);
                    }
                });
        }
    }

    if (!signal_has_value(key))
    {
        data->static_data.key.clear();
        data->static_data.value.clear();
    }
    else if (!data->key_id.matches(key.value_id()))
    {
        // Record the new key.
        data->static_data.key = read_signal(key);

        // Query the initial value.
        html::storage_object storage(storage_name);
        if (storage.has_item(data->static_data.key))
        {
            data->static_data.value.nonconst_ref()
                = storage.get_item(data->static_data.key);
        }
        else
        {
            data->static_data.value.clear();
        }

        data->key_id.capture(key.value_id());
    }

    refresh_handler(ctx, [&](auto ctx) {
        data->static_data.value.refresh_container(
            get_active_component_container(ctx));
    });

    return html::storage_signal(&data->static_data);
}

html::storage_signal
get_session_state(html::context ctx, readable<std::string> key)
{
    return get_storage_state(ctx, "sessionStorage", key);
}

html::storage_signal
get_local_state(html::context ctx, readable<std::string> key)
{
    return get_storage_state(ctx, "localStorage", key);
}

using namespace alia::html::bootstrap;

modal_handle
scrollable_modal(
    html::context ctx,
    alia::function_view<void(internal_modal_handle&)> content)
{
    modal_data* data;
    if (get_cached_data(ctx, &data))
        create_as_modal_root(data->root.object);

    scoped_tree_root<html::element_object> scoped_root;
    if (is_refresh_event(ctx))
        scoped_root.begin(get<html::tree_traversal_tag>(ctx), data->root);

    html::element_handle modal = html::element(ctx, "div");
    modal.class_("modal")
        .attr("tabindex", "-1")
        .attr("role", "dialog")
        .content([&] {
            div(ctx,
                "modal-dialog modal-dialog-centered modal-dialog-scrollable")
                .attr("role", "document")
                .content([&] {
                    ALIA_IF(data->active)
                    {
                        div(ctx, "modal-content", [&] {
                            internal_modal_handle handle(modal);
                            content(handle);
                        });
                        refresh_handler(ctx, [&](auto ctx) {
                            EM_ASM(
                                {
                                    jQuery(Module['nodes'][$0])
                                        .modal('handleUpdate');
                                },
                                modal.asmdom_id());
                        });
                    }
                    ALIA_END
                });
        });
    modal.init([&](auto&) {
        EM_ASM(
            {
                jQuery(Module['nodes'][$0])
                    .on(
                        "hidden.bs.modal", function(e) {
                            Module['nodes'][$0].dispatchEvent(
                                new CustomEvent("bs.modal.hidden"));
                        });
            },
            modal.asmdom_id());
    });
    modal.handler("bs.modal.hidden", [&](auto) { data->active = false; });

    return modal_handle(modal, *data);
}
