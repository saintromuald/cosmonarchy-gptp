#pragma once

struct Image;

namespace hooks {
inline char* get_iscript_file(Image* image);
void inject_iscript_extender();
}