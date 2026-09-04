#include "ccmu.h"
#include "hook_tools.h"
#include "BW/structures.h"
#include "BW/structures/Sprite.h"
#include "BW/structures/Order.h"
#include "BW/structures/Image.h"

CCMU_Extender ccmu_extender;

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
		memory_patch(0x00401EF0, &ccmu_extender.unit_table);
		memory_patch(0x00401F07, UnitLimit);

		// packDetailFinderData (0x00402D10)
		memory_patch(0x00402D62, &ccmu_extender.unit_table);
		memory_patch(0x00402D79, UnitLimit);
		memory_patch(0x00402DCC, &ccmu_extender.unit_table);
		memory_patch(0x00402DE3, UnitLimit);
		memory_patch(0x00402E35, &ccmu_extender.unit_table);
		memory_patch(0x00402E4C, UnitLimit);
		memory_patch(0x00402E9E, &ccmu_extender.unit_table);
		memory_patch(0x00402EB5, UnitLimit);
		memory_patch(0x00402F08, &ccmu_extender.unit_table);
		memory_patch(0x00402F1F, UnitLimit);

		// unpackDetailFinderData (0x00402FB0)
		memory_patch(0x00402FC8, UnitIDMask);
		memory_patch(0x00402FD4, &ccmu_extender.unit_table[-1]);

		// packAICaptainData (0x00403480)
		memory_patch(0x004034B3, &ccmu_extender.unit_table);
		memory_patch(0x004034CA, UnitLimit);
		memory_patch(0x004034F2, &ccmu_extender.unit_table);
		memory_patch(0x00403509, UnitLimit);
		memory_patch(0x00403531, &ccmu_extender.unit_table);
		memory_patch(0x00403548, UnitLimit);
		memory_patch(0x0040356C, &ccmu_extender.unit_table);
		memory_patch(0x00403583, UnitLimit);

		// unpackAICaptainData (0x004035B0)
		memory_patch(0x004035D4, UnitIDMask);
		memory_patch(0x004035E0, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004035F4, UnitIDMask);
		memory_patch(0x00403600, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00403614, UnitIDMask);
		memory_patch(0x00403620, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00403634, UnitIDMask);
		memory_patch(0x00403640, &ccmu_extender.unit_table[-1]);

		// unpackAllPathingData (0x00403780)
		memory_patch(0x004037A2, UnitIDMask);
		memory_patch(0x004037AE, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00403832, UnitIDMask);
		memory_patch(0x0040383E, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004038CF, UnitIDMask);
		memory_patch(0x004038DB, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00403970, UnitIDMask);
		memory_patch(0x0040397C, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00403A11, UnitIDMask);
		memory_patch(0x00403A1D, &ccmu_extender.unit_table[-1]);

		// packTownMgr_03_Unit_Data (0x00403AE0)
		memory_patch(0x00403B36, &ccmu_extender.unit_table);
		memory_patch(0x00403B4D, UnitLimit);
		memory_patch(0x00403B75, &ccmu_extender.unit_table);
		memory_patch(0x00403B8C, UnitLimit);
		memory_patch(0x00403BB4, &ccmu_extender.unit_table);
		memory_patch(0x00403BCB, UnitLimit);
		memory_patch(0x00403BF3, &ccmu_extender.unit_table);
		memory_patch(0x00403C0A, UnitLimit);
		memory_patch(0x00403C32, &ccmu_extender.unit_table);
		memory_patch(0x00403C49, UnitLimit);
		memory_patch(0x00403C6D, &ccmu_extender.unit_table);
		memory_patch(0x00403C84, UnitLimit);

		// unpackTownMgr_03_Unit_Data (0x00403CB0)
		memory_patch(0x00403CEE, UnitIDMask);
		memory_patch(0x00403CFA, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00403D0E, UnitIDMask);
		memory_patch(0x00403D1A, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00403D2E, UnitIDMask);
		memory_patch(0x00403D3A, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00403D4E, UnitIDMask);
		memory_patch(0x00403D5A, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00403D6E, UnitIDMask);
		memory_patch(0x00403D7A, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00403D8E, UnitIDMask);
		memory_patch(0x00403D9A, &ccmu_extender.unit_table[-1]);

		// packTownMgr_01_Data (0x00403E50)
		memory_patch(0x00403E9F, &ccmu_extender.unit_table);
		memory_patch(0x00403EB6, UnitLimit);
		memory_patch(0x00403F3B, &ccmu_extender.unit_table);
		memory_patch(0x00403F52, UnitLimit);
		memory_patch(0x00403FD5, &ccmu_extender.unit_table);
		memory_patch(0x00403FEC, UnitLimit);
		memory_patch(0x00404071, &ccmu_extender.unit_table);
		memory_patch(0x00404088, UnitLimit);
		memory_patch(0x0040410D, &ccmu_extender.unit_table);
		memory_patch(0x00404124, UnitLimit);

		// unpackTownMgr_01_Data (0x004041F0)
		memory_patch(0x00404209, UnitIDMask);
		memory_patch(0x00404215, &ccmu_extender.unit_table[-1]);

		// sub_422160 (0x00422160)
		memory_patch(0x00422195, &ccmu_extender.unit_finder_y);
		memory_patch(0x004221C6, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x004221D3, &ccmu_extender.unit_finder_y);
		memory_patch(0x004221E3, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004221F3, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00422209, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00422217, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004222B9, &ccmu_extender.unit_finder_x);
		memory_patch(0x004222E6, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004222F3, &ccmu_extender.unit_finder_x);
		memory_patch(0x00422303, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00422313, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00422329, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00422337, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x004223D9, &ccmu_extender.unit_finder_y);
		memory_patch(0x00422406, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00422413, &ccmu_extender.unit_finder_y);
		memory_patch(0x00422423, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00422433, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00422449, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00422457, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004224F8, &ccmu_extender.unit_finder_x);
		memory_patch(0x00422526, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00422533, &ccmu_extender.unit_finder_x);
		memory_patch(0x00422543, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00422553, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00422569, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00422577, &ccmu_extender.unit_finder_y[0].position);

		// set_spell_special_button_graphic (0x00424BA0)
		memory_patch(0x00424C34, UnitIDMask);
		memory_patch(0x00424C40, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x00424C48, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00424E65, UnitIDMask);
		memory_patch(0x00424E71, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x00424E79, &ccmu_extender.unit_table[-1]);

		// unitstatcond_transport (0x00424F10)
		memory_patch(0x00424F52, UnitIDMask);
		memory_patch(0x00424F5E, &ccmu_extender.unit_table[-1]);

		// transport_status_update_drawn_values (0x00424FC0)
		memory_patch(0x00424FF3, UnitIDMask);
		memory_patch(0x00424FFF, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x00425007, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00425043, UnitIDMask);
		memory_patch(0x0042504F, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x00425057, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00425094, UnitIDMask);
		memory_patch(0x004250A0, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004250A8, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004250E6, UnitIDMask);
		memory_patch(0x004250F2, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004250FA, &ccmu_extender.unit_table[-1]);

		// sub_42E0E0 (0x0042E110)
		memory_patch(0x0042E11C, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0042E12F, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0042E142, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x0042E155, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x0042FEAC, &ccmu_extender.get_all_units_in_bounds_list);

		// resetUnitBoundsLists_GetMaxUnitTypeSize (0x0042FEE0)
		memory_patch(0x0042FEE6, UnitLimit + 1);
		memory_patch(0x0042FEEB, &ccmu_extender.get_all_units_in_bounds_index);
		memory_patch(0x0042FEF2, UnitLimit + 1);
		memory_patch(0x0042FEF7, &ccmu_extender.get_all_units_in_bounds_list);

		// get_all_units_in_bounds (0x0042FF80)
		memory_patch(0x0042FF9F, &ccmu_extender.get_all_units_in_bounds_list);
		memory_patch(0x0042FFD7, &ccmu_extender.unit_finder_y);
		memory_patch(0x0042FFF5, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430023, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00430057, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00430060, &ccmu_extender.unit_finder_x);
		memory_patch(0x0043006D, &ccmu_extender.get_all_units_in_bounds_index);
		memory_patch(0x00430078, &ccmu_extender.get_all_units_in_bounds_index);
		memory_patch(0x00430084, &ccmu_extender.unit_table[-1].finder_index.bottom);
		memory_patch(0x0043008C, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004300BA, &ccmu_extender.get_all_units_in_bounds_list);

		// get_units_at_point (0x00430130)
		memory_patch(0x00430139, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00430148, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0043015B, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x0043016A, &ccmu_extender.unit_finder_y[0].position);

		// UnitRelManyFinderUMScoutPath (0x00430190)
		memory_patch(0x004301AC, &ccmu_extender.get_all_units_in_bounds_list);
		memory_patch(0x004301CC, &ccmu_extender.get_all_units_in_bounds_list);
		memory_patch(0x004301F3, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0043020E, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430221, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430249, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00430252, &ccmu_extender.unit_finder_x);
		memory_patch(0x00430262, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00430276, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430289, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x004302B3, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004302C4, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x004302E5, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430306, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0043030F, &ccmu_extender.unit_finder_x);
		memory_patch(0x0043031F, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00430334, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430347, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x0043035F, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430379, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0043038B, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004303AA, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x004303B3, &ccmu_extender.unit_finder_y);
		memory_patch(0x004303C3, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004303D8, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004303EB, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0043040A, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430418, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00430434, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00430456, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x0043045F, &ccmu_extender.unit_finder_y);
		memory_patch(0x0043046F, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00430484, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00430497, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004304B0, &ccmu_extender.get_all_units_in_bounds_list);

		// unkUnitRelManyFinderEx (0x004304D0)
		memory_patch(0x00430506, &ccmu_extender.get_all_units_in_bounds_list);
		memory_patch(0x00430526, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00430541, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430557, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430569, &ccmu_extender.unit_finder_x[-1].position);
		memory_patch(0x00430596, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004305A3, &ccmu_extender.unit_finder_x);
		memory_patch(0x004305B3, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004305C8, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x004305DB, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430611, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0043062C, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430642, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430654, &ccmu_extender.unit_finder_x[1].position);
		memory_patch(0x00430676, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0043067F, &ccmu_extender.unit_finder_x);
		memory_patch(0x0043068F, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004306A4, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x004306B7, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x004306E6, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x004306FD, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00430712, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00430724, &ccmu_extender.unit_finder_y[-1].position);
		memory_patch(0x00430756, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00430763, &ccmu_extender.unit_finder_y);
		memory_patch(0x00430773, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00430788, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0043079B, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004307C6, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x004307DD, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004307F2, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00430804, &ccmu_extender.unit_finder_y[1].position);
		memory_patch(0x00430826, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x0043082F, &ccmu_extender.unit_finder_y);
		memory_patch(0x0043083F, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00430854, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00430867, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0043087C, &ccmu_extender.get_all_units_in_bounds_list);

		// find_all_units (0x004308A0)
		memory_patch(0x004308BB, &ccmu_extender.get_all_units_in_bounds_list);
		memory_patch(0x0043093E, &ccmu_extender.unit_finder_x);
		memory_patch(0x00430962, &ccmu_extender.unit_finder_y);
		memory_patch(0x004309B8, &ccmu_extender.unit_finder_x);
		memory_patch(0x004309D0, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004309FA, &ccmu_extender.get_all_units_in_bounds_index);
		memory_patch(0x00430A07, &ccmu_extender.get_all_units_in_bounds_index);
		memory_patch(0x00430A28, &ccmu_extender.unit_finder_y);
		memory_patch(0x00430A40, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00430A67, &ccmu_extender.get_all_units_in_bounds_index);
		memory_patch(0x00430A71, &ccmu_extender.get_all_units_in_bounds_index);
		memory_patch(0x00430A93, &ccmu_extender.unit_finder_x);
		memory_patch(0x00430A9A, &ccmu_extender.get_all_units_in_bounds_index);
		memory_patch(0x00430AB3, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00430AC8, &ccmu_extender.get_all_units_in_bounds_index);
		memory_patch(0x00430ADC, &ccmu_extender.get_all_units_in_bounds_list);

		// unpackTownMgr_02_Unit_Data (0x00432810)
		memory_patch(0x00432818, UnitIDMask);
		memory_patch(0x00432824, &ccmu_extender.unit_table[-1]);
		memory_patch(0x0043287B, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);

		// prepare_building_ai_for_saving (0x004328E0)
		memory_patch(0x004328F8, &ccmu_extender.unit_table);
		memory_patch(0x0043290F, UnitLimit);

		// 0x004329F0
		memory_patch(0x004329F8, UnitIDMask);
		memory_patch(0x00432A04, &ccmu_extender.unit_table[-1]);

		// 0x00432A30
		memory_patch(0x00432A40, &ccmu_extender.unit_table);
		memory_patch(0x00432A57, UnitLimit);

		// 0x00438200
		memory_patch(0x00438208, UnitIDMask);
		memory_patch(0x00438214, &ccmu_extender.unit_table[-1]);

		// packAIPathingData (0x00438240)
		memory_patch(0x00438250, &ccmu_extender.unit_table);
		memory_patch(0x00438267, UnitLimit);

		// GetDisabledUnitInRange (0x00440420)
		memory_patch(0x00440433, &ccmu_extender.unit_finder_x);
		memory_patch(0x00440447, &ccmu_extender.unit_finder_y);

		// check_arbiter_cloaking_range (0x004410C0)
		memory_patch(0x004410E1, &ccmu_extender.unit_finder_x);
		memory_patch(0x004410F4, &ccmu_extender.unit_finder_y);

		//init_resareas (0x00446350)
		memory_patch(0x00446355, UnitLimit);
		memory_patch(0x004464C8, (UnitLimit + 1) / 4);
		memory_patch(0x004464CE, -(UnitLimit + 1));
		memory_patch(0x004464DC, -UnitLimit);
		memory_patch(0x0044659A, -UnitLimit);
		memory_patch(0x004465A8, -UnitLimit);

		// WriteAIControllerData (0x00447ED0)
		memory_patch(0x00447EF2, &ccmu_extender.unit_table);
		memory_patch(0x00447F09, UnitLimit);
		memory_patch(0x00447F37, &ccmu_extender.unit_table);
		memory_patch(0x00447F4E, UnitLimit);
		memory_patch(0x00447F7C, &ccmu_extender.unit_table);
		memory_patch(0x00447F93, UnitLimit);
		memory_patch(0x00447FC1, &ccmu_extender.unit_table);
		memory_patch(0x00447FD8, UnitLimit);
		memory_patch(0x00448027, UnitIDMask);
		memory_patch(0x00448033, &ccmu_extender.unit_table[-1]);
		memory_patch(0x0044804D, UnitIDMask);
		memory_patch(0x00448059, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00448073, UnitIDMask);
		memory_patch(0x0044807F, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00448099, UnitIDMask);
		memory_patch(0x004480A5, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004480BF, UnitIDMask);
		memory_patch(0x004480CB, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004480E5, UnitIDMask);
		memory_patch(0x004480F1, &ccmu_extender.unit_table[-1]);
		memory_patch(0x0044810B, UnitIDMask);
		memory_patch(0x00448117, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00448131, UnitIDMask);
		memory_patch(0x0044813D, &ccmu_extender.unit_table[-1]);

		// ReadAIScriptControllerData (0x00448230)
		memory_patch(0x0044826C, UnitIDMask);
		memory_patch(0x00448278, &ccmu_extender.unit_table[-1]);
		memory_patch(0x0044828F, UnitIDMask);
		memory_patch(0x0044829B, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004482B2, UnitIDMask);
		memory_patch(0x004482BE, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004482D5, UnitIDMask);
		memory_patch(0x004482E1, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004482F8, UnitIDMask);
		memory_patch(0x00448304, &ccmu_extender.unit_table[-1]);
		memory_patch(0x0044831B, UnitIDMask);
		memory_patch(0x00448327, &ccmu_extender.unit_table[-1]);
		memory_patch(0x0044833E, UnitIDMask);
		memory_patch(0x0044834A, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00448361, UnitIDMask);
		memory_patch(0x0044836D, &ccmu_extender.unit_table[-1]);

		// status_screen_button (0x00458220)
		memory_patch(0x004582BF, &ccmu_extender.unit_table);
		memory_patch(0x004582D6, UnitLimit);

		// sub_45F870 (0x0045F870)
		memory_patch(0x0045F88B, UnitIDMask);
		memory_patch(0x0045F897, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x0045F89F, &ccmu_extender.unit_table[-1]);

		// 0x004627C0
		memory_patch(0x004627CA, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);

		// LoadUnitFinderData (0x00462AC0)
		memory_patch(0x00462B09, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462B26, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462B43, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462B60, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462B7D, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462B9A, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462BB7, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462BE0, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);

		// WriteUnitFinderData (0x00462BF0)
		memory_patch(0x00462D61, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462D82, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462DA3, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462DC4, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462DE5, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462E06, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462E27, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);
		memory_patch(0x00462E53, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);

		// isUnitInBoundsOfOtherUnit (0x00469B60)
		memory_patch(0x00469B70, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00469B77, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00469B90, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00469B97, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00469BAC, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00469BB3, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00469BC8, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00469BCF, &ccmu_extender.unit_finder_y[0].position);

		// 0x00469BF0;
		memory_patch(0x00469BF5, UnitLimit * 4);
		memory_patch(0x00469BFA, &ccmu_extender.unit_finder_x);
		memory_patch(0x00469C01, UnitLimit * 4);
		memory_patch(0x00469C09, &ccmu_extender.unit_finder_y);

		// assignUnitSorting_Y (0x00469C20)
		memory_patch(0x00469C34, &ccmu_extender.unit_table[-1]);

		// assignUnitSorting_X (0x00469C90)
		memory_patch(0x00469CA4, &ccmu_extender.unit_table[-1]);

		// SortTopBottom (0x00469D00)
		memory_patch(0x00469D1F, &ccmu_extender.unit_table[-1]);

		// sortLeftRight (0x00469D70)
		memory_patch(0x00469D8F, &ccmu_extender.unit_table[-1]);

		// setUnitSorting_Y (0x00469DE0)
		memory_patch(0x00469DFF, &ccmu_extender.unit_table[-1]);

		// setUnitSorting_X (0x00469E50)
		memory_patch(0x00469E6F, &ccmu_extender.unit_table[-1]);

		// updateUnitSortingInfo (0x0046A000)
		memory_patch(0x0046A028, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0046A031, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0046A03E, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0046A047, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0046A054, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x0046A05E, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x0046A06E, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x0046A07D, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x0046A0A9, &ccmu_extender.unit_finder_x[1].position);
		memory_patch(0x0046A0B4, &ccmu_extender.unit_finder_x);
		memory_patch(0x0046A0F9, &ccmu_extender.unit_finder_x[1].position);
		memory_patch(0x0046A108, &ccmu_extender.unit_finder_x);
		memory_patch(0x0046A149, &ccmu_extender.unit_finder_x[-1].position);
		memory_patch(0x0046A156, &ccmu_extender.unit_finder_x);
		memory_patch(0x0046A189, &ccmu_extender.unit_finder_x[-1].position);
		memory_patch(0x0046A196, &ccmu_extender.unit_finder_x);
		memory_patch(0x0046A1DC, &ccmu_extender.unit_finder_y[1].position);
		memory_patch(0x0046A1E7, &ccmu_extender.unit_finder_y);
		memory_patch(0x0046A22C, &ccmu_extender.unit_finder_y[1].position);
		memory_patch(0x0046A23B, &ccmu_extender.unit_finder_y);
		memory_patch(0x0046A28C, &ccmu_extender.unit_finder_y[-1].position);
		memory_patch(0x0046A299, &ccmu_extender.unit_finder_y);
		memory_patch(0x0046A2CC, &ccmu_extender.unit_finder_y[-1].position);
		memory_patch(0x0046A2D9, &ccmu_extender.unit_finder_y);

		// sortUnit (0x0046A300)
		memory_patch(0x0046A31D, &ccmu_extender.unit_finder_x);
		memory_patch(0x0046A337, &ccmu_extender.unit_finder_y);
		memory_patch(0x0046A35C, &ccmu_extender.unit_finder_x);
		memory_patch(0x0046A376, &ccmu_extender.unit_finder_y);

		// updateUnitOrderFinder (0x0046A3A0)
		memory_patch(0x0046A3B3, &ccmu_extender.unit_table);
		memory_patch(0x0046A407, &ccmu_extender.unit_finder_x);
		memory_patch(0x0046A41E, &ccmu_extender.unit_finder_x);
		memory_patch(0x0046A436, &ccmu_extender.unit_finder_x);
		memory_patch(0x0046A446, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0046A451, &ccmu_extender.unit_finder_y);
		memory_patch(0x0046A462, &ccmu_extender.unit_finder_y);
		memory_patch(0x0046A480, &ccmu_extender.unit_finder_y);
		memory_patch(0x0046A48D, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x0046A4BD, &ccmu_extender.unit_finder_x);
		memory_patch(0x0046A4D7, &ccmu_extender.unit_finder_x);
		memory_patch(0x0046A4EF, &ccmu_extender.unit_finder_x);
		memory_patch(0x0046A4FF, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x0046A50A, &ccmu_extender.unit_finder_y);
		memory_patch(0x0046A51B, &ccmu_extender.unit_finder_y);
		memory_patch(0x0046A530, &ccmu_extender.unit_finder_y);
		memory_patch(0x0046A540, &ccmu_extender.unit_finder_y[0].position);

		// UMSlideFree (0x0046B000)
		memory_patch(0x0046B052, UnitIDMask);
		memory_patch(0x0046B05E, &ccmu_extender.unit_table[-1].sprite);

		// UMRepath (0x0046BC30)
		memory_patch(0x0046B066, &ccmu_extender.unit_table[-1]);
		memory_patch(0x0046BC47, UnitIDMask);
		memory_patch(0x0046BC53, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x0046BC5B, &ccmu_extender.unit_table[-1]);

		// UMTurnAndStart (0x0046BCC0)
		memory_patch(0x0046BCD7, UnitIDMask);
		memory_patch(0x0046BCE3, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x0046BCEB, &ccmu_extender.unit_table[-1]);

		// UMFixCollision (0x0046BF60)
		memory_patch(0x0046BFA4, UnitIDMask);
		memory_patch(0x0046BFB0, &ccmu_extender.unit_table[-1]);

		// is_tile_blocked_by (0x00473300)
		memory_patch(0x0047339A, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004733A9, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004733C1, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x004733D0, &ccmu_extender.unit_finder_y[0].position);

		// does_building_block (0x00473410)
		memory_patch(0x0047343A, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00473451, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x00473460, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x00473477, &ccmu_extender.unit_finder_y[0].position);

		// minorUnpackSharedSaveData (0x00479E60)
		memory_patch(0x00479E7C, UnitIDMask);
		memory_patch(0x00479E88, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00479E9B, UnitIDMask);
		memory_patch(0x00479EA7, &ccmu_extender.unit_table[-1]);

		// minorPackSharedSaveData (0x00479EC0)
		memory_patch(0x00479ED0, &ccmu_extender.unit_table);
		memory_patch(0x00479EE7, UnitLimit);
		memory_patch(0x00479F0F, &ccmu_extender.unit_table);
		memory_patch(0x00479F26, UnitLimit);

		// unit_to_unit_id (0x0047B1D0)
		memory_patch(0x0047B1D8, &ccmu_extender.unit_table);
		memory_patch(0x0047B1EF, UnitLimit);

		// unitIDtoUnitMemFast (0x0047B210)
		memory_patch(0x0047B215, UnitIDMask);
		memory_patch(0x0047B221, &ccmu_extender.unit_table[-1]);

		// unitIDtoUnit (0x0047B230)
		memory_patch(0x0047B237, UnitIDMask);
		memory_patch(0x0047B243, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x0047B24B, &ccmu_extender.unit_table[-1]);

		// unpackBulletData (0x0048A8D0)
		memory_patch(0x0048A8E4, UnitIDMask);
		memory_patch(0x0048A8F0, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x0048A8F8, &ccmu_extender.unit_table[-1]);
		memory_patch(0x0048A927, UnitIDMask);
		memory_patch(0x0048A933, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x0048A93B, &ccmu_extender.unit_table[-1]);

		// packBulletData (0x0048A9A0)
		memory_patch(0x0048A9BB, &ccmu_extender.unit_table);
		memory_patch(0x0048A9D2, UnitLimit);
		memory_patch(0x0048A9FA, &ccmu_extender.unit_table);
		memory_patch(0x0048AA11, UnitLimit);

		// AirSplashProc (0x0048B150)
		memory_patch(0x0048B1B1, &ccmu_extender.air_splash_list);
		memory_patch(0x0048B1CA, &ccmu_extender.air_splash_list);

		// bullet_damage (0x0048B770)
		memory_patch(0x0048B896, &ccmu_extender.air_splash_list);

		// unpackOrderData (0x0048C770)
		memory_patch(0x0048C781, UnitIDMask);
		memory_patch(0x0048C78D, &ccmu_extender.unit_table[-1]);

		// packOrderData (0x0048C7D0)
		memory_patch(0x0048C7E9, &ccmu_extender.unit_table);
		memory_patch(0x0048C800, UnitLimit);

		// InitializeBurrowedUnitsLinkedListRef (0x00491570)
		memory_patch(0x00491594, UnitIDMask);
		memory_patch(0x004915A0, &ccmu_extender.unit_table[-1]);

		// writeBurrowedUnitIndex (0x004915C0)
		memory_patch(0x004915D6, &ccmu_extender.unit_table);
		memory_patch(0x004915ED, UnitLimit);
		memory_patch(0x00491633, UnitIDMask);
		memory_patch(0x0049163F, &ccmu_extender.unit_table[-1]);

		// 0x00494C60
		memory_patch(0x00494C90, UnitIDMask);
		memory_patch(0x00494C9C, &ccmu_extender.unit_table[-1]);

		// packFlingyData (0x00494CE0)
		memory_patch(0x00494CF3, &ccmu_extender.unit_table);
		memory_patch(0x00494D0A, UnitLimit);

		// packUnpackFlingyData (0x00495370)
		memory_patch(0x004953A5, UnitIDMask);
		memory_patch(0x004953B1, &ccmu_extender.unit_table[-1]);

		// WriteFlingys (0x00495B50)
		memory_patch(0x00495C44, UnitIDMask);
		memory_patch(0x00495C50, &ccmu_extender.unit_table[-1]);

		// command_save_hotkey_group (0x004965D0)
		memory_patch(0x0049660F, UnitIDMask);
		memory_patch(0x0049661B, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x00496623, &ccmu_extender.unit_table[-1]);
		memory_patch(0x00496700, &ccmu_extender.unit_table);
		memory_patch(0x00496717, UnitLimit);

		// centerviewUnitGroup (0x004967E0)
		memory_patch(0x0049687E, UnitIDMask);
		memory_patch(0x0049688A, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x00496892, &ccmu_extender.unit_table[-1]);

		// add_to_control_group (0x00496940)
		memory_patch(0x00496A02, UnitIDMask);
		memory_patch(0x00496A0E, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x00496A16, &ccmu_extender.unit_table[-1]);

		// selectUnitGroup (0x00496B40)
		memory_patch(0x00496BA0, UnitIDMask);
		memory_patch(0x00496BAC, &ccmu_extender.unit_table[-1]);

		// 0x0049E1E0
		memory_patch(0x0049E1E1, &ccmu_extender.unit_table);

		// InitializeEmptyUnitsLinkedListRef (0x0049E1F0)
		memory_patch(0x0049E200, &ccmu_extender.unit_table);

		// 0x0049E260
		memory_patch(0x0049E24F, UnitLimit);
		memory_patch(0x0049E268, UnitLimit);

		// ReadUnitDataInfo (0x0049E6C0)
		memory_patch(0x0049E74C, &ccmu_extender.unit_table);

		// ReadUnitDataInfoEx (0x0049E790)
		memory_patch(0x0049E81C, &ccmu_extender.unit_table);

		// read_units_array (0x0049E910)
		memory_patch(0x0049E915, UnitLimit + 4);
		memory_patch(0x0049E91B, UnitLimit);
		memory_patch(0x0049E921, -(UnitLimit + 4));
		memory_patch(0x0049E951, sizeof(ccmu_extender.unit_table) / 4);
		memory_patch(0x0049E956, &ccmu_extender.unit_table);
		memory_patch(0x0049E995, UnitLimit);
		memory_patch(0x0049E99A, &ccmu_extender.unit_table[UnitLimit]);
		memory_patch(0x0049E9A3, -(UnitLimit + 5));
		memory_patch(0x0049E9BD, &ccmu_extender.unit_table);
		memory_patch(0x0049EB19, -(UnitLimit + 4));

		// resetOrdersUnitsDAT (0x0049F380)
		memory_patch(0x0049F3C3, sizeof(ccmu_extender.unit_table) / 4);
		memory_patch(0x0049F3C8, &ccmu_extender.unit_table);
		memory_patch(0x0049F41F, &ccmu_extender.unit_table);
		memory_patch(0x0049F424, UnitLimit);
		memory_patch(0x0049F46F, UnitLimit * 4);
		memory_patch(0x0049F474, &ccmu_extender.unit_finder_x);
		memory_patch(0x0049F47E, UnitLimit * 4);
		memory_patch(0x0049F483, &ccmu_extender.unit_finder_y);

		// kill_all_loaded_units (0x0049FDD0)
		memory_patch(0x0049FE4D, UnitIDMask);
		memory_patch(0x0049FE59, &ccmu_extender.unit_table[-1]);

		// unit_constructor (0x004A06C0)
		memory_patch(0x004A06C9, UnitLimit);

		// CMDRECV_ShiftDeselect (0x004BFB40)
		memory_patch(0x004BFB7E, UnitIDMask);
		memory_patch(0x004BFB8A, &ccmu_extender.unit_table[-1]);

		// CMDACT_Unload (0x004BFDB0)
		memory_patch(0x004BFDC0, &ccmu_extender.unit_table);
		memory_patch(0x004BFDD7, UnitLimit);

		// CMDACT_TargetOrder (0x004C0300)
		memory_patch(0x004C0325, &ccmu_extender.unit_table);
		memory_patch(0x004C033C, UnitLimit);

		// cmdact_right_click_order (0x004C0380)
		memory_patch(0x004C03A2, &ccmu_extender.unit_table);
		memory_patch(0x004C03B9, UnitLimit);

		// cmdact_select (0x004C0860)
		memory_patch(0x004C0929, &ccmu_extender.unit_table);
		memory_patch(0x004C0940, UnitLimit);
		memory_patch(0x004C09B6, &ccmu_extender.unit_table);
		memory_patch(0x004C09CD, UnitLimit);
		memory_patch(0x004C0A35, &ccmu_extender.unit_table);
		memory_patch(0x004C0A4C, UnitLimit);

		// CMDRECV_Unload (0x004C24F0)
		memory_patch(0x004C24FC, UnitIDMask);
		memory_patch(0x004C2508, &ccmu_extender.unit_table[-1]);

		// cmdrecv_shift_select (0x004C2560)
		memory_patch(0x004C2613, UnitIDMask);
		memory_patch(0x004C261F, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004C2627, &ccmu_extender.unit_table[-1]);

		// cmdrecv_select (0x004C2750)
		memory_patch(0x004C279E, UnitIDMask);
		memory_patch(0x004C27AA, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004C27B2, &ccmu_extender.unit_table[-1]);

		// kill_units_at_location_proc (0x004C7E20)
		memory_patch(0x004C7ED3, UnitIDMask);
		memory_patch(0x004C7EDF, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004C7EE7, &ccmu_extender.unit_table[-1]);

		// unpack_all_player_selection_groups (0x004CEDA0)
		memory_patch(0x004CEDB9, UnitIDMask);
		memory_patch(0x004CEDC5, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004CEDCD, &ccmu_extender.unit_table[-1]);

		// pack_all_player_selection_groups (0x004CEE00)
		memory_patch(0x004CEE1E, &ccmu_extender.unit_table);
		memory_patch(0x004CEE35, UnitLimit);

		// lose_sight_selection (0x004D0820)
		memory_patch(0x004D0893, &ccmu_extender.unit_table);
		memory_patch(0x004D08AA, UnitLimit);

		// unpackImageGrpData (0x004D5600)
		memory_patch(0x004D565B, UnitIDMask);
		memory_patch(0x004D5667, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004D566F, &ccmu_extender.unit_table[-1]);

		// packImageData (0x004D6330)
		memory_patch(0x004D638E, &ccmu_extender.unit_table);
		memory_patch(0x004D63A5, UnitLimit);

		// sub_4E2CB0 (0x004E2CB0)
		memory_patch(0x004E2CD5, &ccmu_extender.unit_finder_y[UnitLimit * 2 - 3]);

		// unpack_unit_data (0x004E2E20)
		memory_patch(0x004E2E83, UnitIDMask);
		memory_patch(0x004E2E8F, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E2EA2, UnitIDMask);
		memory_patch(0x004E2EAE, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E2EC1, UnitIDMask);
		memory_patch(0x004E2ECD, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E2F08, UnitIDMask);
		memory_patch(0x004E2F14, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E2F2A, UnitIDMask);
		memory_patch(0x004E2F36, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E2F58, UnitIDMask);
		memory_patch(0x004E2F64, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E2FD8, UnitIDMask);
		memory_patch(0x004E2FE4, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E2FFD, UnitIDMask);
		memory_patch(0x004E3009, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E303A, UnitIDMask);
		memory_patch(0x004E3046, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E305F, UnitIDMask);
		memory_patch(0x004E306B, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E3090, UnitIDMask);
		memory_patch(0x004E309C, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E30B5, UnitIDMask);
		memory_patch(0x004E30C1, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E30DA, UnitIDMask);
		memory_patch(0x004E30E6, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E3105, UnitIDMask);
		memory_patch(0x004E3111, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E3131, UnitIDMask);
		memory_patch(0x004E313D, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E31BA, UnitIDMask);
		memory_patch(0x004E31C6, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E31EB, UnitIDMask);
		memory_patch(0x004E31F7, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E3219, UnitIDMask);
		memory_patch(0x004E3225, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E323E, UnitIDMask);
		memory_patch(0x004E324A, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E3263, UnitIDMask);
		memory_patch(0x004E326F, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E3288, UnitIDMask);
		memory_patch(0x004E3294, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E32B8, UnitIDMask);
		memory_patch(0x004E32C4, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E32DD, UnitIDMask);
		memory_patch(0x004E32E9, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E3309, UnitIDMask);
		memory_patch(0x004E3315, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E332E, UnitIDMask);
		memory_patch(0x004E333A, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E335E, UnitIDMask);
		memory_patch(0x004E336A, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E339B, UnitIDMask);
		memory_patch(0x004E33A7, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E33CE, UnitIDMask);
		memory_patch(0x004E33DA, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E33EC, UnitIDMask);
		memory_patch(0x004E33F8, &ccmu_extender.unit_table[-1]);

		// pack_unit_data (0x004E3410)
		memory_patch(0x004E3470, &ccmu_extender.unit_table);
		memory_patch(0x004E3487, UnitLimit);
		memory_patch(0x004E34AF, &ccmu_extender.unit_table);
		memory_patch(0x004E34C6, UnitLimit);
		memory_patch(0x004E34EE, &ccmu_extender.unit_table);
		memory_patch(0x004E3505, UnitLimit);
		memory_patch(0x004E3575, &ccmu_extender.unit_table);
		memory_patch(0x004E358C, UnitLimit);
		memory_patch(0x004E35B7, &ccmu_extender.unit_table);
		memory_patch(0x004E35CE, UnitLimit);
		memory_patch(0x004E3605, &ccmu_extender.unit_table);
		memory_patch(0x004E361C, UnitLimit);
		memory_patch(0x004E365C, &ccmu_extender.unit_table);
		memory_patch(0x004E3673, UnitLimit);
		memory_patch(0x004E36A1, &ccmu_extender.unit_table);
		memory_patch(0x004E36B8, UnitLimit);
		memory_patch(0x004E3706, &ccmu_extender.unit_table);
		memory_patch(0x004E371D, UnitLimit);
		memory_patch(0x004E374B, &ccmu_extender.unit_table);
		memory_patch(0x004E3762, UnitLimit);
		memory_patch(0x004E37A4, &ccmu_extender.unit_table);
		memory_patch(0x004E37BB, UnitLimit);
		memory_patch(0x004E37E9, &ccmu_extender.unit_table);
		memory_patch(0x004E3800, UnitLimit);
		memory_patch(0x004E382E, &ccmu_extender.unit_table);
		memory_patch(0x004E3845, UnitLimit);
		memory_patch(0x004E387A, &ccmu_extender.unit_table);
		memory_patch(0x004E3891, UnitLimit);
		memory_patch(0x004E38C7, &ccmu_extender.unit_table);
		memory_patch(0x004E38DE, UnitLimit);
		memory_patch(0x004E3983, &ccmu_extender.unit_table);
		memory_patch(0x004E399A, UnitLimit);
		memory_patch(0x004E39D8, &ccmu_extender.unit_table);
		memory_patch(0x004E39EF, UnitLimit);
		memory_patch(0x004E3A2F, &ccmu_extender.unit_table);
		memory_patch(0x004E3A46, UnitLimit);
		memory_patch(0x004E3A74, &ccmu_extender.unit_table);
		memory_patch(0x004E3A8B, UnitLimit);
		memory_patch(0x004E3AB9, &ccmu_extender.unit_table);
		memory_patch(0x004E3AD0, UnitLimit);
		memory_patch(0x004E3AFE, &ccmu_extender.unit_table);
		memory_patch(0x004E3B15, UnitLimit);
		memory_patch(0x004E3B55, &ccmu_extender.unit_table);
		memory_patch(0x004E3B6C, UnitLimit);
		memory_patch(0x004E3B9A, &ccmu_extender.unit_table);
		memory_patch(0x004E3BB1, UnitLimit);
		memory_patch(0x004E3BEB, &ccmu_extender.unit_table);
		memory_patch(0x004E3C02, UnitLimit);
		memory_patch(0x004E3C30, &ccmu_extender.unit_table);
		memory_patch(0x004E3C47, UnitLimit);
		memory_patch(0x004E3C80, &ccmu_extender.unit_table);
		memory_patch(0x004E3C97, UnitLimit);
		memory_patch(0x004E3CDB, &ccmu_extender.unit_table);
		memory_patch(0x004E3CF2, UnitLimit);
		memory_patch(0x004E3D32, &ccmu_extender.unit_table);
		memory_patch(0x004E3D49, UnitLimit);
		memory_patch(0x004E3D6C, &ccmu_extender.unit_table);
		memory_patch(0x004E3D83, UnitLimit);

		// get_loaded_unit_from_index (0x004E6C40)
		memory_patch(0x004E6C50, UnitIDMask);
		memory_patch(0x004E6C5C, &ccmu_extender.unit_table[-1]);

		// get_first_loaded_unit (0x004E6C90)
		memory_patch(0x004E6CBA, UnitIDMask);
		memory_patch(0x004E6CC6, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004E6CCE, &ccmu_extender.unit_table[-1]);

		// iterate_loaded_units (0x004E6D00)
		memory_patch(0x004E6D2B, UnitIDMask);
		memory_patch(0x004E6D37, &ccmu_extender.unit_table[-1]);

		// get_first_loaded_worker (0x004E6D90)
		memory_patch(0x004E6DAA, UnitIDMask);
		memory_patch(0x004E6DB6, &ccmu_extender.unit_table[-1]);

		// is_carrying_flag (0x004E7080)
		memory_patch(0x004E709A, UnitIDMask);
		memory_patch(0x004E70A6, &ccmu_extender.unit_table[-1]);

		// is_unit_loaded (0x004E7110)
		memory_patch(0x004E712D, UnitIDMask);
		memory_patch(0x004E7139, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004E7141, &ccmu_extender.unit_table[-1]);

		// get_loaded_space_amount (0x004E7170)
		memory_patch(0x004E719C, UnitIDMask);
		memory_patch(0x004E71A8, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004E71B0, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E71E7, UnitIDMask);
		memory_patch(0x004E71F3, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004E71FB, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E7233, UnitIDMask);
		memory_patch(0x004E723F, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004E7247, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E727F, UnitIDMask);
		memory_patch(0x004E728B, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004E7293, &ccmu_extender.unit_table[-1]);

		// load_unit_proc (0x004E78E0)
		memory_patch(0x004E7911, UnitIDMask);
		memory_patch(0x004E791D, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004E7924, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E79A7, &ccmu_extender.unit_table);
		memory_patch(0x004E79BE, UnitLimit);

		// unload_unit_ext (0x004E7E10)
		memory_patch(0x004E7E66, &ccmu_extender.unit_table);
		memory_patch(0x004E7E7D, UnitLimit);

		// orders_Unload (0x004E80D0)
		memory_patch(0x004E8112, UnitIDMask);
		memory_patch(0x004E811E, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004E8126, &ccmu_extender.unit_table[-1]);

		// UnitDistanceMapping (0x004E8320)
		memory_patch(0x004E8395, &ccmu_extender.unit_finder_x);
		memory_patch(0x004E83A5, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E842E, &ccmu_extender.unit_finder_x);
		memory_patch(0x004E8442, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E84C7, &ccmu_extender.unit_finder_y);
		memory_patch(0x004E84DB, &ccmu_extender.unit_table[-1]);
		memory_patch(0x004E8564, &ccmu_extender.unit_finder_y);
		memory_patch(0x004E8578, &ccmu_extender.unit_table[-1]);

		// 0x004E865B
		memory_patch(0x004E8670, &ccmu_extender.unit_finder_x);
		memory_patch(0x004E8683, &ccmu_extender.unit_finder_y);

		// GetNearestUnitFrom (0x004E86C0)
		memory_patch(0x004E86EB, &ccmu_extender.unit_finder_x);
		memory_patch(0x004E8702, &ccmu_extender.unit_finder_y);

		// ai_best_unit (0x004E8740)
		memory_patch(0x004E877D, &ccmu_extender.unit_finder_x);
		memory_patch(0x004E8796, &ccmu_extender.unit_finder_y);

		// 0x004EA940
		memory_patch(0x004EA944, &ccmu_extender.unit_table);

		// writeUnitPointer (0x004EA970)
		memory_patch(0x004EA97C, &ccmu_extender.unit_table);
		memory_patch(0x004EA993, UnitLimit);

		// sub_4EA9D0 (0x004EA9D0)
		memory_patch(0x004EAA2D, &ccmu_extender.unit_table);
		memory_patch(0x004EAA4C, &ccmu_extender.unit_table);

		// 0x004EAA80
		memory_patch(0x004EAA9C, UnitLimit);
		memory_patch(0x004EAAA1, &ccmu_extender.unit_table[UnitLimit]);
		memory_patch(0x004EAAD4, &ccmu_extender.unit_table);

		// WriteUnits (0x004EAAF0)
		memory_patch(0x004EAAF5, UnitLimit + 0x10);
		memory_patch(0x004EAAFD, UnitLimit);
		memory_patch(0x004EAB03, -(UnitLimit + 0x10));
		memory_patch(0x004EAB0F, UnitLimit);
		memory_patch(0x004EAB14, &ccmu_extender.unit_table[UnitLimit]);
		memory_patch(0x004EAB23, -(UnitLimit + 0x11));
		memory_patch(0x004EAB3E, &ccmu_extender.unit_table);
		memory_patch(0x004EAB49, UnitLimit);
		memory_patch(0x004EAB4F, -(UnitLimit + 0x10));
		memory_patch(0x004EABC7, &ccmu_extender.unit_table[0].id);
		memory_patch(0x004EABCE, UnitLimit);
		memory_patch(0x004EABD5, -(UnitLimit + 0x10));
		memory_patch(0x004EAC31, UnitLimit);
		memory_patch(0x004EAC36, &ccmu_extender.unit_table[UnitLimit]);
		memory_patch(0x004EAC43, -(UnitLimit + 0x11));
		memory_patch(0x004EAC5D, &ccmu_extender.unit_table);

		// initialize_global_unit_count (0x004EAD50)
		memory_patch(0x004EAD58, UnitLimit);

		// set_under_disruption_field (0x004EB170)
		memory_patch(0x004EB1DD, UnitIDMask);
		memory_patch(0x004EB1E9, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004EB1F1, &ccmu_extender.unit_table[-1]);

		// update_disruption_web_status (0x004EB2F0)
		memory_patch(0x004EB37E, UnitIDMask);
		memory_patch(0x004EB38A, &ccmu_extender.unit_table[-1].sprite);
		memory_patch(0x004EB392, &ccmu_extender.unit_table[-1]);

		// sub_4F2A70 (0x004F2A70)
		memory_patch(0x004F2AE8, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004F2B07, &ccmu_extender.unit_finder_x[0].position);
		memory_patch(0x004F2B26, &ccmu_extender.unit_finder_y[0].position);
		memory_patch(0x004F2B3B, &ccmu_extender.unit_finder_y[0].position);
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
		memory_patch(0x0042D343, &ccmu_extender.sprite_list_pointers);

		// get_first_sprite (0x0042D460)
		memory_patch(0x0042D47B, &ccmu_extender.sprite_list_pointers);
		memory_patch(0x0042D4B7, &ccmu_extender.sprite_list_pointers);

		// 0x0042D4C0
		jump_patch(sub_42D4C0_patch_2D4F2, 0x0042D4F2); // Needed for sprite entries above 8191.

		// drawAllSprites (0x00498D40)
		memory_patch(0x00498D67, &ccmu_extender.sprite_list_pointers);

		// minorUnpackSharedSaveData (0x00479E60)
		memory_patch(0x00479E6D, &ccmu_extender.sprite_table[-1]);

		// minorPackSharedSaveData (0x00479EC0)
		memory_patch(0x00479F4F, &ccmu_extender.sprite_table);

		// 0x00487350
		memory_patch(0x00487360, &ccmu_extender.sprite_table[-1]);

		// 0x004873E0
		memory_patch(0x004873EC, &ccmu_extender.sprite_table);

		// packThingyData (0x00487540)
		memory_patch(0x00487559, &ccmu_extender.sprite_table);
		memory_patch(0x004875A9, &ccmu_extender.sprite_table);

		// unpack_thingy_data (0x004875F0)
		memory_patch(0x0048760D, &ccmu_extender.sprite_table[-1]);
		memory_patch(0x00487655, &ccmu_extender.sprite_table[-1]);

		// 0x00494C70
		memory_patch(0x00494C80, &ccmu_extender.sprite_table[-1]);

		// packFlingyData (0x00494CE0)
		memory_patch(0x00494D2F, &ccmu_extender.sprite_table);

		// packUnpackFlingyData (0x00495370)
		memory_patch(0x00495392, &ccmu_extender.sprite_table[-1]);

		// WriteFlingys (0x00495B50)
		memory_patch(0x00495C31, &ccmu_extender.sprite_table[-1]);

		// 0x00496EB0
		memory_patch(0x00496EB9, &ccmu_extender.sprite_table);

		// 0x00496FB0
		memory_patch(0x00496FBB, &ccmu_extender.sprite_table[-1]);

		// initializeSpriteArray (0x00497230)
		memory_patch(0x00497236, &ccmu_extender.sprite_table[0].images);
		memory_patch(0x00497240, &ccmu_extender.sprite_table[1].images);
		memory_patch(0x0049724A, &ccmu_extender.sprite_table[2].images);
		memory_patch(0x00497254, &ccmu_extender.sprite_table[3].images);
		memory_patch(0x0049725E, &ccmu_extender.sprite_table[4].images);
		memory_patch(0x00497271, sizeof(ccmu_extender.sprite_table));
		memory_patch(0x0049728A, SpriteLimit);
		memory_patch(0x004972AB, &ccmu_extender.sprite_table);
		memory_patch(0x004972B1, SpriteLimit);
		memory_patch(0x004972CA, SpriteLimit);
		memory_patch(0x004972D1, &ccmu_extender.sprite_table);

		// unpackSpriteData (0x00497310)
		memory_patch(0x00497361, &ccmu_extender.sprite_table[-1]);
		memory_patch(0x00497378, &ccmu_extender.sprite_table[-1]);

		// packSpriteData (0x00497380)
		memory_patch(0x004973CD, &ccmu_extender.sprite_table);
		memory_patch(0x004973ED, &ccmu_extender.sprite_table);

		// 0x00497D80
		memory_patch(0x00497D81, &ccmu_extender.sprite_table);
		memory_patch(0x00497D86, SpriteLimit);

		// 0x00497DA0
		memory_patch(0x00497DA2, &ccmu_extender.sprite_table);
		memory_patch(0x00497DA7, SpriteLimit);

		// read_sprites_array (0x00498570)
		memory_patch(0x0049859E, sizeof(ccmu_extender.sprite_table) / 4);
		memory_patch(0x004985A3, &ccmu_extender.sprite_table);
		memory_patch(0x00498631, &ccmu_extender.sprite_table);
		memory_patch(0x0049865B, &ccmu_extender.sprite_table);
		memory_patch(0x00498660, SpriteLimit);
		memory_patch(0x004986B2, &ccmu_extender.sprite_table[-1]);
		memory_patch(0x00498702, &ccmu_extender.sprite_table[-1]);
		memory_patch(0x00498718, &ccmu_extender.sprite_table[0].index);
		memory_patch(0x00498721, sizeof(ccmu_extender.sprite_table));

		// writeSprites (0x00498740)
		memory_patch(0x00498749, &ccmu_extender.sprite_table);
		memory_patch(0x0049874E, SpriteLimit);
		memory_patch(0x00498763, &ccmu_extender.sprite_table[1].images);
		memory_patch(0x00498793, &ccmu_extender.sprite_table[SpriteLimit + 1].images);
		memory_patch(0x004987FC, &ccmu_extender.sprite_table);
		memory_patch(0x00498839, (u16)SpriteLimit);
		memory_patch(0x00498868, &ccmu_extender.sprite_table);
		memory_patch(0x0049886D, SpriteLimit);
		memory_patch(0x004988A2, &ccmu_extender.sprite_table);
		memory_patch(0x004988F6, &ccmu_extender.sprite_table[-1]);
		memory_patch(0x00498923, &ccmu_extender.sprite_table);
		memory_patch(0x00498975, &ccmu_extender.sprite_table[-1]);

		// InitializeSpriteArray (0x00499900)
		memory_patch(0x00499915, &ccmu_extender.sprite_table);
		memory_patch(0x0049991A, sizeof(ccmu_extender.sprite_table) / 4);
		memory_patch(0x0049993C, &ccmu_extender.sprite_table[0].index);
		memory_patch(0x00499952, &ccmu_extender.sprite_table[1]);
		memory_patch(0x00499957, (SpriteLimit - 1) / 3);

		// unpackImageGrpData (0x004D5600)
		memory_patch(0x004D56B9, &ccmu_extender.sprite_table[-1]);

		// packImageData (0x004D6330)
		memory_patch(0x004D63DA, &ccmu_extender.sprite_table);

		// unpack_unit_data (0x004E2E20)
		memory_patch(0x004E3196, &ccmu_extender.sprite_table[-1]);

		// pack_unit_data (0x004E3410)
		memory_patch(0x004E394C, &ccmu_extender.sprite_table);

		// RemoveAllSelectionCircles (0x004989A0)
		memory_patch(0x004989A8, &ccmu_extender.sprite_table[0].flags);
	}
}

