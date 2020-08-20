#include "update_unit_state.h"
#include "update_status_effects.h"
#include <SCBW/api.h>
#include <algorithm>


namespace {
	//Helper function: Returns true if the unit's HP <= 33%.
	bool unitHpIsInRedZone(CUnit* unit);					//0x004022C0
	void RestoreAllUnitStats(CUnit* unit);					//0x00492F70 (hooked by hooks\update_status_effects.cpp)
	void setAllImageGroupFlagsPal11(CSprite* sprite);		//0x00497430
} //unnamed namespace


  //copied from recall_spell, replace later
/*
const u32 Func_DoWeaponDamage = 0x00479930;
void func_479930(CUnit* attacker, CUnit* target, u32 base_damage, 
				u32 direction, u32 dmg_divide, u32 weapon_id, u32 player) {
	_asm {
		PUSHAD
		MOV EAX, base_damage
		MOV EDI, target
		PUSH player
		PUSH attacker
		PUSH direction
		PUSH dmg_divide
		PUSH weapon_id
		CALL Func_DoWeaponDamage
		POPAD
	}
}

const u32 Func_CreateThingy = 0x00488210;
CThingy* function_createthingy(u32 spriteId, s16 x, s16 y, u32 playerId) {

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
*/
const u32 Func_RemoveOverlays2 = 0x00492370;
void removeoverlays2(u32 from, u32 to, CUnit* unit) {
	__asm {
		PUSHAD
		MOV EAX,from
		MOV EBX,unit
		PUSH to
		CALL Func_RemoveOverlays2
		POPAD
	}
}
#include "SCBW\UnitFinder.h"
#include "SCBW\api.h"
class wyvernMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CUnit* gunship;
public:
	wyvernMatch(CUnit* gunship)
		: gunship(gunship) {}

	bool match(CUnit* unit) {
		if (unit == gunship) {
			return false;
		}
		if (unit->status & UnitStatus::InAir) {
			return false;
		}
		if (scbw::isAlliedTo(gunship->playerId, unit->playerId)) {
			return false;
		}
		if (scbw::getDistanceFast(unit->sprite->position.x, unit->sprite->position.y,
			gunship->sprite->position.x, gunship->sprite->position.y) > weapons_dat::MaxRange[WeaponId::WyvernWeapon]) {
			return false;
		}
		return true;
	}
};
namespace hooks {

	/// This function regenerates energy for spellcasters and drains energy for
	/// cloaking units.
	/// Equivalent to function sub_4EB4B0 @ 0x004EB4B0

