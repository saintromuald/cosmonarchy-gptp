#pragma once
#include "SCBW/scbwdata.h"

namespace hooks
{
	s32 select_dual_birth_units(Unit* unit);				// 0045D040
	bool is_selected_unit_group_enabled();					// 0049A2C0
	char select_unit_initial();								// 0049B690

	void inject_selection_extender_hooks();
}

/*
Notes:
	We need to replace THREE arrays with extended ones, we are aiming for 18,
	but it should be possible to try pseudo "unlimited" selection once we are done.
	Easiest thing to do, without need for hooks and ultimate overhead of changing entire game logic,
	would be to have the arrays with fixed count and don't bother with dynamic arrays (vectors).
	Then most of the work would be to memorypatch pointer addresses with addresses of new arrays,
	I don't think there is need for hooks in most cases, making those would probably take ages
	we must also make sure that conditions check for 18 instead 12, so the new array is
	properly iterated on.

	I am not sure 100% sure about `All Player Selection Groups`, it is serialized within the
	game's logic, and that controls loading the selection, we should probably leave those
	(last few addresses in 3rd block) as they are, and serialize the extended ones using aise
	or forthcoming globals serializer.
	- Veeq7
*/

/*
 
	Hook xrefs to 0049AEF0 
 
	Current Selection Groups

	Line 138572: 0x0045D04D  |.  BE B8846200   MOV ESI,006284B8							// 0045D040 select_dual_birth_units; hooked by Pr0nogo

	Line 165630: 0x0046FA97  |.  A1 B8846200   MOV EAX,DWORD PTR DS:[6284B8]			// 0046FA40 get_selected_units_in_box; hooked in selection
	Line 165635: 0x0046FAA7  |.  BE B8846200   MOV ESI,006284B8							// 

	Line 165738: 0x0046FBAA  |.  A1 B8846200   MOV EAX,DWORD PTR DS:[6284B8]			// 0046FB40 unit_selection_click; hooked in selection
	Line 165803: 0x0046FC95  |.  BE B8846200   MOV ESI,006284B8							//
	Line 165837: 0x0046FCF6  |>  B9 B8846200   MOV ECX,006284B8							//

	Line 229198: 0x00499A70  |>  8B349D B88462>/MOV ESI,DWORD PTR DS:[EBX*4+6284B8]		// 00499A60 create_all_selection_circles; unhooked	--  old comment: memory patch address and cmp 12
	Line 229209: 0x00499A98  |.  89148D B88462>|MOV DWORD PTR DS:[ECX*4+6284B8],EDX

	Line 230030: 0x0049A2C6  |.  BA B8846200   MOV EDX,006284B8							// 0049A2C0 is_selected_unit_group_enabled; hooked by Pr0nogo

	Line 230073: 0x0049A328   .  BF B8846200   MOV EDI,006284B8							// 0049A320 very small function; https://pastebin.com/raw/jC5bW6LM

	Line 231207: 0x0049AE48   .  BE B8846200   MOV ESI,006284B8							// 0049AE40 create_new_unit_selections_from_list; hooked in selection
	Line 231229: 0x0049AE92   .  89349D B88462>MOV DWORD PTR DS:[EBX*4+6284B8],ESI		//
	Line 231238: 0x0049AEB6   .  890C85 B88462>MOV DWORD PTR DS:[EAX*4+6284B8],ECX		//
	Line 231242: 0x0049AEC6   >  8B149D B88462>MOV EDX,DWORD PTR DS:[EBX*4+6284B8]		//

	Line 231936: 0x0049B6A0  |>  8B349D B88462>/MOV ESI,DWORD PTR DS:[EBX*4+6284B8]		// 0049B690 select_unit_initial; unhooked
	Line 231960: 0x0049B6F7  |.  890C85 B88462>|MOV DWORD PTR DS:[EAX*4+6284B8],ECX		//

	Line 237841: 0x0049F7C0  |>  8B0C9D B88462>/MOV ECX,DWORD PTR DS:[EBX*4+6284B8]		// 0049F7A0 remove_from_client_selection_3; unhooked
	Line 237848: 0x0049F7D3  |.  BE B8846200   MOV ESI,006284B8							//

	Line 290742: 0x004C38B8   .  BE B8846200   MOV ESI,006284B8							// 004C38B0 update_selected_unit_data; unhooked
	Line 290990: 0x004C3B49  |.  B9 B8846200   MOV ECX,006284B8							//

	Line 318679: 0x004D603C  |.  8B3C8D B88462>MOV EDI,DWORD PTR DS:[ECX*4+6284B8]		// 004D6010 compile_health_bar; unhooked

	Line 342125: 0x004E627D   .  89148D B88462>MOV DWORD PTR DS:[ECX*4+6284B8],EDX		// 004E6260 very small function; https://pastebin.com/raw/40Xexh8u

	Line 355391: 0x004EED18  |.  BF B8846200   MOV EDI,006284B8							// 004EED10 load_game_core; unhooked
*/

