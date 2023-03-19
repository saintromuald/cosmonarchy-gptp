#include "ccmu.h"
#include "hook_tools.h"
#include "scbw/structures.h"
#include "scbw/structures/Sprite.h"
#include "scbw/structures/Order.h"
#include "scbw/structures/Image.h"

Unit new_unit_table[UnitLimit];
int new_air_splash_list[UnitLimit];
UnitFinderData new_unit_finder_x[UnitLimit * 2];
UnitFinderData new_unit_finder_y[UnitLimit * 2];
int new_get_all_units_in_bounds_list[UnitLimit + 1]; // Both arrays require an extra DWORD at the end to work properly. Looks like they also have an extra one at the beginning, judging by the IDA struct, but seems to work fine without them.
int new_get_all_units_in_bounds_index[UnitLimit + 1];

Sprite new_sprite_table[SpriteLimit];
Sprite* new_sprite_list_pointers[SpriteLimit];// 0x006C2318

Order new_order_table[OrderLimit];

Image new_image_table[ImageLimit];
int new_image_list_pointers[ImageLimit]; // 0x006C4A38, a DWORD leads this array at 6C4A30, but I've never seen it be accessed.

Thingy new_thingy_table[ThingyLimit]; // 0x006509D8
Thingy new_fow_sprite_table[FowSpriteLimit]; // 0x00652928

