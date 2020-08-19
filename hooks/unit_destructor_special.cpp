//All functions in this file are heavily involved with memory management.
//Do NOT modify anything unless you really know what you are doing.

#include "unit_destructor_special.h"
#include "../SCBW/api.h"
#include "../hook_tools.h"
#include "hooks/weapons/wpnspellhit.h"
#include "logger.h"
//Identical to 00466170  sub_466170
//Not hooked but used by hooked unitDestructorSpecialHook
#include <SCBW/UnitFinder.h>

class customPlagueProc_2 : public scbw::UnitFinderCallbackProcInterface
	{
	private:
	CUnit* defiler;

	public:
	customPlagueProc_2(CUnit* defiler)
		: defiler(defiler) {}
	void proc(CUnit* unit)
	{
		if(unit->plagueTimer>0){
			unit->_unknown_0x026 |=0x4;
		}
	}
};


void killAllHangarUnits(CUnit* unit) {

	u32 random_value;

	while(unit->carrier.inHangarCount != 0) {
		CUnit* childInside = unit->carrier.inHangarChild;
		unit->carrier.inHangarChild = childInside->interceptor.hangar_link.prev;
		childInside->interceptor.parent = NULL;
		GPTP::logger << "Removehangar -" << std::endl;
		childInside->remove();
		unit->carrier.inHangarCount--;
	}

	while (unit->carrier.outHangarCount != 0) {

		CUnit* childOutside = unit->carrier.outHangarChild;

		childOutside->interceptor.parent = NULL;

		//Kill interceptors only (Scarabs will defuse anyway)
		if (childOutside->id != UnitId::scarab) {

			u16 randomDeathTimer;

			//hardcoded RNG management
			if(*IS_IN_GAME_LOOP) {

				static u32* randomnessCounter_0051C6AC = (u32*)0x0051C6AC;
				static u32* randomnessCounter_0051CA18 = (u32*)0x0051CA18;

				*randomnessCounter_0051C6AC = *randomnessCounter_0051C6AC + 1;
				*randomnessCounter_0051CA18 = *randomnessCounter_0051CA18 + 1;
				*lastRandomNumber = (*lastRandomNumber * 0x015A4E35) + 1;

				//number between 0 and 32766
				random_value = (*lastRandomNumber / 65536) & 0x7FFF;

			}
			else
				random_value = 0;

			//should produce a number between 15 and 45 (or maybe 46)
			randomDeathTimer = ( (random_value * 31) / 32768 ) + 15;

			if (
				childOutside->removeTimer == 0 || 
				randomDeathTimer < childOutside->removeTimer
			)
				childOutside->removeTimer = randomDeathTimer;

		}

		//66239
		unit->carrier.outHangarChild = childOutside->interceptor.hangar_link.prev;
		childOutside->interceptor.hangar_link.prev = NULL;
		childOutside->interceptor.hangar_link.next = NULL;
		unit->carrier.outHangarCount--;

	} //while (unit->carrier.outHangarCount != 0)

	unit->carrier.outHangarChild = NULL;

}


//Identical to 00468770 sub_468770
//Not hooked but used by hooked unitDestructorSpecialHook
void freeResourceContainer(CUnit* resource) {

	resource->building.resource.gatherQueueCount = 0;

	CUnit* worker = resource->building.resource.nextGatherer;
	CUnit* nextWorker;

	while (worker != NULL) {

		if (worker->worker.harvest_link.prev != NULL) //untie the previous worker from current worker (tie it to next)
			(worker->worker.harvest_link.prev)->worker.harvest_link.next = worker->worker.harvest_link.next;
		else
			resource->building.resource.nextGatherer = worker->worker.harvest_link.next;

		if (worker->worker.harvest_link.next != NULL) //untie the next worker from current worker (tie it to previous)
			(worker->worker.harvest_link.next)->worker.harvest_link.prev = worker->worker.harvest_link.prev;

		nextWorker = worker->worker.harvest_link.next;

		worker->worker.harvestTarget = NULL;
		worker->worker.harvest_link.prev = NULL;
		worker->worker.harvest_link.next = NULL;

		worker = nextWorker;

	}

}




const u32 Func_CreateThingy = 0x00488210;
CThingy* createThingyFunc(u32 spriteId, s16 x, s16 y, u32 playerId) {

	static CThingy* thingy;
	s32 x_ = x;

	__asm {
		PUSHAD
		PUSH playerId
		MOVSX EDI, y
		PUSH x_
		PUSH spriteId
		CALL Func_CreateThingy
		MOV thingy, EAX
		POPAD
	}

	return thingy;

}

