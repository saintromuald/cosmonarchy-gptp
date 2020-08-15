#include "update_status_effects.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "hooks/weapons/wpnspellhit.h"

class irradiateProc : public scbw::UnitFinderCallbackProcInterface {

	private:
		CUnit* irradiatedUnit;

	public:
		irradiateProc(CUnit* irradiatedUnit)
			: irradiatedUnit(irradiatedUnit) {}

	void proc(CUnit* unit) {
		//Damage organic units only
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Organic))
			return;

		//Don't damage buildings
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Building)
			return;

		//Don't damage larvae, eggs, and lurker eggs
		if (unit->id == UnitId::larva || unit->id == UnitId::egg || unit->id == UnitId::lurker_egg)
			return;

		//Irradiate splash damage does not affect burrowed units
		if (unit != irradiatedUnit && unit->status & UnitStatus::Burrowed)
			return;

		//Check if the unit is within distance, or is inside the same transport
		if (irradiatedUnit->status & UnitStatus::InTransport
			|| irradiatedUnit->getDistanceToTarget(unit) <= 32)
		{
//			const s32 damage = weapons_dat::DamageAmount[WeaponId::Irradiate] * 256 / weapons_dat::Cooldown[WeaponId::Irradiate];
			s32 damage = (125*256)/24;
			if(irradiatedUnit!=NULL){
				if(irradiatedUnit->irradiateTimer==0){
					damage += (125*256)%24;
					//scbw::printText("Add additional damage");
				}
			}
			unit->damageWith(damage, WeaponId::Irradiate, irradiatedUnit->irradiatedBy, irradiatedUnit->irradiatePlayerId);
		}
		}
};


class sublimeShepherdProc : public scbw::UnitFinderCallbackProcInterface {

private:
	CUnit* shepherd;

public:
	sublimeShepherdProc(CUnit* shepherd)
		: shepherd(shepherd) {}

	void proc(CUnit* unit) {
		if (unit == shepherd) {
			return;
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Building)
			return;
		scbw::sendGPTP_aise_cmd(shepherd, unit, GptpId::SetSublimeShepherd, 1, 0, 0);
	}
};

class resourceBoostProc : public scbw::UnitFinderCallbackProcInterface {
private:
	CUnit* booster;

public:
	resourceBoostProc(CUnit* booster)
		: booster(booster) {}

	void proc(CUnit* unit) {
		if (booster == unit) {
			return;
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::ResourceContainer) {
			scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericTimer, ValueId::MineralSpecialRegenTimer, 24, 0);
		}
	}
};
class othstolethProc : public scbw::UnitFinderCallbackProcInterface {
private:
	CUnit* booster;

public:
	othstolethProc(CUnit* booster)
		: booster(booster) {}

	void proc(CUnit* unit) {
		if (booster == unit) {
			return;
		}
		if (scbw::isAlliedTo(unit->playerId, booster->playerId) && scbw::isZergEgg(unit->id,true,false)) {
			scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericTimer, ValueId::EggBoostTimer, 24, 0);
		}
	}
};

namespace hooks {

//Applies Irradiate effects for @p unit (which is Irradiated)
void doIrradiateDamage(CUnit* irradiatedUnit) {
	//Default StarCraft behavior

	irradiateProc irradiation=irradiateProc(irradiatedUnit);

	//No splash if burrowed
	if (irradiatedUnit->status & UnitStatus::Burrowed) {
		irradiation.proc(irradiatedUnit);
	}
	//If inside a transport, damage all units loaded within
	else if (irradiatedUnit->status & UnitStatus::InTransport) {

		CUnit* transport = irradiatedUnit->connectedUnit;

		if (transport != NULL) {
			for (int i = 0; i < units_dat::SpaceProvided[transport->id]; ++i) {
				CUnit* loadedUnit = transport->getLoadedUnit(i);
				if (loadedUnit)
			irradiation.proc(loadedUnit);
			}
		}

	}
	//Find and iterate nearby units
	else {
		scbw::UnitFinder unitFinder(irradiatedUnit->getX() - 160,
									irradiatedUnit->getY() - 160,
									irradiatedUnit->getX() + 160,
									irradiatedUnit->getY() + 160);
		unitFinder.forEach(irradiation);
	}
}

//Hook function for UpdateStatusEffects() (AKA RestoreAllUnitStats())
//Note: This function is called every 8 ticks (when unit->cycleCounter reaches 8 == 0)


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



//00469B60 = DoUnitsCollide(), ecx Unit *first, eax Unit *second

const u32 Func_DoUnitsCollide = 0x00469B60;
bool func_dounitscollide(CUnit* first, CUnit* second){
	static u32 result = 0;
	__asm {
		PUSHAD
		MOV EAX,first
		MOV ECX,second
		CALL Func_DoUnitsCollide
		MOV result, EAX
		POPAD
	}
	return result;
}


class safetyUnburrow : public scbw::UnitFinderCallbackMatchInterface {