namespace hooks {
	void inject_unit_extender() {
		// unit "unique identifier" bitshift, don't feel like organizing them into the categories below atm
		memory_patch(0x00401F1A, (u8)0xC);
		memory_patch(0x00402D8C, (u8)0xC);
		memory_patch(0x00402DF6, (u8)0xC);
		memory_patch(0x00402E5F, (u8)0xC);
		memory_patch(0x00402EC8, (u8)0xC);
		memory_patch(0x00402F32, (u8)0xC);
		memory_patch(0x004034DD, (u8)0xC);
		memory_patch(0x0040351C, (u8)0xC);
		memory_patch(0x0040355B, (u8)0xC);
		memory_patch(0x00403599, (u8)0xC);
		memory_patch(0x00403B60, (u8)0xC);
		memory_patch(0x00403B9F, (u8)0xC);
		memory_patch(0x00403BDE, (u8)0xC);
		memory_patch(0x00403C1D, (u8)0xC);
		memory_patch(0x00403C5C, (u8)0xC);
		memory_patch(0x00403C9A, (u8)0xC);
		memory_patch(0x00403EC9, (u8)0xC);
		memory_patch(0x00403F65, (u8)0xC);
		memory_patch(0x00403FFF, (u8)0xC);
		memory_patch(0x0040409B, (u8)0xC);
		memory_patch(0x00404137, (u8)0xC);
		memory_patch(0x00424C62, (u8)0xC);
		memory_patch(0x00424E93, (u8)0xC);
		memory_patch(0x00424F7F, (u8)0xC);
		memory_patch(0x00425023, (u8)0xC);
		memory_patch(0x00425073, (u8)0xC);
		memory_patch(0x004250C4, (u8)0xC);
		memory_patch(0x00425116, (u8)0xC);
		memory_patch(0x00431225, (u8)0xC);
		memory_patch(0x004312E5, (u8)0xC);
		memory_patch(0x00432922, (u8)0xC);
		memory_patch(0x00432A6A, (u8)0xC);
		memory_patch(0x0043827A, (u8)0xC);
		memory_patch(0x00447F1C, (u8)0xC);
		memory_patch(0x00447F61, (u8)0xC);
		memory_patch(0x00447FA6, (u8)0xC);
		memory_patch(0x00447FEB, (u8)0xC);
		memory_patch(0x0045835C, (u8)0xC);
		memory_patch(0x0045F8BB, (u8)0xC);
		memory_patch(0x0046B081, (u8)0xC);
		memory_patch(0x0046BC77, (u8)0xC);
		memory_patch(0x0046BD07, (u8)0xC);
		memory_patch(0x0046BFD9, (u8)0xC);
		memory_patch(0x00479EFA, (u8)0xC);
		memory_patch(0x00479F39, (u8)0xC);
		memory_patch(0x0047B201, (u8)0xC);
		memory_patch(0x0047B267, (u8)0xC);
		memory_patch(0x0048A913, (u8)0xC);
		memory_patch(0x0048A956, (u8)0xC);
		memory_patch(0x0048A9E5, (u8)0xC);
		memory_patch(0x0048AA24, (u8)0xC);
		memory_patch(0x0048C813, (u8)0xC);
		memory_patch(0x00491600, (u8)0xC);
		memory_patch(0x00494D1D, (u8)0xC);
		memory_patch(0x0049663F, (u8)0xC);
		memory_patch(0x00496726, (u8)0xC);
		memory_patch(0x004968AE, (u8)0xC);
		memory_patch(0x00496A3D, (u8)0xC);
		memory_patch(0x00496BCC, (u8)0xC);
		memory_patch(0x0049FE7A, (u8)0xC);
		memory_patch(0x004BFBAB, (u8)0xC);
		memory_patch(0x004BFDEA, (u8)0xC);
		memory_patch(0x004C034F, (u8)0xC);
		memory_patch(0x004C03CC, (u8)0xC);
		memory_patch(0x004C0953, (u8)0xC);
		memory_patch(0x004C09E0, (u8)0xC);
		memory_patch(0x004C0A5F, (u8)0xC);
		memory_patch(0x004C2529, (u8)0xC);
		memory_patch(0x004C264B, (u8)0xC);
		memory_patch(0x004C27CE, (u8)0xC);
		memory_patch(0x004C7F03, (u8)0xC);
		memory_patch(0x004CEDE8, (u8)0xC);
		memory_patch(0x004CEE48, (u8)0xC);
		memory_patch(0x004D08BD, (u8)0xC);
		memory_patch(0x004D568C, (u8)0xC);
		memory_patch(0x004D63B8, (u8)0xC);
		memory_patch(0x004E349A, (u8)0xC);
		memory_patch(0x004E34D9, (u8)0xC);
		memory_patch(0x004E3518, (u8)0xC);
		memory_patch(0x004E359F, (u8)0xC);
		memory_patch(0x004E35E1, (u8)0xC);
		memory_patch(0x004E362F, (u8)0xC);
		memory_patch(0x004E3686, (u8)0xC);
		memory_patch(0x004E36CB, (u8)0xC);
		memory_patch(0x004E3730, (u8)0xC);
		memory_patch(0x004E3775, (u8)0xC);
		memory_patch(0x004E37CE, (u8)0xC);
		memory_patch(0x004E3813, (u8)0xC);
		memory_patch(0x004E3858, (u8)0xC);
		memory_patch(0x004E38A4, (u8)0xC);
		memory_patch(0x004E38F1, (u8)0xC);
		memory_patch(0x004E39AD, (u8)0xC);
		memory_patch(0x004E3A02, (u8)0xC);
		memory_patch(0x004E3A59, (u8)0xC);
		memory_patch(0x004E3A9E, (u8)0xC);
		memory_patch(0x004E3AE3, (u8)0xC);
		memory_patch(0x004E3B28, (u8)0xC);
		memory_patch(0x004E3B7F, (u8)0xC);
		memory_patch(0x004E3BC4, (u8)0xC);
		memory_patch(0x004E3C15, (u8)0xC);
		memory_patch(0x004E3C5A, (u8)0xC);
		memory_patch(0x004E3CAA, (u8)0xC);
		memory_patch(0x004E3D05, (u8)0xC);
		memory_patch(0x004E3D5C, (u8)0xC);
		memory_patch(0x004E3DA4, (u8)0xC);
		memory_patch(0x004E6C7D, (u8)0xC);
		memory_patch(0x004E6CEA, (u8)0xC);
		memory_patch(0x004E6D58, (u8)0xC);
		memory_patch(0x004E6DD7, (u8)0xC);
		memory_patch(0x004E70C7, (u8)0xC);
		memory_patch(0x004E715C, (u8)0xC);
		memory_patch(0x004E71CA, (u8)0xC);
		memory_patch(0x004E7215, (u8)0xC);
		memory_patch(0x004E7261, (u8)0xC);
		memory_patch(0x004E72AD, (u8)0xC);
		memory_patch(0x004E793F, (u8)0xC);
		memory_patch(0x004E79D7, (u8)0xC);
		memory_patch(0x004E7E92, (u8)0xC);
		memory_patch(0x004E8141, (u8)0xC);
		memory_patch(0x004EA9A6, (u8)0xC);
		memory_patch(0x004EB20C, (u8)0xC);
		memory_patch(0x004EB3AE, (u8)0xC);

		// Added to prevent writes to index bitfield variable as a safety -Veeq7
		memory_patch(0x004A03F2, (u8)0xF);

		// update_unit_transport_data (0x00401EA0)
		memory_patch(0x00401EF0, &new_unit_table);
		memory_patch(0x00401F07, UnitLimit);

		// packDetailFinderData (0x00402D10)
		memory_patch(0x00402D62, &new_unit_table);
		memory_patch(0x00402D79, UnitLimit);
		memory_patch(0x00402DCC, &new_unit_table);
		memory_patch(0x00402DE3, UnitLimit);
		memory_patch(0x00402E35, &new_unit_table);
		memory_patch(0x00402E4C, UnitLimit);
		memory_patch(0x00402E9E, &new_unit_table);
		memory_patch(0x00402EB5, UnitLimit);
		memory_patch(0x00402F08, &new_unit_table);
		memory_patch(0x00402F1F, UnitLimit);

		// unpackDetailFinderData (0x00402FB0)
		memory_patch(0x00402FC8, UnitIDMask);
		memory_patch(0x00402FD4, &new_unit_table[-1]);

		// packAICaptainData (0x00403480)
		memory_patch(0x004034B3, &new_unit_table);
		memory_patch(0x004034CA, UnitLimit);
		memory_patch(0x004034F2, &new_unit_table);
		memory_patch(0x00403509, UnitLimit);
		memory_patch(0x00403531, &new_unit_table);
		memory_patch(0x00403548, UnitLimit);
		memory_patch(0x0040356C, &new_unit_table);
		memory_patch(0x00403583, UnitLimit);

		// unpackAICaptainData (0x004035B0)
		memory_patch(0x004035D4, UnitIDMask);
		memory_patch(0x004035E0, &new_unit_table[-1]);
		memory_patch(0x004035F4, UnitIDMask);
		memory_patch(0x00403600, &new_unit_table[-1]);
		memory_patch(0x00403614, UnitIDMask);
		memory_patch(0x00403620, &new_unit_table[-1]);
		memory_patch(0x00403634, UnitIDMask);
		memory_patch(0x00403640, &new_unit_table[-1]);

		// unpackAllPathingData (0x00403780)
		memory_patch(0x004037A2, UnitIDMask);
		memory_patch(0x004037AE, &new_unit_table[-1]);
		memory_patch(0x00403832, UnitIDMask);
		memory_patch(0x0040383E, &new_unit_table[-1]);
		memory_patch(0x004038CF, UnitIDMask);
		memory_patch(0x004038DB, &new_unit_table[-1]);
		memory_patch(0x00403970, UnitIDMask);
		memory_patch(0x0040397C, &new_unit_table[-1]);
		memory_patch(0x00403A11, UnitIDMask);
		memory_patch(0x00403A1D, &new_unit_table[-1]);

		// packTownMgr_03_Unit_Data (0x00403AE0)
		memory_patch(0x00403B36, &new_unit_table);
		memory_patch(0x00403B4D, UnitLimit);
		memory_patch(0x00403B75, &new_unit_table);
		memory_patch(0x00403B8C, UnitLimit);
		memory_patch(0x00403BB4, &new_unit_table);
		memory_patch(0x00403BCB, UnitLimit);
		memory_patch(0x00403BF3, &new_unit_table);
		memory_patch(0x00403C0A, UnitLimit);
		memory_patch(0x00403C32, &new_unit_table);
		memory_patch(0x00403C49, UnitLimit);
		memory_patch(0x00403C6D, &new_unit_table);
		memory_patch(0x00403C84, UnitLimit);

		// unpackTownMgr_03_Unit_Data (0x00403CB0)
		memory_patch(0x00403CEE, UnitIDMask);
		memory_patch(0x00403CFA, &new_unit_table[-1]);
		memory_patch(0x00403D0E, UnitIDMask);
		memory_patch(0x00403D1A, &new_unit_table[-1]);
		memory_patch(0x00403D2E, UnitIDMask);
		memory_patch(0x00403D3A, &new_unit_table[-1]);
		memory_patch(0x00403D4E, UnitIDMask);
		memory_patch(0x00403D5A, &new_unit_table[-1]);
		memory_patch(0x00403D6E, UnitIDMask);
		memory_patch(0x00403D7A, &new_unit_table[-1]);
		memory_patch(0x00403D8E, UnitIDMask);
		memory_patch(0x00403D9A, &new_unit_table[-1]);

		// packTownMgr_01_Data (0x00403E50)
		memory_patch(0x00403E9F, &new_unit_table);
		memory_patch(0x00403EB6, UnitLimit);
		memory_patch(0x00403F3B, &new_unit_table);
		memory_patch(0x00403F52, UnitLimit);
		memory_patch(0x00403FD5, &new_unit_table);
		memory_patch(0x00403FEC, UnitLimit);
		memory_patch(0x00404071, &new_unit_table);
		memory_patch(0x00404088, UnitLimit);
		memory_patch(0x0040410D, &new_unit_table);
		memory_patch(0x00404124, UnitLimit);

		// unpackTownMgr_01_Data (0x004041F0)
		memory_patch(0x00404209, UnitIDMask);
		memory_patch(0x00404215, &new_unit_table[-1]);

		// sub_422160 (0x00422160)
		memory_patch(0x00422195, &new_unit_finder_y);
		memory_patch(0x004221C6, &new_unit_finder_y[0].position);
		memory_patch(0x004221D3, &new_unit_finder_y);
		memory_patch(0x004221E3, &new_unit_table[-1]);
		memory_patch(0x004221F3, &new_unit_finder_y[0].position);
		memory_patch(0x00422209, &new_unit_finder_x[0].position);
		memory_patch(0x00422217, &new_unit_finder_x[0].position);
		memory_patch(0x004222B9, &new_unit_finder_x);
		memory_patch(0x004222E6, &new_unit_finder_x[0].position);
		memory_patch(0x004222F3, &new_unit_finder_x);
		memory_patch(0x00422303, &new_unit_table[-1]);
		memory_patch(0x00422313, &new_unit_finder_x[0].position);
		memory_patch(0x00422329, &new_unit_finder_y[0].position);
		memory_patch(0x00422337, &new_unit_finder_y[0].position);
		memory_patch(0x004223D9, &new_unit_finder_y);
		memory_patch(0x00422406, &new_unit_finder_y[0].position);
		memory_patch(0x00422413, &new_unit_finder_y);
		memory_patch(0x00422423, &new_unit_table[-1]);
		memory_patch(0x00422433, &new_unit_finder_y[0].position);
		memory_patch(0x00422449, &new_unit_finder_x[0].position);
		memory_patch(0x00422457, &new_unit_finder_x[0].position);
		memory_patch(0x004224F8, &new_unit_finder_x);
		memory_patch(0x00422526, &new_unit_finder_x[0].position);
		memory_patch(0x00422533, &new_unit_finder_x);
		memory_patch(0x00422543, &new_unit_table[-1]);
		memory_patch(0x00422553, &new_unit_finder_x[0].position);
		memory_patch(0x00422569, &new_unit_finder_y[0].position);
		memory_patch(0x00422577, &new_unit_finder_y[0].position);

		// set_spell_special_button_graphic (0x00424BA0)
		memory_patch(0x00424C34, UnitIDMask);
		memory_patch(0x00424C40, &new_unit_table[-1].sprite);
		memory_patch(0x00424C48, &new_unit_table[-1]);
		memory_patch(0x00424E65, UnitIDMask);
		memory_patch(0x00424E71, &new_unit_table[-1].sprite);
		memory_patch(0x00424E79, &new_unit_table[-1]);

		// unitstatcond_transport (0x00424F10)
		memory_patch(0x00424F52, UnitIDMask);
		memory_patch(0x00424F5E, &new_unit_table[-1]);

		// transport_status_update_drawn_values (0x00424FC0)
		memory_patch(0x00424FF3, UnitIDMask);
		memory_patch(0x00424FFF, &new_unit_table[-1].sprite);
		memory_patch(0x00425007, &new_unit_table[-1]);
		memory_patch(0x00425043, UnitIDMask);
		memory_patch(0x0042504F, &new_unit_table[-1].sprite);
		memory_patch(0x00425057, &new_unit_table[-1]);
		memory_patch(0x00425094, UnitIDMask);
		memory_patch(0x004250A0, &new_unit_table[-1].sprite);
		memory_patch(0x004250A8, &new_unit_table[-1]);
		memory_patch(0x004250E6, UnitIDMask);
		memory_patch(0x004250F2, &new_unit_table[-1].sprite);
		memory_patch(0x004250FA, &new_unit_table[-1]);

		// sub_42E0E0 (0x0042E110)
		memory_patch(0x0042E11C, &new_unit_finder_x[0].position);
		memory_patch(0x0042E12F, &new_unit_finder_x[0].position);
		memory_patch(0x0042E142, &new_unit_finder_y[0].position);
		memory_patch(0x0042E155, &new_unit_finder_y[0].position);
		memory_patch(0x0042FEAC, &new_get_all_units_in_bounds_list);

		// resetUnitBoundsLists_GetMaxUnitTypeSize (0x0042FEE0)
		memory_patch(0x0042FEE6, UnitLimit + 1);
		memory_patch(0x0042FEEB, &new_get_all_units_in_bounds_index);
		memory_patch(0x0042FEF2, UnitLimit + 1);
		memory_patch(0x0042FEF7, &new_get_all_units_in_bounds_list);

		// get_all_units_in_bounds (0x0042FF80)
		memory_patch(0x0042FF9F, &new_get_all_units_in_bounds_list);
		memory_patch(0x0042FFD7, &new_unit_finder_y);
		memory_patch(0x0042FFF5, &new_unit_finder_y[0].position);
		memory_patch(0x00430023, &new_unit_finder_x[0].position);
		memory_patch(0x00430057, &new_unit_finder_x[0].position);
		memory_patch(0x00430060, &new_unit_finder_x);
		memory_patch(0x0043006D, &new_get_all_units_in_bounds_index);
		memory_patch(0x00430078, &new_get_all_units_in_bounds_index);
		memory_patch(0x00430084, &new_unit_table[-1].finder_index.bottom);
		memory_patch(0x0043008C, &new_unit_table[-1]);
		memory_patch(0x004300BA, &new_get_all_units_in_bounds_list);

		// get_units_at_point (0x00430130)
		memory_patch(0x00430139, &new_unit_finder_x[0].position);
		memory_patch(0x00430148, &new_unit_finder_x[0].position);
		memory_patch(0x0043015B, &new_unit_finder_y[0].position);
		memory_patch(0x0043016A, &new_unit_finder_y[0].position);

		// UnitRelManyFinderUMScoutPath (0x00430190)
		memory_patch(0x004301AC, &new_get_all_units_in_bounds_list);
		memory_patch(0x004301CC, &new_get_all_units_in_bounds_list);
		memory_patch(0x004301F3, &new_unit_finder_x[0].position);
		memory_patch(0x0043020E, &new_unit_finder_y[0].position);
		memory_patch(0x00430221, &new_unit_finder_y[0].position);
		memory_patch(0x00430249, &new_unit_finder_x[0].position);
		memory_patch(0x00430252, &new_unit_finder_x);
		memory_patch(0x00430262, &new_unit_table[-1]);
		memory_patch(0x00430276, &new_unit_finder_y[0].position);
		memory_patch(0x00430289, &new_unit_finder_y[0].position);
		memory_patch(0x004302B3, &new_unit_finder_x[0].position);
		memory_patch(0x004302C4, &new_unit_finder_y[0].position);
		memory_patch(0x004302E5, &new_unit_finder_y[0].position);
		memory_patch(0x00430306, &new_unit_finder_x[0].position);
		memory_patch(0x0043030F, &new_unit_finder_x);
		memory_patch(0x0043031F, &new_unit_table[-1]);
		memory_patch(0x00430334, &new_unit_finder_y[0].position);
		memory_patch(0x00430347, &new_unit_finder_y[0].position);
		memory_patch(0x0043035F, &new_unit_finder_y[0].position);
		memory_patch(0x00430379, &new_unit_finder_x[0].position);
		memory_patch(0x0043038B, &new_unit_finder_x[0].position);
		memory_patch(0x004303AA, &new_unit_finder_y[0].position);
		memory_patch(0x004303B3, &new_unit_finder_y);
		memory_patch(0x004303C3, &new_unit_table[-1]);
		memory_patch(0x004303D8, &new_unit_finder_x[0].position);
		memory_patch(0x004303EB, &new_unit_finder_x[0].position);
		memory_patch(0x0043040A, &new_unit_finder_y[0].position);
		memory_patch(0x00430418, &new_unit_finder_x[0].position);
		memory_patch(0x00430434, &new_unit_finder_x[0].position);
		memory_patch(0x00430456, &new_unit_finder_y[0].position);
		memory_patch(0x0043045F, &new_unit_finder_y);
		memory_patch(0x0043046F, &new_unit_table[-1]);
		memory_patch(0x00430484, &new_unit_finder_x[0].position);
		memory_patch(0x00430497, &new_unit_finder_x[0].position);
		memory_patch(0x004304B0, &new_get_all_units_in_bounds_list);

		// unkUnitRelManyFinderEx (0x004304D0)
		memory_patch(0x00430506, &new_get_all_units_in_bounds_list);
		memory_patch(0x00430526, &new_unit_finder_x[0].position);
		memory_patch(0x00430541, &new_unit_finder_y[0].position);
		memory_patch(0x00430557, &new_unit_finder_y[0].position);
		memory_patch(0x00430569, &new_unit_finder_x[-1].position);
		memory_patch(0x00430596, &new_unit_finder_x[0].position);
		memory_patch(0x004305A3, &new_unit_finder_x);
		memory_patch(0x004305B3, &new_unit_table[-1]);
		memory_patch(0x004305C8, &new_unit_finder_y[0].position);
		memory_patch(0x004305DB, &new_unit_finder_y[0].position);
		memory_patch(0x00430611, &new_unit_finder_x[0].position);
		memory_patch(0x0043062C, &new_unit_finder_y[0].position);
		memory_patch(0x00430642, &new_unit_finder_y[0].position);
		memory_patch(0x00430654, &new_unit_finder_x[1].position);
		memory_patch(0x00430676, &new_unit_finder_x[0].position);
		memory_patch(0x0043067F, &new_unit_finder_x);
		memory_patch(0x0043068F, &new_unit_table[-1]);
		memory_patch(0x004306A4, &new_unit_finder_y[0].position);
		memory_patch(0x004306B7, &new_unit_finder_y[0].position);
		memory_patch(0x004306E6, &new_unit_finder_y[0].position);
		memory_patch(0x004306FD, &new_unit_finder_x[0].position);
		memory_patch(0x00430712, &new_unit_finder_x[0].position);
		memory_patch(0x00430724, &new_unit_finder_y[-1].position);
		memory_patch(0x00430756, &new_unit_finder_y[0].position);
		memory_patch(0x00430763, &new_unit_finder_y);
		memory_patch(0x00430773, &new_unit_table[-1]);
		memory_patch(0x00430788, &new_unit_finder_x[0].position);
		memory_patch(0x0043079B, &new_unit_finder_x[0].position);
		memory_patch(0x004307C6, &new_unit_finder_y[0].position);
		memory_patch(0x004307DD, &new_unit_finder_x[0].position);
		memory_patch(0x004307F2, &new_unit_finder_x[0].position);
		memory_patch(0x00430804, &new_unit_finder_y[1].position);
		memory_patch(0x00430826, &new_unit_finder_y[0].position);
		memory_patch(0x0043082F, &new_unit_finder_y);
		memory_patch(0x0043083F, &new_unit_table[-1]);
		memory_patch(0x00430854, &new_unit_finder_x[0].position);
		memory_patch(0x00430867, &new_unit_finder_x[0].position);
		memory_patch(0x0043087C, &new_get_all_units_in_bounds_list);

		// find_all_units (0x004308A0)
		memory_patch(0x004308BB, &new_get_all_units_in_bounds_list);
		memory_patch(0x0043093E, &new_unit_finder_x);
		memory_patch(0x00430962, &new_unit_finder_y);
		memory_patch(0x004309B8, &new_unit_finder_x);
		memory_patch(0x004309D0, &new_unit_table[-1]);
		memory_patch(0x004309FA, &new_get_all_units_in_bounds_index);
		memory_patch(0x00430A07, &new_get_all_units_in_bounds_index);
		memory_patch(0x00430A28, &new_unit_finder_y);
		memory_patch(0x00430A40, &new_unit_table[-1]);
		memory_patch(0x00430A67, &new_get_all_units_in_bounds_index);
		memory_patch(0x00430A71, &new_get_all_units_in_bounds_index);
		memory_patch(0x00430A93, &new_unit_finder_x);
		memory_patch(0x00430A9A, &new_get_all_units_in_bounds_index);
		memory_patch(0x00430AB3, &new_unit_table[-1]);
		memory_patch(0x00430AC8, &new_get_all_units_in_bounds_index);
		memory_patch(0x00430ADC, &new_get_all_units_in_bounds_list);

		// unpackTownMgr_02_Unit_Data (0x00432810)
		memory_patch(0x00432818, UnitIDMask);
		memory_patch(0x00432824, &new_unit_table[-1]);
		memory_patch(0x0043287B, &new_unit_finder_y[UnitLimit * 2 - 3]);

		// prepare_building_ai_for_saving (0x004328E0)
		memory_patch(0x004328F8, &new_unit_table);
		memory_patch(0x0043290F, UnitLimit);

		// 0x004329F0
		memory_patch(0x004329F8, UnitIDMask);
		memory_patch(0x00432A04, &new_unit_table[-1]);

		// 0x00432A30
		memory_patch(0x00432A40, &new_unit_table);
		memory_patch(0x00432A57, UnitLimit);

		// 0x00438200
		memory_patch(0x00438208, UnitIDMask);
		memory_patch(0x00438214, &new_unit_table[-1]);

		// packAIPathingData (0x00438240)
		memory_patch(0x00438250, &new_unit_table);
		memory_patch(0x00438267, UnitLimit);

		// GetDisabledUnitInRange (0x00440420)
		memory_patch(0x00440433, &new_unit_finder_x);
		memory_patch(0x00440447, &new_unit_finder_y);

		// check_arbiter_cloaking_range (0x004410C0)
		memory_patch(0x004410E1, &new_unit_finder_x);
		memory_patch(0x004410F4, &new_unit_finder_y);

		// sub_446350 (0x00446350)
		memory_patch(0x00446355, UnitLimit);
		memory_patch(0x004464CE, -UnitLimit);
		memory_patch(0x004464DC, -UnitLimit);
		memory_patch(0x0044659A, -UnitLimit);
		memory_patch(0x004465A8, -UnitLimit);

		// WriteAIControllerData (0x00447ED0)
		memory_patch(0x00447EF2, &new_unit_table);
		memory_patch(0x00447F09, UnitLimit);
		memory_patch(0x00447F37, &new_unit_table);
		memory_patch(0x00447F4E, UnitLimit);
		memory_patch(0x00447F7C, &new_unit_table);
		memory_patch(0x00447F93, UnitLimit);
		memory_patch(0x00447FC1, &new_unit_table);
		memory_patch(0x00447FD8, UnitLimit);
		memory_patch(0x00448027, UnitIDMask);
		memory_patch(0x00448033, &new_unit_table[-1]);
		memory_patch(0x0044804D, UnitIDMask);
		memory_patch(0x00448059, &new_unit_table[-1]);
		memory_patch(0x00448073, UnitIDMask);
		memory_patch(0x0044807F, &new_unit_table[-1]);
		memory_patch(0x00448099, UnitIDMask);
		memory_patch(0x004480A5, &new_unit_table[-1]);
		memory_patch(0x004480BF, UnitIDMask);
		memory_patch(0x004480CB, &new_unit_table[-1]);
		memory_patch(0x004480E5, UnitIDMask);
		memory_patch(0x004480F1, &new_unit_table[-1]);
		memory_patch(0x0044810B, UnitIDMask);
		memory_patch(0x00448117, &new_unit_table[-1]);
		memory_patch(0x00448131, UnitIDMask);
		memory_patch(0x0044813D, &new_unit_table[-1]);

		// ReadAIScriptControllerData (0x00448230)
		memory_patch(0x0044826C, UnitIDMask);
		memory_patch(0x00448278, &new_unit_table[-1]);
		memory_patch(0x0044828F, UnitIDMask);
		memory_patch(0x0044829B, &new_unit_table[-1]);
		memory_patch(0x004482B2, UnitIDMask);
		memory_patch(0x004482BE, &new_unit_table[-1]);
		memory_patch(0x004482D5, UnitIDMask);
		memory_patch(0x004482E1, &new_unit_table[-1]);
		memory_patch(0x004482F8, UnitIDMask);
		memory_patch(0x00448304, &new_unit_table[-1]);
		memory_patch(0x0044831B, UnitIDMask);
		memory_patch(0x00448327, &new_unit_table[-1]);
		memory_patch(0x0044833E, UnitIDMask);
		memory_patch(0x0044834A, &new_unit_table[-1]);
		memory_patch(0x00448361, UnitIDMask);
		memory_patch(0x0044836D, &new_unit_table[-1]);

		// status_screen_button (0x00458220)
		memory_patch(0x004582BF, &new_unit_table);
		memory_patch(0x004582D6, UnitLimit);

		// sub_45F870 (0x0045F870)
		memory_patch(0x0045F88B, UnitIDMask);
		memory_patch(0x0045F897, &new_unit_table[-1].sprite);
		memory_patch(0x0045F89F, &new_unit_table[-1]);

		// 0x004627C0
		memory_patch(0x004627CA, &new_unit_finder_y[UnitLimit * 2 - 3]);

		// LoadUnitFinderData (0x00462AC0)
		memory_patch(0x00462B09, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462B26, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462B43, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462B60, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462B7D, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462B9A, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462BB7, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462BE0, &new_unit_finder_y[UnitLimit * 2 - 3]);

		// WriteUnitFinderData (0x00462BF0)
		memory_patch(0x00462D61, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462D82, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462DA3, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462DC4, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462DE5, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462E06, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462E27, &new_unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462E53, &new_unit_finder_y[UnitLimit * 2 - 3]);

		// isUnitInBoundsOfOtherUnit (0x00469B60)
		memory_patch(0x00469B70, &new_unit_finder_x[0].position);
		memory_patch(0x00469B77, &new_unit_finder_x[0].position);
		memory_patch(0x00469B90, &new_unit_finder_x[0].position);
		memory_patch(0x00469B97, &new_unit_finder_x[0].position);
		memory_patch(0x00469BAC, &new_unit_finder_y[0].position);
		memory_patch(0x00469BB3, &new_unit_finder_y[0].position);
		memory_patch(0x00469BC8, &new_unit_finder_y[0].position);
		memory_patch(0x00469BCF, &new_unit_finder_y[0].position);

		// 0x00469BF0;
		memory_patch(0x00469BF5, UnitLimit * 4);
		memory_patch(0x00469BFA, &new_unit_finder_x);
		memory_patch(0x00469C01, UnitLimit * 4);
		memory_patch(0x00469C09, &new_unit_finder_y);

		// assignUnitSorting_Y (0x00469C20)
		memory_patch(0x00469C34, &new_unit_table[-1]);

		// assignUnitSorting_X (0x00469C90)
		memory_patch(0x00469CA4, &new_unit_table[-1]);

		// SortTopBottom (0x00469D00)
		memory_patch(0x00469D1F, &new_unit_table[-1]);

		// sortLeftRight (0x00469D70)
		memory_patch(0x00469D8F, &new_unit_table[-1]);

		// setUnitSorting_Y (0x00469DE0)
		memory_patch(0x00469DFF, &new_unit_table[-1]);

		// setUnitSorting_X (0x00469E50)
		memory_patch(0x00469E6F, &new_unit_table[-1]);

		// updateUnitSortingInfo (0x0046A000)
		memory_patch(0x0046A028, &new_unit_finder_x[0].position);
		memory_patch(0x0046A031, &new_unit_finder_x[0].position);
		memory_patch(0x0046A03E, &new_unit_finder_x[0].position);
		memory_patch(0x0046A047, &new_unit_finder_x[0].position);
		memory_patch(0x0046A054, &new_unit_finder_y[0].position);
		memory_patch(0x0046A05E, &new_unit_finder_y[0].position);
		memory_patch(0x0046A06E, &new_unit_finder_y[0].position);
		memory_patch(0x0046A07D, &new_unit_finder_y[0].position);
		memory_patch(0x0046A0A9, &new_unit_finder_x[1].position);
		memory_patch(0x0046A0B4, &new_unit_finder_x);
		memory_patch(0x0046A0F9, &new_unit_finder_x[1].position);
		memory_patch(0x0046A108, &new_unit_finder_x);
		memory_patch(0x0046A149, &new_unit_finder_x[-1].position);
		memory_patch(0x0046A156, &new_unit_finder_x);
		memory_patch(0x0046A189, &new_unit_finder_x[-1].position);
		memory_patch(0x0046A196, &new_unit_finder_x);
		memory_patch(0x0046A1DC, &new_unit_finder_y[1].position);
		memory_patch(0x0046A1E7, &new_unit_finder_y);
		memory_patch(0x0046A22C, &new_unit_finder_y[1].position);
		memory_patch(0x0046A23B, &new_unit_finder_y);
		memory_patch(0x0046A28C, &new_unit_finder_y[-1].position);
		memory_patch(0x0046A299, &new_unit_finder_y);
		memory_patch(0x0046A2CC, &new_unit_finder_y[-1].position);
		memory_patch(0x0046A2D9, &new_unit_finder_y);

		// sortUnit (0x0046A300)
		memory_patch(0x0046A31D, &new_unit_finder_x);
		memory_patch(0x0046A337, &new_unit_finder_y);
		memory_patch(0x0046A35C, &new_unit_finder_x);
		memory_patch(0x0046A376, &new_unit_finder_y);

		// updateUnitOrderFinder (0x0046A3A0)
		memory_patch(0x0046A3B3, &new_unit_table);
		memory_patch(0x0046A407, &new_unit_finder_x);
		memory_patch(0x0046A41E, &new_unit_finder_x);
		memory_patch(0x0046A436, &new_unit_finder_x);
		memory_patch(0x0046A446, &new_unit_finder_x[0].position);
		memory_patch(0x0046A451, &new_unit_finder_y);
		memory_patch(0x0046A462, &new_unit_finder_y);
		memory_patch(0x0046A480, &new_unit_finder_y);
		memory_patch(0x0046A48D, &new_unit_finder_y[0].position);
		memory_patch(0x0046A4BD, &new_unit_finder_x);
		memory_patch(0x0046A4D7, &new_unit_finder_x);
		memory_patch(0x0046A4EF, &new_unit_finder_x);
		memory_patch(0x0046A4FF, &new_unit_finder_x[0].position);
		memory_patch(0x0046A50A, &new_unit_finder_y);
		memory_patch(0x0046A51B, &new_unit_finder_y);
		memory_patch(0x0046A530, &new_unit_finder_y);
		memory_patch(0x0046A540, &new_unit_finder_y[0].position);

		// UMSlideFree (0x0046B000)
		memory_patch(0x0046B052, UnitIDMask);
		memory_patch(0x0046B05E, &new_unit_table[-1].sprite);

		// UMRepath (0x0046BC30)
		memory_patch(0x0046B066, &new_unit_table[-1]);
		memory_patch(0x0046BC47, UnitIDMask);
		memory_patch(0x0046BC53, &new_unit_table[-1].sprite);
		memory_patch(0x0046BC5B, &new_unit_table[-1]);

		// UMTurnAndStart (0x0046BCC0)
		memory_patch(0x0046BCD7, UnitIDMask);
		memory_patch(0x0046BCE3, &new_unit_table[-1].sprite);
		memory_patch(0x0046BCEB, &new_unit_table[-1]);

		// UMFixCollision (0x0046BF60)
		memory_patch(0x0046BFA4, UnitIDMask);
		memory_patch(0x0046BFB0, &new_unit_table[-1]);

		// is_tile_blocked_by (0x00473300)
		memory_patch(0x0047339A, &new_unit_finder_x[0].position);
		memory_patch(0x004733A9, &new_unit_finder_x[0].position);
		memory_patch(0x004733C1, &new_unit_finder_y[0].position);
		memory_patch(0x004733D0, &new_unit_finder_y[0].position);

		// does_building_block (0x00473410)
		memory_patch(0x0047343A, &new_unit_finder_x[0].position);
		memory_patch(0x00473451, &new_unit_finder_x[0].position);
		memory_patch(0x00473460, &new_unit_finder_y[0].position);
		memory_patch(0x00473477, &new_unit_finder_y[0].position);

		// minorUnpackSharedSaveData (0x00479E60)
		memory_patch(0x00479E7C, UnitIDMask);
		memory_patch(0x00479E88, &new_unit_table[-1]);
		memory_patch(0x00479E9B, UnitIDMask);
		memory_patch(0x00479EA7, &new_unit_table[-1]);

		// minorPackSharedSaveData (0x00479EC0)
		memory_patch(0x00479ED0, &new_unit_table);
		memory_patch(0x00479EE7, UnitLimit);
		memory_patch(0x00479F0F, &new_unit_table);
		memory_patch(0x00479F26, UnitLimit);

		// unit_to_unit_id (0x0047B1D0)
		memory_patch(0x0047B1D8, &new_unit_table);
		memory_patch(0x0047B1EF, UnitLimit);

		// unitIDtoUnitMemFast (0x0047B210)
		memory_patch(0x0047B215, UnitIDMask);
		memory_patch(0x0047B221, &new_unit_table[-1]);

		// unitIDtoUnit (0x0047B230)
		memory_patch(0x0047B237, UnitIDMask);
		memory_patch(0x0047B243, &new_unit_table[-1].sprite);
		memory_patch(0x0047B24B, &new_unit_table[-1]);

		// unpackBulletData (0x0048A8D0)
		memory_patch(0x0048A8E4, UnitIDMask);
		memory_patch(0x0048A8F0, &new_unit_table[-1].sprite);
		memory_patch(0x0048A8F8, &new_unit_table[-1]);
		memory_patch(0x0048A927, UnitIDMask);
		memory_patch(0x0048A933, &new_unit_table[-1].sprite);
		memory_patch(0x0048A93B, &new_unit_table[-1]);

		// packBulletData (0x0048A9A0)
		memory_patch(0x0048A9BB, &new_unit_table);
		memory_patch(0x0048A9D2, UnitLimit);
		memory_patch(0x0048A9FA, &new_unit_table);
		memory_patch(0x0048AA11, UnitLimit);

		// AirSplashProc (0x0048B150)
		memory_patch(0x0048B1B1, &new_air_splash_list);
		memory_patch(0x0048B1CA, &new_air_splash_list);

		// bullet_damage (0x0048B770)
		memory_patch(0x0048B896, &new_air_splash_list);

		// unpackOrderData (0x0048C770)
		memory_patch(0x0048C781, UnitIDMask);
		memory_patch(0x0048C78D, &new_unit_table[-1]);

		// packOrderData (0x0048C7D0)
		memory_patch(0x0048C7E9, &new_unit_table);
		memory_patch(0x0048C800, UnitLimit);

		// InitializeBurrowedUnitsLinkedListRef (0x00491570)
		memory_patch(0x00491594, UnitIDMask);
		memory_patch(0x004915A0, &new_unit_table[-1]);

		// writeBurrowedUnitIndex (0x004915C0)
		memory_patch(0x004915D6, &new_unit_table);
		memory_patch(0x004915ED, UnitLimit);
		memory_patch(0x00491633, UnitIDMask);
		memory_patch(0x0049163F, &new_unit_table[-1]);

		// 0x00494C60
		memory_patch(0x00494C90, UnitIDMask);
		memory_patch(0x00494C9C, &new_unit_table[-1]);

		// packFlingyData (0x00494CE0)
		memory_patch(0x00494CF3, &new_unit_table);
		memory_patch(0x00494D0A, UnitLimit);

		// packUnpackFlingyData (0x00495370)
		memory_patch(0x004953A5, UnitIDMask);
		memory_patch(0x004953B1, &new_unit_table[-1]);

		// WriteFlingys (0x00495B50)
		memory_patch(0x00495C44, UnitIDMask);
		memory_patch(0x00495C50, &new_unit_table[-1]);

		// command_save_hotkey_group (0x004965D0)
		memory_patch(0x0049660F, UnitIDMask);
		memory_patch(0x0049661B, &new_unit_table[-1].sprite);
		memory_patch(0x00496623, &new_unit_table[-1]);
		memory_patch(0x00496700, &new_unit_table);
		memory_patch(0x00496717, UnitLimit);

		// centerviewUnitGroup (0x004967E0)
		memory_patch(0x0049687E, UnitIDMask);
		memory_patch(0x0049688A, &new_unit_table[-1].sprite);
		memory_patch(0x00496892, &new_unit_table[-1]);

		// add_to_control_group (0x00496940)
		memory_patch(0x00496A02, UnitIDMask);
		memory_patch(0x00496A0E, &new_unit_table[-1].sprite);
		memory_patch(0x00496A16, &new_unit_table[-1]);

		// selectUnitGroup (0x00496B40)
		memory_patch(0x00496BA0, UnitIDMask);
		memory_patch(0x00496BAC, &new_unit_table[-1]);

		// 0x0049E1E0
		memory_patch(0x0049E1E1, &new_unit_table);

		// InitializeEmptyUnitsLinkedListRef (0x0049E1F0)
		memory_patch(0x0049E200, &new_unit_table);

		// 0x0049E260
		memory_patch(0x0049E24F, UnitLimit);
		memory_patch(0x0049E268, UnitLimit);

		// ReadUnitDataInfo (0x0049E6C0)
		memory_patch(0x0049E74C, &new_unit_table);

		// ReadUnitDataInfoEx (0x0049E790)
		memory_patch(0x0049E81C, &new_unit_table);

		// read_units_array (0x0049E910)
		memory_patch(0x0049E915, UnitLimit + 4);
		memory_patch(0x0049E91B, UnitLimit);
		memory_patch(0x0049E921, -(UnitLimit + 4));
		memory_patch(0x0049E951, sizeof(new_unit_table) / 4);
		memory_patch(0x0049E956, &new_unit_table);
		memory_patch(0x0049E995, UnitLimit);
		memory_patch(0x0049E99A, &new_unit_table[UnitLimit]);
		memory_patch(0x0049E9A3, -(UnitLimit + 5));
		memory_patch(0x0049E9BD, &new_unit_table);
		memory_patch(0x0049EB19, -(UnitLimit + 4));

		// resetOrdersUnitsDAT (0x0049F380)
		memory_patch(0x0049F3C3, sizeof(new_unit_table) / 4);
		memory_patch(0x0049F3C8, &new_unit_table);
		memory_patch(0x0049F41F, &new_unit_table);
		memory_patch(0x0049F424, UnitLimit);
		memory_patch(0x0049F46F, UnitLimit * 4);
		memory_patch(0x0049F474, &new_unit_finder_x);
		memory_patch(0x0049F47E, UnitLimit * 4);
		memory_patch(0x0049F483, &new_unit_finder_y);

		// kill_all_loaded_units (0x0049FDD0)
		memory_patch(0x0049FE4D, UnitIDMask);
		memory_patch(0x0049FE59, &new_unit_table[-1]);

		// unit_constructor (0x004A06C0)
		memory_patch(0x004A06C9, UnitLimit);

		// CMDRECV_ShiftDeselect (0x004BFB40)
		memory_patch(0x004BFB7E, UnitIDMask);
		memory_patch(0x004BFB8A, &new_unit_table[-1]);

		// CMDACT_Unload (0x004BFDB0)
		memory_patch(0x004BFDC0, &new_unit_table);
		memory_patch(0x004BFDD7, UnitLimit);

		// CMDACT_TargetOrder (0x004C0300)
		memory_patch(0x004C0325, &new_unit_table);
		memory_patch(0x004C033C, UnitLimit);

		// cmdact_right_click_order (0x004C0380)
		memory_patch(0x004C03A2, &new_unit_table);
		memory_patch(0x004C03B9, UnitLimit);

		// cmdact_select (0x004C0860)
		memory_patch(0x004C0929, &new_unit_table);
		memory_patch(0x004C0940, UnitLimit);
		memory_patch(0x004C09B6, &new_unit_table);
		memory_patch(0x004C09CD, UnitLimit);
		memory_patch(0x004C0A35, &new_unit_table);
		memory_patch(0x004C0A4C, UnitLimit);

		// CMDRECV_Unload (0x004C24F0)
		memory_patch(0x004C24FC, UnitIDMask);
		memory_patch(0x004C2508, &new_unit_table[-1]);

		// cmdrecv_shift_select (0x004C2560)
		memory_patch(0x004C2613, UnitIDMask);
		memory_patch(0x004C261F, &new_unit_table[-1].sprite);
		memory_patch(0x004C2627, &new_unit_table[-1]);

		// cmdrecv_select (0x004C2750)
		memory_patch(0x004C279E, UnitIDMask);
		memory_patch(0x004C27AA, &new_unit_table[-1].sprite);
		memory_patch(0x004C27B2, &new_unit_table[-1]);

		// kill_units_at_location_proc (0x004C7E20)
		memory_patch(0x004C7ED3, UnitIDMask);
		memory_patch(0x004C7EDF, &new_unit_table[-1].sprite);
		memory_patch(0x004C7EE7, &new_unit_table[-1]);

		// unpack_all_player_selection_groups (0x004CEDA0)
		memory_patch(0x004CEDB9, UnitIDMask);
		memory_patch(0x004CEDC5, &new_unit_table[-1].sprite);
		memory_patch(0x004CEDCD, &new_unit_table[-1]);

		// pack_all_player_selection_groups (0x004CEE00)
		memory_patch(0x004CEE1E, &new_unit_table);
		memory_patch(0x004CEE35, UnitLimit);

		// lose_sight_selection (0x004D0820)
		memory_patch(0x004D0893, &new_unit_table);
		memory_patch(0x004D08AA, UnitLimit);

		// unpackImageGrpData (0x004D5600)
		memory_patch(0x004D565B, UnitIDMask);
		memory_patch(0x004D5667, &new_unit_table[-1].sprite);
		memory_patch(0x004D566F, &new_unit_table[-1]);

		// packImageData (0x004D6330)
		memory_patch(0x004D638E, &new_unit_table);
		memory_patch(0x004D63A5, UnitLimit);

		// sub_4E2CB0 (0x004E2CB0)
		memory_patch(0x004E2CD5, &new_unit_finder_y[UnitLimit * 2 - 3]);

		// unpack_unit_data (0x004E2E20)
		memory_patch(0x004E2E83, UnitIDMask);
		memory_patch(0x004E2E8F, &new_unit_table[-1]);
		memory_patch(0x004E2EA2, UnitIDMask);
		memory_patch(0x004E2EAE, &new_unit_table[-1]);
		memory_patch(0x004E2EC1, UnitIDMask);
		memory_patch(0x004E2ECD, &new_unit_table[-1]);
		memory_patch(0x004E2F08, UnitIDMask);
		memory_patch(0x004E2F14, &new_unit_table[-1]);
		memory_patch(0x004E2F2A, UnitIDMask);
		memory_patch(0x004E2F36, &new_unit_table[-1]);
		memory_patch(0x004E2F58, UnitIDMask);
		memory_patch(0x004E2F64, &new_unit_table[-1]);
		memory_patch(0x004E2FD8, UnitIDMask);
		memory_patch(0x004E2FE4, &new_unit_table[-1]);
		memory_patch(0x004E2FFD, UnitIDMask);
		memory_patch(0x004E3009, &new_unit_table[-1]);
		memory_patch(0x004E303A, UnitIDMask);
		memory_patch(0x004E3046, &new_unit_table[-1]);
		memory_patch(0x004E305F, UnitIDMask);
		memory_patch(0x004E306B, &new_unit_table[-1]);
		memory_patch(0x004E3090, UnitIDMask);
		memory_patch(0x004E309C, &new_unit_table[-1]);
		memory_patch(0x004E30B5, UnitIDMask);
		memory_patch(0x004E30C1, &new_unit_table[-1]);
		memory_patch(0x004E30DA, UnitIDMask);
		memory_patch(0x004E30E6, &new_unit_table[-1]);
		memory_patch(0x004E3105, UnitIDMask);
		memory_patch(0x004E3111, &new_unit_table[-1]);
		memory_patch(0x004E3131, UnitIDMask);
		memory_patch(0x004E313D, &new_unit_table[-1]);
		memory_patch(0x004E31BA, UnitIDMask);
		memory_patch(0x004E31C6, &new_unit_table[-1]);
		memory_patch(0x004E31EB, UnitIDMask);
		memory_patch(0x004E31F7, &new_unit_table[-1]);
		memory_patch(0x004E3219, UnitIDMask);
		memory_patch(0x004E3225, &new_unit_table[-1]);
		memory_patch(0x004E323E, UnitIDMask);
		memory_patch(0x004E324A, &new_unit_table[-1]);
		memory_patch(0x004E3263, UnitIDMask);
		memory_patch(0x004E326F, &new_unit_table[-1]);
		memory_patch(0x004E3288, UnitIDMask);
		memory_patch(0x004E3294, &new_unit_table[-1]);
		memory_patch(0x004E32B8, UnitIDMask);
		memory_patch(0x004E32C4, &new_unit_table[-1]);
		memory_patch(0x004E32DD, UnitIDMask);
		memory_patch(0x004E32E9, &new_unit_table[-1]);
		memory_patch(0x004E3309, UnitIDMask);
		memory_patch(0x004E3315, &new_unit_table[-1]);
		memory_patch(0x004E332E, UnitIDMask);
		memory_patch(0x004E333A, &new_unit_table[-1]);
		memory_patch(0x004E335E, UnitIDMask);
		memory_patch(0x004E336A, &new_unit_table[-1]);
		memory_patch(0x004E339B, UnitIDMask);
		memory_patch(0x004E33A7, &new_unit_table[-1]);
		memory_patch(0x004E33CE, UnitIDMask);
		memory_patch(0x004E33DA, &new_unit_table[-1]);
		memory_patch(0x004E33EC, UnitIDMask);
		memory_patch(0x004E33F8, &new_unit_table[-1]);

		// pack_unit_data (0x004E3410)
		memory_patch(0x004E3470, &new_unit_table);
		memory_patch(0x004E3487, UnitLimit);
		memory_patch(0x004E34AF, &new_unit_table);
		memory_patch(0x004E34C6, UnitLimit);
		memory_patch(0x004E34EE, &new_unit_table);
		memory_patch(0x004E3505, UnitLimit);
		memory_patch(0x004E3575, &new_unit_table);
		memory_patch(0x004E358C, UnitLimit);
		memory_patch(0x004E35B7, &new_unit_table);
		memory_patch(0x004E35CE, UnitLimit);
		memory_patch(0x004E3605, &new_unit_table);
		memory_patch(0x004E361C, UnitLimit);
		memory_patch(0x004E365C, &new_unit_table);
		memory_patch(0x004E3673, UnitLimit);
		memory_patch(0x004E36A1, &new_unit_table);
		memory_patch(0x004E36B8, UnitLimit);
		memory_patch(0x004E3706, &new_unit_table);
		memory_patch(0x004E371D, UnitLimit);
		memory_patch(0x004E374B, &new_unit_table);
		memory_patch(0x004E3762, UnitLimit);
		memory_patch(0x004E37A4, &new_unit_table);
		memory_patch(0x004E37BB, UnitLimit);
		memory_patch(0x004E37E9, &new_unit_table);
		memory_patch(0x004E3800, UnitLimit);
		memory_patch(0x004E382E, &new_unit_table);
		memory_patch(0x004E3845, UnitLimit);
		memory_patch(0x004E387A, &new_unit_table);
		memory_patch(0x004E3891, UnitLimit);
		memory_patch(0x004E38C7, &new_unit_table);
		memory_patch(0x004E38DE, UnitLimit);
		memory_patch(0x004E3983, &new_unit_table);
		memory_patch(0x004E399A, UnitLimit);
		memory_patch(0x004E39D8, &new_unit_table);
		memory_patch(0x004E39EF, UnitLimit);
		memory_patch(0x004E3A2F, &new_unit_table);
		memory_patch(0x004E3A46, UnitLimit);
		memory_patch(0x004E3A74, &new_unit_table);
		memory_patch(0x004E3A8B, UnitLimit);
		memory_patch(0x004E3AB9, &new_unit_table);
		memory_patch(0x004E3AD0, UnitLimit);
		memory_patch(0x004E3AFE, &new_unit_table);
		memory_patch(0x004E3B15, UnitLimit);
		memory_patch(0x004E3B55, &new_unit_table);
		memory_patch(0x004E3B6C, UnitLimit);
		memory_patch(0x004E3B9A, &new_unit_table);
		memory_patch(0x004E3BB1, UnitLimit);
		memory_patch(0x004E3BEB, &new_unit_table);
		memory_patch(0x004E3C02, UnitLimit);
		memory_patch(0x004E3C30, &new_unit_table);
		memory_patch(0x004E3C47, UnitLimit);
		memory_patch(0x004E3C80, &new_unit_table);
		memory_patch(0x004E3C97, UnitLimit);
		memory_patch(0x004E3CDB, &new_unit_table);
		memory_patch(0x004E3CF2, UnitLimit);
		memory_patch(0x004E3D32, &new_unit_table);
		memory_patch(0x004E3D49, UnitLimit);
		memory_patch(0x004E3D6C, &new_unit_table);
		memory_patch(0x004E3D83, UnitLimit);

		// get_loaded_unit_from_index (0x004E6C40)
		memory_patch(0x004E6C50, UnitIDMask);
		memory_patch(0x004E6C5C, &new_unit_table[-1]);

		// get_first_loaded_unit (0x004E6C90)
		memory_patch(0x004E6CBA, UnitIDMask);
		memory_patch(0x004E6CC6, &new_unit_table[-1].sprite);
		memory_patch(0x004E6CCE, &new_unit_table[-1]);

		// iterate_loaded_units (0x004E6D00)
		memory_patch(0x004E6D2B, UnitIDMask);
		memory_patch(0x004E6D37, &new_unit_table[-1]);

		// get_first_loaded_worker (0x004E6D90)
		memory_patch(0x004E6DAA, UnitIDMask);
		memory_patch(0x004E6DB6, &new_unit_table[-1]);

		// is_carrying_flag (0x004E7080)
		memory_patch(0x004E709A, UnitIDMask);
		memory_patch(0x004E70A6, &new_unit_table[-1]);

		// is_unit_loaded (0x004E7110)
		memory_patch(0x004E712D, UnitIDMask);
		memory_patch(0x004E7139, &new_unit_table[-1].sprite);
		memory_patch(0x004E7141, &new_unit_table[-1]);

		// get_loaded_space_amount (0x004E7170)
		memory_patch(0x004E719C, UnitIDMask);
		memory_patch(0x004E71A8, &new_unit_table[-1].sprite);
		memory_patch(0x004E71B0, &new_unit_table[-1]);
		memory_patch(0x004E71E7, UnitIDMask);
		memory_patch(0x004E71F3, &new_unit_table[-1].sprite);
		memory_patch(0x004E71FB, &new_unit_table[-1]);
		memory_patch(0x004E7233, UnitIDMask);
		memory_patch(0x004E723F, &new_unit_table[-1].sprite);
		memory_patch(0x004E7247, &new_unit_table[-1]);
		memory_patch(0x004E727F, UnitIDMask);
		memory_patch(0x004E728B, &new_unit_table[-1].sprite);
		memory_patch(0x004E7293, &new_unit_table[-1]);

		// load_unit_proc (0x004E78E0)
		memory_patch(0x004E7911, UnitIDMask);
		memory_patch(0x004E791D, &new_unit_table[-1].sprite);
		memory_patch(0x004E7924, &new_unit_table[-1]);
		memory_patch(0x004E79A7, &new_unit_table);
		memory_patch(0x004E79BE, UnitLimit);

		// unload_unit_ext (0x004E7E10)
		memory_patch(0x004E7E66, &new_unit_table);
		memory_patch(0x004E7E7D, UnitLimit);

		// orders_Unload (0x004E80D0)
		memory_patch(0x004E8112, UnitIDMask);
		memory_patch(0x004E811E, &new_unit_table[-1].sprite);
		memory_patch(0x004E8126, &new_unit_table[-1]);

		// UnitDistanceMapping (0x004E8320)
		memory_patch(0x004E8395, &new_unit_finder_x);
		memory_patch(0x004E83A5, &new_unit_table[-1]);
		memory_patch(0x004E842E, &new_unit_finder_x);
		memory_patch(0x004E8442, &new_unit_table[-1]);
		memory_patch(0x004E84C7, &new_unit_finder_y);
		memory_patch(0x004E84DB, &new_unit_table[-1]);
		memory_patch(0x004E8564, &new_unit_finder_y);
		memory_patch(0x004E8578, &new_unit_table[-1]);

		// 0x004E865B
		memory_patch(0x004E8670, &new_unit_finder_x);
		memory_patch(0x004E8683, &new_unit_finder_y);

		// GetNearestUnitFrom (0x004E86C0)
		memory_patch(0x004E86EB, &new_unit_finder_x);
		memory_patch(0x004E8702, &new_unit_finder_y);

		// ai_best_unit (0x004E8740)
		memory_patch(0x004E877D, &new_unit_finder_x);
		memory_patch(0x004E8796, &new_unit_finder_y);

		// 0x004EA940
		memory_patch(0x004EA944, &new_unit_table);

		// writeUnitPointer (0x004EA970)
		memory_patch(0x004EA97C, &new_unit_table);
		memory_patch(0x004EA993, UnitLimit);

		// sub_4EA9D0 (0x004EA9D0)
		memory_patch(0x004EAA2D, &new_unit_table);
		memory_patch(0x004EAA4C, &new_unit_table);

		// 0x004EAA80
		memory_patch(0x004EAA9C, UnitLimit);
		memory_patch(0x004EAAA1, &new_unit_table[UnitLimit]);
		memory_patch(0x004EAAD4, &new_unit_table);

		// WriteUnits (0x004EAAF0)
		memory_patch(0x004EAAF5, UnitLimit + 0x10);
		memory_patch(0x004EAAFD, UnitLimit);
		memory_patch(0x004EAB03, -(UnitLimit + 0x10));
		memory_patch(0x004EAB0F, UnitLimit);
		memory_patch(0x004EAB14, &new_unit_table[UnitLimit]);
		memory_patch(0x004EAB23, -(UnitLimit + 0x11));
		memory_patch(0x004EAB3E, &new_unit_table);
		memory_patch(0x004EAB49, UnitLimit);
		memory_patch(0x004EAB4F, -(UnitLimit + 0x10));
		memory_patch(0x004EABC7, &new_unit_table[0].id);
		memory_patch(0x004EABCE, UnitLimit);
		memory_patch(0x004EABD5, -(UnitLimit + 0x10));
		memory_patch(0x004EAC31, UnitLimit);
		memory_patch(0x004EAC36, &new_unit_table[UnitLimit]);
		memory_patch(0x004EAC43, -(UnitLimit + 0x11));
		memory_patch(0x004EAC5D, &new_unit_table);

		// initialize_global_unit_count (0x004EAD50)
		memory_patch(0x004EAD58, UnitLimit);

		// set_under_disruption_field (0x004EB170)
		memory_patch(0x004EB1DD, UnitIDMask);
		memory_patch(0x004EB1E9, &new_unit_table[-1].sprite);
		memory_patch(0x004EB1F1, &new_unit_table[-1]);

		// update_disruption_web_status (0x004EB2F0)
		memory_patch(0x004EB37E, UnitIDMask);
		memory_patch(0x004EB38A, &new_unit_table[-1].sprite);
		memory_patch(0x004EB392, &new_unit_table[-1]);

		// sub_4F2A70 (0x004F2A70)
		memory_patch(0x004F2AE8, &new_unit_finder_x[0].position);
		memory_patch(0x004F2B07, &new_unit_finder_x[0].position);
		memory_patch(0x004F2B26, &new_unit_finder_y[0].position);
		memory_patch(0x004F2B3B, &new_unit_finder_y[0].position);
		memory_patch(0x004F9B12, UnitIDMask);
		memory_patch(0x004FB077, UnitIDMask);
	}
}

