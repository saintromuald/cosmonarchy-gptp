#include "ccmu.h"
#include "extender.h"
#include "globals.h"

void CCMUExtender::extend() {
//	const u32 lim_ext = NEW_ARRAY_LENGTH + 1;
	const u32 lim_ext = UNIT_ARRAY_LENGTH + 1;
	const u32 index_start = reinterpret_cast<u32>(&new_unit_table.units[0])-336;
	const u32 table_start = reinterpret_cast<u32>(&new_unit_table.units[0]);
	const u32 orderingXstart = reinterpret_cast<u32>(&new_unit_table.orderingX);
	const u32 orderingYstart = reinterpret_cast<u32>(&new_unit_table.orderingY);
	const u32 position_search_units_start = reinterpret_cast<u32>(&new_unit_table.position_search_units);
	const u32 position_search_temp_start = reinterpret_cast<u32>(&new_unit_table.position_search_temp);

//	const u32 table_end = table_start + 336*NEW_ARRAY_LENGTH;
	const u32 table_end = table_start + 336 * UNIT_ARRAY_LENGTH;
	Globals.extended_unit_table = 1;
	memoryPatch(0x00402FD2 + 2, index_start);
	memoryPatch(0x0042FEE5+1, lim_ext);
	memoryPatch(0x0042FEF1+1, lim_ext);
	memoryPatch(0x004EABC4 + 3, table_start+0x64);//unit Id of first unit
	memoryPatch(0x00430082 + 2, index_start+0x148);//unit sorting
	//orderingX
	//orderingY
	uplink(orderingXlist(), orderingXstart);
	uplink(orderingYlist(), orderingYstart);
	uplink(orderingXlist_off4(), orderingXstart+0x4);
	uplink(orderingYlist_off4(), orderingYstart+0x4);
	uplink(orderingY_min18endlist(), orderingYstart + (UNIT_ARRAY_LENGTH * 16) - 0x18);
	memoryPatch(0x00430651 + 3, orderingXstart+0xc);
	memoryPatch(0x0046A0A6 + 3, orderingXstart+0xc);
	memoryPatch(0x0046A0F6 + 3, orderingXstart+0xc);
	memoryPatch(0x00430721 + 3, orderingYstart-0x4);
	memoryPatch(0x0046A289 + 3, orderingYstart - 0x4);
	memoryPatch(0x0046A2C9 + 3, orderingYstart - 0x4);
	memoryPatch(0x00430801 + 3, orderingYstart + 0xc);
	memoryPatch(0x0046A1D9 + 3, orderingYstart + 0xc);
	memoryPatch(0x0046A229 + 3, orderingYstart + 0xc);
	memoryPatch(0x004E8C9D + 1, orderingYstart + (UNIT_ARRAY_LENGTH * 16));
	memoryPatch(0x004E8CA6 + 1, orderingYstart + (UNIT_ARRAY_LENGTH * 16));
	memoryPatch(0x004EA7D4 + 2, orderingYstart + (UNIT_ARRAY_LENGTH * 16));
	memoryPatch(0x004EA7E8 + 1, orderingYstart + (UNIT_ARRAY_LENGTH * 16));
	memoryPatch(0x0042FEA9 + 3, position_search_units_start);
	memoryPatch(0x0042FEF6 + 1, position_search_units_start);
	memoryPatch(0x0042FF9C + 3, position_search_units_start);
	memoryPatch(0x004300B9 + 1, position_search_units_start);
	memoryPatch(0x004301A9 + 3, position_search_units_start);
	memoryPatch(0x004301CA + 2, position_search_units_start);
	memoryPatch(0x004304AE + 2, position_search_units_start);
	memoryPatch(0x00430503 + 3, position_search_units_start);
	memoryPatch(0x0043087B + 1, position_search_units_start);
	memoryPatch(0x004308B8 + 3, position_search_units_start);
	memoryPatch(0x00430ADB + 1, position_search_units_start);
	//
	memoryPatch(0x0042FEEA + 1, position_search_temp_start);
	memoryPatch(0x0043006A + 3, position_search_temp_start);
	memoryPatch(0x00430075 + 3, position_search_temp_start);
	memoryPatch(0x004309F7 + 3, position_search_temp_start);
	memoryPatch(0x00430A04 + 3, position_search_temp_start);
	memoryPatch(0x00430A64 + 3, position_search_temp_start);
	memoryPatch(0x00430A6E + 3, position_search_temp_start);
	memoryPatch(0x00430A97 + 3, position_search_temp_start);
	memoryPatch(0x004309F7 + 3, position_search_temp_start);
	memoryPatch(0x00430AC5 + 3, position_search_temp_start);
	//
	uplink(get_size_list(), UNIT_ARRAY_LENGTH);
	uplink(table_start_list(), table_start);
	uplink(get_address_list(), index_start);
	uplink(table_end_list(), table_end);
	uplink(sprite_offset_list(), table_start + 0xc);
}

