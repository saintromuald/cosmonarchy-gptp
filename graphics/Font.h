//Based on BWAPI's BW/Font.h
#pragma once
#include "../types.h"

#include <string_view>

#pragma pack(push, 1)

//V241 for VS2008

namespace graphics {

struct FontChar {
      s32 getWidth() const;

      u8  width, height, x, y;
    u8  data[1];
};

struct Font {
      FontChar* getChar(char c) const;
    s32 getTextWidth(std::string_view text) const;
    s32 getTextHeight(std::string_view text) const;
    s32 getCharWidth(char c, char prev_c) const;
    s32 maxWidth() const;
    static s32 getTextWidth(std::string_view text, s32 size);
    static s32 getTextHeight(std::string_view text, s32 size);

      u32 magic;
    u8  low, high;
    u8  xMax, yMax;
    FontChar* chrs[1];
};

}

#pragma pack(pop)
