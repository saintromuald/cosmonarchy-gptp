#include "draw_hook.h"
#include "../SCBW/api.h"
#include "../hook_tools.h"
#include "graphics_misc.h"

namespace
{

	//-------- Draw hook taken from BWAPI --------//
	bool wantRefresh = false;
	void __stdcall DrawHook(graphics::Bitmap* surface, Bounds* bounds)
	{
		if (wantRefresh)
		{
			wantRefresh = false;
			scbw::refreshScreen();
		}
		/*
		if (deathTable[0].player[11] == 1) {
			u16 l = deathTable[1].player[11];
			u16 t = deathTable[2].player[11];
			u16 r = deathTable[3].player[11];
			u16 b = deathTable[4].player[11];
			u16 x = deathTable[6].player[11];
			u16 y = deathTable[7].player[11];
			u16 unit= deathTable[5].player[11];
			//graphics::drawBox(l, t, r, b, graphics::YELLOW, graphics::ON_MAP);
		  //  graphics::drawDot(x, y, graphics::GREEN, graphics::ON_MAP);
		}*/
		oldDrawGameProc(surface, bounds);

		//if ( BW::BWDATA::GameScreenBuffer->isValid() )
		//{
		//  u32 numShapes = BWAPI::BroodwarImpl.drawShapes();
		//  
		//  if ( numShapes )
		//    wantRefresh = true;
		//}
		if (graphics::drawAllShapes() > 0)
			wantRefresh = true;
	}

} //unnamed namespace

namespace hooks
{

	void injectDrawHook()
	{
		memoryPatch(0x004BD68D, &DrawHook);
	}

} //hooks
