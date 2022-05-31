//Based on BWAPI's BW/Bitmap.h
#pragma once
#include "graphics.h"
#pragma pack(1)

//V241 for VS2008

namespace graphics
{

	struct Bitmap
	{
		bool blitString(const char* pszStr, s32 x, s32 y, u8 size);
		u16 getWidth() const;
		u16 getHeight() const;

		void drawDot(s32 x, s32 y, ColorId color);
		void draw_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color);
		void drawBox(s32 left, s32 top, s32 right, s32 bottom, ColorId color);
		void drawFilledBox(s32 left, s32 top, s32 right, s32 bottom, ColorId color);
		void drawCircle(s32 x, s32 y, s32 radius, ColorId color);
		void drawFilledCircle(s32 x, s32 y, s32 radius, ColorId color);
		void drawEllipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color);
		void drawDottedLine(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color);
		void drawDottedEllipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color);

		u16 width, height;
		u8* data;

		void drawHorizontalLine(s32 x1, s32 x2, s32 y, ColorId color);
		void drawVerticalLine(s32 x, s32 y1, s32 y2, ColorId color);
		void drawDotUnsafe(s32 x, s32 y, ColorId color);
		void drawHorizontalLineUnsafe(s32 x1, s32 x2, s32 y, ColorId color);
		void drawVerticalLineUnsafe(s32 x, s32 y1, s32 y2, ColorId color);

		void blitKoreanChar(const char* ch, s32& x, s32& y, u8 fontSize, u8 color);

		//Trivially checks whether the line should be drawn; based on the trivial
		//check in the Cohen-Sutherland line clipping algorithm.
		bool isLineTriviallyIgnorable(s32 x1, s32 y1, s32 x2, s32 y2);
		s32 computeOutcode(s32 x, s32 y);
	};

} //graphics

#pragma pack()
