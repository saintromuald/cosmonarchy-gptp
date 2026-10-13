//Based on BWAPI's BW/Font.cpp
#include "Font.h"
#include "../BW/bwdata.h"
#include "BW/enumerations.h"
#include "systems/text/decoder.h"
#include "systems/text/measure.h"
#include "systems/text/text.h"

#include <cassert>
#include <utility>

namespace graphics
{

    FontChar* Font::getChar(char c) const
    {
        u8 ch = (u8)c;
        if (this != nullptr
            && this->low <= ch && ch <= this->high
            && this->chrs[ch - this->low] != (FontChar*)this)
            return this->chrs[ch - this->low];
        return nullptr;
    }

    s32 Font::getCharWidth(char c, char prev_c) const {
        if (prev_c == std::to_underlying(TextColor::PlayerColor)
            || prev_c == std::to_underlying(TextColor::PlayerColor2)) {
            return 0;
        }
        const u8 byte = (u8)c;
        if (byte >= text::ExtendedColorFirst && byte <= text::ExtendedColorLast
            && byte != text::TrademarkByte) {
            return 0; // extended colour codes render nothing
        }
        switch (c) {
            case '\t':
                return this->maxWidth() * 2;
            case ' ':
                return this->maxWidth() / 2;
            case std::to_underlying(TextColor::PlayerColor):
            case std::to_underlying(TextColor::PlayerColor2):
                return 0;
            default:
                return this->getChar(c)->getWidth();
        }
    }

    s32 Font::getTextWidth(std::string_view text) const {
        assert(this);

        if (text::TextSystem::instance().available()) {
            if (const auto slot = text::slot_of(this)) {
                return text::text_width(*slot, text);
            }
        }

        s32 size = 0;
        char prev_c = 0;
        for (const char c : text) {
            size += this->getCharWidth(c, prev_c);
            prev_c = c;
        }
        return size;
    }

    s32 Font::getTextHeight(std::string_view text) const
    {
        if (this == nullptr) return 0;

        if (text::TextSystem::instance().available()) {
            if (const auto slot = text::slot_of(this)) {
                return text::text_height(*slot, text);
            }
        }

        s32 size = this->yMax;
        for (const char c : text)
        {
            if (c == '\n')
                size += this->yMax;
        }
        return size;
    }

    s32 Font::maxWidth() const
    {
        if (this != nullptr)
            return this->xMax;
        return 0;
    }

    s32 Font::maxHeight() const
    {
        if (this != nullptr)
            return this->yMax;
        return 0;
    }

    s32 Font::getTextWidth(std::string_view text, s32 size)
    {
        if (text::TextSystem::instance().available() && 0 <= size && size <= 3)
            return text::text_width((u8)size, text);
        if (0 <= size && size <= 3)
            return font_base[size]->getTextWidth(text);
        return 0;
    }

    s32 Font::getTextHeight(std::string_view text, s32 size)
    {
        if (text::TextSystem::instance().available() && 0 <= size && size <= 3)
            return text::text_height((u8)size, text);
        if (0 <= size && size <= 3)
            return font_base[size]->getTextHeight(text);
        return 0;
    }

    // ---------------- FontChar ----------------

    s32 FontChar::getWidth() const
    {
        if (this != nullptr)
            return this->width;
        return 0;
    }

    s32 FontChar::getHeight() const
    {
        if (this != nullptr)
            return this->height;
        return 0;
    }

    s32 FontChar::get_x() const
    {
        if (this != nullptr)
            return this->x;
        return 0;
    }

    s32 FontChar::get_y() const
    {
        if (this != nullptr)
            return this->y;
        return 0;
    }

    s32 FontChar::colorMask(s32 index) const
    {
        assert(this != nullptr);
        return this->data[index] & 7;
    }

    s32 FontChar::pixelOffset(s32 index) const
    {
        assert(this != nullptr);
        return this->data[index] >> 3;
    }

} // namespace graphics