// Adds 1 to the sprite's vertical offset if it's a subunit, to ensure that it stays on top of the base unit.
// Previously it was added to the sprite's index, but that's not possible with more than 8191 entries.
// Will be buggy in maps larger than 256x256, if those will ever exist.
const u32 sub_42D4C0_2D4F7 = 0x0042D4F7;
void __declspec(naked) sub_42D4C0_patch_2D4F2() {
	__asm {
		AND		EDX, 0x10
		SHL		EDX, 1
		ADD		EAX, EDX
		JMP		sub_42D4C0_2D4F7
	}
};

namespace hooks {
	void inject_sprite_extender() {
		// 0x0042D330
		memory_patch(0x0042D343, &new_sprite_list_pointers);

		// get_first_sprite (0x0042D460)
		memory_patch(0x0042D47B, &new_sprite_list_pointers);
		memory_patch(0x0042D4B7, &new_sprite_list_pointers);

		// 0x0042D4C0
		jump_patch(sub_42D4C0_patch_2D4F2, 0x0042D4F2);// Needed for sprite entries above 8191.

		// drawAllSprites (0x00498D40)
		memory_patch(0x00498D67, &new_sprite_list_pointers);

		// minorUnpackSharedSaveData (0x00479E60)
		memory_patch(0x00479E6D, &new_sprite_table[-1]);

		// minorPackSharedSaveData (0x00479EC0)
		memory_patch(0x00479F4F, &new_sprite_table);

		// 0x00487350
		memory_patch(0x00487360, &new_sprite_table[-1]);

		// 0x004873E0
		memory_patch(0x004873EC, &new_sprite_table);

		// packThingyData (0x00487540)
		memory_patch(0x00487559, &new_sprite_table);
		memory_patch(0x004875A9, &new_sprite_table);

		// unpack_thingy_data (0x004875F0)
		memory_patch(0x0048760D, &new_sprite_table[-1]);
		memory_patch(0x00487655, &new_sprite_table[-1]);

		// 0x00494C70
		memory_patch(0x00494C80, &new_sprite_table[-1]);

		// packFlingyData (0x00494CE0)
		memory_patch(0x00494D2F, &new_sprite_table);

		// packUnpackFlingyData (0x00495370)
		memory_patch(0x00495392, &new_sprite_table[-1]);

		// WriteFlingys (0x00495B50)
		memory_patch(0x00495C31, &new_sprite_table[-1]);

		// 0x00496EB0
		memory_patch(0x00496EB9, &new_sprite_table);

		// 0x00496FB0
		memory_patch(0x00496FBB, &new_sprite_table[-1]);

		// initializeSpriteArray (0x00497230)
		memory_patch(0x00497236, &new_sprite_table[0].images);
		memory_patch(0x00497240, &new_sprite_table[1].images);
		memory_patch(0x0049724A, &new_sprite_table[2].images);
		memory_patch(0x00497254, &new_sprite_table[3].images);
		memory_patch(0x0049725E, &new_sprite_table[4].images);
		memory_patch(0x00497271, sizeof(new_sprite_table));
		memory_patch(0x0049728A, SpriteLimit);
		memory_patch(0x004972AB, &new_sprite_table);
		memory_patch(0x004972B1, SpriteLimit);
		memory_patch(0x004972CA, SpriteLimit);
		memory_patch(0x004972D1, &new_sprite_table);

		// unpackSpriteData (0x00497310)
		memory_patch(0x00497361, &new_sprite_table[-1]);
		memory_patch(0x00497378, &new_sprite_table[-1]);

		// packSpriteData (0x00497380)
		memory_patch(0x004973CD, &new_sprite_table);
		memory_patch(0x004973ED, &new_sprite_table);

		// 0x00497D80
		memory_patch(0x00497D81, &new_sprite_table);
		memory_patch(0x00497D86, SpriteLimit);

		// 0x00497DA0
		memory_patch(0x00497DA2, &new_sprite_table);
		memory_patch(0x00497DA7, SpriteLimit);

		// read_sprites_array (0x00498570)
		memory_patch(0x0049859E, sizeof(new_sprite_table) / 4);
		memory_patch(0x004985A3, &new_sprite_table);
		memory_patch(0x00498631, &new_sprite_table);
		memory_patch(0x0049865B, &new_sprite_table);
		memory_patch(0x00498660, SpriteLimit);
		memory_patch(0x004986B2, &new_sprite_table[-1]);
		memory_patch(0x00498702, &new_sprite_table[-1]);
		memory_patch(0x00498718, &new_sprite_table[0].index);
		memory_patch(0x00498721, sizeof(new_sprite_table));

		// writeSprites (0x00498740)
		memory_patch(0x00498749, &new_sprite_table);
		memory_patch(0x0049874E, SpriteLimit);
		memory_patch(0x00498763, &new_sprite_table[1].images);
		memory_patch(0x00498793, &new_sprite_table[SpriteLimit + 1].images);
		memory_patch(0x004987FC, &new_sprite_table);
		memory_patch(0x00498839, (u16)SpriteLimit);
		memory_patch(0x00498868, &new_sprite_table);
		memory_patch(0x0049886D, SpriteLimit);
		memory_patch(0x004988A2, &new_sprite_table);
		memory_patch(0x004988F6, &new_sprite_table[-1]);
		memory_patch(0x00498923, &new_sprite_table);
		memory_patch(0x00498975, &new_sprite_table[-1]);

		// InitializeSpriteArray (0x00499900)
		memory_patch(0x00499915, &new_sprite_table);
		memory_patch(0x0049991A, sizeof(new_sprite_table) / 4);
		memory_patch(0x0049993C, &new_sprite_table[0].index);
		memory_patch(0x00499952, &new_sprite_table[1]);
		memory_patch(0x00499957, (SpriteLimit - 1) / 3);

		// unpackImageGrpData (0x004D5600)
		memory_patch(0x004D56B9, &new_sprite_table[-1]);

		// packImageData (0x004D6330)
		memory_patch(0x004D63DA, &new_sprite_table);

		// unpack_unit_data (0x004E2E20)
		memory_patch(0x004E3196, &new_sprite_table[-1]);

		// pack_unit_data (0x004E3410)
		memory_patch(0x004E394C, &new_sprite_table);

		// RemoveAllSelectionCircles (0x004989A0)
		memory_patch(0x004989A8, &new_sprite_table[0].flags);
	}
}