namespace hooks {
	void inject_order_extender() {
		// targeted_order (0x004754F0)
		memory_patch(0x00475576, NewCheckLimit);

		// 0x0048C4A0
		memory_patch(0x0048C4AB, &ccmu_extender.order_table[-1]);
		memory_patch(0x0048C4B6, &ccmu_extender.order_table);

		// 0x0048C720
		memory_patch(0x0048C726, NewCheckLimit);

		// InitializeOrderArray (0x0048C590)
		memory_patch(0x0048C595, sizeof(ccmu_extender.order_table) / 4);
		memory_patch(0x0048C59A, &ccmu_extender.order_table);
		memory_patch(0x0048C5AF, &ccmu_extender.order_table);
		memory_patch(0x0048C5B4, OrderLimit / 5);

		// 0x0048C740
		memory_patch(0x0048C759, OrderLimit);

		// unpackOrderData (0x0048C770)
		memory_patch(0x0048C771, &ccmu_extender.order_table[OrderLimit]);
		memory_patch(0x0048C7A4, &ccmu_extender.order_table[-1]);
		memory_patch(0x0048C7B7, &ccmu_extender.order_table[-1]);
		memory_patch(0x0048C7BC, &ccmu_extender.order_table);

		// packOrderData (0x0048C7D0)
		memory_patch(0x0048C7D3, &ccmu_extender.order_table[OrderLimit]);
		memory_patch(0x0048C820, &ccmu_extender.order_table);
		memory_patch(0x0048C843, &ccmu_extender.order_table);
		memory_patch(0x0048C85E, &ccmu_extender.order_table);

		// read_orders_array (0x0048C870)
		memory_patch(0x0048C872, sizeof(ccmu_extender.order_table));
		memory_patch(0x0048C877, &ccmu_extender.order_table);
		memory_patch(0x0048C8C8, &ccmu_extender.order_table[-1]);
		memory_patch(0x0048C8E1, &ccmu_extender.order_table[-1]);
		memory_patch(0x0048C8FA, OrderLimit);

		// WriteOrders (0x0048C910)
		memory_patch(0x0048C917, sizeof(ccmu_extender.order_table));
		memory_patch(0x0048C91C, &ccmu_extender.order_table);
		memory_patch(0x0048C93D, &ccmu_extender.order_table);
		memory_patch(0x0048C965, &ccmu_extender.order_table);
		memory_patch(0x0048C9C4, &ccmu_extender.order_table[-1]);
		memory_patch(0x0048C9E5, &ccmu_extender.order_table[-1]);

		// unpack_unit_data (0x004E2E20)
		memory_patch(0x004E2EE5, &ccmu_extender.order_table[-1]);
		memory_patch(0x004E2EF9, &ccmu_extender.order_table[-1]);

		// pack_unit_data (0x004E3410)
		memory_patch(0x004E3526, &ccmu_extender.order_table);
		memory_patch(0x004E354A, &ccmu_extender.order_table);
	}
}

