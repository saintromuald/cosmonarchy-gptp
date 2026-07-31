#include "dat_load.h"
#include <BW/api.h>
#include <storm/smem.hpp>
#include <storm/sfile.h>
#include <hook_tools.h>
#include "DatExtend/datext.h"

namespace hooks {
	void load_game_data(DatLoad* datTable, const char* normal_filename) {
		u32 size = 0;
		u32 i;
		u32 length;
		u32 pos;
		
		char filename[256] = {};
		if (datTable == (DatLoad*)DatExt::LoadTable::Sfxdata_Dat) {
			sprintf_s(filename, "%s", normal_filename);
		} else {
			sprintf_s(filename, "%sex", normal_filename);
		}

		auto loaded_file = sfile::load_mpq_file(filename);
		auto data = loaded_file.unsafe_cast<u8>();
		size = loaded_file.size;
		pos = 0;

		// DatExt -- Patch dat file
		// Note now every patch happens immediately with size being in DatExt::extend_data()
	//	if (DatExt::set_dat_size(datTable, size) == false) {
	//		helper::fatal_file_not_found(24, filename); // invalid file
	//	}
		
		for (i = 0; datTable[i].address != NULL; i++) {
			length = datTable[i].entries * datTable[i].length;
			if (length > size) break;
			memcpy(datTable[i].address, data.get() + pos, length);
			size -= length;
			pos += length;
		}
		
		if (size != 0) {
			helper::fatal_file_not_found(24, filename);
		}
		
		// Apply Flingy Expand patch to default units.dat file, so that flingy datatype will be consistent whether DatExt is used or not
		if (datTable == (DatLoad*)DatExt::LoadTable::Units_Dat) {
			DatExt::load_unit_names_tbl(); // load extended unit names table
			DatExt::load_unit_icons(); // load extended unit icons
			if (datTable[0].length == 1) {
				DatExt::units_dat_flingy_expand();
			}
		}		
	}
}