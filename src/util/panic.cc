#include "panic.hh"

#include <cstdio>
#include <cstdlib>

namespace hidden {

[[noreturn]]
void panic_impl(const char *s) noexcept {
    std::fputs(s, stderr);
    std::abort();
}

}