namespace hooks {
	void inject_image_extender() {
		// sub_42D370 (0x0042D370)
		memory_patch(0x0042D377, &ccmu_extender.image_list_pointers[1]);
		memory_patch(0x0042D37E, &ccmu_extender.image_list_pointers);
		memory_patch(0x0042D3AA, &ccmu_extender.image_list_pointers[-2]);
		memory_patch(0x0042D3B1, &ccmu_extender.image_list_pointers);
		memory_patch(0x0042D3BB, &ccmu_extender.image_list_pointers[-2]);
		memory_patch(0x0042D3C8, &ccmu_extender.image_list_pointers[-2]);
		memory_patch(0x0042D3CF, &ccmu_extender.image_list_pointers[-1]);
		memory_patch(0x0042D3D6, &ccmu_extender.image_list_pointers[-1]);
		memory_patch(0x0042D3E6, &ccmu_extender.image_list_pointers[-2]);
		memory_patch(0x0042D3ED, &ccmu_extender.image_list_pointers[-1]);

		// sub_42D400 (0x0042D400)
		memory_patch(0x0042D409, &ccmu_extender.image_list_pointers[-2]);
		memory_patch(0x0042D418, &ccmu_extender.image_list_pointers[-1]);
		memory_patch(0x0042D423, &ccmu_extender.image_list_pointers[-2]);
		memory_patch(0x0042D42E, &ccmu_extender.image_list_pointers[-2]);
		memory_patch(0x0042D435, &ccmu_extender.image_list_pointers[-1]);
		memory_patch(0x0042D43C, &ccmu_extender.image_list_pointers[-1]);
		memory_patch(0x0042D44D, &ccmu_extender.image_list_pointers[-1]);
		memory_patch(0x0042D454, &ccmu_extender.image_list_pointers[-2]);

		// get_first_sprite (0x0042D460)
		memory_patch(0x0042D483, &ccmu_extender.image_list_pointers[-2]);
		memory_patch(0x0042D488, &ccmu_extender.image_list_pointers[1]);
		memory_patch(0x0042D495, &ccmu_extender.image_list_pointers[-1]);
		memory_patch(0x0042D48E, &ccmu_extender.image_list_pointers);
		memory_patch(0x0042D49E, &ccmu_extender.image_list_pointers[1]);

		// sub_42D4C0 (0x0042D4C0)
		memory_patch(0x0042D4CA, &ccmu_extender.image_list_pointers[-1]);
		memory_patch(0x0042D506, &ccmu_extender.image_list_pointers[-2]);

		// unpackSpriteData (0x00497310)
		memory_patch(0x0049731C, &ccmu_extender.image_table[-1]);
		memory_patch(0x00497333, &ccmu_extender.image_table[-1]);
		memory_patch(0x0049734A, &ccmu_extender.image_table[-1]);

		// packSpriteData (0x00497380)
		memory_patch(0x00497388, &ccmu_extender.image_table);
		memory_patch(0x0049739F, &ccmu_extender.image_table);
		memory_patch(0x004973B6, &ccmu_extender.image_table);

		// 0x004D4B60
		memory_patch(0x004D4B65, &ccmu_extender.image_table);

		// 0x004D4B80
		memory_patch(0x004D4B89, &ccmu_extender.image_table[-1]);

		// initializeImageArray (0x004D4BA0)
		memory_patch(0x004D4BA3, &ccmu_extender.image_table[1].grp_file);
		memory_patch(0x004D4BD9, &ccmu_extender.image_table[ImageLimit + 1].grp_file);
		memory_patch(0x004D4BF2, ImageLimit);
		memory_patch(0x004D4C14, &ccmu_extender.image_table);
		memory_patch(0x004D4C1A, ImageLimit);
		memory_patch(0x004D4C32, &ccmu_extender.image_table);
		memory_patch(0x004D4C67, &ccmu_extender.image_table[ImageLimit]);

		// unpackImageGrpData (0x004D5600)
		memory_patch(0x004D56CB, &ccmu_extender.image_table[-1]);
		memory_patch(0x004D56E1, &ccmu_extender.image_table[-1]);

		// 0x004D5DE0
		memory_patch(0x004D5DE2, &ccmu_extender.image_table);
		memory_patch(0x004D5DE7, ImageLimit);

		// read_images_array (0x004D6220)
		memory_patch(0x004D624D, sizeof(ccmu_extender.image_table) / 4);
		memory_patch(0x004D6252, &ccmu_extender.image_table);
		memory_patch(0x004D62E0, &ccmu_extender.image_table);
		memory_patch(0x004D6307, &ccmu_extender.image_table);
		memory_patch(0x004D630C, ImageLimit);

		// packImageData (0x004D6330)
		memory_patch(0x004D63F3, &ccmu_extender.image_table);
		memory_patch(0x004D6409, &ccmu_extender.image_table);

		// 0x004D64A0
		memory_patch(0x004D64A3, &ccmu_extender.image_table);
		memory_patch(0x004D64A8, ImageLimit);

		// writeImages (0x004D64C0)
		memory_patch(0x004D64C9, &ccmu_extender.image_table);
		memory_patch(0x004D64CE, ImageLimit);
		memory_patch(0x004D64E6, &ccmu_extender.image_table[0].grp_file);
		memory_patch(0x004D64F1, &ccmu_extender.image_table[1].grp_file);
		memory_patch(0x004D64FC, &ccmu_extender.image_table[2].grp_file);
		memory_patch(0x004D6507, &ccmu_extender.image_table[3].grp_file);
		memory_patch(0x004D6512, &ccmu_extender.image_table[4].grp_file);
		memory_patch(0x004D6521, sizeof(ccmu_extender.image_table));
		memory_patch(0x004D658A, &ccmu_extender.image_table);
		memory_patch(0x004D65C0, (u16)ImageLimit);
		memory_patch(0x004D65EC, &ccmu_extender.image_table);
		memory_patch(0x004D65F1, ImageLimit);

		// InitializeImageArray (0x004D6930)
		memory_patch(0x004D6944, sizeof(ccmu_extender.image_table) / 4);
		memory_patch(0x004D6949, &ccmu_extender.image_table);
		memory_patch(0x004D6CEB, &ccmu_extender.image_table);
		memory_patch(0x004D6CFA, &ccmu_extender.image_table[1]);
		memory_patch(0x004D6CFF, ImageLimit - 1);
		memory_patch(0x004D6D04, &ccmu_extender.image_table);
		memory_patch(0x004D6D19, &ccmu_extender.image_table[0].link.next);
		memory_patch(0x004D6D22, &ccmu_extender.image_table[0].link.next);
		memory_patch(0x004D6D2D, &ccmu_extender.image_table[0].link.next);
	}
}

