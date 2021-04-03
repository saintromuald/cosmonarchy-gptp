//Based on BWAPI's BW/Font.h
#pragma once
#include "../types.h"
#pragma pack(1)

//V241 for VS2008

namespace graphics {

class FontChar {
  public:
    int getWidth() const;
    int getHeight() const;
    int getX() const;
    int getY() const;
    int colorMask(int index) const;
    int pixelOffset(int index) const;

  private:
    uint8  width, height, x, y;
    uint8  data[1];
};

class Font {
  public:
    FontChar* getChar(char c) const;
    int getTextWidth(const char *pszStr) const;
    int getTextHeight(const char *pszStr) const;
    int getCharWidth(char c) const;
    int maxWidth() const;
    int maxHeight() const;
    static int getTextWidth(const char *pszStr, int size);
    static int getTextHeight(const char *pszStr, int size);

  private:
    uint32 magic;
    uint8  low, high;
    uint8  xMax, yMax;
    FontChar* chrs[1];
};

}

#pragma pack()
