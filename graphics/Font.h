//Based on BWAPI's BW/Font.h
#pragma once
#include "../types.h"
#pragma pack(1)

//V241 for VS2008

namespace graphics {

class FontChar {
  public:
    s32 getWidth() const;
    s32 getHeight() const;
    s32 getX() const;
    s32 getY() const;
    s32 colorMask(s32 index) const;
    s32 pixelOffset(s32 index) const;

  private:
    u8  width, height, x, y;
    u8  data[1];
};

class Font {
  public:
    FontChar* getChar(char c) const;
    s32 getTextWidth(const char *pszStr) const;
    s32 getTextHeight(const char *pszStr) const;
    s32 getCharWidth(char c) const;
    s32 maxWidth() const;
    s32 maxHeight() const;
    static s32 getTextWidth(const char *pszStr, s32 size);
    static s32 getTextHeight(const char *pszStr, s32 size);

  private:
    u32 magic;
    u8  low, high;
    u8  xMax, yMax;
    FontChar* chrs[1];
};

}

#pragma pack()
