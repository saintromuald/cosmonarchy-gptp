#include "SCBW\api.h"
#include "SCBW\scbwdata.h"
#include "hook_tools.h"

int maxhp(int id, int playerId) {
	int result = units_dat::MaxHitPoints[id];
	/*
	if(scbw::getUpgradeLevel(playerId,UpgradeId::Heatshield)){
		result+=15*256;
	}
	*/
	return result;
}
const u32 _0140b = 0x0040140B;
namespace hooks {
	//getMaxHpWrapper
	void __declspec(naked) getMaxHpWrapper() {
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id,EAX
			MOV unit,ECX
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX,hp
			JMP _0140b
		}
	}
	void __declspec(naked) isOnYellowHealthWrapper() {
		static u32 id;
		static CUnit* unit;
		static u32 hp;
		__asm {
			MOV id, EAX
			MOV unit, EDX
			PUSHAD
		}
		hp = maxhp(id, unit->playerId);
		__asm {
			POPAD
			MOV EAX, hp
			JMP _0140b
		}
	}
	void injectMaxHpHooks() {
		
		/*jmpPatch(getMaxHpWrapper, 0x00401404, 2);//done
		jmpPatch(isOnYellowHealthWrapper, 0x00402274, 2);//done
														
		jmpPatch(isOnBurningHealthWrapper, 0x004022C4, 2);//eax - eax
		jmpPatch(getMaxHealthWrapper, 0x0040272A, 2);//eax - eax
		jmpPatch(ssDrawHpEnergyWrapper, 0x004263F0, 2);//edi is id, eax is result
		jmpPatch(calculateBaseStrengthWrapper, 0x0043128A, 2);//eax,eax
		jmpPatch(aiRepairSomethingWrapper, 0x00433CB3, 2);//eax,eax
		jmpPatch(aiStimIfNeededWrapper, 0x00440030, 2);//eax,eax
		jmpPatch(unkWireRandomDraw56730_Wrapper, 0x00456734, 2);//eax,eax
		jmpPatch(unkWireRandomDraw567C0_Wrapper, 0x004567E7, 2);//eax,eax
		jmpPatch(startZergBuildingWrapper, 0x0045D392, 2);//MUL DWORD PTR DS:[EDX*4+662350]
		//add changes in building_morph, building morph hooks are required (0045D6A7, 0045D6A0)
		//add changes to nydus hooks nydus hooks are required (orders_Build5)
		jmpPatch(Ai_TryReturnHomeWrapper, 0x00462EEA, 2);////CMP EDX,DWORD PTR DS:[EAX*4+662350], wrapper is not be enough, hook may be needed
		jmpPatch(Ai_OrderHealWrapper, 0x00463584, 1);////CMP EAX,DWORD PTR DS:[EDI+662350], wrapper is not be enough, hook may be needed
		jmpPatch(doMedicWrapper, 0x00463CC8, 2);////ecx -> edx
		jmpPatch(getRepairInfoWrapper, 0x00466D6C, 2);////eax -> edi
		jmpPatch(setHpWrapper, 0x0046734A, 2);////eax -> eax
		//add changes to order_repair hook, it is requirement (0x004673ED, 004676BB)
		//add changes to orders_TerranBuildSelf, it is required (0x00467781, 0x004677C2, 0x0046780C, 0046785E) 
		jmpPatch(createGeyserUnitWrapper, 0x0046797E, 2);//MUL DWORD PTR DS:[ECX*4+662350]
		jmpPatch(canTargetOrderWrapper, 0x00474DF8, 1);//CMP EDX,DWORD PTR DS:[EAX+662350], hook may be required
		jmpPatch(canTargetOrderWrapper, 0x00474E56, 1);//CMP ECX,DWORD PTR DS:[EAX+662350], hook may be required
		jmpPatch(DrawImage_HpBar_Wrapper, 0x0047A877, 2);////edi -> eax
		jmpPatch(DrawImage_HpBar_Wrapper2, 0x0047A906, 2);////edi -> eax
		jmpPatch(DrawImage_HpBar_Wrapper3, 0x0047A877, 2);////edi -> eax
		jmpPatch(DrawImage_HpBar_Wrapper4, 0x0047A906, 2);////edi -> eax
		jmpPatch(setBuildHpWrapper, 0x0047B195, 2);////eax -> ecx
		jmpPatch(orderFollowWrapper, 0x0047C80D, 2);////ecx -> eax
		jmpPatch(getUnitBulletDamageWrapper, 0x0048AD3D, 2);////ecx -> eax
		jmpPatch(showFinishedUnitWrapper, 0x0049ECBF, 2);////ecx -> edx
		jmpPatch(transformUnit_Main_Wrapper, 0x0049F228, 2);////edx -> eax
		jmpPatch(transformUnit_Main_Wrapper2, 0x0049F23B, 2);////ecx -> ecx
		jmpPatch(unk_ai_subunit_Destructor_Wrapper, 0x004A0197, 2);////eax -> edx
		jmpPatch(updateUnitStatsFinishBuilding_Wrapper, 0x004A0203, 2);////eax -> ecx
		jmpPatch(initializeUnitBase_Wrapper, 0x004A0521, 0);////MUL DWORD PTR DS:[ECX*4+662350]
		jmpPatch(modifyUnitHp_Wrapper, 0x004C7B7D, 2);////eax -> ecx
		jmpPatch(unk_ccc4e_Wrapper, 0x004CCC4E, 3);////IMUL ECX,DWORD PTR DS:[EAX*4+662350]
		jmpPatch(editUnitFlags_Wrapper, 0x004CD493, 3);////IMUL ECX,DWORD PTR DS:[EAX*4+662350]
		jmpPatch(updateUnitDamageOverlay_Wrapper, 0x004E6096, 2);//eax->edi
		jmpPatch(ai_findTransport_Wrapper, 0x004E6F88, 2);//edi->eax
		//infestation hooks required, 0x004EA624 (morph into infested)
		jmpPatch(unk_ebdc8_Wrapper, 0x004EBDC8, 1);//CMP EDX,DWORD PTR DS:[ECX+662350], hook may be required
		//update unitState hook must be enabled (0x004EC37C)
		jmpPatch(ConvertToHallucination_Wrapper, 0x004F61E7, 2);//edx->ecx
		*/


		/*
	Line 462: 00401404  |.  8B0485 502366>MOV EAX,DWORD PTR DS:[EAX*4+662350]
	Line 1990: 00402274  |.  8B0485 502366>MOV EAX,DWORD PTR DS:[EAX*4+662350]
	Line 2018: 004022C4  |.  8B0485 502366>MOV EAX,DWORD PTR DS:[EAX*4+662350]
	Line 2492: 0040272A  |>  8B0485 502366>MOV EAX,DWORD PTR DS:[EAX*4+662350]
	Line 56687: 004263F0  |.  8B04BD 502366>MOV EAX,DWORD PTR DS:[EDI*4+662350]
	Line 72976: 0043128A  |>  8B0485 502366>MOV EAX,DWORD PTR DS:[EAX*4+662350]
	Line 77202: 00433CB3  |.  8B0485 502366>|MOV EAX,DWORD PTR DS:[EAX*4+662350]
	Line 94881: 00440030  |.  8B0485 502366>MOV EAX,DWORD PTR DS:[EAX*4+662350]
	Line 129075: 00456734  |.  8B0485 502366>MOV EAX,DWORD PTR DS:[EAX*4+662350]
	Line 129119: 004567E7  |.  8B0485 502366>MOV EAX,DWORD PTR DS:[EAX*4+662350]
	Line 138827: 0045D392  |.  F72495 502366>MUL DWORD PTR DS:[EDX*4+662350]
	Line 139053: 0045D6A0   .  8B0C9D 502366>MOV ECX,DWORD PTR DS:[EBX*4+662350]
	Line 139054: 0045D6A7   .  2B0C95 502366>SUB ECX,DWORD PTR DS:[EDX*4+662350]
	Line 139657: 0045DD23  |.  F72495 502366>MUL DWORD PTR DS:[EDX*4+662350]
	Line 147134: 00462EEA  |.  3B1485 502366>CMP EDX,DWORD PTR DS:[EAX*4+662350]
	Line 147784: 00463584  |.  3B87 50236600 CMP EAX,DWORD PTR DS:[EDI+662350]
	Line 148482: 00463CC8  |.  8B148D 502366>MOV EDX,DWORD PTR DS:[ECX*4+662350]
	Line 152848: 00466D6C  |.  8B3C85 502366>MOV EDI,DWORD PTR DS:[EAX*4+662350]
	Line 153423: 0046734A  |.  8B0485 502366>MOV EAX,DWORD PTR DS:[EAX*4+662350]
	Line 153477: 004673ED  |.  3B048D 502366>CMP EAX,DWORD PTR DS:[ECX*4+662350]
	Line 153708: 004676BB  |.  3B148D 502366>CMP EDX,DWORD PTR DS:[ECX*4+662350]
	Line 153768: 00467781  |.  F7248D 502366>MUL DWORD PTR DS:[ECX*4+662350]
	Line 153792: 004677C2  |.  8B0C85 502366>MOV ECX,DWORD PTR DS:[EAX*4+662350]
	Line 153819: 0046780C  |.  8B0485 502366>MOV EAX,DWORD PTR DS:[EAX*4+662350]
	Line 153851: 0046785E  |.  8B0C85 502366>MOV ECX,DWORD PTR DS:[EAX*4+662350]
	Line 153943: 0046797E  |.  F7248D 502366>MUL DWORD PTR DS:[ECX*4+662350]
	Line 173715: 00474DF8  |.  3B90 50236600 CMP EDX,DWORD PTR DS:[EAX+662350]
	Line 173736: 00474E56  |.  3B88 50236600 CMP ECX,DWORD PTR DS:[EAX+662350]
	Line 182621: 0047A877  |.  8B04BD 502366>MOV EAX,DWORD PTR DS:[EDI*4+662350]
	Line 182667: 0047A906  |.  8B04BD 502366>MOV EAX,DWORD PTR DS:[EDI*4+662350]
	Line 183550: 0047B195  |.  8B0C85 502366>MOV ECX,DWORD PTR DS:[EAX*4+662350]
	Line 185623: 0047C80D   .  8B048D 502366>MOV EAX,DWORD PTR DS:[ECX*4+662350]
	Line 206951: 0048AD3D  |.  8B048D 502366>MOV EAX,DWORD PTR DS:[ECX*4+662350]
	Line 236885: 0049ECBF  |.  8B148D 502366>MOV EDX,DWORD PTR DS:[ECX*4+662350]
	Line 237302: 0049F228  |.  8B0495 502366>MOV EAX,DWORD PTR DS:[EDX*4+662350]
	Line 237307: 0049F23B  |.  8B0C8D 502366>MOV ECX,DWORD PTR DS:[ECX*4+662350]
	Line 238650: 004A0197  |.  8B1485 502366>MOV EDX,DWORD PTR DS:[EAX*4+662350]
	Line 238690: 004A0203  |.  8B0C85 502366>MOV ECX,DWORD PTR DS:[EAX*4+662350]
	Line 238918: 004A0521  |.  F7248D 502366>MUL DWORD PTR DS:[ECX*4+662350]
	Line 296855: 004C7B7D  |.  8B0C85 502366>MOV ECX,DWORD PTR DS:[EAX*4+662350]
	Line 304302: 004CCC4E   .  0FAF0C85 5023>IMUL ECX,DWORD PTR DS:[EAX*4+662350]
	Line 304985: 004CD493  |.  0FAF0C85 5023>IMUL ECX,DWORD PTR DS:[EAX*4+662350]
	Line 341939: 004E6096  |.  8B3C85 502366>MOV EDI,DWORD PTR DS:[EAX*4+662350]
	Line 343262: 004E6F88  |.  8B04BD 502366>|MOV EAX,DWORD PTR DS:[EDI*4+662350]
	Line 348313: 004EA624  |>  8B8B 50236600 MOV ECX,DWORD PTR DS:[EBX+662350]
	Line 350557: 004EBDC8   .  3B91 50236600 CMP EDX,DWORD PTR DS:[ECX+662350]
	Line 351328: 004EC37C   .  3B81 50236600 CMP EAX,DWORD PTR DS:[ECX+662350]
	Line 365972: 004F61E7  |.  8B0C95 502366>|MOV ECX,DWORD PTR DS:[EDX*4+662350]

		*/
	}
}