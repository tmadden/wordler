#include <alia/html.hpp>
#include <alia/html/routing.hpp>

#include <wordler/ui/creation.h>
#include <wordler/ui/decoder.h>
#include <wordler/ui/solving.h>

using namespace alia;
using namespace html;

void
app_ui(html::context ctx)
{
    document_title(ctx, "Wordler");

    placeholder_root(ctx, "app-content", [&] {
        router(ctx)
            .route("/", [&] { creation_ui(ctx); })
            .route("/puzzle/{}", [&](auto puzzle) { solving_ui(ctx, puzzle); })
            .route("/decoder", [&]() { decoder_ui(ctx); });
    });
}

int
main()
{
    static html::system the_sys;
    initialize(the_sys, app_ui);
    enable_hash_monitoring(the_sys);
};