std::vector<u32> CCMUExtender::orderingY_min18endlist() {
	return {
	0x00432879 + 2,
	0x004627C8 + 2,
	0x00462B07 + 2,
	0x00462B24 + 2,
	0x00462B41 + 2,
	0x00462B5E + 2,
	0x00462B7B + 2,
	0x00462B98 + 2,
	0x00462BB5 + 2,
	0x00462BDE + 2,
	0x00462D5F + 2,
	0x00462D80 + 2,
	0x00462DA1 + 2,
	0x00462DC2 + 2,
	0x00462DE3 + 2,
	0x00462E04 + 2,
	0x00462E25 + 2,
	0x00462E51 + 2,
	0x004E2CD3 + 2,

	};
}

std::vector<u32> CCMUExtender::orderingXlist() {
	return {
0x004222B8 + 1,
0x004222F0 + 3,
0x004224F7 + 1,
0x00422530 + 3,
0x0043005D + 3,
0x0043024F + 3,
0x0043030C + 3,
0x004305A0 + 3,
0x0043067C + 3,
0x0043093D + 1,
0x004309B5 + 3,
0x00430A90 + 3,
0x00440432 + 1,
0x004410E0 + 1,
0x00469BF9 + 1,
0x0046A0B3 + 1,
0x0046A0B3 + 1,
0x0046A107 + 1,
0x0046A155 + 1,
0x0046A195 + 1,
0x0046A31C + 1,
0x0046A35B + 1,
0x0046A406 + 1,
0x0046A41D + 1,
0x0046A433 + 3,
0x0046A4BC + 1,
0x0046A4D6 + 1,
0x0046A4EC + 3,
0x0049F473 + 1,
0x004E8392 + 3,
0x004E842B + 3,
0x004E866F + 1,
0x004E86EA + 1,
0x004E877C + 1
	};
}
std::vector<u32> CCMUExtender::orderingXlist_off4() {
	return {
		0x00422205 + 4,
		0x00422213 + 4,
		0x004222E3 + 3,
		0x0042230F + 4,
		0x00422445 + 4,
		0x00422453 + 4,
		0x00422523 + 3,
		0x0042254F + 4,
		0x0042E119 + 3,
		0x0042E12C + 3,
		0x00430020 + 3,
		0x00430054 + 3,
		0x00430136 + 3,
		0x00430145 + 3,
		0x004301EF + 4,
		0x00430246 + 3,
		0x004302AF + 4,
		0x00430303 + 3,
		0x00430375 + 4,
		0x00430387 + 4,
		0x004303D5 + 3,
		0x004303E8 + 3,
		0x00430414 + 4,
		0x00430430 + 4,
		0x00430481 + 3,
		0x00430494 + 3,
		0x00430522 + 4,
		0x00430593 + 3,
		0x0043060D + 4,
		0x00430673 + 3,
		0x004306F9 + 4,
		0x0043070E + 4,
		0x00430785 + 3,
		0x00430798 + 3,
		0x004307D9 + 4,
		0x004307EE + 4,
		0x00430851 + 3,
		0x00430864 + 3,
		0x00469B6D + 3,
		0x00469B74 + 3,
		0x00469B8D + 3,
		0x00469B94 + 3,
		0x0046A025 + 3,
		0x0046A02E + 3,
		0x0046A03B + 3,
		0x0046A044 + 3,
		0x0046A443 + 3,
		0x0046A4FC + 3,
		0x00473397 + 3,
		0x004733A6 + 3,
		0x00473437 + 3,
		0x0047344E + 3,
		0x004F2AE4 + 4,
		0x004F2B03 + 4
	};
}

