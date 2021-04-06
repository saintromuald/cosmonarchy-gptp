//Based on BWAPI's BW/Font.cpp
#include "Font.h"
#include "../SCBW/scbwdata.h"
#include <cassert>

namespace graphics {

	FontChar* Font::getChar(char c) const {
		uint8 ch = (uint8) c;
		if (this != nullptr
			&& this->low <= ch && ch <= this->high
			&& this->chrs[ch - this->low] != (FontChar*) this)
			return this->chrs[ch - this->low];
		return nullptr;
	}

	int Font::getCharWidth(char c) const {
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
	int Font::getTextWidth(const char* pszStr) const {
		// verify valid size index
		assert(this);
		if (!pszStr) return 0;

		// Retrieve size
		int size = 0;
		for (; *pszStr; ++pszStr)
			size += this->getCharWidth(*pszStr);
		return size;
	}

	// ----------------- GET WIDTH ---------------------
	int Font::getTextHeight(const char* pszStr) const {
		// verify valid size index
		if (!pszStr || this == nullptr) return 0;

		int size = this->yMax;
		for (; *pszStr; ++pszStr) {
			if (*pszStr == '\n')
				size += this->yMax;
		}
		return size;
	}

	int Font::maxWidth() const {
		if (this != nullptr)
			return this->xMax;
		return 0;
	}

	int Font::maxHeight() const {
		if (this != nullptr)
			return this->yMax;
		return 0;
	}

	int Font::getTextWidth(const char* pszStr, int size) {
		if (0 <= size && size <= 3)
			return fontBase[size]->getTextWidth(pszStr);
		return 0;
	}

	int Font::getTextHeight(const char* pszStr, int size) {
		if (0 <= size && size <= 3)
			return fontBase[size]->getTextHeight(pszStr);
		return 0;
	}

	// ---------------- FontChar ----------------

	int FontChar::getWidth() const {
		if (this != nullptr)
			return this->width;
		return 0;
	}

	int FontChar::getHeight() const {
		if (this != nullptr)
			return this->height;
		return 0;
	}

	int FontChar::getX() const {
		if (this != nullptr)
			return this->x;
		return 0;
	}

	int FontChar::getY() const {
		if (this != nullptr)
			return this->y;
		return 0;
	}

	int FontChar::colorMask(int index) const {
		assert(this != nullptr);
		return this->data[index] & 7;
	}

	int FontChar::pixelOffset(int index) const {
		assert(this != nullptr);
		return this->data[index] >> 3;
	}

} //graphics
