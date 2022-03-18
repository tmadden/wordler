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
