//Based on BWAPI's BW/Font.cpp
#include "Font.h"
#include "../SCBW/scbwdata.h"
#include <cassert>

namespace graphics {

	FontChar* Font::getChar(char c) const {
		u8 ch = (u8)c;
		if (this != nullptr
			&& this->low <= ch && ch <= this->high
			&& this->chrs[ch - this->low] != (FontChar*)this)
			return this->chrs[ch - this->low];
		return nullptr;
	}

	s32 Font::getCharWidth(char c) const {
		switch (c) {
			case '\t':
				return this->maxWidth() * 2;
			case ' ':
				return this->maxWidth() / 2;
			default:
				return this->getChar(c)->getWidth();
		}
	}

	// ----------------- GET WIDTH ---------------------
	s32 Font::getTextWidth(const char* pszStr) const {
		// verify valid size index
		assert(this);
		if (!pszStr) return 0;

		// Retrieve size
		s32 size = 0;
		for (; *pszStr; ++pszStr)
			size += this->getCharWidth(*pszStr);
		return size;
	}

	// ----------------- GET WIDTH ---------------------
	s32 Font::getTextHeight(const char* pszStr) const {
		// verify valid size index
		if (!pszStr || this == nullptr) return 0;

		s32 size = this->yMax;
		for (; *pszStr; ++pszStr) {
			if (*pszStr == '\n')
				size += this->yMax;
		}
		return size;
	}

	s32 Font::maxWidth() const {
		if (this != nullptr)
			return this->xMax;
		return 0;
	}

	s32 Font::maxHeight() const {
		if (this != nullptr)
			return this->yMax;
		return 0;
	}

	s32 Font::getTextWidth(const char* pszStr, s32 size) {
		if (0 <= size && size <= 3)
			return fontBase[size]->getTextWidth(pszStr);
		return 0;
	}

	s32 Font::getTextHeight(const char* pszStr, s32 size) {
		if (0 <= size && size <= 3)
			return fontBase[size]->getTextHeight(pszStr);
		return 0;
	}

	// ---------------- FontChar ----------------

	s32 FontChar::getWidth() const {
		if (this != nullptr)
			return this->width;
		return 0;
	}

	s32 FontChar::getHeight() const {
		if (this != nullptr)
			return this->height;
		return 0;
	}

	s32 FontChar::getX() const {
		if (this != nullptr)
			return this->x;
		return 0;
	}

	s32 FontChar::getY() const {
		if (this != nullptr)
			return this->y;
		return 0;
	}

	s32 FontChar::colorMask(s32 index) const {
		assert(this != nullptr);
		return this->data[index] & 7;
	}

	s32 FontChar::pixelOffset(s32 index) const {
		assert(this != nullptr);
		return this->data[index] >> 3;
	}

} //graphics
