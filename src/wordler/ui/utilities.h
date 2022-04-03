#pragma once

#include <emscripten/html5.h>
#include <emscripten/val.h>

#include <alia/html.hpp>

using namespace alia;

// Candidate code for incorporation into alia/HTML...

void
copy_to_clipboard(std::string const& text);

void
share_text(std::string const& title, std::string const& text);

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

html::storage_signal
get_storage_state(
    html::context ctx, char const* storage_name, readable<std::string> key);

html::storage_signal
get_session_state(html::context ctx, readable<std::string> key);

html::storage_signal
get_local_state(html::context ctx, readable<std::string> key);

// Candidate code for incorporation into alia...

// mask(a, s), where :a is an action and :s is a signal, returns an equivalent
// action that's only ready if :a is ready and :s has a value that evaluates to
// :true.
//
template<class Wrapped, class ReadinessMask, class Action>
struct action_masking_adaptor;

template<class Wrapped, class ReadinessMask, class... Args>
struct action_masking_adaptor<
    Wrapped,
    ReadinessMask,
    action_interface<Args...>> :
    // TODO: Why doesn't this work?
    // typename Wrapped::action_type
    action_interface<>
{
    action_masking_adaptor(Wrapped wrapped, ReadinessMask mask)
        : wrapped_(std::move(wrapped)), mask_(std::move(mask))
    {
    }

    bool
    is_ready() const override
    {
        return wrapped_.is_ready() && signal_has_value(mask_)
               && read_signal(mask_);
    }

    void
    perform(
        function_view<void()> const& intermediary, Args... args) const override
    {
        wrapped_.perform(intermediary, std::move(args)...);
    }

 private:
    Wrapped wrapped_;
    ReadinessMask mask_;
};

template<class Wrapped, class ReadinessMask>
auto
mask_action(Wrapped wrapped, ReadinessMask readiness_mask)
{
    return action_masking_adaptor<
        Wrapped,
        ReadinessMask,
        // TODO: Why doesn't this work?
        // typename Wrapped::action_type
        action_interface<>>(std::move(wrapped), std::move(readiness_mask));
}

// minimize_id_changes(ctx, x), where :x is a signal, yields a new signal to
// x's value with a local ID that only changes when x's value actually changes.
template<class Value>
struct id_change_minimization_data
{
    captured_id input_id;
    counter_type version = 0;
    Value value;
    bool is_valid = false;
};
template<class Wrapped>
struct id_change_minimization_signal
    : signal_wrapper<id_change_minimization_signal<Wrapped>, Wrapped>
{
    id_change_minimization_signal(
        Wrapped wrapped,
        id_change_minimization_data<typename Wrapped::value_type>* data)
        : id_change_minimization_signal::signal_wrapper(std::move(wrapped)),
          data_(data)
    {
    }
    id_interface const&
    value_id() const
    {
        id_ = make_id(data_->version);
        return id_;
    }

 private:
    id_change_minimization_data<typename Wrapped::value_type>* data_;
    mutable simple_id<counter_type> id_;
};
template<class Signal>
void
update_id_change_minimization_data(
    id_change_minimization_data<typename Signal::value_type>* data,
    Signal const& x)
{
    if (!data->input_id.matches(x.value_id()))
    {
        // Only change the output ID if the value has actually changed.
        if (!(data->is_valid && signal_has_value(x)
              && data->value == read_signal(x)))
        {
            ++data->version;
            data->is_valid = false;
        }
        data->input_id.capture(x.value_id());
    }
    if (!data->is_valid && signal_has_value(x))
    {
        data->value = read_signal(x);
        data->is_valid = true;
    }
}
template<class Signal, class Value>
id_change_minimization_signal<Signal>
minimize_id_changes(
    dataless_context ctx, id_change_minimization_data<Value>* data, Signal x)
{
    if (is_refresh_event(ctx))
        update_id_change_minimization_data(data, x);
    return id_change_minimization_signal<Signal>(std::move(x), data);
}
template<class Signal>
id_change_minimization_signal<Signal>
minimize_id_changes(context ctx, Signal x)
{
    id_change_minimization_data<typename Signal::value_type>* data;
    get_cached_data(ctx, &data);
    return minimize_id_changes(ctx, data, std::move(x));
}