;


const u32 Func_GetAllUnitsInBounds = 0x0042FF80;
CUnit** getAllUnitsInBounds_copy(Box16* coords) {

	static CUnit** units_in_bounds;

	__asm {
		PUSHAD
		MOV EAX, coords
		CALL Func_GetAllUnitsInBounds
		MOV units_in_bounds, EAX
		POPAD
	}

	return units_in_bounds;

}

;

//Identical to setAllImageGroupFlagsPal11 @ 0x00497430;
void setAllImageGroupFlagsPal11(CSprite* sprite) {

	for(
		CImage* current_image = sprite->images.head; 
		current_image != NULL;
		current_image = current_image->link.next
	)
	{
		if(current_image->paletteType == PaletteType::RLE_HPFLOATDRAW)
			current_image->flags |= CImage_Flags::Redraw;
	}

}

;

const u32 Func_SetThingyVisibilityFlags = 0x004878F0;
bool setThingyVisibilityFlagsFunc(CThingy* thingy) {

	static Bool32 bPreResult;

	__asm {
		PUSHAD
		MOV ESI, thingy
		CALL Func_SetThingyVisibilityFlags
		MOV bPreResult, EAX
		POPAD
	}

	return (bPreResult != 0);

}
;


const u32 Func_IterateUnitsAtLocationTargetProc = 0x004E8280;
void IterateUnitsAtLocationTargetProc_EnsnareFunc(CUnit* attacker, Box16* coords) {

	const u32 EnsnareProc_function = 0x004F46C0;

	__asm {
		PUSHAD
		PUSH attacker
		MOV EAX, coords
		MOV EBX, EnsnareProc_function
		CALL Func_IterateUnitsAtLocationTargetProc
		POPAD
	}

}
void IterateUnitsAtLocationTargetProc_Plague(CUnit* attacker, Box16* coords) {

	const u32 PlagueProc_function = 0x004F4AF0;

	__asm {
		PUSHAD
		PUSH attacker
		MOV EAX, coords
		MOV EBX, PlagueProc_function
		CALL Func_IterateUnitsAtLocationTargetProc
		POPAD
	}

}


