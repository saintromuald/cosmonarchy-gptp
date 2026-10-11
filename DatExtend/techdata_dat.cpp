#include <BW/api.h>
#include <hook_tools.h>
#include "datext.h"

// bwdata modified by DatExt
DATEXT_VAR_REF(_bwTechs*, TechBw_var);

namespace {

	// List of address reference replacements.
	s32 techdata_r[] = {/* minerals */ 0,   0x00447B78 +4, 0x004486C9 +3, 0x0042CDD5 +4, 0x00453D3A +3, 0x00453D98 +3, 0x00453E4E +3, 0x00454B7F +4, 0x0042D249 +3, 0x00459618 +3, 0x004BF1F5 +1, 0x004BF1FA +1, 0x004BF30A +1, 0x004BF4D0 +1, 0x004CA77C +4, 0x004CA87C +4, 0x00436840 +4,  0, 0,
						/* gas */      0,   0x00447B89 +4, 0x004486D0 +3, 0x0042CDE4 +4, 0x00453D5B +3, 0x00453D9F +3, 0x00453E55 +3, 0x00454B92 +4, 0x0042D250 +3, 0x00459611 +3, 0x004BF214 +1, 0x004BF219 +1, 0x004BF31F +1, 0x004BF4E5 +1, 0x004CA789 +4, 0x004CA88C +4, 0x00436848 +4,  0, 0,
						/* time */     0,   0x00426749 +4, 0x00454C00 +4, 0x004BF233 +1, 0x004BF238 +1, 0x004BF334 +1, 0x004BF4FA +1, 0x004CA796 +4, 0x004CA899 +4,  0, 0,
						/* energy */   0,   0x004235B1 +3, 0x00423669 +4, 0x004595CB +4, 0x004595DF +4, 0x004595F6 +4, 0x0046F624 +4, 0x00491B8A +4, 0x00492714 +4, 0x00492929 +3, 0x0049E1D0 +4, 0x004A11F5 +4, 0x004BF252 +1, 0x004BF257 +1, 0x004BF349 +1, 0x004BF50F +1, 0x004CA79E +4, 0x004CA8A1 +4,  0,
									   2*4, 0x0043C6BF +2, 0x00464E57 +2, 0x00464FAF +2,  0,
									   2*21, 0x0045C600 +3, 0x0049450F +2,  0,
									   2*25, 0x0045C5AA +3,  0,
									   0,
						/* req 1 */    0,   0x0046D559 +4, 0x0046D57E +1, 0x0046DEB6 +4, 0x0046DF77 +4,  0,
									   44,  0x0046D590 +1,  0,
									   0,
						/* req 2 */    0,   0x0046D4E9 +4, 0x0046D50E +1, 0x0046DDAA +4, 0x0046DE59 +4,  0,
									   44,  0x0046D520 +1,  0,
									   0,
						/* icon */     0,   0x004267B0 +4,  0, 0,
						/* label */    0,   0x00457622 +4,  0, 0,
						/* race */     0,   0, 0,
						/* unused */   0,   0, 0,
						/* broodwar */ 0,   0, 0
	};
	
	// Techdata.dat tables
	const int TECH_ARR_COUNT = 2;
	void* tech_newArrs[TECH_ARR_COUNT] = {0};
	
			   // ID offset, ptr, ptr, ptr, ..., 0, ...
	s32 tech_bwAvail[] = {0,     0x004CCCF4 +1,  0,
						  0 -24, 0x004CB755 +3, 0x004CB781 +3, 0x004CB8B8 +3, 0x004CB8E4 +3, 0x004CE8CB/*function 1*/,  0,
						  0
	};
	
				  // ID offset, ptr, ptr, ptr, ..., 0, ...
	s32 tech_bwResearch[] = {0,     0x004CCD0E +1,  0,
							 0 -24, 0x004CB7AA +3, 0x004CB90D +3, 0x004CE87B/*function 2*/,  0,
							 0
	};
	