/*
	Active Selection Group

	Line 306884: 0x004CE709   .  BF 7C6B5900   MOV EDI,00596B7C							// 004CE700 very small function; https://pastebin.com/raw/6kTCBkgS

	Line 309870: 0x004D083D  |.  BE 7C6B5900   MOV ESI,00596B7C							// 004D0820 lose_sight_selection; unhooked

	Line 355442: 0x004EEDDC  |.  BF 7C6B5900   MOV EDI,00596B7C							// 004EED10 load_game_core; unhooked
*/

/*
	All Player Selection Groups

	Line 224175: 0x004966D2  |.  81C3 E8846200 ADD EBX,006284E8							// 004965D0 command_save_hotkey_group; unhooked
	Line 224522: 0x00496A89  |.  890C85 E88462>|MOV DWORD PTR DS:[EAX*4+6284E8],ECX		//

	Line 229953: 0x0049A226  |.  8DB9 E8846200 LEA EDI,DWORD PTR DS:[ECX+6284E8]		// 0049A170 remove_from_selection_2; unhooked
	Line 229961: 0x0049A241  |.  C7048D E88462>MOV DWORD PTR DS:[ECX*4+6284E8],0		//

	Line 230048: 0x0049A303  |.  81FA E8846200 |CMP EDX,006284E8						// 0049A2C0 is_selected_unit_group_enabled; hooked by Pr0nogo

	Line 230076: 0x0049A334   .  BF E8846200   MOV EDI,006284E8							// not shown in ida, ?

	Line 230481: 0x0049A758  |.  81C7 E8846200 ADD EDI,006284E8							// 0049A740 clear_selection; unhooked
	Line 230490: 0x0049A770  |.  81C6 E8846200 ADD ESI,006284E8							//

	Line 230605: 0x0049A870  |>  8B04B5 E88462>/MOV EAX,DWORD PTR DS:[ESI*4+6284E8]		// 0049A850 get_active_player_first_selection; hooked in utils

	Line 231219: 0x0049AE75   .  81FE E8846200 CMP ESI,006284E8							// 0049AE40 create_new_unit_selections_from_list; hooked in selection

	Line 231352: 0x0049AFBB  |.  893485 E88462>MOV DWORD PTR DS:[EAX*4+6284E8],ESI		// 0049AF80 add_to_player_selection; unhooked

	Line 231974: 0x0049B728  |.  BF E8846200   MOV EDI,006284E8							// 0049B690 select_unit_initial; unhooked

	Line 289085: 0x004C265B  |.  05 E8846200   |ADD EAX,006284E8						// 004C2560 cmdrecv_shift_select; unhooked
	Line 289117: 0x004C26B6  |.  893495 E88462>|MOV DWORD PTR DS:[EDX*4+6284E8],ESI		//

	Line 289211: 0x004C27D9  |.  05 E8846200   |ADD EAX,006284E8						// 004C2750 cmdrecv_select; unhooked

	Line 289646: 0x004C2D22  |.  B8 E8846200   MOV EAX,006284E8							// 004C2910 cmdrecv_save_game; unhooked

	Line 291000: 0x004C3B62  |.  81F9 E8846200 |CMP ECX,006284E8						// 004C3B40 update_building_land_unit_selection; unhooked

	Line 307503: 0x004CEDA2  |.  BA E8846200   MOV EDX,006284E8							// 004CEDA0 unpack_all_player_selection_groups; unhooked

	Line 307541: 0x004CEE03  |.  BF E8846200   MOV EDI,006284E8							// 004CEE00 pack_all_player_selection_groups; unhooked

	Line 309288: 0x004D013E  |.  68 E8846200   PUSH 006284E8							// 004CFEF0 load_game_full; unhooked

	Line 309712: 0x004D068D  |.  B8 E8846200   MOV EAX,006284E8							// 004D02D0 large function; should check ghidra, not shown in ida; https://pastebin.com/raw/nbyBkBSP

	Line 355394: 0x004EED24  |.  BF E8846200   MOV EDI,006284E8							// 004EED10 load_game_core; unhooked
	Line 355440: 0x004EEDD1  |.  81C6 E8846200 ADD ESI,006284E8							//
	Line 355446: 0x004EEDEA  |.  BF E8846200   MOV EDI,006284E8							//
*/