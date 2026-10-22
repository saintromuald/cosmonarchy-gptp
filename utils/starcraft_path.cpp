#include "starcraft_path.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace bw {
std::filesystem::path get_starcraft_path() {
    wchar_t buffer[MAX_PATH] = {};
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
}
} // namespace bw