				 // factor,  add,  size,        ptr, ptr, ptr, ...,  0,  ...
	s32 tech_consts[] = {//1,  -24,  sizeof(u8),  0x004CB713 +2, 0x004CB876 +2,  0,
						 1,  -24,  sizeof(u32), 0x004CB712, 0x004CB86F, 0x004CE874/*function 2*/, 0x004CE8C4/*function 1*/,  0,
	/* (count-24)*12/4 */3,-3*24,  sizeof(u32), 0x004CCCEF +1, 0x004CCD09 +1,  0,
	  /* (count-24)*12 */12,-12*24,sizeof(u32), 0x004CB6FB +1, 0x004CB85C +1,  0,
						 1,    0,  sizeof(u8),  0x004548C0 +1, 0x004E2C4A +2,  0,
						 
					  // Code replacements:
					  // Replaces some sign-extended byte opcodes with dword opcodes
						 // fix SUB EDI,(count-24) at 0x004CB713
						 0, 0x648D05EB, 4,  0x004CB706,  0, // fix JMP, change 7-byte NOP to 4-byte NOP
					0, (s32)0x83900024, 4,  0x004CB70A,  0, // NOP / SUB ESI,24
					0, (s32)0xEF8118EE, 4,  0x004CB70E,  0, // SUB EDI,(count-24)
						 0, 0x4C,       1,  0x004CB7BD,  0, // JNZ 0x004CB70D
	
						 // fix SUB EDI,(count-24) at 0x004CB876
					0, (s32)0xEF81FF8B, 4,  0x004CB86B,  0, // NOP / SUB EDI,(count-24)
						 0, 0x2CEB,     2,  0x004CB877,  0, // SUB EBX,44
						 0, 0x49,       1,  0x004CB920,  0, // JNZ 0x004CB86D
	
					  // Replaces some LEA multiplication with more generalized multiplication
						 // function 1 -- eax = player, ecx = tech id -- returns pointer in eax
						 0, 0xC069,     2,  0x004CE8C2,  0, // IMUL EAX,EAX,(count-24)
						 0, 0x08848D,   3,  0x004CE8C8,  0, // LEA EAX,DWORD [EAX+ECX+bwAvail-24]
						 
						 // function 2 -- edx = player, ecx = tech id -- returns pointer in edx
						 0, 0xD269,     2,  0x004CE872,  0, // IMUL EDX,EDX,(count-24)
						 0, 0x11948D,   3,  0x004CE878,  0, // LEA EDX,DWORD [ECX+EDX+bwResearch-24]
						 0, 0xC3,       1,  0x004CE87F, 0x004CE8CF,  0, // RETN
						 
						 // calling these functions is generally of the form:
						 //   movzx ecx,cx
						 //   mov edx,eax
						 //   call function _
						 //   mov _l,[e_x] or [e_x],_l
						 // ... though there are tons of exceptions and special cases
						 0, 0xD08B,     2,  0x00432549, 0x00454908, 0x00454956, 0x0047B3E8, 0x0047B41A, 0x004CE867,  0, // mov edx,eax
						 0, 0xD18B,     2,  0x0046D761, 0x004E2C6C,  0, // mov edx,ecx
						 0, 0x008A,     2,  0x004CE8BC,  0, // mov al,[edx]
						 0, 0x0288,     2,  0x0047B426, 0x004CE849,  0, // mov [edx],al
						 0, 0x028A,     2,  0x0045490F, 0x0046D76B, 0x004CE86E,  0, // mov al,[edx]
						 0, 0x0A8A,     2,  0x00432550,  0, // mov cl,[edx]
						 0, 0x1088,     2,  0x004CE899,  0, // mov [edx],dl
						 0, 0x128A,     2,  0x0047B3EF, 0x004E2C76,  0, // mov dl,[edx]
						 0, 0x1A88,     2,  0x0045495D,  0, // mov [edx],bl
						 0, 0xC8B70F,   3,  0x0046D763, 0x004E2C6E,  0, // movzx ecx,ax
						 0, 0xC9B70F,   3,  0x00454905, 0x00454953, 0x004CE840, 0x004CE864, 0x004CE8B4,  0, // movzx ecx,cx
						 0, 0xB8,       1,  0x0047B421,  0, // mov eax,...
						 0, 1,          4,  0x0047B422,  0, //      ...1
						 0, 0x92,       1,  0x004CE843,  0, // xchg eax,edx
						 0, 0x90,       1,  0x00432552, 0x00454911, 0x0045495F, 0x0046D76D, 0x0047B3F1, 0x0047B428, 0x004CE84B, 0x004CE84C, 0x004CE870, 0x004E2C78,  0, // nop
					0, (s32)0x909090C3, 4,  0x004CE89B, 0x004CE8BE,  0, // retn / nop fill
						 
						 0, 0xE8,       1,  0x0043254B, 0x0045490A, 0x00454958, 0x0046D766, 0x0047B3EA, 0x0047B41C, 0x004CE844, 0x004CE869, 0x004CE894, 0x004CE8B7, 0x004E2C71,  0, // call ...
						 0, 0x004CE8C2 - 0x004CE899, 4, 0x004CE895,  0, // ... function 1
						 0, 0x004CE8C2 - 0x004CE8BC, 4, 0x004CE8B8,  0,
						 0, 0x004CE872 - 0x00432550, 4, 0x0043254C,  0, // ... function 2
						 0, 0x004CE872 - 0x0045490F, 4, 0x0045490B,  0,
						 0, 0x004CE872 - 0x0045495D, 4, 0x00454959,  0,
						 0, 0x004CE872 - 0x0046D76B, 4, 0x0046D767,  0,
						 0, 0x004CE872 - 0x0047B3EF, 4, 0x0047B3EB,  0,
						 0, 0x004CE872 - 0x0047B421, 4, 0x0047B41D,  0,
						 0, 0x004CE872 - 0x004CE849, 4, 0x004CE845,  0,
						 0, 0x004CE872 - 0x004CE86E, 4, 0x004CE86A,  0,
						 0, 0x004CE872 - 0x004E2C76, 4, 0x004E2C72,  0,
						 
						 0,0,0,0
	};
}

