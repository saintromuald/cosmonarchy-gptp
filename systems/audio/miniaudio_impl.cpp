// Single translation unit that compiles the miniaudio implementation exactly once
// Excluded from the unity build so MINIAUDIO_IMPLEMENTATION is defined in one TU only
// Decode-only configuration is set in preprocessor defines (MA_NO_*)

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#undef STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>
