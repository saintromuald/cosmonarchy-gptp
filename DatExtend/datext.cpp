#include <BW/api.h>
#include <BW/Storm.h>
#include <hook_tools.h>
#include "datext.h"

// from structures.h
int UNIT_TYPE_COUNT		= 228;
int WEAPON_TYPE_COUNT	= 130;
int FLINGY_TYPE_COUNT	= 209;
int SPRITES_TYPE_COUNT	= 517;
int IMAGE_TYPE_COUNT	= 999;
int UPGRADE_TYPE_COUNT	= 61;
int TECH_TYPE_COUNT		= 44;

namespace DatExt {
	struct {
	   const u8  name[13];
	   const u32 datTablePtr;
	   const u32 defaultSize;
	   const u32 entrySize;
	   const u32 newLimit;
	   bool loaded;
	} datFiles[] = {
		{ "units.dat",    LoadTable::Units_Dat,    19876, 97, UnitsIdLimit,    false, },
		{ "weapons.dat",  LoadTable::Weapons_Dat,   5460, 42, WeaponsIdLimit,  false, },
		{ "flingy.dat",   LoadTable::Flingy_Dat,    3135, 15, FlingyIdLimit,   false, },
		{ "sprites.dat",  LoadTable::Sprites_Dat,   3229,  7, SpritesIdLimit,  false, },
		{ "images.dat",   LoadTable::Images_Dat,   37962, 38, ImagesIdLimit,   false, },
		{ "upgrades.dat", LoadTable::Upgrades_Dat,  1281, 21, UpgradesIdLimit, false, },
		{ "techdata.dat", LoadTable::Techdata_Dat,   836, 19, TechdataIdLimit, false, },
		{ "orders.dat",   LoadTable::Orders_Dat,    4158, 22, OrdersIdLimit,   false, },
	//	{ "sfxdata.dat",  LoadTable::Sfxdata_Dat,  10296,  9, SfxdataIdLimit,  false, },
		{ "portdata.dat", LoadTable::Portdata_Dat,  1320, 12, PortdataIdLimit, false, },
		{ "mapdata.dat",  LoadTable::Mapdata_Dat,    260,  4, MapdataIdLimit,  false, },
	};

	StringTbl* g_unit_names_tbl = nullptr;
	GrpHead* g_unit_icons = nullptr;
	GrpHead** const g_unit_cmdicons = g_cmdicons_grp; // default value

	bool set_dat_size(DatLoad* dat_table_ptr, u32 size) {
		void* new_mem = nullptr;

		// get dat file table index
		u32 i = 0;
		for (i = 0; i < DatFile::COUNT && (u32)dat_table_ptr != datFiles[i].datTablePtr; i++);

		// Unknown file; return success to load normally
		if (i == DatFile::COUNT) {
			return true;
		}

		// Default file; return success to load normally
		if (size == datFiles[i].defaultSize) {
			return true;
		}
		
		// Not an integer number of entries -- invalid file
		if ((size % datFiles[i].entrySize) != 0) {
			return false;
		}

		// Dat file already patched; return success
		if (datFiles[i].loaded) {
			return true;
		}

		// Get number of entries
		u32 new_count = size / datFiles[i].entrySize;
		u32 mem_count = new_count;

		// units.dat special cases
		if (i == DatFile::Units_Dat) {
			// Count must be a multiple of 12, so round up
			if ((new_count % 12) != 0) {
				mem_count += 12 - (new_count % 12);
			}

			dat_table_ptr[0].length = 2; // flingy ID to u16

			// these normally have a length of 2 and the number of entries multiplied accordingly, but for simplicity the length will be multiplied here instead
			dat_table_ptr[36].length = 4; // placement box
			dat_table_ptr[37].length = 4; // addon position
			dat_table_ptr[38].length = 8; // unit size
		}

		// portdata.dat special cases
		if (i == DatFile::Portdata_Dat) {
			// Count must be a multiple of 5, so round up
			if ((new_count % 5) != 0) {
			  mem_count += 5 - (new_count % 5);
			}
		}

		// Load new entry counts
		for (u32 j = 0; dat_table_ptr[j].address; j++) {
			dat_table_ptr[j].entries = new_count;

			new_mem = calloc(mem_count, dat_table_ptr[j].length);
			if (!new_mem) {
				// error allocating memory
				return false; // or something
			}
			dat_table_ptr[j].address = new_mem;
		}

		// Load patches for dat file
		switch (i) {
			case DatFile::Units_Dat: {
				units_dat_do_patch(mem_count); // rounded up to a multiple of 12
				UNIT_TYPE_COUNT = new_count;
			} break;
			case DatFile::Weapons_Dat: {
				weapons_dat_do_patch(new_count);
				WEAPON_TYPE_COUNT = new_count;
			} break;
			case DatFile::Flingy_Dat: {
				flingy_dat_do_patch(new_count);
				FLINGY_TYPE_COUNT = new_count;
			} break;
			case DatFile::Sprites_Dat: {
				sprites_dat_do_patch(new_count);
				SPRITES_TYPE_COUNT = new_count;
			} break;
			case DatFile::Images_Dat: {
				images_dat_do_patch(new_count);
				IMAGE_TYPE_COUNT = new_count;
			} break;
			case DatFile::Upgrades_Dat: {
				upgrades_dat_do_patch(new_count);
				UPGRADE_TYPE_COUNT = new_count;
			} break;
			case DatFile::Techdata_Dat: {
				techdata_dat_do_patch(new_count);
				TECH_TYPE_COUNT = new_count;
			} break;
			case DatFile::Orders_Dat: {
				orders_dat_do_patch(new_count);
			} break;
			// case DatFile::Sfxdata_Dat: {
			// 	sfxdata_dat_do_patch(new_count);
			// } break;
			case DatFile::Portdata_Dat: {
				portdata_dat_do_patch(mem_count); // rounded up to a multiple of 5
			} break;
			case DatFile::Mapdata_Dat: {
		//		mapdata_dat_do_patch(new_count);
			} break;
		}

		datFiles[i].loaded = true;
		return true;
	}