namespace DatExt {
	void techdata_dat_do_patch(u32 newCount) {
		u32 i, offs;
		dat_table_patch((DatLoad*)LoadTable::Techdata_Dat, techdata_r);
		// tech_bwAvail
		tech_newArrs[0] = malloc((newCount - 24) * PLAYER_COUNT);
		// - Null-terminated list of null-terminated lists of addresses
		// - First value in list is the image ID referenced
		for (i = 0; tech_bwAvail[i] != 0 || i == 0; i++)
		{
			if (tech_bwAvail[i] == ARRAY_END) 
			{
				offs = (u32)tech_newArrs[0] + newCount - 24;
				i++;
			}
			else
			{
				offs = (u32)tech_newArrs[0] + tech_bwAvail[i++]; // ID offset
			}
			i += simple_replace((u32*)&tech_bwAvail[i], offs);
		}

		// tech_bwResearch
		tech_newArrs[1] = malloc((newCount - 24) * PLAYER_COUNT);
		// - Null-terminated list of null-terminated lists of addresses
		// - First value in list is the image ID referenced
		for (i = 0; tech_bwResearch[i] != 0 || i == 0; i++)
		{
			if(tech_bwResearch[i] == ARRAY_END)
			{
				offs = (u32)tech_newArrs[1] + newCount - 24;
			i++;
			}
			else
			{
				offs = (u32)tech_newArrs[1] + tech_bwResearch[i++]; // ID offset
			}
			i += simple_replace((u32*)&tech_bwResearch[i], offs);
		}   
	
		const_replace(tech_consts, newCount);

		// Apply 'newArrs' to appropriate bwdata.h variables
		for (i = 0; i < PLAYER_COUNT; i++)
		{
			TechBw_var->is_enabled[i] = (u8*)(tech_newArrs[0]) + (newCount - 24) * i;
			TechBw_var->is_researched[i] = (u8*)(tech_newArrs[1]) + (newCount - 24) * i;
		}
	}

  
	void techdata_dat_unpatch() {
		for (int i = 0; i < TECH_ARR_COUNT; i++)
		{
			if (tech_newArrs[i] != NULL)
			{
			   free(tech_newArrs[i]);
				tech_newArrs[i] = NULL;
			}
		}
	}
}