namespace hooks {
	void inject_order_extender() {
		// targeted_order (0x004754F0)
		memory_patch(0x00475576, NewCheckLimit);

		// 0x0048C4A0
		memory_patch(0x0048C4AB, &new_order_table[-1]);
		memory_patch(0x0048C4B6, &new_order_table);

		// 0x0048C720
		memory_patch(0x0048C726, NewCheckLimit);

		// InitializeOrderArray (0x0048C590)
		memory_patch(0x0048C595, sizeof(new_order_table) / 4);
		memory_patch(0x0048C59A, &new_order_table);
		memory_patch(0x0048C5AF, &new_order_table);
		memory_patch(0x0048C5B4, OrderLimit / 5);

		// 0x0048C740
		memory_patch(0x0048C759, OrderLimit);

		// unpackOrderData (0x0048C770)
		memory_patch(0x0048C771, &new_order_table[OrderLimit]);
		memory_patch(0x0048C7A4, &new_order_table[-1]);
		memory_patch(0x0048C7B7, &new_order_table[-1]);
		memory_patch(0x0048C7BC, &new_order_table);

		// packOrderData (0x0048C7D0)
		memory_patch(0x0048C7D3, &new_order_table[OrderLimit]);
		memory_patch(0x0048C820, &new_order_table);
		memory_patch(0x0048C843, &new_order_table);
		memory_patch(0x0048C85E, &new_order_table);

		// read_orders_array (0x0048C870)
		memory_patch(0x0048C872, sizeof(new_order_table));
		memory_patch(0x0048C877, &new_order_table);
		memory_patch(0x0048C8C8, &new_order_table[-1]);
		memory_patch(0x0048C8E1, &new_order_table[-1]);
		memory_patch(0x0048C8FA, OrderLimit);

		// WriteOrders (0x0048C910)
		memory_patch(0x0048C917, sizeof(new_order_table));
		memory_patch(0x0048C91C, &new_order_table);
		memory_patch(0x0048C93D, &new_order_table);
		memory_patch(0x0048C965, &new_order_table);
		memory_patch(0x0048C9C4, &new_order_table[-1]);
		memory_patch(0x0048C9E5, &new_order_table[-1]);

		// unpack_unit_data (0x004E2E20)
		memory_patch(0x004E2EE5, &new_order_table[-1]);
		memory_patch(0x004E2EF9, &new_order_table[-1]);

		// pack_unit_data (0x004E3410)
		memory_patch(0x004E3526, &new_order_table);
		memory_patch(0x004E354A, &new_order_table);
	}
}