	void updateUnitEnergy(CUnit* unit) {
		
		//was hardcoded instead of function calling in original code
		if (
			unit->isValidCaster() &&				//If the unit is hallucination or not a spellcaster, don't regenerate energy
			(unit->status & UnitStatus::Completed)	//If the unit is not fully constructed, don't regenerate energy
			)
		{

			bool bStopHere = false;
			//Spend energy for cloaked units
			if (
				unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection)	//If the unit is cloaked
			&&	!(unit->status & UnitStatus::CloakingForFree)							//...and must consume energy to stay cloaked (i.e. not under an Arbiter)
				)
			{

				u16 cloakingEnergyCost;

				//Equivalent to getCloakingEnergyConsumption @ 0x00491A90
				if (unit->id == UnitId::TerranGhost
				|| unit->id == UnitId::Hero_SarahKerrigan
				|| unit->id == UnitId::Hero_InfestedKerrigan
				|| unit->id == UnitId::Hero_SamirDuran
				|| unit->id == UnitId::Hero_InfestedDuran
				|| unit->id == UnitId::Hero_AlexeiStukov
					)
					cloakingEnergyCost = 10;
				else
					if (unit->id == UnitId::TerranWraith
					|| unit->id == UnitId::Hero_TomKazansky
						)
						cloakingEnergyCost = 13;
					else
						cloakingEnergyCost = 0;

				if (!(unit->hasEnergy(cloakingEnergyCost)))
				{

					if (unit->secondaryOrderId == OrderId::Cloak)
						unit->setSecondaryOrder(OrderId::Nothing2); //Supposedly, immediately decloaks the unit.

					bStopHere = true;

				}

				if (!bStopHere)	//the function prevent energy consumption if THE GATHERING enabled
					unit->spendUnitEnergy(cloakingEnergyCost);

			}
			else if (unit->id == UnitId::ProtossDarkArchon && scbw::get_aise_value(unit,NULL,AiseId::SumMindControlCost,0,0)>0) {
				auto sum = scbw::get_aise_value(unit, NULL, AiseId::SumMindControlCost, 0, 0)/2;
				unit->energy -= std::min(unit->energy, (u16)(16 * sum));
				if (unit->energy == 0) {
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::FreeMindControlled, 0, 0, 0);
				}
			}
			else if (unit->id == UnitId::TerranAzazel && scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::SublimeShepherd, 0) == 1) {
				unit->energy -= std::min(unit->energy, (u16)13);
				if (unit->energy == 0) {
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::SublimeShepherd, 0, 0);
				}
			}
			else if (unit->id == UnitId::corsair && scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::DisruptionWeb, 0)) {
				unit->energy -= std::min(unit->energy, (u16)13);
				if (unit->energy == 0) {
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::DisruptionWeb, 0, 0);
				}
			}
			else { //EB526

				int maxEnergy;

				if (unit->id == UnitId::ProtossDarkArchon &&
					unit->mainOrderId == OrderId::CompletingArchonSummon &&
					unit->mainOrderState == 0
					)
					maxEnergy = 12800;  //50 * 256; Identical to energy amount on spawn
				else
					maxEnergy = unit->getMaxEnergy();

				if (unit->energy == maxEnergy)
					bStopHere = true;
				else {

					unit->energy += 8;

					//Omitting hardcoded recalculation of maxEnergy

					//Omitting recalculation of maxEnergy through
					//GetDarkArchonEnergy @ 0x00491C00 since it
					//calculate maxEnergy the same way the previous
					//code did, and nothing could have changed the
					//parameters used since the calculation
					if (unit->energy > maxEnergy)
						unit->energy = maxEnergy;

				}

			}

			//If the unit is currently selected, redraw its graphics
			if (!bStopHere && unit->sprite->flags & CSprite_Flags::Selected)
				setAllImageGroupFlagsPal11(unit->sprite);

		}

	}

	;

	void updateMineralPatchImage(CUnit* mineralPatch) {
		IscriptAnimation::Enum anim;
		if (mineralPatch->building.resource.resourceAmount >= 750)
			anim = IscriptAnimation::WorkingToIdle;
		else
			if (mineralPatch->building.resource.resourceAmount >= 500)
				anim = IscriptAnimation::AlmostBuilt;
			else
				if (mineralPatch->building.resource.resourceAmount >= 250)
					anim = IscriptAnimation::SpecialState2;
				else
					if (mineralPatch->building.resource.resourceAmount >= 50)
						anim = IscriptAnimation::SpecialState1;
					else
						anim = IscriptAnimation::Landing;
		if (anim != mineralPatch->building.resource.resourceIscript) {
			mineralPatch->building.resource.resourceIscript = anim;
			mineralPatch->sprite->playIscriptAnim(anim);
		}
	}



	const u32 Func_SetUnitPathing = 0x00401360;
	void setUnitPathing(CUnit* unit, u8 unkPathRelated) {

		__asm {
			PUSHAD
			MOV AL, unkPathRelated
			MOV ECX, unit
			CALL Func_SetUnitPathing
			POPAD
		}

	}
	;

	class matchPylon : public scbw::UnitFinderCallbackMatchInterface {
		u8 _playerId;
	public:
		matchPylon(u8 playerId) {
			playerId = _playerId;
		}

		bool match(CUnit* unit) {
			if (unit->playerId == _playerId
				&& unit->id == UnitId::pylon
				&& unit->status & UnitStatus::Completed) {
				return true;
			}
			return false;
		}
	};


	/// Updates unit timers, regenerates hp and shields, and burns down Terran buildings.
	/// Identical to function @ 0x004EC290
	void updateUnitStateHook(CUnit* unit) {

		//Timers


		if (unit->mainOrderTimer)
			unit->mainOrderTimer--;
		if (unit->groundWeaponCooldown)
			unit->groundWeaponCooldown--;
		if (unit->airWeaponCooldown)
			unit->airWeaponCooldown--;
		if (unit->spellCooldown)
			unit->spellCooldown--;
		if (unit->id == UnitId::TerranWyvern && scbw::get_aise_value(unit,NULL,AiseId::GenericValue,ValueId::ReverseThrust,0)==1
			&& unit->groundWeaponCooldown==0) {

			bool can_free_fire = true;
			if (unit->mainOrderId == OrderId::Attack1 || unit->mainOrderId == OrderId::Attack2) {
				if (unit->orderTarget.unit != NULL) {
					can_free_fire = false;
				}
			}
			if (can_free_fire) {
				auto range = weapons_dat::MaxRange[WeaponId::WyvernWeapon];
				s32 forwardOffset = weapons_dat::ForwardOffset[WeaponId::WyvernWeapon];
				auto x = unit->getX() + scbw::getPolarX(forwardOffset, unit->currentDirection1);
				auto y = unit->getY() + scbw::getPolarY(forwardOffset, unit->currentDirection1);
				
				unit->groundWeaponCooldown = weapons_dat::Cooldown[WeaponId::WyvernWeapon];
				bool seek_unit = true;
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::WyvernReverseLandgrabOn, 0)==1) {
					auto targ_x = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::WyvernReverseLandgrabX, 0);
					auto targ_y = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::WyvernReverseLandgrabY, 0);
					auto direction = scbw::getAngle(x, y, targ_x, targ_y);
					if (scbw::getDistanceFast(unit->position.x, unit->position.y, targ_x, targ_y) <= range) {
						seek_unit = false;
						scbw::artificalBulletFrom(WeaponId::WyvernWeapon, unit->playerId, unit, NULL, targ_x, targ_y);
					}
					else {
						scbw::sendGPTP_aise_cmd(unit, NULL, AiseId::GenericValue, ValueId::WyvernReverseLandgrabOn, 0, 0);
					}
				}
				if(seek_unit) {
					auto a = scbw::UnitFinder::getNearestTarget(
						unit->getX() - range, unit->getY() - range,
						unit->getX() + range, unit->getY() + range,
						unit, wyvernMatch(unit)
					);
					if (a != NULL) {
						auto direction = scbw::getAngle(x, y, a->getX(), a->getY());
						scbw::artificalBulletFrom(WeaponId::WyvernWeapon, unit->playerId, unit, a, a->position);						
					}
				}
				unit->updateSpeed();
			}

		}

		if (unit->id == UnitId::ProtossReaver) {
			CUnit* scarab = unit->carrier.inHangarChild;
			while (scarab != NULL) {
				if (scarab->id == UnitId::ProtossScarab) {
					if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::Relocators) && 
						!(scarab->status & UnitStatus::InAir)) {
						scarab->sprite->elevationLevel = 12;
					}
					scarab->shields = unit->shields;
				}
				scarab = scarab->interceptor.hangar_link.prev;
			}
		}
		if (unit->mainOrderId == OrderId::MiningMinerals) {
			if (unit->orderTarget.unit != NULL) {
				//add mining status, decrease the regeneration rate
				unit->orderTarget.unit->unusedTimer = 180;
			}
		}
		if (unit->_padding_0x132 & 0x10) {//signify/shaman update speed flag
			unit->_padding_0x132 &= ~0x10;
			unit->updateSpeed();
		}
		if (unit->_unknown_0x08C & 0x1) {
			unit->_unknown_0x08C &= ~0x1;
			u32 overlayImageId;
			if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
				overlayImageId = ImageId::MaelstromOverlay_Medium;
			else
				if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
					overlayImageId = ImageId::MaelstromOverlay_Large;
				else
					overlayImageId = ImageId::MaelstromOverlay_Small;
			if (!unit->getOverlay(overlayImageId)) {
				unit->sprite->createTopOverlay(overlayImageId,
					0, 0, 0);
				if (!scbw::get_aise_value(unit, NULL, AiseId::HasMalice, 0, 0) == 1){
					//unit->sprite->images.head->setRemapping(ColorRemapping::BExpl);
					//new maelstrom visual code
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetMaelstromOffset, 75, 0, 0);
				}
				else {
					//new maelstrom visual code
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetMaelstromOffset, 50, 0, 0);
				}
			}
		}
		
		if (unit->_unknown_0x08C & 0x2) {
			unit->_unknown_0x08C &= ~0x2;
			u32 overlayImageId;
			if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
				overlayImageId = ImageId::MaelstromOverlay_Medium;
			else
				if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
					overlayImageId = ImageId::MaelstromOverlay_Large;
				else
					overlayImageId = ImageId::MaelstromOverlay_Small;
			if (unit->getOverlay(overlayImageId)) {
				unit->removeOverlay(ImageId::MaelstromOverlay_Small, ImageId::MaelstromOverlay_Large);
			}
		}

		if (unit->_padding_0x132 & 0x20 && unit==playersSelections->unit[*ACTIVE_NATION_ID][0]) {
			unit->_padding_0x132 &= ~0x20;
			scbw::refreshConsole();
		}


		if (unit->id == UnitId::mineral_field_1 ||
			unit->id == UnitId::mineral_field_2 ||
			unit->id == UnitId::mineral_field_3 || 
			unit->id == UnitId::ResourceVespeneGeyser) {

				if (unit->unusedTimer > 0) {
					unit->unusedTimer--;
				}
				if (unit->_unknown_0x066 > 0) {
				unit->_unknown_0x066--;
				}
				else {
				if(unit->unusedTimer>0){
					unit->_unknown_0x066 = 72;
					if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::MineralSpecialRegenTimer, 0) > 0) {
						unit->_unknown_0x066 = 36;
					}
				}
				else {
					unit->_unknown_0x066 = 36;
					if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::MineralSpecialRegenTimer, 0) > 0) {
						unit->_unknown_0x066 = 18;
					}
				}
				
				
				//resource regeneration
				bool update_image = true;
				if (unit->building.resource.resourceAmount < 5000 && unit->id == UnitId::ResourceVespeneGeyser) {
					unit->building.resource.resourceAmount++;
					//scbw::printText("update vespene");
					scbw::refreshConsole();
				}
				else if (unit->building.resource.resourceAmount < 1500) {
					unit->building.resource.resourceAmount++;		
					updateMineralPatchImage(unit);
	
					scbw::refreshConsole();
				}
			}
		}

		//Shield regeneration
		if (units_dat::ShieldsEnabled[unit->id] != 0) {

			s32 maxShields = (s32)(units_dat::MaxShieldPoints[unit->id]) * 256;
			if (unit->id == UnitId::ProtossAugur) {
				unit->shields -= std::min(7, unit->shields);
			}
			else {
				if (unit->shields != maxShields && unit->id != UnitId::scarab) {
					unit->shields += 7;

					if (unit->shields > maxShields)
						unit->shields = maxShields;

					if (unit->sprite->flags & CSprite_Flags::Selected)  //If the unit is currently selected, redraw its graphics
						setAllImageGroupFlagsPal11(unit->sprite);

				}
			}
		}

		//Supposedly, allows Zerglings to attack after unburrowing without delay.
		if (unit->id == UnitId::ZergZergling || unit->id == UnitId::Hero_DevouringOne)
			if (unit->groundWeaponCooldown == 0)
				unit->orderQueueTimer = 0;

		//Clear the healing flag every frame
		unit->isBeingHealed = 0;

		//Update unit status effects (stim, maelstrom, plague, etc.)
		if (unit->status & UnitStatus::Completed || !(unit->sprite->flags & CSprite_Flags::Hidden)) {

			unit->cycleCounter++;

			if (unit->cycleCounter >= 8) {
				unit->cycleCounter = 0;
				RestoreAllUnitStats(unit); //use update_status_effects hook if available
			}

		}

		//Only for fully-constructed units and buildings
		if (unit->status & UnitStatus::Completed) {

			bool bStopHere = false;

			//HP regeneration
			if (
				units_dat::BaseProperty[unit->id] & UnitProperty::RegeneratesHP &&
				unit->hitPoints > 0 &&
				unit->hitPoints / 256 != unit->getMaxHpInGame()
				)
				unit->setHp(unit->hitPoints + 4);
			if (unit->defensiveMatrixHp != 0) {			
				if (
					unit->hitPoints > 0 &&
					unit->hitPoints / 256 != unit->getMaxHpInGame()
					) 
				{
					auto caster = scbw::get_aise_value(unit, NULL, AiseId::GetMatrixCasterPlayer, 0, 0);
					
					if (caster < 8) {						
						unit->setHp(unit->hitPoints + 53);//256 * 5 / 24 = 53.333..
					}
				}

			}

			//Update unit energy (energy regen/drain)
			//call StarCraft.exe+EB4B0		//sub_4EB4B0
			updateUnitEnergy(unit);

			//Recent order timer
			if (unit->recentOrderTimer != 0)
				unit->recentOrderTimer--;

			//Self-destruct timer
			if (unit->id == UnitId::ProtossAugur && unit->shields == 0) {
				unit->remove();
				//add final hour code later
				bStopHere = true;
			}
			if (unit->removeTimer != 0) {

				unit->removeTimer--;

				if (unit->removeTimer == 0) {
					unit->remove();
					bStopHere = true;
				}

			}
			
			if (unit->id == UnitId::ProtossAurora) {
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::LastOrdersInit, 0)==1) {
					auto distance = scbw::getDistanceFast(unit->position.x, unit->position.y,
						unit->orderTarget.pt.x, unit->orderTarget.pt.y);
					if (distance<32) {
						unit->status |= UnitStatus::Invincible;
						scbw::createBullet(121, unit, unit->position.x, unit->position.y, unit->playerId,
							unit->currentDirection1);
						unit->remove();
					}
				}
				
			}

			//Terran building burn-down
			if (!bStopHere) {

				RaceId::Enum raceId = unit->getRace();

				if (
					raceId != RaceId::Zerg &&
					raceId != RaceId::Protoss &&
					raceId == RaceId::Terran
					)

					//Check if the unit is a grounded or lifted building
					if (
						unit->status & UnitStatus::GroundedBuilding ||
						units_dat::BaseProperty[unit->id] & UnitProperty::FlyingBuilding)
					{

						//...whose current HP is less or equal to 33% of max HP
						if (unitHpIsInRedZone(unit))
							unit->damageHp(20, NULL, unit->lastAttackingPlayer, true);

					}

			} //if (unit->getRace() == RaceId::Terran)


		} //if (unit->status & UnitStatus::Completed) {

	};

	void incrementUnitScoresEx(int count, CUnit* unit, int increment_scores /*int updateMorphMergeUnitScore*/) { // 0x00488D50
		if (unit->status & UnitStatus::IsHallucination) {
			return;
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit) {
			return;
		}
		completed_unit_counts[unit->id].player[unit->playerId] += count;
		int race = unit->getRace();
		int supply = units_dat::SupplyProvided[unit->id];
		int supply_new = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::SupplyProvidedValue, 0);
		if (supply_new) supply = supply_new;
		scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::SupplyProvidedValue, supply, 0);
		raceSupply[race].provided[unit->playerId] += count * supply;
		if (units_dat::GroupFlags[unit->id].isFactory) {
			factoriesOwned[unit->playerId] += count;
		}
		if (units_dat::GroupFlags[unit->id].isBuilding) {
			buildingsOwned[unit->playerId] += count;
		}
		if (units_dat::GroupFlags[unit->id].isMen) {
			unitsProduced[unit->playerId] += count;
		}
		if (increment_scores!=0) {
			if (units_dat::GroupFlags[unit->id].isMen) {
				if (!scbw::isAnyMorphMorphResult(unit->id)) {
					unitsOwned[unit->playerId] += count;
				}
				unitScore[unit->playerId] += count;	
			}
			else if (units_dat::GroupFlags[unit->id].isBuilding) {
				if (!scbw::isZergBuildingMorphResult(unit->id)) {
					buildingsOwned[unit->playerId] += count;
				}
				buildingScore[unit->playerId] += count * units_dat::BuildScore[unit->id];
			}
		}
		if (completed_unit_counts[unit->id].player[unit->playerId] < 0) {
			completed_unit_counts[unit->id].player[unit->playerId] = 0;
		}
		/*unsigned __int16 v3; // dx
		GroupFlag groupFlag; // dl
		int unitId = unit->id; // esi
		int v6; // edx
		u32* v7; // eax
		int v8; // [esp+0h] [ebp-8h]
		unsigned __int8 playerID; // [esp+7h] [ebp-1h]
		
		if (!(unit->status & UnitStatus::IsHallucination)) {
			v3 = unitId;
			if (!(unit->isSubunit())) {
				playerID = unit->playerId;
				v6 = playerID + 12 * v3;
				completed_unit_counts[0].player[v6] += count;
				groupFlag = units_dat::GroupFlags[(unsigned __int16)unit->id];

				u8 supply_provided_amount;
				//if (scbw::get_aise_value(unit, NULL, AiseId::GenericSwitch, ValueId::SupplyProvidedOverwritten, 0)) {
				//	supply_provided_amount = count * scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::SupplyProvidedValue, 0);
				//} else {
					supply_provided_amount = count * units_dat::SupplyProvided[unitId];
				//}

				if (groupFlag.isZerg) {
					suppliesAvailableZerg[playerID] += supply_provided_amount;
				}
				else if (groupFlag.isTerran) {
					suppliesAvailableTerran[playerID] += supply_provided_amount;
				}
				else if (groupFlag.isProtoss) {
					suppliesAvailableProtoss[playerID] += supply_provided_amount;
				}
				if (groupFlag.isFactory)
					factoriesOwned[playerID] += count;
				if (groupFlag.isMen) {
					unitsProduced[playerID] += count;
				}
				else if (groupFlag.isBuilding) {
					structuresBuilt[playerID] += count;
				}
				if (updateMorphMergeUnitScore) {
					if (playerID != 11)
					{
						if (groupFlag.isMen)
						{
							switch (unitId)
							{
							case UnitId::ZergGuardian:
							case UnitId::ZergDevourer:
							case UnitId::ProtossDarkArchon:
							case UnitId::ProtossArchon:
							case UnitId::ZergLurker:
								unitScore[playerID] += count * (unsigned __int16)units_dat::BuildScore[(unsigned __int16)unit->id];
								break;
							default:
								unitsOwned[playerID] += count;
								v7 = &unitScore[playerID];
								*v7 += count * (unsigned __int16)units_dat::BuildScore[(unsigned __int16)unit->id];
								break;
							}
						}
						else if (groupFlag.isBuilding)
						{
							switch (unitId)
							{
							case UnitId::ZergLair:
							case UnitId::ZergHive:
							case UnitId::ZergGuardianRoost:
							case UnitId::ZergSporeColony:
							case UnitId::ZergSunkenColony:
								buildingScore[playerID] += count * (unsigned __int16)units_dat::BuildScore[(unsigned __int16)unit->id];
								break;
							default:
								buildingsOwned[playerID] += count;
								v7 = &buildingScore[playerID];
								*v7 += count * (unsigned __int16)units_dat::BuildScore[(unsigned __int16)unit->id];
								break;
							}
						}
					}
				}
				if ((completed_unit_counts[0].player[v6] & 0x80000000) != 0)
					completed_unit_counts[0].player[v6] = 0;
			}
		}*/
	}
} //hooks

namespace {

	/**** Helper function definitions. Do not modify anything below this! ****/

	const u32 Helper_UnitHpIsInRedZone = 0x004022C0;
	bool unitHpIsInRedZone(CUnit* unit) {

		static Bool32 result;

		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Helper_UnitHpIsInRedZone
			MOV result, EAX
			POPAD
		}

		return result != 0;
	}

	;

	//hooked by hooks\update_status_effects.cpp
	const u32 Func_RestoreAllUnitStats = 0x00492F70;
	void RestoreAllUnitStats(CUnit* unit) {

		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RestoreAllUnitStats
			POPAD
		}

	}

	;

	//Identical to setAllImageGroupFlagsPal11 @ 0x00497430;
	void setAllImageGroupFlagsPal11(CSprite* sprite) {

		for (
			CImage* current_image = sprite->images.head;
			current_image != NULL;
			current_image = current_image->link.next
			)
		{
			if (current_image->paletteType == PaletteType::RLE_HPFLOATDRAW)
				current_image->flags |= CImage_Flags::Redraw;
		}

	}

	;

}