std::vector<u32> CCMUExtender::orderingYlist_off4() {
	return {



		0x004221C3 + 3,
		0x004221EF + 4,
		0x00422325 + 4,
		0x00422333 + 4,



		0x00422403 + 3,
		0x0042242F + 4,
		0x00422565 + 4,
		0x00422573 + 4,
		0x0042E13F + 3,
		0x0042E152 + 3,
		0x0042FFF2 + 3,
		0x00430158 + 3,
		0x00430167 + 3,
		0x0043020A + 4,

		0x0043021D + 4,
		0x00430273 + 3,
		0x00430286 + 3,
		0x004302C0 + 4,
		0x004302E1 + 4,
		0x00430331 + 3,
		0x00430344 + 3,
		0x0043035B + 4,
		0x004303A7 + 3,
		0x00430406 + 4,
		0x00430453 + 3,
		0x0043053D + 4,
		0x00430553 + 4,
		0x004305C5 + 3,

		0x004305D8 + 3,
		0x00430628 + 4,
		0x0043063E + 4,
		0x004306A1 + 3,
		0x004306B4 + 3,
		0x004306E2 + 4,
		0x00430753 + 3,
		0x004307C2 + 4,
		0x00430823 + 3,
		0x00469BA9 + 3,
		0x00469BB0 + 3,
		0x00469BC5 + 3,
		0x00469BCC + 3,
		0x0046A051 + 3,
		0x0046A05B + 3,
		0x0046A06B + 3,
		0x0046A07A + 3,
		0x0046A48A + 3,
		0x0046A53D + 3,
		0x004733BE + 3,
		0x004733CD + 3,
		0x0047345D + 3,
		0x00473474 + 3,
		0x004F2B22 + 4,
		0x004F2B37 + 4
	};
}

std::vector<u32> CCMUExtender::orderingYlist() {
	return {
0x00422194 + 1,
0x004221D0 + 3,
0x004223D8 + 1,
0x00422410 + 3,
0x0042FFD6 + 1,
0x004303B0 + 3,
0x0043045C + 3,
0x00430760 + 3,
0x0043082C + 3,
0x00430961 + 1,
0x00430A25 + 3,
0x00440446 + 1,
0x004410F3 + 1,
0x00469C08 + 1,
0x0046A1E6 + 1,
0x0046A23A + 1,
0x0046A298 + 1,
0x0046A2D8 + 1,
0x0046A336 + 1,
0x0046A375 + 1,
0x0046A450 + 1,
0x0046A461 + 1,
0x0046A47D + 3,
0x0046A509 + 1,
0x0046A51A + 1,
0x0046A52D + 3,
0x0049F482 + 1,
0x004E84C4 + 3,
0x004E8561 + 3,
0x004E8682 + 1,
0x004E8701 + 1,
0x004E8795 + 1
	};
}
std::vector<u32> CCMUExtender::sprite_offset_list() {
	return { 0x00424C3E + 2,
0x00424E6F + 2,
0x00424FFD + 2,
0x0042504D + 2,
0x0042509E + 2,
0x004250F0 + 2,
0x0045F895 + 2,
0x0046B05C + 2,
0x0046BC51 + 2,
0x0046BCE1 + 2,
0x0047B241 + 2,
0x0048A8EE + 2,
0x0048A931 + 2,
0x00496619 + 2,
0x00496888 + 2,
0x00496A0C + 2,
0x004C261D + 2,
0x004C27A8 + 2,
0x004C7EDD + 2,
0x004CEDC3 + 2,
0x004D5665 + 2,
0x004E6CC4 + 2,
0x004E7137 + 2,
0x004E71A6 + 2,
0x004E71F1 + 2,
0x004E723D + 2,
0x004E7289 + 2,
0x004E791B + 2,
0x004E811C + 2,
0x004EB1E7 + 2,
0x004EB388 + 2
	};
}

