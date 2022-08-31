#include <SCBW/api.h>
#include <SCBW/Storm.h>
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

	StringTbl* unitNamesTbl = NULL;
	GrpHead* unitIcons = NULL;
	GrpHead** const cmdIcons = (GrpHead**)(0x0068C1E0); // default value

	bool setDatSize(DatLoad* datTablePtr, u32 size)
	{

		u32 i,j;
		u32 newCount, memCount;
		void* newMem = NULL;

		// get dat file table index
		for (i = 0; i < DatFile::COUNT && (u32)datTablePtr != datFiles[i].datTablePtr; i++);

		// Unknown file; return success to load normally
		if (i == DatFile::COUNT)
		{
			return true;
		}

		// Default file; return success to load normally
		if (size == datFiles[i].defaultSize)
		{
			return true;
		}
		
		// Not an integer number of entries -- invalid file
		if ((size % datFiles[i].entrySize) != 0)
		{
			return false;
		}

		// Dat file already patched; return success
		if (datFiles[i].loaded)
		{
			return true;
		}

		// Get number of entries
		newCount = size / datFiles[i].entrySize;
		memCount = newCount;

		// units.dat special cases
		if (i == DatFile::Units_Dat) {
			// Count must be a multiple of 12, so round up
			if ((newCount % 12) != 0) {
				memCount += 12 - (newCount % 12);
			}

			datTablePtr[0].length = 2; // flingy ID to u16

			// these normally have a length of 2 and the number of entries multiplied accordingly, but for simplicity the length will be multiplied here instead
			datTablePtr[36].length = 4; // placement box
			datTablePtr[37].length = 4; // addon position
			datTablePtr[38].length = 8; // unit size
		}

		// portdata.dat special cases
		if (i == DatFile::Portdata_Dat) {
			// Count must be a multiple of 5, so round up
			if ((newCount % 5) != 0)
			{
			  memCount += 5 - (newCount % 5);
			}
		}

		// Load new entry counts
		for (j = 0; datTablePtr[j].address != NULL; j++) {
			datTablePtr[j].entries = newCount;

			newMem = calloc(memCount, datTablePtr[j].length);
			if (newMem == NULL) {
				// error allocating memory
				return false; // or something
			}
			datTablePtr[j].address = newMem;
		}

		// Load patches for dat file
		switch (i) {
			case DatFile::Units_Dat:
			{
				units_dat_doPatch(memCount); // rounded up to a multiple of 12
				UNIT_TYPE_COUNT = newCount;
			} break;
			case DatFile::Weapons_Dat:
			{
				weapons_dat_doPatch(newCount);
				WEAPON_TYPE_COUNT = newCount;
			} break;
			case DatFile::Flingy_Dat:
			{
				flingy_dat_doPatch(newCount);
				FLINGY_TYPE_COUNT = newCount;
			} break;
			case DatFile::Sprites_Dat:
			{
				sprites_dat_doPatch(newCount);
				SPRITES_TYPE_COUNT = newCount;
			} break;
			case DatFile::Images_Dat:
			{
				images_dat_doPatch(newCount);
				IMAGE_TYPE_COUNT = newCount;
			} break;
			case DatFile::Upgrades_Dat:
			{
				upgrades_dat_doPatch(newCount);
				UPGRADE_TYPE_COUNT = newCount;
			} break;
			case DatFile::Techdata_Dat:
			{
				techdata_dat_doPatch(newCount);
				TECH_TYPE_COUNT = newCount;
			} break;
			case DatFile::Orders_Dat:
			{
				orders_dat_doPatch(newCount);
			} break;
			// case DatFile::Sfxdata_Dat:
			// {
			// 	sfxdata_dat_doPatch(newCount);
			// } break;
			case DatFile::Portdata_Dat:
			{
				portdata_dat_doPatch(memCount); // rounded up to a multiple of 5
			} break;
			case DatFile::Mapdata_Dat:
			{
		//		mapdata_dat_doPatch(newCount);
			} break;
		}

		datFiles[i].loaded = true;
		return true;
	}

	void extend_data() {
		for (u32 i = 0; i < DatFile::COUNT; ++i) {
			if (!setDatSize((DatLoad*)datFiles[i].datTablePtr, datFiles[i].newLimit * datFiles[i].entrySize)) {
				helper::SysWarn_FileNotFound(24, (char*)datFiles[i].name); // invalid file
			}
		}
	}

	// Frees allocated memory
	void unloadDatFiles() {
		u32 i, j;
		DatLoad* datTablePtr;
	
		for (i = 0; i < DatFile::COUNT; i++)
		{
			datTablePtr = (DatLoad*)(datFiles[i].datTablePtr);
			if (i == DatFile::Units_Dat && datTablePtr[0].address != (void*)(0x0059CCCC)) // Flingy ID
			{
				if (datTablePtr[0].address != NULL)
				{
					free(datTablePtr[0].address);
					datTablePtr[0].address = (void*)(0x0059CCCC);
				}
			}
			if (datFiles[i].loaded == false)
			{
				continue;
			}
			for (j = 0; datTablePtr[j].entries != 0; j++)
			{
				if (datTablePtr[j].address != NULL)
				{
					free(datTablePtr[j].address);
					datTablePtr[j].address = NULL;
				}
			}
		}
	
		units_dat_unpatch();
		images_dat_unpatch();
		upgrades_dat_unpatch();
		techdata_dat_unpatch();
	
		if (unitNamesTbl != NULL) {
			SMemFree(unitNamesTbl, __FILE__, __LINE__, 0);
			unitNamesTbl = NULL;
		}

		if (unitIcons != NULL) {
			SMemFree(unitIcons, __FILE__, __LINE__, 0);
			unitIcons = NULL;
		}
	}

	// Loads rez\unitnames.tbl, if it exists, for extended unit ID names
	void loadUnitNamesTbl()
	{
		if (unitNamesTbl != NULL)
		{
			return; // already loaded
		}
		unitNamesTbl = (StringTbl*)FastFileRead("rez\\unitnames.tbl", NULL, true, 0, NULL);
	}

	// Loads unit\cmdbtns\uniticons.grp, if it exists, for extended unit ID build queue icons
	void loadUnitIcons() {
		if (unitIcons != NULL) return; // already loaded

		unitIcons = (GrpHead*)FastFileRead("unit\\cmdbtns\\uniticons.grp", NULL, true, 0, NULL);

		// patch if hooks aren't enabled
		memory_patch(0x00426002 + 2, getUnitIconsPtr()); // status_buildmorphtrain --> stats_AddOnBuilder
		memory_patch(0x00426A33 + 2, getUnitIconsPtr()); // status_buildmorphtrain --> stats_QueueProgress
		memory_patch(0x004270DB + 2, getUnitIconsPtr()); // status_nukesilo_resources --> stats_nukesilo
		memory_patch(0x004272F5 + 2, getUnitIconsPtr()); // unit_stat_act --> UnitStatAct_Egg
	}

	// Returns uniticons.grp pointer if was loaded, otherwise returns default cmdicons.grp pointer
	GrpHead** const getUnitIconsPtr() {
		if (unitIcons != NULL) return &unitIcons;
		return cmdIcons;
	}

	// general patching functions  
	// Overwrites references to dat file arrays
	void datTablePatch(DatLoad* datTablePtr, s32* list)
	{
		u32 i, j;
		u32 offs;
		// Null-terminated list of null-terminated lists of addresses
		// First value is table offset for the sub-list (i.e., 12 would mean the new address + 12 bytes is written)

		for (i = 0, j = 0; datTablePtr[i].entries != 0; i++, j++)
		{
			do
			{
				// Get array offset
				if (list[j] == ARRAY_END)
				{
					offs = (u32)(datTablePtr[i].address) + datTablePtr[i].entries * datTablePtr[i].length;
				}
				else
				{
					offs = (u32)(datTablePtr[i].address) + list[j];
				}

				// Apply patches
				for (j++; list[j] != 0; j++)
				{
					memory_patch(list[j], offs);
				}
				
				j++;
			} while (list[j] != 0);
		}
	}

	// Writes a value with a list of modifiers to a list of addresses, defined by "constList" struct
	void constReplace(s32* list, u32 val) {
		u32 i;
		s32 newVal;
		// - Null-terminated list of null-terminated lists of addresses
		// - First value is the #entries factor, second value is a value to be added to the product
	  
		// cl->ptr[j] == 0 terminator of previous list, so cl->ptr[j+1] is the beginning of the next struct.
		for (constList *cl = (constList*)list; cl->ptr[0] != 0; cl = (constList*)&cl->ptr[i + 1])
		{
			newVal = val * cl->factor + cl->add;
			for (i = 0; cl->ptr[i] != 0; i++)
			{
				memory_patch(cl->ptr[i], (u8*)&newVal, cl->size);
			}
		}
	}

	// Writes a constant value to a list of addresses
	u32 simpleReplace(u32* list, u32 val)
	{
		u32 i;
		for (i = 0; list[i] != 0; i++)
		{
			memory_patch(list[i], val);
		}
		return i;
	}
}