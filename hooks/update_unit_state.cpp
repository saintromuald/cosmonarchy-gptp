#include "update_unit_state.h"
#include "update_status_effects.h"
#include <SCBW/api.h>
#include <algorithm>
#include "hooks/weapons/weapon_damage.h"
#include "hooks/weapon_effect.h"
#include "hooks/tactical_ai.h"
#include "globals.h"
#include "SCBW\UnitFinder.h"
#include "hooks/weapons/weapon_cooldown.h"
#include "hooks/unit_destructor_special.h"

namespace hooks
{
	/// This function regenerates energy for spellcasters and drains energy for cloaking units.
	/// Equivalent to function sub_4EB4B0 @ 0x004EB4B0

	void updateUnitEnergy(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		// was hardcoded instead of function calling in original code
		if (unit->isValidCaster()
			&& (unit->status & UnitStatus::Completed))
		{
			bool bStopHere = false;
			// Spend energy for cloaked units
			if (unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection)		// If the unit is cloaked
				&& !(unit->status & UnitStatus::CloakingForFree))							// and must consume energy to stay cloaked (i.e. not under a Didact)
			{
				u16 cloakingEnergyCost = 0;
				if (!(unit->hasEnergy(cloakingEnergyCost)))
				{
					if (unit->secondaryOrderId == OrderId::Cloak)
					{
						unit->setSecondaryOrder(OrderId::Nothing2); // Supposedly, immediately decloaks the unit.
					}
					bStopHere = true;
				}
				if (!bStopHere) // the function prevents energy consumption if THE GATHERING is enabled
				{
					unit->spendUnitEnergy(cloakingEnergyCost);
				}
			}
			else if (unitExt->values[ValueGodsMadeFleshActive])
			{
				unit->spendUnitEnergy(GodsMadeFleshEnergyDrain);
			}
			else if (unit->id == UnitId::ProtossMindTyrant && scbw::get_aise_value(unit, nullptr, AiseId::SumMindControlCost, 0, 0) > 0)
			{
				auto sum = scbw::get_aise_value(unit, nullptr, AiseId::SumMindControlCost, 0, 0) / 2;
				unit->spendUnitEnergy(std::min(unit->energy, (u16)(16 * sum)));
				if (unit->energy == 0)
				{
					scbw::sendGPTP_aise_cmd(unit, nullptr, GptpId::FreeMindControlled, 0, 0, 0);
				}
			}
			else
			{ // 004EB526
				if (unit == nullptr)
				{
					return;
				}
				s32 maxEnergy;
				if (unit->id == UnitId::ProtossMindTyrant
					&& unit->mainOrderId == OrderId::CompletingArchonSummon
					&& unit->mainOrderState == 0)
				{
					maxEnergy = 12800; // 50 * 256; Identical to energy amount on spawn
				}
				else
				{
					maxEnergy = unit->getMaxEnergy();
				}
				bool phobosCondition = false;
				if (unit->id == UnitId::TerranPhobos && unitExt->values[ValuePhobosDischarge] > 0)
				{
					unit->spendUnitEnergy(426); // (200 * 256) / (5 * 24)  ~ 426.66
					if (unit->energy < 426)
					{
						unit->energy = 0;
						unitExt->values[ValuePhobosDischarge] = 0;
					}
					phobosCondition = true;
				}
				if (unit->energy == maxEnergy && !phobosCondition)
				{
					bStopHere = true;
				}
				else
				{
					unit->addUnitEnergy(11, maxEnergy);
				}
			}

			// If the unit is currently selected, redraw its graphics
			if (!bStopHere && unit->sprite->flags & SpriteFlags::Selected)
			{
				helper::setAllImageGroupFlagsPal11(unit->sprite);
			}
		}
	}

	static bool wyvernCanFireAt(Unit* unit, Unit* target)
	{
		const u32 unitOffset = std::max(unit->getRight() - unit->getLeft(), unit->getBottom() - unit->getTop()) + 32;
		return scbw::canWeaponTargetUnit(WeaponId::WyvernJacksonCannon, target, unit)
			&& scbw::getDistanceFast(target->sprite->position.x, target->sprite->position.y, unit->sprite->position.x, unit->sprite->position.y) <= weapons_dat::MaxRange[WeaponId::WyvernJacksonCannon] + unitOffset
			&& unit->canAttackTarget(target);
	}

	/// Updates unit timers, regenerates hp and shields, and burns down Terran buildings.
	/// Identical to function @ 0x004EC290
	void updateUnitStateHook(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		// Timers
		if (unit->mainOrderTimer)
		{
			unit->mainOrderTimer--;
		}
		if (unit->groundWeaponCooldown)
		{
			unit->groundWeaponCooldown--;
		}
		if (unit->airWeaponCooldown)
		{
			unit->airWeaponCooldown--;
		}
		if (unit->spellCooldown)
		{
			unit->spellCooldown--;
		}

		// Wyvern clear target when changed owner to allied
		if (unit->id == UnitId::TerranWyvern
			&& unitExt->units[UnitWyvernTargetLock])
		{
			Unit* target = unitExt->units[UnitWyvernTargetLock];
			UnitExt* targetExt = globals.get_unit_ext(target);
			if (targetExt->timers[TimerOwnerChangedRecently]
				&& scbw::isAlliedTo(unit->playerId, target->playerId))
			{
				unitExt->units[UnitWyvernTargetLock] = nullptr;
			}
		}

		// Wyvern Reverse Thrust - fire while moving
		if (unit->id == UnitId::TerranWyvern
			&& unitExt->timers[TimerWyvernAttackCooldown] == 0)
		{
			auto range = weapons_dat::MaxRange[WeaponId::WyvernJacksonCannon];
			s32 forwardOffset = weapons_dat::ForwardOffset[WeaponId::WyvernJacksonCannon];
			auto x = unit->getX() + scbw::getPolarX(forwardOffset, unit->currentDirection1);
			auto y = unit->getY() + scbw::getPolarY(forwardOffset, unit->currentDirection1);

			bool seek_unit = true;
			if (unitExt->values[ValueWyvernLandGrabOn] == 1)
			{
				auto targ_x = unitExt->values[ValueWyvernLandGrabX];
				auto targ_y = unitExt->values[ValueWyvernLandGrabY];
				auto direction = scbw::getAngle(x, y, targ_x, targ_y);
				if (scbw::getDistanceFast(unit->position.x, unit->position.y, targ_x, targ_y) <= range)
				{
					u32 sound = scbw::randBetween(SoundId::WyvernAttack01, SoundId::WyvernAttack02);
					helper::playSoundFromSource(sound, unit, 1, 0);
					scbw::artificalBulletFrom(WeaponId::WyvernJacksonCannon, unit->playerId, unit, nullptr, targ_x, targ_y);
					unitExt->timers[TimerWyvernAttackCooldown] = hooks::getModifiedWeaponCooldownHook(unit, WeaponId::WyvernJacksonCannon);
					seek_unit = false;
				}
				else
				{
					unitExt->values[ValueWyvernLandGrabOn] = 0;
				}
			}

			if (seek_unit)
			{
				if (unit->orderTarget.unit
					&& unit->mainOrderId != OrderId::Follow
					&& unit->mainOrderId != OrderId::Patrol
					&& unit->mainOrderId != OrderId::WyvernLandGrab)
				{
					Unit* newTarget = unit->orderTarget.unit;
					if (wyvernCanFireAt(unit, newTarget))
					{
						unitExt->units[UnitWyvernTargetLock] = newTarget;
					}
				}
				Unit* target = unitExt->units[UnitWyvernTargetLock];
				if (!target || !wyvernCanFireAt(unit, target))
				{
					target = scbw::UnitFinder::getNearestTarget(
						unit->getX() - range,
						unit->getY() - range,
						unit->getX() + range,
						unit->getY() + range,
						unit, [&](Unit* target)
					{
						if (target == unit)
						{
							return false;
						}
						if (scbw::isAlliedTo(unit->playerId, target->playerId))
						{
							return false;
						}
						if (!wyvernCanFireAt(unit, target))
						{
							return false;
						}
						return true;
					});	
				}
				
				if (target)
				{
					auto direction = scbw::getAngle(x, y, target->getX(), target->getY());
					helper::playSoundFromSource(scbw::randBetween(SoundId::WyvernAttack01, SoundId::WyvernAttack02), unit, 1, 0);
					scbw::artificalBulletFrom(WeaponId::WyvernJacksonCannon, unit->playerId, unit, target, target->position);
					unitExt->timers[TimerWyvernAttackCooldown] = hooks::getModifiedWeaponCooldownHook(unit, WeaponId::WyvernJacksonCannon);
				}
			}
		}

		// Clear Cleric healing field
		unit->isBeingHealed = 0;

		// Accantor Plasma Shells get shields equal to the Accantor that launches them
		if (unit->id == UnitId::ProtossAccantor)
		{
			Unit* scarab = unit->carrier.inHangarChild;
			while (scarab != nullptr)
			{
				if (scarab->id == UnitId::ProtossPlasmaShell)
				{
					scarab->shields = unit->shields;
				}
				scarab = scarab->interceptor.hangar_link.prev;
			}
		}

		// Add mining status, decrease the regeneration rate
		if (unit->mainOrderId == OrderId::MiningMinerals)
		{
			if (unit->orderTarget.unit != nullptr)
			{
				unit->orderTarget.unit->unusedTimer = 180;
			}
		}

		// Cowardice slow flag
		if (unit->_padding_0x132 & 0x10)
		{
			unit->_padding_0x132 &= ~0x10;
			unit->updateSpeed();
		}

		// Cowardice / Malice overlays
		if (unit->_unknown_0x08C & 0x1)
		{
			unit->_unknown_0x08C &= ~0x1;
			u32 overlayImageId = ImageId::Cowardice_Small + scbw::getOverlayOffset(unit->id);
			if (!unit->getOverlay(overlayImageId) && scbw::get_aise_value(unit, nullptr, AiseId::HasMalice, 0, 0) == 0)
			{
				unit->sprite->createTopOverlay(overlayImageId, 0, 0, 0);
				unitExt->values[ValueHierophantOverlay] = 1;
			}
		}

		if (unitExt->values[ValueHierophantOverlay] == 1)
		{
			auto overlayId = ImageId::Cowardice_Small + scbw::getOverlayOffset(unit->id);
			auto overlay = unit->getOverlay(overlayId);
			if (scbw::get_aise_value(unit, nullptr, AiseId::HasMalice, 0, 0) == 1)
			{
				if (unit->getOverlay(overlayId))
				{
					unit->removeOverlay(overlayId);
				}
				u32 maliceId = ImageId::Malice_Small + scbw::getOverlayOffset(unit->id);
				if (!unit->getOverlay(maliceId))
				{
					unit->sprite->createTopOverlay(maliceId, 0, 0, 0);
				}
				unitExt->values[ValueHierophantOverlay] = 0;
			}
		}

		// Clear Malice & Cowardice overlays
		if (unit->_unknown_0x08C & 0x2)
		{
			unit->_unknown_0x08C &= ~0x2;
			u32 maliceId = ImageId::Malice_Small + scbw::getOverlayOffset(unit->id);
			u32 cowardiceId = ImageId::Cowardice_Small + scbw::getOverlayOffset(unit->id);
			unitExt->values[ValueHierophantOverlay] = 0;
			if (unit->getOverlay(maliceId))
			{
				unit->removeOverlay(maliceId);
			}
			if (unit->getOverlay(cowardiceId))
			{
				unit->removeOverlay(cowardiceId);
			}
		}

		// Something to do with player-specific Signify overlays?
		if (unit->_padding_0x132 & 0x20 && unit == playersSelections->unit[*ACTIVE_NATION_ID][0])
		{
			unit->_padding_0x132 &= ~0x20;
			scbw::refreshConsole();
		}

		if (units_dat::BaseProperty[unit->id] & UnitProperty::ResourceContainer)
		{
			// Decrement resource regen timers
			if (unit->unusedTimer > 0)
			{
				unit->unusedTimer--;
			}

			if (unit->_unknown_0x066 > 0)
			{
				unit->_unknown_0x066--;
			}
			else
			{
				// Set regen timers
				if (unit->unusedTimer > 0)
				{
					unit->_unknown_0x066 = 72;
					if (unitExt->timers[TimerResourceBoostedRegen])
					{
						unit->_unknown_0x066 = 36;
					}
				}
				else
				{
					unit->_unknown_0x066 = 36;
					if (unitExt->timers[TimerResourceBoostedRegen])
					{
						unit->_unknown_0x066 = 18;
					}
				}

				// Actual resource regeneration
				bool update_image = true;
				if (unit->building.resource.resourceAmount < 7500
				&& (unit->id == UnitId::ResourceVespeneGeyser
				|| (scbw::unit_id_is_gas_structure(unit->id, true) && unitExt->timers[TimerResourceBoostedRegen])))
				{
					unit->building.resource.resourceAmount++;
					scbw::refreshConsole();
				}
				else if (unit->building.resource.resourceAmount < 2500)
				{
					unit->building.resource.resourceAmount++;
					if (unit->id >= UnitId::ResourceMineralField && unit->id <= UnitId::ResourceMineralFieldType3)
					{
						helper::updateMineralPatchImage(unit);
					}
					scbw::refreshConsole();
				}
			}
		}

		// Shield regeneration
		if (units_dat::ShieldsEnabled[unit->id] != 0)
		{
			s32 maxShields = (s32)(units_dat::MaxShieldPoints[unit->id]) * 256;
			if (unit->id == UnitId::ProtossAugur && unit->status & UnitStatus::Completed)
			{
				unit->shields -= std::min(14, unit->shields);
			}
			else
			{
				if (unit->shields != maxShields && unit->id != UnitId::ProtossPlasmaShell && unitExt->damageReceiveTimer > 2 * 24)
				{
					unit->shields += 11 * (1 + 0.05 * unitExt->unitVectors[UnitVectorCrucibles].size());
					if (unit->shields > maxShields)
					{
						unit->shields = maxShields;
					}
					if (unit->sprite->flags & SpriteFlags::Selected)
					{	// If the unit is currently selected, redraw its graphics
						helper::setAllImageGroupFlagsPal11(unit->sprite);
					}
				}
			}
		}

		// supposedly, allows Zerglings to attack after unburrowing without delay
		if (unit->id == UnitId::ZergZethrokor)
		{
			if (unit->groundWeaponCooldown == 0)
			{
				unit->orderQueueTimer = 0;
			}
		}

		// update unit status effects (stim, maelstrom, plague, etc.)
		if (unit->status & UnitStatus::Completed || !(unit->sprite->flags & SpriteFlags::Hidden))
		{
			unit->cycleCounter++;
			if (unit->cycleCounter >= 8)
			{
				unit->cycleCounter = 0;
				helper::RestoreAllUnitStats(unit); // use update_status_effects hook if available
			}
		}

		// only for fully-constructed units and buildings
		if (unit->status & UnitStatus::Completed)
		{
			bool bStopHere = false;

			// remove units with timed lives
			if (unit->removeTimer != 0)
			{
				unit->removeTimer -= 1;
				if (unit->removeTimer == 0)
				{
					unit->remove();
					bStopHere = true;
				}
			}

			// update unit energy (energy regen/drain)
			updateUnitEnergy(unit);

			if (unit->recentOrderTimer != 0)
			{
				unit->recentOrderTimer--;
			}

			// Augur - Final Hour
			if (unit->id == UnitId::ProtossAugur && unit->shields == 0)
			{
				augurSacrifice(unit);
				bStopHere = true;
			}

			// TODO: Reduce defensiveMatrixTimer
			// Cleanse debuffs on initial Defensive Matrix application
			if (unit->defensiveMatrixTimer == 168)
			{
				scbw::restoreFunc(unit);
			}

			// HP regeneration
			if (units_dat::BaseProperty[unit->id] & UnitProperty::RegeneratesHP
				&& unit->hitPoints > 0
				&& unit->hitPoints / 256 != unit->getMaxHpInGame()
				&& unitExt->damageReceiveTimer > 2 * 24)
			{
				s32 regen = 11; // 1 HP per second

				regen *=  1 + 0.05 * unitExt->unitVectors[UnitVectorCrucibles].size();
				unit->setHp(unit->hitPoints + regen);
			}

			// Deliberate healing
			{
				s32 tickHeal = 0;
				// Alaszileth - Dermal Accelerant - 500% life regen
				if (unitExt->timers[TimerAlaszilethRegenBoost] > 0)
				{
					tickHeal += 11 * 5;
				}

				// Azazel Innervating Matrices - heal over time
				if (unit->defensiveMatrixHp != 0)
				{
					if (unit->hitPoints > 0
						&& unit->hitPoints / 256 != unit->getMaxHpInGame())
					{
						auto caster = scbw::get_aise_value(unit, nullptr, AiseId::GetMatrixCasterPlayer, 0, 0);
						if (caster < 8)
						{
							tickHeal += AzazelDefensiveMatrixHeal;
						}
					}
				}

				if (tickHeal)
				{
					unit->heal(tickHeal);
				}
			}

			// Last Orders - Aurora passive
			if (unit->id == UnitId::ProtossAurora)
			{
				if (unitExt->values[ValueLastOrdersInit] == 1)
				{
					auto distance = scbw::getDistanceFast(unit->position.x, unit->position.y,
						unit->orderTarget.pt.x, unit->orderTarget.pt.y);
					if (distance < 32)
					{
						unit->status |= UnitStatus::Invincible;

						auto match = cosmonarchyFinderMatch(unit, Flags::NoAir | Flags::Enemy, {});
						const u32 radius = 64;
						scbw::UnitFinder(
							unit->getX() - radius,
							unit->getY() - radius,
							unit->getX() + radius,
							unit->getY() + radius
						).forEachInMatch(match, [&](Unit* target)
						{
							u32 dist = scbw::getDistanceFast(unit->position, target->position);
							u32 damage = 20 * 256;
							if (dist > 48)
							{
								damage /= 4;
							}
							else if (dist > 32)
							{
								damage /= 2;
							}
							target->damageWithCustom(damage, DamageType::Explosive, unit, unit->playerId, 0, 1, false);
						});
						unit->remove();
						bStopHere = true;
					}
				}
			}

			// Terran building burn-down
			if (!bStopHere)
			{
				if (unit->getRace() == RaceId::Terran)
				{
					if (unit->status & UnitStatus::GroundedBuilding
						|| units_dat::BaseProperty[unit->id] & UnitProperty::FlyingBuilding)
					{
						if (helper::unitHpIsInRedZone(unit))
						{
							unit->damageHp(20, nullptr, unit->lastAttackingPlayer, true);
						}
					}
				}
			}
		}
	};

	// Hooked to help with the cloak rewrite - Pr0nogo
	void updateUnits() // 004ED000
	{
		s32 v0;
		Unit* v1;
		char v2;
		char i;
		Unit* v4;
		Unit* v5;
		s32 v6;
		Unit* v7;
		Unit* v8;
		Unit* v9;
		char v10;
		Unit* v11;
		Unit* v12;
		Unit* v13;
		Unit* v14;
		s32 v15;
		Unit* v16;
		Unit* v17;

		*interceptorFocusedThisFrame = 0;
		v0 = helper::isSelectedUnitGroupEnabled();
		if (v0)
		{
			if (*IS_PLACING_BUILDING)
			{
				helper::refreshLayers3And4();
				helper::endBuildingPlacement();
			}
			if (*isTargeting)
			{
				helper::cancelTargetOrder();
			}
		}
		v1 = *firstVisibleUnit;
		if (!--(*orderTimerReassignCount))
		{
			v2 = 0;
			if (*firstVisibleUnit)
			{
				do
				{
					v1->orderQueueTimer = v2++;
					if (v2 == 8)
						v2 = 0;
					v1 = v1->link.next;
				}
				while (v1);
				v1 = *firstVisibleUnit;
			}
			*orderTimerReassignCount = 150;
		}
		if (!--(*secondaryOrderTimerReassignCount))
		{
			for (i = 0; v1; v1 = v1->link.next)
			{
				v1->secondaryOrderTimer = i++;
				if (i == 30)
					i = 0;
			}
			*secondaryOrderTimerReassignCount = 300;
		}
		// Now handled in global effect
		//helper::updateDisruptionWebStatus();
		v4 = *firstDyingUnit;
		if (*firstDyingUnit)
		{
			do
			{
				v5 = v4->link.next;
				*activeIscriptEntity = v4;
				*activeIscriptUnit = v4;
				helper::progressDyingUnitFrame(v4);
				if (v4->sprite)
				{
					if (*visionUpdated)
					{
						v6 = v4->playerId;
						if (playerTable[v6].type == 2)
						{ // PLAYER_OCCUPIED
							helper::revealSightAtLocation(1, playerVision->flags[v6], v4->sprite->position.x, v4->sprite->position.y, (v4->status >> 2) & 1); // is air unit
						}
					}
					helper::updateVisibility(v4);
				}
				v4 = v5;
			}
			while (v5);
		}
		v7 = *firstVisibleUnit;
		if (*firstVisibleUnit)
		{
			do
			{
				*activeIscriptEntity = v7;
				*activeIscriptUnit = v7;
				helper::progressActiveUnitFrame(v7);
				v7 = v7->link.next;
			}
			while (v7);
			v7 = *firstVisibleUnit;
		}
		if (*visionUpdated)
		{
			v8 = *firstScannerSweep;
			if (*firstScannerSweep)
			{
				do
				{
					v9 = v8->link.next;
					*activeIscriptEntity = v8;
					*activeIscriptUnit = v8;
					helper::refreshUnitVision(v8);
					v8 = v9;
				}
				while (v9);
				v7 = *firstVisibleUnit;
			}
		}
		if (v7)
		{
			UnitExt* unitExt = globals.get_unit_ext(v7);
			do
			{
				helper::updateVisibility(v7);
				if (v7->status & 0x300 && unitExt->timers[TimerDecloak] == 0)
				{
					v10 = v7->secondaryOrderTimer;
					v7->isCloaked = 0;
					if (v10)
					{
						v7->secondaryOrderTimer = v10 - 1;
					}
					else
					{
						helper::checkUnitVisibility(v7);
						v7->secondaryOrderTimer = 30;
					}
				}
				v7 = v7->link.next;
			}
			while (v7);
			v7 = *firstVisibleUnit;
		}
		v11 = v7;
		if (v7)
		{
			do
			{
				v12 = v11->link.next;
				*activeIscriptEntity = v11;
				*activeIscriptUnit = v11;
				progressUnitFrame(v11);
				v11 = v12;
			}
			while (v12);
		}
		v13 = *firstHiddenUnit;
		if (*firstHiddenUnit)
		{
			do
			{
				v14 = v13->link.next;
				if (v13->status & 0x300)
				{
					v13->isCloaked = 0;
				}
				*activeIscriptEntity = v13;
				*activeIscriptUnit = v13;
				helper::unitUpdate(v13);
				v13 = v14;
			}
			while (v14);
		}
		updateBurrowingCloakingUnits();
		helper::updateUnitPower();
		v15 = (*lurkerSplashListIndex)++ + 1;
		if (*lurkerSplashListIndex == 32)
		{
			v15 = 0;
			*lurkerSplashListIndex = 0;
		}
		memset(&lurkerSplashList[32 * v15], 0, 0x80u);
		v16 = *firstScannerSweep;
		*lastBulletIndex = 0;
		if (*firstScannerSweep)
		{
			do
			{
				v17 = v16->link.next;
				*activeIscriptEntity = v16;
				*activeIscriptUnit = v16;
				progressUnitFrame(v16);
				v16 = v17;
			}
			while (v17);
		}
		*activeIscriptEntity = 0;
		*activeIscriptUnit = 0;
	}

	// Hooked to help with the cloak rewrite - Pr0nogo
	void updateBurrowingCloakingUnits() // 00492EB0
	{
		Unit* v0;
		s32 v1;
		Unit* v2;
		s32 v3;
		s32 v4;
		s32 v5;

		v0 = *firstInvisibleUnit;
		v1 = 0;
		if (*firstInvisibleUnit)
		{
			while (1)
			{
				v2 = v0->burrow_link.next;
				if (helper::isUnitCloaking(v0))
				{
					doCloak(v0, SoundId::Terran_PHOENIX_TPhClo01_WAV);
				}
				else
				{
					v3 = v0->status;
					if (v3 & UnitStatus::Burrowed || v0->secondaryOrderId != OrderId::Cloak || v0->isCloaked != 1 || !(v3 & 0x800))
					{
						v4 = 0;
					}
					else
					{
						v0->status = v3 & 0xFFFFF7FF;
						v4 = 1;
					}
					v1 |= v4;
					v5 = v0->status;
					if (v5 & 0x100)
						goto LABEL_13;
					helper::playSoundFromDirect(v0, 273);
				}
				v1 = 1;
LABEL_13:
				v0 = v2;
				if (!v2)
				{
					if (v1)
					{
						*forceButtonRefresh = 1;
						portraitRedraw[0] = 1;
						*statusScreenRedraw = 1;
						*ctrlUnderMouse = 0;
						*ctrlUnderMouseValue = 0;
					}
					return;
				}
			}
		}
	}

	// Hooked for cloak rewrite animations - Pr0nogo
	void doCloak(Unit* unit, s32 soundId) // 0049B440
	{
		Unit* unitVar = unit;
		if (!(unit->status & UnitStatus::Burrowed))
		{
			helper::playSoundFromSource(soundId, unit, 1, 0);
		}
		if (!IS_IN_REPLAY && unitVar->playerId == *LOCAL_NATION_ID)
		{
			*forceButtonRefresh = 1;
			portraitRedraw[0] = 1;
			*statusScreenRedraw = 1;
			*ctrlUnderMouse = 0;
			*ctrlUnderMouseValue = 0;
		}
		if (unitVar->status & UnitStatus::Burrowed)
		{
			helper::updateSpriteFlagOnPalette(unitVar->sprite);
			helper::showImagesHiddenByCloak(unitVar->sprite);
			unitVar->status &= 0xFFFFFCFF;
		}
		else
		{
			helper::updateSpriteImagesForCloak(unitVar->sprite);
			unitVar->status = unitVar->status & 0xFFFFFEFF | UnitStatus::Cloaked;
			Unit* unitSubunit = unitVar->subunit;
			if (unitSubunit)
			{
				helper::updateSpriteImagesForCloak(unitSubunit->sprite);
				unitSubunit->status = unitSubunit->status & 0xFFFFFEFF | UnitStatus::Cloaked;
			}
		}
	}

	// Hooked to add nullcheck - Pr0nogo
	void progressUnitFrame(Unit* unit) // 004ECF70
	{
		if (unit == nullptr)
		{
			return;
		}

		Unit* unitVar = unit;
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Subunit) && unit->sprite)
		{
			Sprite* unitSprite = unit->sprite;
			if (!(unitSprite->flags & 0x20))
			{
				helper::refreshSprite(unitSprite, selectionCircleColor[unit->playerId]);
			}
		}
		updateUnitStateHook(unitVar);
		helper::progressOrder(unitVar);
		s32 unused = 0;
		helper::performSecondaryOrders(unitVar, unused);

		Unit* unitSubunit = unitVar->subunit;
		if (unitSubunit)
		{
			if (!(units_dat::BaseProperty[unitVar->id] & UnitProperty::Subunit))
			{
				*activeIscriptUnit = unitVar->subunit;
				progressUnitFrame(unitSubunit);
				*activeIscriptUnit = unitVar;
			}
		}

		Sprite* unitSprite2 = unitVar->sprite;
		if (unitSprite2 != nullptr)
		{
			helper::spriteToIscriptLoop(unitSprite2);
			if (!unitVar->sprite->images.head)
			{
				unitVar->sprite = 0;
			}
		}
	}
}