//Hooked down there instead of a specific inject.cpp
void unitDestructorSpecialHook(CUnit* unit) {
	if(unit->irradiateTimer > 0){
		if(unit->_unknown_0x026 & 0x2){
				//scbw::printText("Radiation Shockwave detected");
				unit->_unknown_0x026 &= ~0x2;
				//radiation shockwave

				CThingy* emp_sprite;
				emp_sprite = createThingyFunc(371, unit->position.x, unit->position.y, 0);
				if (emp_sprite != NULL) {
					emp_sprite->sprite->elevationLevel = 19; //0x13
					setThingyVisibilityFlagsFunc(emp_sprite);
				}				

				static u16* const maxBoxRightValue =			(u16*) 0x00628450;
				static u16* const maxBoxBottomValue =			(u16*) 0x006284B4;	
				Box16 area_of_effect;	
				CUnit** unitsInAreaOfEffect;
				CUnit* current_unit;	
				area_of_effect.left = (u16)unit->sprite->position.x -  weapons_dat::InnerSplashRadius[WeaponId::EMP_Shockwave];
				area_of_effect.right = (u16)unit->sprite->position.x +  weapons_dat::InnerSplashRadius[WeaponId::EMP_Shockwave];
				area_of_effect.top = (u16)unit->sprite->position.y -  weapons_dat::InnerSplashRadius[WeaponId::EMP_Shockwave];
				area_of_effect.bottom = (u16)unit->sprite->position.y +  weapons_dat::InnerSplashRadius[WeaponId::EMP_Shockwave];
				if(area_of_effect.left < 0)
					area_of_effect.left = 0;
				else
				if(area_of_effect.right > *maxBoxRightValue)
					area_of_effect.right = *maxBoxRightValue;
		
				//check and fix effect if beyond height of map
				if(area_of_effect.top < 0)
					area_of_effect.top = 0;
				else
				if(area_of_effect.bottom > *maxBoxBottomValue)
					area_of_effect.bottom = *maxBoxBottomValue;
				unitsInAreaOfEffect = getAllUnitsInBounds_copy(&area_of_effect);
				current_unit = *unitsInAreaOfEffect;	
				while(current_unit != NULL) {
					if(
						current_unit != unit->irradiatedBy &&				//EMP doesn't affect the attacker
						(
							unit->irradiatedBy == NULL ||					//EMP doesn't affect the attacker
							current_unit != unit->irradiatedBy->subunit	//subunit
						)
					)
					{
		
						if(current_unit->status & UnitStatus::IsHallucination)
							current_unit->remove();
						else
						if(current_unit->stasisTimer == 0) { //don't work against units in stasis
		
							current_unit->energy = 0;
							current_unit->shields = 0;
		
							if(
								current_unit->sprite->flags & CSprite_Flags::Selected &&
								current_unit->sprite != NULL
							) 
								setAllImageGroupFlagsPal11(current_unit->sprite);
		
						}
		
					}
		
					unitsInAreaOfEffect++;					//go on next unit of the list (or null)
					current_unit = *unitsInAreaOfEffect;
		
				} 
				*tempUnitsListArraysCountsListLastIndex = *tempUnitsListArraysCountsListLastIndex - 1;
				*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsList[*tempUnitsListArraysCountsListLastIndex];
		}
		else {
				//scbw::printText("Radiation Shockwave is not detected");
		}
	}

	// Parasite - spawning broodlings on death
	if (unit->parasiteFlags != 0) {
		int supplyCount = scbw::getBroodlingCountFromSupply(unit);
		for (int player = 0; player < PLAYER_COUNT; player++) {
			if (unit->parasiteFlags & (1 << player) && scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::ParasiteArmTimer, 0) <= 1) {
				scbw::spawnBroodlings(supplyCount, player, unit->position.x, unit->position.y, 720, scbw::getSourceQueens(unit)[1]);
			}
		}
	}
	
	if (unit->id==UnitId::infested_terran) {
		Box16 coords;
		CThingy* ensnare_effect_sprite;
		ensnare_effect_sprite = createThingyFunc(SpriteId::Ensnare, unit->position.x, unit->position.y, 0);
		if (ensnare_effect_sprite != NULL) {
			ensnare_effect_sprite->sprite->elevationLevel = 19; //0x13
			setThingyVisibilityFlagsFunc(ensnare_effect_sprite);
		}
		coords.top = unit->position.y - 64;
		coords.right = unit->position.x + 64;
		coords.left = unit->position.x - 64;
		coords.bottom = unit->position.y + 64;
		IterateUnitsAtLocationTargetProc_EnsnareFunc(unit, &coords);
	}

	//defiler plague upgrade (unknown id), add condition later
	if ((unit->_unknown_0x026 & 0x4) && unit->plagueTimer>0) {
		scbw::createBullet(WeaponId::Plague, unit, unit->position.x, unit->position.y, unit->playerId, 
			unit->currentDirection1);	
		Box16 coords;
		coords.left = unit->position.x - 48;
		coords.bottom = unit->position.y + 48;
		coords.top = unit->position.y -48;
		coords.right = unit->position.x + 48;
		IterateUnitsAtLocationTargetProc_Plague(unit, &coords);
		customPlagueProc_2 UpgPlague = customPlagueProc_2(unit);
		u32 range = 48;
		scbw::UnitFinder unitFinder(unit->getX() - range,
			unit->getY() - range,
			unit->getX() + range,
			unit->getY() + range);
		unitFinder.forEach(UpgPlague);
		//add finder later to trace plague flag
	}
	//Destroy interceptors and scarabs
	if (
		unit->id == UnitId::ProtossCarrier || 
		unit->id == UnitId::Hero_Gantrithor || 
		unit->id == UnitId::ProtossReaver || 
		unit->id == UnitId::Hero_Warbringer
	) 
	{
		killAllHangarUnits(unit);
	}
	else

	//Destroy nuclear missiles mid-launch
	if (
		unit->id == UnitId::ghost || 
		unit->id == UnitId::sarah_kerrigan || 
		unit->id == UnitId::Hero_AlexeiStukov|| 
		unit->id == UnitId::Hero_SamirDuran|| 
		unit->id == UnitId::Hero_InfestedDuran
	) 
	{

		if (unit->building.ghostNukeMissile != NULL) {
			(unit->building.ghostNukeMissile)->sprite->playIscriptAnim(IscriptAnimation::Death,true);
			unit->building.ghostNukeMissile = NULL;
		} 

	}
	else

	//Is a harvestable mineral patch or gas building
	if (
		(unit->id >= UnitId::ResourceMineralField && unit->id <= UnitId::ResourceMineralFieldType3) ||
		(	(unit->id == UnitId::TerranRefinery || unit->id == UnitId::ProtossAssimilator || unit->id == UnitId::ZergExtractor)
			&& unit->status & UnitStatus::Completed
		)	/*this test is equivalent to function unit_isGeyserUnit @ 004688B0*/
	  ) 
	{
		freeResourceContainer(unit);
	}
	else

	if(unit->id >= UnitId::TerranNuclearMissile && unit->id <= UnitId::ProtossPylon) {

		//In original code, this part is handled by an internal id_based array and a
		//switch-like system to execute the code corresponding to the unit
		//If the id were within the previous test, the resource units would go through
		//those tests

		//Is a scarab or interceptor
		if (unit->id == UnitId::ProtossScarab || unit->id == UnitId::ProtossInterceptor) {

			if (unit->status & UnitStatus::Completed) {

				//following code is equivalent to a jump to function 004652A0
				//with unit as parameter in EAX

				if (unit->interceptor.parent != NULL) {

					if (unit->interceptor.hangar_link.next != NULL) //untie the next interceptor/scarab from current interceptor/scarab (tie it to previous)
						(unit->interceptor.hangar_link.next)->interceptor.hangar_link.prev = unit->interceptor.hangar_link.prev;
					else {
						if (unit->interceptor.isOutsideHangar)
							unit->interceptor.parent->carrier.outHangarChild = unit->interceptor.hangar_link.prev;
						else
							unit->interceptor.parent->carrier.inHangarChild = unit->interceptor.hangar_link.prev;
					}

					if (unit->interceptor.isOutsideHangar)
						unit->interceptor.parent->carrier.outHangarCount--;
					else
						unit->interceptor.parent->carrier.inHangarCount--;

					if (unit->interceptor.hangar_link.prev != NULL)  //untie the previous interceptor/scarab from current interceptor/scarab (tie it to next)
						(unit->interceptor.hangar_link.prev)->interceptor.hangar_link.next = unit->interceptor.hangar_link.next;

				}
				else {
					unit->interceptor.hangar_link.prev = NULL;
					unit->interceptor.hangar_link.next = NULL;
				}

			}

		}
		else

		//Is a Nuclear Silo
		if (unit->id == UnitId::TerranNuclearSilo) {

			if (unit->building.silo.nuke != NULL) {
				(unit->building.silo.nuke)->remove();
				unit->building.silo.nuke = NULL;
			}

		}
		else



		//Is a Nuclear Missile
		if (unit->id == UnitId::TerranNuclearMissile) {

			if (unit->connectedUnit != NULL && unit->connectedUnit->id == UnitId::TerranNuclearSilo) {
				(unit->connectedUnit)->building.silo.nuke = NULL;
				(unit->connectedUnit)->building.silo.isReady = false;
			}

		}
		else

		//Is a Protoss Pylon
		if (unit->id == UnitId::ProtossPylon) {

			//code from hooks::removePsiField(CUnit* unit)
			//copied because the content is hardcoded here

			if (unit->building.pylonAura != NULL) {
				unit->building.pylonAura->free();	//should be equivalent to SpriteDestructor @ 00497B40
				unit->building.pylonAura = NULL;
			}

			//begin hardcoding removeFromPsiProviderList @ 00493100 to avoid use of a helper

			if (unit->psi_link.prev != NULL)  //untie the previous pylon from current pylon (tie it to next)
				(unit->psi_link.prev)->psi_link.next = unit->psi_link.next;
			if (unit->psi_link.next != NULL)  //untie the next pylon from current pylon (tie it to previous)
				(unit->psi_link.next)->psi_link.prev = unit->psi_link.prev;

			if (unit == *firstPsiFieldProvider)
				*firstPsiFieldProvider = unit->psi_link.next;

			unit->psi_link.prev = NULL;
			unit->psi_link.next = NULL;

			//end hardcoding removeFromPsiProviderList @ 00493100 to avoid use of a helper

			*canUpdatePoweredStatus = true;

		}
		else
		//Is a Nydus Canal
		if (unit->id == UnitId::ZergNydusCanal) {
			CUnit* nydusExit = unit->building.nydusExit;
			if (nydusExit != NULL) {
				unit->building.nydusExit = NULL;
				nydusExit->building.nydusExit = NULL;
				nydusExit->remove();
			}
		}
	}
}

//-------- Actual hooking --------//

void __declspec(naked) unitDestructorSpecialWrapper() {

	static CUnit* unit;

	__asm {
		MOV unit, EAX
		PUSHAD
	}

	unitDestructorSpecialHook(unit);

	__asm {
		POPAD
		RETN
  }
}

namespace hooks {

void injectUnitDestructorSpecial() {
	jmpPatch(unitDestructorSpecialWrapper, 0x0049F4A0, 2);
}

} //hooks
