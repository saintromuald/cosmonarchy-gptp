#include "update_unit_state.h"
#include "update_status_effects.h"
#include <SCBW/api.h>
#include <algorithm>
#include "hooks/weapons/weapon_damage.h"
#include "hooks/weapon_effect.h"
#include "hooks/hydra_unit_ai.h"
#include "globals.h"
namespace {

	bool unitHpIsInRedZone(CUnit* unit);						//0x004022C0
	void RestoreAllUnitStats(CUnit* unit);						//0x00492F70 (hooked by hooks\update_status_effects.cpp)
	void setAllImageGroupFlagsPal11(CSprite* sprite);			//0x00497430

	// For AI_Burrower
	void techUseAllowed(CUnit* a1, int16 a2, int32 a3);			//0046DD80

	// For irradiate function
	CThingy* createThingy(uint32 spriteId, int16 x, int16 y, uint32 playerId);
	bool setThingyVisibilityFlags(CThingy* thingy);
	CUnit** getAllUnitsInBounds(Box16* coords);
}

const uint32 Func_RemoveOverlays2 = 0x00492370;
void removeoverlays2(uint32 from, uint32 to, CUnit* unit) {
	__asm {
		PUSHAD
		MOV EAX, from
		MOV EBX, unit
		PUSH to
		CALL Func_RemoveOverlays2
		POPAD
	}
}

#include "SCBW\UnitFinder.h"
#include "SCBW\api.h"

namespace hooks {

	/// This function regenerates energy for spellcasters and drains energy for cloaking units.
	/// Equivalent to function sub_4EB4B0 @ 0x004EB4B0