	void extend_data() {
		for (u32 i = 0; i < DatFile::COUNT; ++i) {
			if (!set_dat_size((DatLoad*)datFiles[i].datTablePtr, datFiles[i].newLimit * datFiles[i].entrySize)) {
				helper::syswarn_file_not_found(24, (char*)datFiles[i].name); // invalid file
			}
		}
	}

	// Frees allocated memory
	void unload_dat_files() {
		for (u32 i = 0; i < DatFile::COUNT; i++) {
			DatLoad* dat_table_ptr = (DatLoad*)(datFiles[i].datTablePtr);
			if (i == DatFile::Units_Dat && dat_table_ptr[0].address != (void*)(0x0059CCCC)) { // Flingy ID
				if (dat_table_ptr[0].address) {
					free(dat_table_ptr[0].address);
					dat_table_ptr[0].address = (void*)(0x0059CCCC);
				}
			}
			if (!datFiles[i].loaded) {
				continue;
			}
			for (u32 j = 0; dat_table_ptr[j].entries != 0; j++) {
				if (dat_table_ptr[j].address) {
					free(dat_table_ptr[j].address);
					dat_table_ptr[j].address = nullptr;
				}
			}
		}
	
		units_dat_unpatch();
		sprites_dat_unpatch();
		images_dat_unpatch();
		upgrades_dat_unpatch();
		techdata_dat_unpatch();
	
		if (g_unit_names_tbl) {
			SMemFree(g_unit_names_tbl, __FILE__, __LINE__, 0);
			g_unit_names_tbl = nullptr;
		}
		if (g_unit_icons) {
			SMemFree(g_unit_icons, __FILE__, __LINE__, 0);
			g_unit_icons = nullptr;
		}
	}

	// Loads rez\unitnames.tbl, if it exists, for extended unit ID names
	void load_unit_names_tbl() {
		if (g_unit_names_tbl) {
			return; // already loaded
		}
		g_unit_names_tbl = (StringTbl*)FastFileRead("rez\\unitnames.tbl", nullptr, true, 0, nullptr);
	}

	// Loads unit\cmdbtns\uniticons.grp, if it exists, for extended unit ID build queue icons
	void load_unit_icons() {
		if (g_unit_icons != NULL) { // already loaded
			return;
		}
		g_unit_icons = (GrpHead*)FastFileRead("unit\\cmdbtns\\uniticons.grp", nullptr, true, 0, nullptr);

		// patch if hooks aren't enabled
		memory_patch(0x00426002 + 2, get_unit_icons_pointer()); // status_buildmorphtrain --> stats_addon_builder
		memory_patch(0x00426A33 + 2, get_unit_icons_pointer()); // status_buildmorphtrain --> stats_queue_progress
		memory_patch(0x004270DB + 2, get_unit_icons_pointer()); // status_nukesilo_resources --> stats_nukesilo
		memory_patch(0x004272F5 + 2, get_unit_icons_pointer()); // unit_stat_act --> UnitStatAct_Egg
	}

	// Returns uniticons.grp pointer if was loaded, otherwise returns default cmdicons.grp pointer
	GrpHead** const get_unit_icons_pointer() {
		if (g_unit_icons) {
			return &g_unit_icons;
		}
		return g_unit_cmdicons;
	}

	// general patching functions  
	// Overwrites references to dat file arrays
	void dat_table_patch(DatLoad* datTablePtr, s32* list) {
		// Null-terminated list of null-terminated lists of addresses
		// First value is table offset for the sub-list (i.e., 12 would mean the new address + 12 bytes is written)
		u32 i, j;
		for (i = 0, j = 0; datTablePtr[i].entries; i++, j++) {
			do {
				// Get array offset
				u32 offs;
				if (list[j] == ARRAY_END) {
					offs = (u32)(datTablePtr[i].address) + datTablePtr[i].entries * datTablePtr[i].length;
				} else {
					offs = (u32)(datTablePtr[i].address) + list[j];
				}

				// Apply patches
				for (j++; list[j]; j++) {
					memory_patch(list[j], offs);
				}
				j++;
			} while (list[j]);
		}
	}

	// Writes a value with a list of modifiers to a list of addresses, defined by "constList" struct
	void const_replace(s32* list, u32 val) {
		// - Null-terminated list of null-terminated lists of addresses
		// - First value is the #entries factor, second value is a value to be added to the product
	  
		// cl->ptr[j] == 0 terminator of previous list, so cl->ptr[j+1] is the beginning of the next struct.
		u32 i = 0;
		for (constList *cl = (constList*)list; cl->ptr[0] != 0; cl = (constList*)&cl->ptr[i + 1]) {
			s32 new_value = val * cl->factor + cl->add;
			for (i = 0; cl->ptr[i] != 0; i++) {
				memory_patch(cl->ptr[i], (u8*)&new_value, cl->size);
			}
		}
	}

	// Writes a constant value to a list of addresses
	u32 simple_replace(u32* list, u32 val) {
		u32 i = 0;
		for (; list[i] != 0; i++) {
			memory_patch(list[i], val);
		}
		return i;
	}
}