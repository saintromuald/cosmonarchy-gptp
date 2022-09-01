#include "draw_hook.h"
#include "../SCBW/api.h"
#include "../hook_tools.h"
#include "graphics_misc.h"

namespace
{
	bool wantRefresh = false;
	void __stdcall DrawHook(graphics::Bitmap* surface, Bounds* bounds)
	{
		if (wantRefresh)
		{
			wantRefresh = false;
			scbw::refresh_screen();
		}
		/*
		if (g_unit_deaths_table[0].player[11] == 1) {
			u16 l = g_unit_deaths_table[1].player[11];
			u16 t = g_unit_deaths_table[2].player[11];
			u16 r = g_unit_deaths_table[3].player[11];
			u16 b = g_unit_deaths_table[4].player[11];
			u16 x = g_unit_deaths_table[6].player[11];
			u16 y = g_unit_deaths_table[7].player[11];
			u16 unit= g_unit_deaths_table[5].player[11];
			//graphics::draw_box(l, t, r, b, graphics::YELLOW, graphics::ON_MAP);
		  //  graphics::draw_dot(x, y, graphics::GREEN, graphics::ON_MAP);
		}*/
		oldDrawGameProc(surface, bounds);

		//if ( BW::BWDATA::GameScreenBuffer->isValid() )
		//{
		//  u32 numShapes = BWAPI::BroodwarImpl.drawShapes();
		//  
		//  if ( numShapes )
		//    wantRefresh = true;
		//}
		if (graphics::draw_all_shapes() > 0)
		{
			wantRefresh = true;
		}
	}
}

namespace hooks
{
	void inject_draw_hook()
	{
		memory_patch(0x004BD68D, &DrawHook);
	}
}