	void updateUnitEnergy(CUnit* unit) {
		//was hardcoded instead of function calling in original code
		if (unit->isValidCaster()
			&& (unit->status & UnitStatus::Completed)) {
			bool bStopHere = false;
			//Spend energy for cloaked units
			if (
				unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection)		// If the unit is cloaked
				&& !(unit->status & UnitStatus::CloakingForFree)							// ...and must consume energy to stay cloaked (i.e. not under an Arbiter)
				) {
				uint16 cloakingEnergyCost;
				//Equivalent to getCloakingEnergyConsumption @ 0x00491A90
				if (unit->id == UnitId::TerranEidolon
					|| unit->id == UnitId::Hero_SarahKerrigan
					|| unit->id == UnitId::Hero_InfestedKerrigan
					|| unit->id == UnitId::Hero_SamirDuran
					|| unit->id == UnitId::Hero_InfestedDuran
					|| unit->id == UnitId::Hero_AlexeiStukov
					) {
					cloakingEnergyCost = 10;
				} else
					if (unit->id == UnitId::TerranWraith) {
						cloakingEnergyCost = 13;
					} else {
						cloakingEnergyCost = 0;
					}

				if (!(unit->hasEnergy(cloakingEnergyCost))) {
					if (unit->secondaryOrderId == OrderId::Cloak) {
						unit->setSecondaryOrder(OrderId::Nothing2); // Supposedly, immediately decloaks the unit.
					}
					bStopHere = true;
				}
				if (!bStopHere) { // the function prevent energy consumption if THE GATHERING enabled
					unit->spendUnitEnergy(cloakingEnergyCost);
				}
			} else
				if (unit->id == UnitId::ProtossMindTyrant && scbw::get_aise_value(unit, nullptr, AiseId::SumMindControlCost, 0, 0) > 0) {
					auto sum = scbw::get_aise_value(unit, nullptr, AiseId::SumMindControlCost, 0, 0) / 2;
					unit->spendUnitEnergy(std::min(unit->energy, (uint16)(16 * sum)));
					if (unit->energy == 0) {
						scbw::sendGPTP_aise_cmd(unit, nullptr, GptpId::FreeMindControlled, 0, 0, 0);
					}
				} else
					if (unit->id == UnitId::TerranAzazel && scbw::get_generic_rust_value(unit, ValueId::SublimeShepherd) == 1) {
						unit->spendUnitEnergy(std::min(unit->energy, (uint16)13));
						if (unit->energy == 0) {
							scbw::set_generic_rust_value(unit, ValueId::SublimeShepherd, 0);
						}
					} else
						if (unit->id == UnitId::corsair && scbw::get_generic_rust_value(unit, ValueId::DisruptionWeb)) {
							unit->spendUnitEnergy(std::min(unit->energy, (uint16)13));
							if (unit->energy == 0) {
								scbw::set_generic_rust_value(unit, ValueId::DisruptionWeb, 0);
							}
						}

						else { //EB526
							if (unit == nullptr) {
								return;
							}
							int maxEnergy;
							if (unit->id == UnitId::ProtossMindTyrant
								&& unit->mainOrderId == OrderId::CompletingArchonSummon
								&& unit->mainOrderState == 0) {
								maxEnergy = 12800;  //50 * 256; Identical to energy amount on spawn
							} else {
								maxEnergy = unit->getMaxEnergy();
							}
							bool phobosCondition = false;
							if (unit->id == UnitId::TerranPhobos && scbw::get_generic_value(unit, ValueId::PhobosDischarge) > 0) {
								unit->spendUnitEnergy(426); // (200 * 256) / (5 * 24)  ~ 426.66
								if (unit->energy < 426) {
									unit->energy = 0;
									scbw::set_generic_value(unit, ValueId::PhobosDischarge, 0);
								}
								phobosCondition = true;
							}
							if (unit->energy == maxEnergy && !phobosCondition) {
								bStopHere = true;
							} else {
								unit->addUnitEnergy(8, maxEnergy);
							}
						}

					//If the unit is currently selected, redraw its graphics
					if (!bStopHere && unit->sprite->flags & CSprite_Flags::Selected) {
						setAllImageGroupFlagsPal11(unit->sprite);
					}
		}
	}

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

	const uint32 Func_SetUnitPathing = 0x00401360;
	void setUnitPathing(CUnit* unit, uint8 unkPathRelated) {
		__asm {
			PUSHAD
			MOV AL, unkPathRelated
			MOV ECX, unit
			CALL Func_SetUnitPathing
			POPAD
		}
	}

	/// Updates unit timers, regenerates hp and shields, and burns down Terran buildings.
	/// Identical to function @ 0x004EC290
	void updateUnitStateHook(CUnit* unit) {
		if (unit->pAI != nullptr) {
			hydra_unit_ai(unit);
		}
		//Timers
		if (unit->mainOrderTimer) {
			unit->mainOrderTimer--;
		}
		if (unit->groundWeaponCooldown) {
			unit->groundWeaponCooldown--;
		}
		if (unit->airWeaponCooldown) {
			unit->airWeaponCooldown--;
		}
		if (unit->spellCooldown) {
			unit->spellCooldown--;
		}
		if (unit->id == UnitId::TerranWyvern && scbw::get_generic_rust_value(unit, ValueId::ReverseThrust) == 1
			&& unit->groundWeaponCooldown == 0) {
			bool can_free_fire = true;
			if (unit->mainOrderId == OrderId::Attack1 || unit->mainOrderId == OrderId::Attack2) {
				if (unit->orderTarget.unit != nullptr) {
					can_free_fire = false;
				}
			}
			if (can_free_fire) {
				auto range = weapons_dat::MaxRange[WeaponId::WyvernJacksonCannon];
				int32 forwardOffset = weapons_dat::ForwardOffset[WeaponId::WyvernJacksonCannon];
				auto x = unit->getX() + scbw::getPolarX(forwardOffset, unit->currentDirection1);
				auto y = unit->getY() + scbw::getPolarY(forwardOffset, unit->currentDirection1);

				unit->groundWeaponCooldown = weapons_dat::Cooldown[WeaponId::WyvernJacksonCannon];
				bool seek_unit = true;
				if (scbw::get_generic_value(unit, ValueId::WyvernReverseLandgrabOn) == 1) {
					auto targ_x = scbw::get_generic_value(unit, ValueId::WyvernReverseLandgrabX);
					auto targ_y = scbw::get_generic_value(unit, ValueId::WyvernReverseLandgrabY);
					auto direction = scbw::getAngle(x, y, targ_x, targ_y);
					if (scbw::getDistanceFast(unit->position.x, unit->position.y, targ_x, targ_y) <= range) {
						seek_unit = false;
						scbw::artificalBulletFrom(WeaponId::WyvernJacksonCannon, unit->playerId, unit, nullptr, targ_x, targ_y);
					} else {
						scbw::set_generic_value(unit, ValueId::WyvernReverseLandgrabOn, 0);
					}
				}
				if (seek_unit) {
					auto a = scbw::UnitFinder::getNearestTarget(
						unit->getX() - range, unit->getY() - range,
						unit->getX() + range, unit->getY() + range,
						unit, [&](CUnit* target) {
						if (target == unit) {
							return false;
						}
						if (target->status & UnitStatus::InAir) {
							return false;
						}
						if (scbw::isAlliedTo(unit->playerId, target->playerId)) {
							return false;
						}
						if (scbw::getDistanceFast(target->sprite->position.x, target->sprite->position.y,
							unit->sprite->position.x, unit->sprite->position.y) > weapons_dat::MaxRange[WeaponId::WyvernJacksonCannon]) {
							return false;
						}
						return true;
					});
					if (a != nullptr) {
						auto direction = scbw::getAngle(x, y, a->getX(), a->getY());
						scbw::artificalBulletFrom(WeaponId::WyvernJacksonCannon, unit->playerId, unit, a, a->position);
					}
				}
				unit->updateSpeed();
			}
		}

		if (unit->id == UnitId::ProtossAccantor) {
			CUnit* scarab = unit->carrier.inHangarChild;
			while (scarab != nullptr) {
				if (scarab->id == UnitId::ProtossPlasmaShell) {
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
			if (unit->orderTarget.unit != nullptr) {
				//add mining status, decrease the regeneration rate
				unit->orderTarget.unit->unusedTimer = 180;
			}
		}

		// Cowardice slow flag
		if (unit->_padding_0x132 & 0x10) {
			unit->_padding_0x132 &= ~0x10;
			unit->updateSpeed();
		}

		// Cowardice / Malice overlays
		if (unit->_unknown_0x08C & 0x1) {
			unit->_unknown_0x08C &= ~0x1;
			uint32 overlayImageId = scbw::get_sized_overlay(unit, ImageId::Cowardice_Small);
			if (!unit->getOverlay(overlayImageId) && scbw::get_aise_value(unit, nullptr, AiseId::HasMalice, 0, 0) == 0) {
				unit->sprite->createTopOverlay(overlayImageId,
					0, 0, 0);
				scbw::set_generic_value(unit, ValueId::HierophantOverlay, 1);
			}
		}

		if (scbw::get_generic_value(unit, ValueId::HierophantOverlay) == 1) {
			auto overlayId = scbw::get_sized_overlay(unit, ImageId::Cowardice_Small);
			auto overlay = unit->getOverlay(overlayId);
			if (scbw::get_aise_value(unit, nullptr, AiseId::HasMalice, 0, 0) == 1) {
				if (unit->getOverlay(overlayId)) {
					unit->removeOverlay(overlayId);
				}
				uint32 maliceId = scbw::get_sized_overlay(unit, ImageId::Malice_Small);
				if (!unit->getOverlay(maliceId)) {
					unit->sprite->createTopOverlay(maliceId, 0, 0, 0);
				}
				scbw::set_generic_value(unit, ValueId::HierophantOverlay, 0);
			}
		}

		//Clear Malice/Cowardice overlays
		if (unit->_unknown_0x08C & 0x2) {
			unit->_unknown_0x08C &= ~0x2;
			uint32 maliceId = scbw::get_sized_overlay(unit, ImageId::Malice_Small);
			uint32 cowardiceId = scbw::get_sized_overlay(unit, ImageId::Cowardice_Small);
			scbw::set_generic_value(unit, ValueId::HierophantOverlay, 0);
			if (unit->getOverlay(maliceId)) {
				unit->removeOverlay(maliceId);
			}
			if (unit->getOverlay(cowardiceId)) {
				unit->removeOverlay(cowardiceId);
			}
		}

		if (unit->_padding_0x132 & 0x20 && unit == playersSelections->unit[*ACTIVE_NATION_ID][0]) {
			unit->_padding_0x132 &= ~0x20;
			scbw::refreshConsole();
		}

		if (units_dat::BaseProperty[unit->id] & UnitProperty::ResourceContainer) {
			// Decrement resource regen timers
			if (unit->unusedTimer > 0) {
				unit->unusedTimer--;
			}
			if (unit->_unknown_0x066 > 0) {
				unit->_unknown_0x066--;
			} else {
				// Set regen timers
				if (unit->unusedTimer > 0) {
					unit->_unknown_0x066 = 72;
					if (scbw::get_generic_timer(unit, ValueId::MineralSpecialRegenTimer) > 0) {
						unit->_unknown_0x066 = 36;
					}
				} else {
					unit->_unknown_0x066 = 36;
					if (scbw::get_generic_timer(unit, ValueId::MineralSpecialRegenTimer) > 0) {
						unit->_unknown_0x066 = 18;
					}
				}
				// Actual resource regeneration
				bool update_image = true;
				if (unit->building.resource.resourceAmount < 5000
					&& (unit->id == UnitId::ResourceVespeneGeyser
						|| scbw::get_generic_timer(unit, ValueId::MineralSpecialRegenTimer) > 0)) {
					unit->building.resource.resourceAmount++;
					scbw::refreshConsole();
				} else if (unit->building.resource.resourceAmount < 5000) {
					unit->building.resource.resourceAmount++;
					if (unit->id >= UnitId::mineral_field_1 && unit->id <= UnitId::mineral_field_3) {
						updateMineralPatchImage(unit);
					}
					scbw::refreshConsole();
				}
			}
		}

		//Shield regeneration
		if (units_dat::ShieldsEnabled[unit->id] != 0) {
			int32 maxShields = (int32)(units_dat::MaxShieldPoints[unit->id]) * 256;
			if (unit->id == UnitId::ProtossAugur && unit->status & UnitStatus::Completed) {
				unit->shields -= std::min(14, unit->shields);
			} else {
				if (unit->shields != maxShields && unit->id != UnitId::scarab) {
					unit->shields += 7;
					if (unit->shields > maxShields) {
						unit->shields = maxShields;
					}
					if (unit->sprite->flags & CSprite_Flags::Selected) {  // If the unit is currently selected, redraw its graphics
						setAllImageGroupFlagsPal11(unit->sprite);
					}
				}
			}
		}

		//Supposedly, allows Zerglings to attack after unburrowing without delay.
		if (unit->id == UnitId::ZergZergling || unit->id == UnitId::Hero_DevouringOne)
			if (unit->groundWeaponCooldown == 0) {
				unit->orderQueueTimer = 0;
			}

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

			// TODO: Finish Defensive Matrix changes
			if (unit->defensiveMatrixTimer == 168) {
				scbw::restoreFunc(unit);
			}

			//HP regeneration
			if (units_dat::BaseProperty[unit->id] & UnitProperty::RegeneratesHP &&
				unit->hitPoints > 0 &&
				unit->hitPoints / 256 != unit->getMaxHpInGame()) {
				unit->setHp(unit->hitPoints + 4);
			}
			if (unit->defensiveMatrixHp != 0) {
				if (unit->hitPoints > 0 &&
					unit->hitPoints / 256 != unit->getMaxHpInGame()) {
					auto caster = scbw::get_aise_value(unit, nullptr, AiseId::GetMatrixCasterPlayer, 0, 0);
					if (caster < 8) {
						unit->setHp(unit->hitPoints + 53); //256 * 5 / 24 = 53.333..
					}
				}
			}

			// Update unit energy (energy regen/drain)
			updateUnitEnergy(unit);

			// Recent order timer
			if (unit->recentOrderTimer != 0) {
				unit->recentOrderTimer--;
			}

			// Self-destruct timer
			if (unit->id == UnitId::ProtossAugur && unit->shields == 0) {
				augurSacrifice(unit);
				bStopHere = true;
			}

			if (unit->removeTimer != 0) {
				unit->removeTimer--;
				if (unit->removeTimer == 0) {
					unit->remove();
					bStopHere = true;
				}
			}

			// Last Orders - Aurora passive
			if (unit->id == UnitId::ProtossAurora) {
				if (scbw::get_generic_value(unit, ValueId::LastOrdersInit) == 1) {
					auto distance = scbw::getDistanceFast(unit->position.x, unit->position.y,
						unit->orderTarget.pt.x, unit->orderTarget.pt.y);
					if (distance < 32) {
						unit->status |= UnitStatus::Invincible;
						CBullet* bullet = scbw::createBulletRet(WeaponId::AuroraLastOrders, unit, unit->position.x, unit->position.y, unit->playerId, unit->currentDirection1);
						hooks::splash(bullet, WeaponId::AuroraLastOrders, SplashType::FriendlyNew, nullptr);
						bullet->time_remaining = 0;
						// TODO: Add impact graphic
						unit->remove();
					}
				}
			}

			//Terran building burn-down
			if (!bStopHere) {
				RaceId::Enum raceId = unit->getRace();
				if (raceId != RaceId::Zerg
					&& raceId != RaceId::Protoss
					&& raceId == RaceId::Terran)

					//Check if the unit is a grounded or lifted building
					if (unit->status & UnitStatus::GroundedBuilding
						|| units_dat::BaseProperty[unit->id] & UnitProperty::FlyingBuilding) {
						//...whose current HP is less or equal to 33% of max HP
						if (unitHpIsInRedZone(unit)) {
							unit->damageHp(20, nullptr, unit->lastAttackingPlayer, true);
						}
					}
			} //if (unit->getRace() == RaceId::Terran)
		}
	};

	void incrementUnitScoresEx(int count, CUnit* unit, int increment_scores) { // 0x00488D50
		if (unit->status & UnitStatus::IsHallucination) {
			return;
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit) {
			return;
		}
		auto player = unit->playerId;
		auto unit_id = unit->id;
		completed_unit_counts[unit->id].player[unit->playerId] += count;

		int race = unit->getRace();
		int supply = units_dat::SupplyProvided[unit->id];
		if (scbw::isZergEggNew(unit, false, false)) {
			if (unit->buildQueue[unit->buildQueueSlot % 5] == UnitId::ZergIroleth) {
				supply = units_dat::SupplyProvided[UnitId::ZergOvileth];
			}
			if (unit->buildQueue[unit->buildQueueSlot % 5] == UnitId::ZergOthstoleth) {
				supply = units_dat::SupplyProvided[UnitId::ZergIroleth];
			}
			if (unit->buildQueue[unit->buildQueueSlot % 5] == UnitId::ZergAlaszileth) {
				supply = units_dat::SupplyProvided[UnitId::ZergOthstoleth];
			}
		}

		if (*elapsedTimeFrames > 0) {
			uint32 supply_new = scbw::get_generic_value(unit, ValueId::SupplyProvided);
			if (supply_new) {
				supply = supply_new;
			}
			scbw::set_generic_value(unit, ValueId::SupplyProvided, supply);
			auto supply_overlord_modifier = scbw::get_generic_value(unit, ValueId::OverlordSupplyGimmick);
			if (supply_overlord_modifier > 0) {
				switch (supply_overlord_modifier) {
					case 1:
						supply = units_dat::SupplyProvided[UnitId::ZergIroleth] - units_dat::SupplyProvided[UnitId::ZergOvileth];
						break;
					case 2:
						supply = units_dat::SupplyProvided[UnitId::ZergOthstoleth] - units_dat::SupplyProvided[UnitId::ZergIroleth];
						break;
					default:
						break;
				}
			} else
				if (unit->id == UnitId::ZergOthstoleth) {
					supply = units_dat::SupplyProvided[unit->id];
				}
			scbw::set_generic_value(unit, ValueId::OverlordSupplyGimmick, 0);
		}
		if (race <= RaceId::Protoss) {
			raceSupply[race].provided[player] += count * supply;
		}
		if (units_dat::GroupFlags[unit_id].isFactory) {
			factoriesOwned[unit->playerId] += count;
		}
		if (units_dat::GroupFlags[unit_id].isBuilding) {
			buildingsOwned[unit->playerId] += count;
		}
		if (units_dat::GroupFlags[unit_id].isMen) {
			unitsProduced[unit->playerId] += count;
		}
		if (increment_scores != 0) {
			if (units_dat::GroupFlags[unit_id].isMen) {
				if (!scbw::isAnyMorphMorphResult(unit_id)) {
					unitsOwned[unit->playerId] += count;
				}
				unitScore[unit->playerId] += count;
			} else if (units_dat::GroupFlags[unit_id].isBuilding) {
				if (!scbw::isZergBuildingMorphResult(unit_id)) {
					buildingsOwned[unit->playerId] += count;
				}
				buildingScore[unit->playerId] += count * units_dat::BuildScore[unit_id];
			}
		}
		if (completed_unit_counts[unit_id].player[unit->playerId] < 0) {
			completed_unit_counts[unit_id].player[unit->playerId] = 0;
		}
	}
} //hooks