namespace hooks {
	void inject_image_extender() {
		// sub_42D370 (0x0042D370)
		memory_patch(0x0042D377, &new_image_list_pointers[1]);
		memory_patch(0x0042D37E, &new_image_list_pointers);
		memory_patch(0x0042D3AA, &new_image_list_pointers[-2]);
		memory_patch(0x0042D3B1, &new_image_list_pointers);
		memory_patch(0x0042D3BB, &new_image_list_pointers[-2]);
		memory_patch(0x0042D3C8, &new_image_list_pointers[-2]);
		memory_patch(0x0042D3CF, &new_image_list_pointers[-1]);
		memory_patch(0x0042D3D6, &new_image_list_pointers[-1]);
		memory_patch(0x0042D3E6, &new_image_list_pointers[-2]);
		memory_patch(0x0042D3ED, &new_image_list_pointers[-1]);

		// sub_42D400 (0x0042D400)
		memory_patch(0x0042D409, &new_image_list_pointers[-2]);
		memory_patch(0x0042D418, &new_image_list_pointers[-1]);
		memory_patch(0x0042D423, &new_image_list_pointers[-2]);
		memory_patch(0x0042D42E, &new_image_list_pointers[-2]);
		memory_patch(0x0042D435, &new_image_list_pointers[-1]);
		memory_patch(0x0042D43C, &new_image_list_pointers[-1]);
		memory_patch(0x0042D44D, &new_image_list_pointers[-1]);
		memory_patch(0x0042D454, &new_image_list_pointers[-2]);

		// get_first_sprite (0x0042D460)
		memory_patch(0x0042D483, &new_image_list_pointers[-2]);
		memory_patch(0x0042D488, &new_image_list_pointers[1]);
		memory_patch(0x0042D495, &new_image_list_pointers[-1]);
		memory_patch(0x0042D48E, &new_image_list_pointers);
		memory_patch(0x0042D49E, &new_image_list_pointers[1]);

		// sub_42D4C0 (0x0042D4C0)
		memory_patch(0x0042D4CA, &new_image_list_pointers[-1]);
		memory_patch(0x0042D506, &new_image_list_pointers[-2]);

		// unpackSpriteData (0x00497310)
		memory_patch(0x0049731C, &new_image_table[-1]);
		memory_patch(0x00497333, &new_image_table[-1]);
		memory_patch(0x0049734A, &new_image_table[-1]);

		// packSpriteData (0x00497380)
		memory_patch(0x00497388, &new_image_table);
		memory_patch(0x0049739F, &new_image_table);
		memory_patch(0x004973B6, &new_image_table);

		// 0x004D4B60
		memory_patch(0x004D4B65, &new_image_table);

		// 0x004D4B80
		memory_patch(0x004D4B89, &new_image_table[-1]);

		// initializeImageArray (0x004D4BA0)
		memory_patch(0x004D4BA3, &new_image_table[1].grp_file);
		memory_patch(0x004D4BD9, &new_image_table[ImageLimit + 1].grp_file);
		memory_patch(0x004D4BF2, ImageLimit);
		memory_patch(0x004D4C14, &new_image_table);
		memory_patch(0x004D4C1A, ImageLimit);
		memory_patch(0x004D4C32, &new_image_table);
		memory_patch(0x004D4C67, &new_image_table[ImageLimit]);

		// unpackImageGrpData (0x004D5600)
		memory_patch(0x004D56CB, &new_image_table[-1]);
		memory_patch(0x004D56E1, &new_image_table[-1]);

		// 0x004D5DE0
		memory_patch(0x004D5DE2, &new_image_table);
		memory_patch(0x004D5DE7, ImageLimit);

		// read_images_array (0x004D6220)
		memory_patch(0x004D624D, sizeof(new_image_table) / 4);
		memory_patch(0x004D6252, &new_image_table);
		memory_patch(0x004D62E0, &new_image_table);
		memory_patch(0x004D6307, &new_image_table);
		memory_patch(0x004D630C, ImageLimit);

		// packImageData (0x004D6330)
		memory_patch(0x004D63F3, &new_image_table);
		memory_patch(0x004D6409, &new_image_table);

		// 0x004D64A0
		memory_patch(0x004D64A3, &new_image_table);
		memory_patch(0x004D64A8, ImageLimit);

		// writeImages (0x004D64C0)
		memory_patch(0x004D64C9, &new_image_table);
		memory_patch(0x004D64CE, ImageLimit);
		memory_patch(0x004D64E6, &new_image_table[0].grp_file);
		memory_patch(0x004D64F1, &new_image_table[1].grp_file);
		memory_patch(0x004D64FC, &new_image_table[2].grp_file);
		memory_patch(0x004D6507, &new_image_table[3].grp_file);
		memory_patch(0x004D6512, &new_image_table[4].grp_file);
		memory_patch(0x004D6521, sizeof(new_image_table));
		memory_patch(0x004D658A, &new_image_table);
		memory_patch(0x004D65C0, (u16)ImageLimit);
		memory_patch(0x004D65EC, &new_image_table);
		memory_patch(0x004D65F1, ImageLimit);

		// InitializeImageArray (0x004D6930)
		memory_patch(0x004D6944, sizeof(new_image_table) / 4);
		memory_patch(0x004D6949, &new_image_table);
		memory_patch(0x004D6CEB, &new_image_table);
		memory_patch(0x004D6CFA, &new_image_table[1]);
		memory_patch(0x004D6CFF, ImageLimit - 1);
		memory_patch(0x004D6D04, &new_image_table);
		memory_patch(0x004D6D19, &new_image_table[0].link.next);
		memory_patch(0x004D6D22, &new_image_table[0].link.next);
		memory_patch(0x004D6D2D, &new_image_table[0].link.next);
	}
}