namespace hooks {
	void inject_thingy_extender() {
		//0x00487310
		memory_patch(0x00487316, &ccmu_extender.fow_sprite_table);

		//0x00487320
		memory_patch(0x0048732A, &ccmu_extender.fow_sprite_table[-1]);

		//0x00487330
		memory_patch(0x00487336, &ccmu_extender.thingy_table);

		//0x00487340
		memory_patch(0x0048734A, &ccmu_extender.thingy_table[-1]);

		//0x00487350
		memory_patch(0x0048737D, &ccmu_extender.thingy_table[-1]);
		memory_patch(0x00487398, &ccmu_extender.thingy_table[-1]);
		memory_patch(0x004873A5, &ccmu_extender.fow_sprite_table);
		memory_patch(0x004873B6, &ccmu_extender.fow_sprite_table[-1]);
		memory_patch(0x004873C8, &ccmu_extender.fow_sprite_table[-1]);

		//0x004873E0
		memory_patch(0x00487410, &ccmu_extender.thingy_table);
		memory_patch(0x0048742B, &ccmu_extender.thingy_table);
		memory_patch(0x0048743C, &ccmu_extender.fow_sprite_table);
		memory_patch(0x00487449, &ccmu_extender.fow_sprite_table);
		memory_patch(0x0048745B, &ccmu_extender.fow_sprite_table);

		//packThingyData (0x00487540)
		memory_patch(0x00487542, &ccmu_extender.thingy_table[0].next);
		memory_patch(0x00487547, ThingyLimit);
		memory_patch(0x00487573, &ccmu_extender.thingy_table);
		memory_patch(0x00487585, &ccmu_extender.thingy_table);
		memory_patch(0x00487596, &ccmu_extender.fow_sprite_table[0].next);
		memory_patch(0x0048759B, FowSpriteLimit);
		memory_patch(0x004875C3, &ccmu_extender.fow_sprite_table);
		memory_patch(0x004875D5, &ccmu_extender.fow_sprite_table);

		//unpack_thingy_data (0x004875F0)
		memory_patch(0x004875F1, &ccmu_extender.thingy_table[0].next);
		memory_patch(0x004875F6, ThingyLimit);
		memory_patch(0x00487620, &ccmu_extender.thingy_table[-1]);
		memory_patch(0x00487632, &ccmu_extender.thingy_table[-1]);
		memory_patch(0x0048763F, &ccmu_extender.fow_sprite_table[0].next);
		memory_patch(0x00487644, FowSpriteLimit);
		memory_patch(0x00487668, &ccmu_extender.fow_sprite_table[-1]);
		memory_patch(0x0048767A, &ccmu_extender.fow_sprite_table[-1]);

		//initialize_thingy_arrays (0x00487990)
		memory_patch(0x00487996, &ccmu_extender.thingy_table);
		memory_patch(0x004879A5, ThingyLimit);
		memory_patch(0x004879B4, &ccmu_extender.thingy_table);
		memory_patch(0x004879C3, ThingyLimit);
		memory_patch(0x004879D2, &ccmu_extender.fow_sprite_table);
		memory_patch(0x004879E1, FowSpriteLimit);
		memory_patch(0x004879F0, &ccmu_extender.fow_sprite_table);
		memory_patch(0x004879FF, FowSpriteLimit);

		//read_thingy_arrays (0x00488100)
		memory_patch(0x00488101, ThingyLimit);
		memory_patch(0x00488106, &ccmu_extender.thingy_table);
		memory_patch(0x00488115, FowSpriteLimit);
		memory_patch(0x0048811A, &ccmu_extender.fow_sprite_table);

		//0x00488140
		memory_patch(0x00488147, ThingyLimit);
		memory_patch(0x0048814C, &ccmu_extender.thingy_table);
		memory_patch(0x00488157, FowSpriteLimit);
		memory_patch(0x0048815C, &ccmu_extender.fow_sprite_table);

		//InitializeThingyArray (0x00488550)
		memory_patch(0x00488556, &ccmu_extender.fow_sprite_table);
		memory_patch(0x0048855B, sizeof(ccmu_extender.fow_sprite_table) / 4);
		memory_patch(0x00488560, &ccmu_extender.thingy_table);
		memory_patch(0x00488567, sizeof(ccmu_extender.thingy_table) / 4);
		memory_patch(0x00488572, &ccmu_extender.thingy_table);
		memory_patch(0x0048857C, &ccmu_extender.thingy_table);
		memory_patch(0x004885AB, &ccmu_extender.thingy_table[1]);
		memory_patch(0x004885B0, ThingyLimit - 1);
		memory_patch(0x004885E9, &ccmu_extender.fow_sprite_table[1]);
		memory_patch(0x004885EE, FowSpriteLimit - 1);

		//cmdrecv_save_game (0x004C2910)
		memory_patch(0x004C2B78, ThingyLimit);
		memory_patch(0x004C2B7D, &ccmu_extender.thingy_table);
		memory_patch(0x004C2B88, FowSpriteLimit);
		memory_patch(0x004C2B8D, &ccmu_extender.fow_sprite_table);

		//0x004D02D0
		memory_patch(0x004D04E3, ThingyLimit);
		memory_patch(0x004D04E8, &ccmu_extender.thingy_table);
		memory_patch(0x004D04F3, FowSpriteLimit);
		memory_patch(0x004D04F8, &ccmu_extender.fow_sprite_table);

		//unpack_unit_data (0x004E2E20)
		memory_patch(0x004E3178, &ccmu_extender.thingy_table[-1]);

		//pack_unit_data (0x004E3410)
		memory_patch(0x004E3927, &ccmu_extender.thingy_table);
	}
}