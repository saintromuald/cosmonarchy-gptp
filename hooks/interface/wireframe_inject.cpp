#include "wireframe.h"
#include <hook_tools.h>

namespace {
	void __declspec(naked) statdata_UnitWireframeUpdate_FragmentWrapper() {
		static const u32 JUMPBACK_POINT_WIREFRAME = 0x00456955;
		static Unit* unit;
		static u16 unit_id;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		unit_id = hooks::stat_data_unit_wireframe_update_fragment(unit);
		__asm {
			POPAD
			MOVZX ECX, unit_id
			JMP JUMPBACK_POINT_WIREFRAME
		}
	}

	void __declspec(naked) wireframe_shields_wrapper() {
		static Unit* unit;
		static u8 result;
		__asm {
			MOV unit, EDX
			PUSHAD
		}
		result = hooks::wireframe_shields(unit);
		__asm {
			POPAD
			MOV AL, result
			RETN
		}
	}
}

namespace hooks {
	void inject_wireframe_hooks() {
		// unused right now, must be updated for new unit ids if used
	//	jump_patch(statdata_UnitWireframeUpdate_FragmentWrapper,				0x00456911, 2);
		jump_patch(wireframe_shields_wrapper,						0x004566B0, 5);

	//	set_spell_special_button_graphic (0x00424BA0)
		memory_patch(0x00424CCE, (u16)0x0314); // Small Size Hallucination
		memory_patch(0x00424CE2, (u16)0x0214); // Small Size Hero
		memory_patch(0x00424CEC, (u8)0xA1);	   // Small Size Normal/Parasite; Change AL to AH.
		memory_patch(0x00424CF1, (u16)0x14B0); // MOV AL, 0x14;
		nops(0x00424CF3, 8);
		memory_patch(0x00424D2E, (u16)0x0316); // Large Size Hallucination
		memory_patch(0x00424D42, (u16)0x0216); // Large Size Hero
		memory_patch(0x00424D49, (u8)0xA1);	   // Large Size Normal/Parasite; Change AL to AH.
		memory_patch(0x00424D4E, (u16)0x16B0); // MOV AL, 0x16;
		nops(0x00424D50, 8);
		memory_patch(0x00424D8B, (u16)0x0315); // Medium Size Hallucination
		memory_patch(0x00424D9C, (u16)0x0215); // Medium Size Hero
		memory_patch(0x00424DA3, (u8)0xA1);	   // Medium Size Normal/Parasite; Change AL to AH.
		memory_patch(0x00424DA8, (u16)0x15B0); // MOV AL, 0x15;
		nops(0x00424DAA, 8);
	//	stat_data_unit_wireframe_select_update (0x00456F50)
		memory_patch(0x00456F70, (u16)0x0314); // Hallucination
		memory_patch(0x00456F86, (u16)0x0214); // Hero
		memory_patch(0x00456F8B, (u8)0xAE);    // Normal/Parasite; Change CL to CH.
		memory_patch(0x00456F90, (u16)0x14B1); // MOV CL, 0x14;
		nops(0x00456F92, 8);
	//	status_screen_draw_stat_button_border (0x00456D30)
		const u8 _data[] = { 0x66, 0x8B, 0x4F, 0x24, 0x51, 0x8B, 0xD7 }; // MOV CX, [EDI + 0x24]; PUSH ECX; MOV EDX, EDI;
		memory_patch((void*)0x00456D36, _data, 7);
		call_patch(remap_grpwire_button, 0x00456D3D);
		const u8 __data[] = { 0x59, 0xB5, 0x00 }; // POP ECX; MOV CH, 0;
		memory_patch((void*)0x00456D42, __data, 3);
	}
} //hooks