std::vector<u32> CCMUExtender::get_size_list(){
	return {0x0049E91A+1, 0x004EAB48+1, 0x004EABCB+3, 0x004A06C3+6, 
	0x00401F05 + 2,
	0x00402D77 + 2,
	0x00402DE1 + 2,
	0x00402E4A + 2,
	0x00402EB3 + 2,
	0x00402F1D + 2,
	0x004034C8 + 2,
	0x00403507 + 2,
	0x00403546 + 2,
	0x00403581 + 2,
	0x00403B4B + 2,
	0x00403B8A + 2,
	0x00403BC9 + 2,
	0x00403C08 + 2,
	0x00403C47 + 2,
	0x00403C82 + 2,
	0x00403EB4 + 2,
	0x00403F50 + 2,
	0x00403FEA + 2,
	0x00404086 + 2,
	0x00404122 + 2,
	0x0043290D + 2,
	0x00432A55 + 2,
	0x00438265 + 2,
	0x00446353 + 2,
	0x00447F07 + 2,
	0x00447F4C + 2,
	0x00447F91 + 2,
	0x00447FD6 + 2,
	0x004582D4 + 2,
	0x00479EE5 + 2,
	0x00479F24 + 2,
	0x0047B1ED + 2,
	0x0048A9D0 + 2,
	0x0048AA0F + 2,
	0x0048C7FE + 2,
	0x004915EB + 2,
	0x00494D08 + 2,
	0x00496715 + 2,
	0x0049E24D + 2,
	0x004BFDD5 + 2,
	0x004C033A + 2,
	0x004C03B7 + 2,
	0x004C093E + 2,
	0x004C09CB + 2,
	0x004C0A4A + 2,
	0x004CEE33 + 2,
	0x004D08A8 + 2,
	0x004D63A3 + 2,
	0x004E3485 + 2,
	0x004E34C4 + 2,
	0x004E3503 + 2,
	0x004E358A + 2,
	0x004E35CC + 2,
	0x004E361A + 2,
	0x004E3671 + 2,
	0x004E36B6 + 2,
	0x004E371B + 2,
	0x004E3760 + 2,
	0x004E37B9 + 2,
	0x004E37FE + 2,
	0x004E3843 + 2,
	0x004E388F + 2,
	0x004E38DC + 2,
	0x004E3998 + 2,
	0x004E39ED + 2,
	0x004E3A44 + 2,
	0x004E3A89 + 2,
	0x004E3ACE + 2,
	0x004E3B13 + 2,
	0x004E3B6A + 2,
	0x004E3BAF + 2,
	0x004E3C00 + 2,
	0x004E3C45 + 2,
	0x004E3C95 + 2,
	0x004E3CF0 + 2,
	0x004E3D47 + 2,
	0x004E3D81 + 2,
	0x004E79BC + 2,
	0x004E7E7B + 2,
	0x004EA991 + 2,
	0x004EAA9B+1, 0x004EAAFC+1, 0x004EAB0E+1,0x004EAC30+1,0x004EAD57+1,0x0049E267+1,0x0049E994+1,0x0049F423+1};
}