namespace hooks {
	void inject_thingy_extender() {
		//0x00487310
		memory_patch(0x00487316, &new_fow_sprite_table);

		//0x00487320
		memory_patch(0x0048732A, &new_fow_sprite_table[-1]);

		//0x00487330
		memory_patch(0x00487336, &new_thingy_table);

		//0x00487340
		memory_patch(0x0048734A, &new_thingy_table[-1]);

		//0x00487350
		memory_patch(0x0048737D, &new_thingy_table[-1]);
		memory_patch(0x00487398, &new_thingy_table[-1]);
		memory_patch(0x004873A5, &new_fow_sprite_table);
		memory_patch(0x004873B6, &new_fow_sprite_table[-1]);
		memory_patch(0x004873C8, &new_fow_sprite_table[-1]);

		//0x004873E0
		memory_patch(0x00487410, &new_thingy_table);
		memory_patch(0x0048742B, &new_thingy_table);
		memory_patch(0x0048743C, &new_fow_sprite_table);
		memory_patch(0x00487449, &new_fow_sprite_table);
		memory_patch(0x0048745B, &new_fow_sprite_table);

		//packThingyData (0x00487540)
		memory_patch(0x00487542, &new_thingy_table[0].next);
		memory_patch(0x00487547, ThingyLimit);
		memory_patch(0x00487573, &new_thingy_table);
		memory_patch(0x00487585, &new_thingy_table);
		memory_patch(0x00487596, &new_fow_sprite_table[0].next);
		memory_patch(0x0048759B, FowSpriteLimit);
		memory_patch(0x004875C3, &new_fow_sprite_table);
		memory_patch(0x004875D5, &new_fow_sprite_table);

		//unpack_thingy_data (0x004875F0)
		memory_patch(0x004875F1, &new_thingy_table[0].next);
		memory_patch(0x004875F6, ThingyLimit);
		memory_patch(0x00487620, &new_thingy_table[-1]);
		memory_patch(0x00487632, &new_thingy_table[-1]);
		memory_patch(0x0048763F, &new_fow_sprite_table[0].next);
		memory_patch(0x00487644, FowSpriteLimit);
		memory_patch(0x00487668, &new_fow_sprite_table[-1]);
		memory_patch(0x0048767A, &new_fow_sprite_table[-1]);

		//initialize_thingy_arrays (0x00487990)
		memory_patch(0x00487996, &new_thingy_table);
		memory_patch(0x004879A5, ThingyLimit);
		memory_patch(0x004879B4, &new_thingy_table);
		memory_patch(0x004879C3, ThingyLimit);
		memory_patch(0x004879D2, &new_fow_sprite_table);
		memory_patch(0x004879E1, FowSpriteLimit);
		memory_patch(0x004879F0, &new_fow_sprite_table);
		memory_patch(0x004879FF, FowSpriteLimit);

		//read_thingy_arrays (0x00488100)
		memory_patch(0x00488101, ThingyLimit);
		memory_patch(0x00488106, &new_thingy_table);
		memory_patch(0x00488115, FowSpriteLimit);
		memory_patch(0x0048811A, &new_fow_sprite_table);

		//0x00488140
		memory_patch(0x00488147, ThingyLimit);
		memory_patch(0x0048814C, &new_thingy_table);
		memory_patch(0x00488157, FowSpriteLimit);
		memory_patch(0x0048815C, &new_fow_sprite_table);

		//InitializeThingyArray (0x00488550)
		memory_patch(0x00488556, &new_fow_sprite_table);
		memory_patch(0x0048855B, sizeof(new_fow_sprite_table) / 4);
		memory_patch(0x00488560, &new_thingy_table);
		memory_patch(0x00488567, sizeof(new_thingy_table) / 4);
		memory_patch(0x00488572, &new_thingy_table);
		memory_patch(0x0048857C, &new_thingy_table);
		memory_patch(0x004885AB, &new_thingy_table[1]);
		memory_patch(0x004885B0, ThingyLimit - 1);
		memory_patch(0x004885E9, &new_fow_sprite_table[1]);
		memory_patch(0x004885EE, FowSpriteLimit - 1);

		//cmdrecv_save_game (0x004C2910)
		memory_patch(0x004C2B78, ThingyLimit);
		memory_patch(0x004C2B7D, &new_thingy_table);
		memory_patch(0x004C2B88, FowSpriteLimit);
		memory_patch(0x004C2B8D, &new_fow_sprite_table);

		//0x004D02D0
		memory_patch(0x004D04E3, ThingyLimit);
		memory_patch(0x004D04E8, &new_thingy_table);
		memory_patch(0x004D04F3, FowSpriteLimit);
		memory_patch(0x004D04F8, &new_fow_sprite_table);

		//unpack_unit_data (0x004E2E20)
		memory_patch(0x004E3178, &new_thingy_table[-1]);

		//pack_unit_data (0x004E3410)
		memory_patch(0x004E3927, &new_thingy_table);
	}
}