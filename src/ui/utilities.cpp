#include "utilities.h"

#include <emscripten/html5.h>
#include <emscripten/val.h>

void
copy_to_clipboard(std::string const& text)
{
    EM_ASM(
        { navigator.clipboard.writeText(Module['UTF8ToString']($0)); },
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
                        std::cout << "storage event!" << std::endl;
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