std::vector<u32> CCMUExtender::table_start_list(){
	return{0x0049E1E0 +1, 0x0049E1FF+1, 0x0049E74B+1, 0x0049E81B+1, 0x0049E955+1,0x0049F3C7+1,0x0049F41E+1,0x004EA943+1,0x004EAA2C+1,
		0x00401EEE + 2,
		0x00402D60 + 2,
		0x00402DCA + 2,
		0x00402E33 + 2,
		0x00402E9C + 2,
		0x00402F06 + 2,
		0x004034B1 + 2,
		0x004034F0 + 2,
		0x0040352F + 2,
		0x0040356A + 2,
		0x00403B34 + 2,
		0x00403B73 + 2,
		0x00403BB2 + 2,
		0x00403BF1 + 2,
		0x00403C30 + 2,
		0x00403C6B + 2,
		0x00403E9D + 2,
		0x00403F39 + 2,
		0x00403FD3 + 2,
		0x0040406F + 2,
		0x0040410B + 2,
		0x004328F6 + 2,
		0x00432A3E + 2,
		0x0043824E + 2,
		0x00447EF0 + 2,
		0x00447F35 + 2,
		0x00447F7A + 2,
		0x00447FBF + 2,
		0x004582BD + 2,
		0x0046A3B1 + 2,
		0x00479ECE + 2,
		0x00479F0D + 2,
		0x0047B1D6 + 2,
		0x0048A9B9 + 2,
		0x0048A9F8 + 2,
		0x0048C7E7 + 2,
		0x004915D4 + 2,
		0x00494CF1 + 2,
		0x004966FE + 2,
		0x0049E9BB + 2,
		0x004BFDBE + 2,
		0x004C0323 + 2,
		0x004C03A0 + 2,
		0x004C0927 + 2,
		0x004C09B4 + 2,
		0x004C0A33 + 2,
		0x004CEE1C + 2,
		0x004D0891 + 2,
		0x004D638C + 2,
		0x004E346E + 2,
		0x004E34AD + 2,
		0x004E34EC + 2,
		0x004E3573 + 2,
		0x004E35B5 + 2,
		0x004E3603 + 2,
		0x004E365A + 2,
		0x004E369F + 2,
		0x004E3704 + 2,
		0x004E3749 + 2,
		0x004E37A2 + 2,
		0x004E37E7 + 2,
		0x004E382C + 2,
		0x004E3878 + 2,
		0x004E38C5 + 2,
		0x004E3981 + 2,
		0x004E39D6 + 2,
		0x004E3A2D + 2,
		0x004E3A72 + 2,
		0x004E3AB7 + 2,
		0x004E3AFC + 2,
		0x004E3B53 + 2,
		0x004E3B98 + 2,
		0x004E3BE9 + 2,
		0x004E3C2E + 2,
		0x004E3C7E + 2,
		0x004E3CD9 + 2,
		0x004E3D30 + 2,
		0x004E3D6A + 2,
		0x004E79A5 + 2,
		0x004E7E64 + 2,
		0x004EA97A + 2,
		0x004EAA4A + 2,
		0x004EAAD2 + 2,
		0x004EAB3C + 2,
		0x004EAC5B + 2
	};


}
std::vector<u32> CCMUExtender::table_end_list(){
	return{0x0049E999 +1, 0x0049F39B+2, 0x004EAAA0 +1, 0x004EAB13+1, 0x004EAC35+1, 0x004ED075+2,0x004ED098+2};
}