	CUnit* burrower;

public:
	//Check if @p unit is suitable unburrow target.
	bool match(CUnit* target) {
		if (target->status & UnitStatus::GroundedBuilding == 0) {
			return false;
		}
		if (target->id == UnitId::BasiliskCreeper) {
			return false;
		}
		if (target == burrower) {
			return false;
		}
		if (!func_dounitscollide(burrower, target)) {
			return false;
		}

		return true;
	}

	//Constructor
	safetyUnburrow(CUnit* burrower) : burrower(burrower) {}
};

const u32 Func_Sub_4E97C0 = 0x004E97C0;
void unburrow(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub_4E97C0
		POPAD
	}

}

;
void updateStatusEffects(CUnit* unit) {

	// Decrement Stasis Field timer (and remove effect if 0)
	if (unit->stasisTimer != 0) {

		unit->stasisTimer--;

		if (unit->stasisTimer == 0)
			unit->removeStasisField();
	}
	
	// Update speed based on upgrades or status effects

	// Passenger Insurance: Dropship acceleration
	if (unit->id == UnitId::TerranDropship && scbw::getUpgradeLevel(unit->playerId, UpgradeId::PassengerInsurance)) {
		unit->updateSpeed();
	}

	// Ion Propulsion: Vulture slow while over impassable terrain
	else if (unit->id == UnitId::TerranVulture && scbw::getUpgradeLevel(unit->playerId, UpgradeId::IonPropulsion)) {
		unit->updateSpeed();
	}

	// Knockout Drivers: Southpaw slow per stack
	else if (scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::KnockoutDriversValue, 0) > 0) {
		unit->updateSpeed();
	}

#ifdef DELIVERANCE
	// Astral Tempest: Ecclesiast movespeed on Astral Blessing proc
	else if (scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::AstralTempestStacks, 0) > 0) {
		unit->updateSpeed();
	}
