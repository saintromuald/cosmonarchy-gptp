#include "dat_load.h"
#include <SCBW/api.h>
#include <SCBW/Storm.h>
#include <hook_tools.h>

namespace hooks {
	void load_game_data(DatLoad* datTable, const char* filename)
	{
		u32 size = 0;
		u32 i;
		u8* data;
		u32 length;
		u32 pos;
		
		data = (u8*)FastFileRead(filename, NULL, false, 0, &size);
		pos = 0;
		
		// DatExt -- Patch dat file
		if (DatExt::setDatSize(datTable, size) == false)
		{
			helper::SysWarn_FileNotFound(24, filename); // invalid file
		}
		
		for (i = 0; datTable[i].address != NULL; i++)
		{
			length = datTable[i].entries * datTable[i].length;
			if (length > size) break;
			memcpy(datTable[i].address, data + pos, length);
			size -= length;
			pos += length;
		}
		
		if (size != 0)
		{
			helper::SysWarn_FileNotFound(24, filename);
		}
		
		SMemFree(data, "Starcraft\\SWAR\\lang\\gamedata.cpp", 402, 0);
		
		// Apply Flingy Expand patch to default units.dat file, so that flingy datatype will be consistent whether DatExt is used or not
		if (datTable == (DatLoad*)DatExt::LoadTable::Units_Dat)
		{
			DatExt::loadUnitNamesTbl(); // load extended unit names table
			DatExt::loadUnitIcons(); // load extended unit icons
			if (datTable[0].length == 1)
			{
				DatExt::units_dat_flingyExpand();
			}
		}		
	}
}