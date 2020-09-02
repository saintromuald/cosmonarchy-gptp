//for 4-byte flingies
#include "fake_unitid_extender.h"
namespace hooks {

	u32 get_flingy_ptr_val() {
		return scbw::get_aise_value(NULL, NULL, AiseId::GetFlingyPtr, 0, 0);
	}

	const u32 wrapper1Jmp = 0x00454317 + 7;
	const u32 wrapper2Jmp = 0x00463A71 + 7;
	const u32 wrapper3Jmp = 0x004683C6 + 7;
	const u32 wrapper4Jmp = 0x0047B854 + 7;
	const u32 wrapper5Jmp = 0x0047B8A4 + 7;
	const u32 wrapper6Jmp = 0x0047B8F4 + 7;
	const u32 wrapper7Jmp = 0x0048D756 + 7;
	const u32 wrapper8Jmp = 0x0048DA8A + 7;
	const u32 wrapper9Jmp = 0x00497148 + 7;
	const u32 wrapper10Jmp = 0x0049ED0B + 7;
	const u32 wrapper11Jmp = 0x004E9A10 + 7;

	void __declspec(naked) flingyWrapper1() {
		static u32 flingyPtr;
		static u32 regPtrOffset;
		__asm {
			MOV regPtrOffset, EAX
			PUSHAD
		}
		flingyPtr = get_flingy_ptr_val()+regPtrOffset;
		__asm {
			POPAD
			PUSH ESI
			MOV ESI,flingyPtr
			MOV EAX,[ESI]
			POP ESI
			JMP wrapper1Jmp
		}
	}
	void unitsDatWrapperPatches() {
		//TO DO: All wrapper patches
		//implement get_flingy_ptr_val() on aise side
		//data reading (get_aise_value) in aise
		//data rewriting in aise
		//use new data in gptp
		//enable everything in aise/gptp
		//test
		jmpPatch(flingyWrapper1, 0x00454317, 2);
		/*
	Line 125772: 00454317  |.  0FB680 F84466>MOVZX EAX,BYTE PTR DS:[EAX+6644F8] //EAX <- EAX //DONE
	Line 148264: 00463A71  |.  0FB68F F84466>MOVZX ECX,BYTE PTR DS:[EDI+6644F8] //ECX <- EDI
	Line 154775: 004683C6   .  0FB691 F84466>MOVZX EDX,BYTE PTR DS:[ECX+6644F8] //EDX <- ECX
	Line 184214: 0047B854  |.  0FB690 F84466>MOVZX EDX,BYTE PTR DS:[EAX+6644F8] //EDX <- EAX
	Line 184242: 0047B8A4  |.  0FB690 F84466>MOVZX EDX,BYTE PTR DS:[EAX+6644F8] //EDX <- EAX
	Line 184269: 0047B8F4   .  0FB688 F84466>MOVZX ECX,BYTE PTR DS:[EAX+6644F8] //ECX <- EAX
	Line 210943: 0048D756  |.  0FB688 F84466>MOVZX ECX,BYTE PTR DS:[EAX+6644F8] //ECX <- EAX
	Line 211212: 0048DA8A  |.  0FB688 F84466>MOVZX ECX,BYTE PTR DS:[EAX+6644F8] //ECX <- EAX
	Line 225181: 00497148  |.  0FB690 F84466>|MOVZX EDX,BYTE PTR DS:[EAX+6644F8] //EDX <- EAX
	Line 236925: 0049ED0B  |.  0FB683 F84466>MOVZX EAX,BYTE PTR DS:[EBX+6644F8] //EAX <- EBX
	Line 347273: 004E9A10   .  0FB682 F84466>MOVZX EAX,BYTE PTR DS:[EDX+6644F8] //EAX <- EDX
		*/

	}
}