namespace {

	const uint32 Helper_UnitHpIsInRedZone = 0x004022C0;
	bool unitHpIsInRedZone(CUnit* unit) {
		static bool32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Helper_UnitHpIsInRedZone
			MOV result, EAX
			POPAD
		}
		return result != 0;
	}

	//hooked by hooks\update_status_effects.cpp
	const uint32 Func_RestoreAllUnitStats = 0x00492F70;
	void RestoreAllUnitStats(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_RestoreAllUnitStats
			POPAD
		}
	}

	//Identical to setAllImageGroupFlagsPal11 @ 0x00497430;
	void setAllImageGroupFlagsPal11(CSprite* sprite) {
		for (
			CImage* current_image = sprite->images.head;
			current_image != nullptr;
			current_image = current_image->link.next
			) {
			if (current_image->paletteType == PaletteType::RLE_HPFLOATDRAW)
				current_image->flags |= CImage_Flags::Redraw;
		}
	}

	// For AI_Burrower
	const uint32 Func_techUseAllowed = 0x0046DD80;
	void techUseAllowed(CUnit* a1, int16 a2, int32 a3) {
		__asm {
			PUSHAD
			PUSH a3
			MOV EAX, a1
			MOVZX DI, a2
			CALL Func_techUseAllowed
			POPAD
		}
	}

	// For irradiate function
	const uint32 Func_CreateThingy = 0x00488210;
	CThingy* createThingy(uint32 spriteId, int16 x, int16 y, uint32 playerId) {
		static CThingy* thingy;
		int32 x_ = x;
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

	const uint32 Func_SetThingyVisibilityFlags = 0x004878F0;
	bool setThingyVisibilityFlags(CThingy* thingy) {
		static bool32 bPreResult;
		__asm {
			PUSHAD
			MOV ESI, thingy
			CALL Func_SetThingyVisibilityFlags
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	const uint32 Func_GetAllUnitsInBounds = 0x0042FF80;
	CUnit** getAllUnitsInBounds(Box16* coords) {
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
}