#endif

	// Forcibly unburrow units that collide with buildings
	if (unit->status & UnitStatus::Burrowed) {
		auto tile = scbw::getActiveTileAt(unit->position.x, unit->position.y);
		if (tile.currentlyOccupied) {
			safetyUnburrow Unburrow = safetyUnburrow(unit);
			CUnit* nearest = scbw::UnitFinder::getNearestTarget(unit->getLeft()-32,
				unit->getTop()-32,
				unit->getRight()+32,
				unit->getBottom()+32, unit, Unburrow);
			if (nearest != NULL) {
				unburrow(unit);
			}
		}		
	}

	// Azazel - Sublime Shepherd ability
	if (unit->id == UnitId::TerranAzazel) {
		if (scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::SublimeShepherd, 0) == 1) {
			sublimeShepherdProc Heal = sublimeShepherdProc(unit);
			scbw::UnitFinder unitFinder(unit->getX() - 96,
				unit->getY() - 96,
				unit->getX() + 96,
				unit->getY() + 96);
			unitFinder.forEach(Heal);
		}
	}

	// Quarry and Iroleth resource regeneration aura
	if (unit->sprite!=NULL && unit->mainOrderId!=OrderId::Die 
		&& (unit->id == UnitId::TerranQuarry || unit->id == UnitId::ZergIroleth || unit->id==UnitId::ZergOthstoleth)) {
		
		u32 radius = unit->id == UnitId::TerranQuarry ? 320 : 224;
		resourceBoostProc Boost = resourceBoostProc(unit);
		scbw::UnitFinder unitFinder(unit->getX() - radius,
			unit->getY() - radius,
			unit->getX() + radius,
			unit->getY() + radius);
		unitFinder.forEach(Boost);
	}
	if (unit->id == UnitId::ZergOthstoleth) {
		u32 radius = 224;
		othstolethProc eggBoost = othstolethProc(unit);
		scbw::UnitFinder unitFinder(unit->getX() - radius,
			unit->getY() - radius,
			unit->getX() + radius,
			unit->getY() + radius);
		unitFinder.forEach(eggBoost);
	}

	// Ghost - Somatic Implants upgrade
	if (unit->id == UnitId::TerranGhost) {
		if (
			!(unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection)))
		{
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::SomaticImplants) && 
				scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::SomaticImplants, 0) == 1) {

				unit->updateSpeed();
				scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::SomaticImplants, 0, 0);
			}
		}
	}

	// 
	if (unit->stimTimer != 0) {

		unit->stimTimer--;

		if (unit->stimTimer == 0)
			unit->updateSpeed();
	}

	// 
	if (unit->id == UnitId::ZergVorvaling || unit->id==UnitId::TerranShaman || unit->id==UnitId::ZergOverlord) {
		unit->updateSpeed();
	}

	// Decrement Ensnare timer (and remove effect if 0)
	if (unit->ensnareTimer != 0) {

		unit->ensnareTimer--;

		if (unit->ensnareTimer == 0) {
			unit->removeOverlay(ImageId::EnsnareOverlay_Small, ImageId::EnsnareOverlay_Large);
			unit->updateSpeed();
		}

	}

	// Shaman - Overcharge upgrade
	if (unit->unusedTimer != 0) {
		//overcharge timer
		unit->unusedTimer--;
		if(unit->unusedTimer==0){
			unit->updateSpeed();
		}

	}

	// Decrement Defensive Matrix timer (and remove effect if 0)
	if (unit->defensiveMatrixTimer != 0) {

		unit->defensiveMatrixTimer--;

		if (unit->defensiveMatrixTimer == 0)
			unit->reduceDefensiveMatrixHp(unit->defensiveMatrixHp);

	}

	// Redraw console/UI elements
	if (unit == *activePortraitUnit) {
		if (unit->isValidCaster()) {
			if (!scbw::isAlliedTo(*LOCAL_NATION_ID, unit->playerId)) {
				scbw::refreshConsole();
			}
		}
	}
	
	// Irradiate ability and Radiation Shockwave upgrade
	if (unit->irradiateTimer != 0) {
		char buf[32];
		//sprintf(buf,"Irradiate timer: %d",unit->irradiateTimer);
		//scbw::printText(buf);
		unit->irradiateTimer--;

		doIrradiateDamage(unit);	//assumed to work, not checked in details

		if (unit->irradiateTimer == 0) {
			if(unit->_unknown_0x026 & 0x2){
				unit->_unknown_0x026 &= ~0x2;
				//radiation shockwave
				//scbw::printText("Radiation Shockwave detected");

				CThingy* emp_sprite;
				emp_sprite = createThingyFunc(371, unit->position.x, unit->position.y, 0);
				if (emp_sprite != NULL) {
					emp_sprite->sprite->elevationLevel = 19; //0x13
					scbw::setThingyVisibilityFlags(emp_sprite);
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

			unit->removeOverlay(ImageId::Irradiate_Small, ImageId::Irradiate_Large);
			unit->irradiatedBy = NULL;
			unit->irradiatePlayerId = 8;
		}

	}

	if (unit->lockdownTimer != 0) {

		unit->lockdownTimer--;

		if (unit->lockdownTimer == 0)
			unit->removeLockdown();

	}

	if (unit->maelstromTimer != 0) {
		unit->maelstromTimer--;
		if (unit->maelstromTimer == 0)
		{
			unit->removeMaelstrom();
			if (scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::Malediction, 0)) {
				scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::Malediction, 0, 0);
				scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericTimer, ValueId::MaledictionSlow, (24*4)+1, 0);
				u32 overlayImageId;
				if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
					overlayImageId = ImageId::MaelstromOverlay_Medium;
				else
					if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
						overlayImageId = ImageId::MaelstromOverlay_Large;
					else
						overlayImageId = ImageId::MaelstromOverlay_Small;
				unit->sprite->createTopOverlay(overlayImageId,
					0, 0, 0);
				//new maelstrom visual code
				//scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetMaelstromOffset, 25, 0, 0);
//				unit->sprite->images.head->setRemapping(ColorRemapping::GFire);
			}
			unit->updateSpeed();
		}

	}
	

	if (unit->plagueTimer != 0) {
		unit->plagueTimer--;
		if (unit->plagueTimer <= 45) {
			unit->plagueTimer = 0;
		}
		if (!(unit->status & UnitStatus::Invincible)) {

			//s32 damage = (weapons_dat::DamageAmount[WeaponId::Plague] << 8) / 76;

			s32 damage = (weapons_dat::DamageAmount[WeaponId::Plague]*256)/30;
			
			/*
			if (true) {//defiler plague upgrade (unknown id), add condition later
				unit->_padding_0x132 |= 0x40;
			}*/

			if (unit->plagueTimer == 0) {

				damage += (weapons_dat::DamageAmount[WeaponId::Plague] * 256)%30;
			}
			/*
			//original bw plague code, disabled
			s32 damage = weapons_dat::DamageAmount[WeaponId::Plague];
			//randomize?
			__asm {
				PUSHAD
				MOV ECX, damage
				SHL ECX, 0x08
				MOV EAX, 0x6BCA1AF3
				IMUL ECX
				SAR EDX, 0x05
				MOV EAX, EDX
				SHR EAX, 0x1F
				ADD EAX, EDX
				MOV damage, EAX
				POPAD
			}*/
			if (unit->hitPoints > damage) 
			{
				if (unit->shields >= 256 && units_dat::ShieldsEnabled[unit->id]) {
					if (unit->shields > damage) {
						unit->shields -= damage;
						damage = 0;
					}
					else {
						damage -= unit->shields;
						unit->shields = 0;
					}
					unit->sprite->createOverlay(ImageId::ShieldOverlay, 0,0, 0);
				}
				if (damage > 0) {
					unit->damageHp(damage, NULL, -1, true);
				}
				
				
				


				//defiler plague upgrade  - add requirement later
				/*				
				if (unit->plagueTimer == 0 && unit->_padding_0x132 & 0x40) {
					unit->_padding_0x132 &= ~0x40;
				}*/
			}
			else {
				//defiler plague upgrade  - add requirement later
				/*
				if (true) {//defiler plague upgrade (unknown id), add condition later
					unit->remove();
				}*/
			}

		}

		if (unit->plagueTimer == 0){
			unit->removeOverlay(ImageId::PlagueOverlay_Small, ImageId::PlagueOverlay_Large);
			if(unit->_unknown_0x026 & 0x4){
				unit->_unknown_0x026 &= ~0x4;
			}
		}

	}

	if (unit->isUnderStorm != 0)
		unit->isUnderStorm--;

	//hardcoding effect of unreferenced_sub_4F42C0 @ 0x004F42C0
	u8 previousAcidSporeCount = unit->acidSporeCount;
	for (int i = 0; i <= 8; ++i) {

		if (unit->acidSporeTime[i] != 0) {

			unit->acidSporeTime[i]--;

			if (unit->acidSporeTime[i] == 0)
				unit->acidSporeCount--;

		}

	}
	if (unit->acidSporeCount != 0) {

		//Calculate the appropriate overlay ID
		u32 acidOverlayId = std::min(3, unit->acidSporeCount / 2)
							+ 4 * scbw::getUnitOverlayAdjustment(unit)
							+ ImageId::AcidSpores_1_Overlay_Small;

		if (!unit->getOverlay(acidOverlayId)) {

			unit->removeOverlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);

			if (unit->subunit != NULL)
				unit = unit->subunit;

			unit->sprite->createTopOverlay(acidOverlayId);

		}

	}
	else if (previousAcidSporeCount != 0) {
		unit->removeOverlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
	}

}

} //hooks