std::vector<u32> CCMUExtender::get_address_list() {//for -1, start
	return{ 0x00402FD2 + 2, 0x004035DE +2, 0x004035FE+2, 0x0040361E+2, 0x0040363E+2,0x004037AC+2,0x0040383C+2,0x004038D9+2,0x0040397A+2,
		    0x00403A1B+2,
		0x00403CF8 + 2,
		0x00403D18 + 2,
		0x00403D38 + 2,
		0x00403D58 + 2,
		0x00403D78 + 2,
		0x00403D98 + 2,
		0x00404213 + 2,
		0x004221E1 + 2,
		0x00422301 + 2,
		0x00422421 + 2,
		0x00422541 + 2,
		0x00424C46 + 2,
		0x00424E77 + 2,
		0x00424F5C + 2,
		0x00425005 + 2,
		0x00425055 + 2,
		0x004250A6 + 2,
		0x004250F8 + 2,
		0x0043008A + 2,
		0x00430260 + 2,
		0x0043031D + 2,
		0x004303C1 + 2,
		0x0043046D + 2,
		0x004305B1 + 2,
		0x0043068D + 2,
		0x00430771 + 2,
		0x0043083D + 2,
		0x004309CE + 2,
		0x00430A3E + 2,
		0x00430AB1 + 2,
		0x00432822 + 2,
		0x00432A02 + 2,
		0x00438212 + 2,
		0x00448031 + 2,
		0x00448057 + 2,
		0x0044807D + 2,
		0x004480A3 + 2,
		0x004480C9 + 2,
		0x004480EF + 2,
		0x00448115 + 2,
		0x0044813B + 2,
		0x00448276 + 2,
		0x00448299 + 2,
		0x004482BC + 2,
		0x004482DF + 2,
		0x00448302 + 2,
		0x00448325 + 2,
		0x00448348 + 2,
		0x0044836B + 2,
		//0x0045CE75 + 2,
		0x0045F89D + 2,
		0x00469C32 + 2,
		0x00469CA2 + 2,
		0x00469D1D + 2,
		0x00469D8D + 2,
		0x00469DFD + 2,
		0x00469E6D + 2,
		0x0046B064 + 2,
		0x0046BC59 + 2,
		0x0046BCE9 + 2,
		0x0046BFAE + 2,
		0x00479E86 + 2,
		0x00479EA5 + 2,
		0x0047B21F + 2,
		0x0047B249 + 2,
		//0x0047D96B + 2,
		0x0048A8F6 + 2,
		0x0048A939 + 2,
		0x0048C78B + 2,
		0x0049159E + 2,
		0x0049163D + 2,
		0x00494C9A + 2,
		0x004953AF + 2,
		0x00495C4E + 2,
		0x00496621 + 2,
		0x00496890 + 2,
		0x00496A14 + 2,
		0x00496BAA + 2,
		//0x0049C60F + 2,
		//0x0049C767 + 2,
		0x0049FE57 + 2,
		//0x004A3790 + 2,
		//0x004A4E36 + 2,
		//0x004A4EFB + 2,
		//0x004A4FCF + 2,
		0x004BFB88 + 2,
		0x004C2506 + 2,
		0x004C2625 + 2,
		0x004C27B0 + 2,
		0x004C7EE5 + 2,
		0x004CEDCB + 2,
		0x004D566D + 2,
		0x004E2E8D + 2,
		0x004E2EAC + 2,
		0x004E2ECB + 2,
		0x004E2F12 + 2,
		0x004E2F34 + 2,
		0x004E2F62 + 2,
		0x004E2FE2 + 2,
		0x004E3007 + 2,
		0x004E3044 + 2,
		0x004E3069 + 2,
		0x004E309A + 2,
		0x004E30BF + 2,
		0x004E30E4 + 2,
		0x004E310F + 2,
		0x004E313B + 2,
		0x004E31C4 + 2,
		0x004E31F5 + 2,
		0x004E3223 + 2,
		0x004E3248 + 2,
		0x004E326D + 2,
		0x004E3292 + 2,
		0x004E32C2 + 2,
		0x004E32E7 + 2,
		0x004E3313 + 2,
		0x004E3338 + 2,
		0x004E3368 + 2,
		0x004E33A5 + 2,
		0x004E33D8 + 2,
		0x004E33F6 + 2,
		0x004E6C5A + 2,
		0x004E6CCC + 2,
		0x004E6D35 + 2,
		0x004E6DB4 + 2,
		0x004E70A4 + 2,
		0x004E713F + 2,
		0x004E71AE + 2,
		0x004E71F9 + 2,
		0x004E7245 + 2,
		0x004E7291 + 2,
		0x004E7922 + 2,
		0x004E8124 + 2,
		0x004E83A3 + 2,
		0x004E8440 + 2,
		0x004E84D9 + 2,
		0x004E8576 + 2,
		0x004EB1EF + 2,
		0x004EB390 + 2,
		//0x004A4E00+1,
		//0x004A4E74+1,
		//0x004A4F48+1,
	};
}


namespace hooks {

	void CCMUExtender() {
		new_unit_table.extend();
		//plugin code transfer notice: CCMU Extender also require CUnit.cpp (index) and load_unload_orders/load_unload_proc (0059cb58)
		//changes
	}
	void injectCCMUHooks() {
		CCMUExtender();
	}
}

CCMUExtender new_unit_table;