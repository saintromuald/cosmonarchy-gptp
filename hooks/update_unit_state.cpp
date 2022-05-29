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
	// This function regenerates energy for spellcasters and drains energy for cloaking units.
	void update_unit_energy(Unit* unit) // 004EB4B0
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		// was hardcoded instead of function calling in original code
		if (unit->is_valid_caster()
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
			else
			{
				if (unit == nullptr)
				{
					return;
				}
				s32 maxEnergy;
				if (unit->id == UnitId::ProtossMindTyrant
					&& unit->mainOrderId == OrderId::CompletingArchonSummon
					&& unit->mainOrderState == 0)
				{
					maxEnergy = 100 * 256; // Identical to energy amount on spawn
				}
				else
				{
					maxEnergy = unit->getMaxEnergy();
				}

				if (unit->id == UnitId::TerranPhobos && unitExt->values[ValuePhobosDischarge] > 0)
				{
					u32 drain = (200 * 256) / (5 * 24);
					if (unit->energy < drain)
					{
						unit->energy = 0;
						unitExt->values[ValuePhobosDischarge] = 0;
					} else {
						unit->spendUnitEnergy(drain);
					}
				}
				else if (unit->energy == maxEnergy)
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

	static bool wyvern_can_fire_at(Unit* unit, Unit* target)
	{
		const u32 unitOffset = std::max(unit->getRight() - unit->getLeft(), unit->getBottom() - unit->getTop()) + 32;
		return scbw::canWeaponTargetUnit(WeaponId::WyvernJacksonCannon, target, unit)
			&& scbw::getDistanceFast(target->sprite->position.x, target->sprite->position.y, unit->sprite->position.x, unit->sprite->position.y) <= weapons_dat::MaxRange[WeaponId::WyvernJacksonCannon] + unitOffset
			&& unit->canAttackTarget(target);
	}

	// Updates unit timers, regenerates hp and shields, and burns down Terran buildings.
	void update_unit_state(Unit* unit) // 004EC290
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
				&& scbw::is_allied_to(unit->player, target->player))
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
					scbw::artificalBulletFrom(WeaponId::WyvernJacksonCannon, unit->player, unit, nullptr, targ_x, targ_y);
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
					if (wyvern_can_fire_at(unit, newTarget))
					{
						unitExt->units[UnitWyvernTargetLock] = newTarget;
					}
				}
				Unit* target = unitExt->units[UnitWyvernTargetLock];
				if (!target || !wyvern_can_fire_at(unit, target))
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
						if (scbw::is_allied_to(unit->player, target->player))
						{
							return false;
						}
						if (!wyvern_can_fire_at(unit, target))
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
					scbw::artificalBulletFrom(WeaponId::WyvernJacksonCannon, unit->player, unit, target, target->position);
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
			while (scarab)
			{
				if (scarab->id == UnitId::ProtossPlasmaShell)
				{
					scarab->shields = unit->shields;
				}
				scarab = scarab->interceptor.hangar_link.prev;
			}
		}

		// Add mining status, decrease the regeneration rate
		if (unit->mainOrderId == OrderId::MiningMinerals ||
			unit->mainOrderId == OrderId::HarvestGas3)
		{
			if (unit->orderTarget.unit)
			{
				unit->orderTarget.unit->cmbw_resource_regen_mining_delay = 5 * 24;
			}
		}

		// Cowardice slow flag
		if (unit->_padding_0x132 & 0x10)
		{
			unit->_padding_0x132 &= ~0x10;
			unit->update_speed();
		}

		// Cowardice / Malice overlays
		if (unit->_unknown_0x08C & 0x1)
		{
			unit->_unknown_0x08C &= ~0x1;
			u32 overlayImageId = ImageId::Cowardice_Small + scbw::get_overlay_offset(unit->id);
			if (!unit->getOverlay(overlayImageId) && scbw::get_aise_value(unit, nullptr, AiseId::HasMalice, 0, 0) == 0)
			{
				unit->sprite->createTopOverlay(overlayImageId, 0, 0, 0);
				unitExt->values[ValueHierophantOverlay] = 1;
			}
		}

		if (unitExt->values[ValueHierophantOverlay] == 1)
		{
			auto overlayId = ImageId::Cowardice_Small + scbw::get_overlay_offset(unit->id);
			auto overlay = unit->getOverlay(overlayId);
			if (scbw::get_aise_value(unit, nullptr, AiseId::HasMalice, 0, 0) == 1)
			{
				if (unit->getOverlay(overlayId))
				{
					unit->removeOverlay(overlayId);
				}
				u32 maliceId = ImageId::Malice_Small + scbw::get_overlay_offset(unit->id);
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
			u32 maliceId = ImageId::Malice_Small + scbw::get_overlay_offset(unit->id);
			u32 cowardiceId = ImageId::Cowardice_Small + scbw::get_overlay_offset(unit->id);
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
			scbw::refresh_console();
		}

		if (units_dat::BaseProperty[unit->id] & UnitProperty::ResourceContainer)
		{
			// Decrement resource regen timers
			if (unit->cmbw_resource_regen_mining_delay > 0)
			{
				unit->cmbw_resource_regen_mining_delay--;
			}
			if (!unit->cmbw_resource_regen_mining_delay || unitExt->timers[TimerResourceBoostedRegen]) {
				if (unit->cmbw_resource_regen_cooldown > 0)
				{
					unit->cmbw_resource_regen_cooldown--;
				}
				else
				{
					// Set regen timers
					unit->cmbw_resource_regen_cooldown = 36;
					if (unitExt->timers[TimerResourceBoostedRegen]) {
						unit->cmbw_resource_regen_cooldown /= 3;
					}

					// Actual resource regeneration
					bool update_image = true;
					if (unit->id == UnitId::ResourceVespeneGeyser || (scbw::unit_id_is_capped_gas_structure(unit->id) && unitExt->timers[TimerResourceBoostedRegen]))
					{
						if (unit->building.resource.resourceAmount < 7500) {
							unit->building.resource.resourceAmount++;
							scbw::refresh_console();
						}
					}
					else if (unit->id >= UnitId::ResourceMineralField && unit->id <= UnitId::ResourceMineralFieldType3)
					{
						if (unit->building.resource.resourceAmount < 2500) {
							unit->building.resource.resourceAmount++;
							helper::updateMineralPatchImage(unit);
							scbw::refresh_console();
						}
					}
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
					unit->shields += 11 * (1 + 0.1 * unitExt->unitVectors[UnitVectorCrucibles].size());
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
			update_unit_energy(unit);

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
			if ((units_dat::BaseProperty[unit->id] & UnitProperty::RegeneratesHP || unitExt->values[ValueBlissfulEmbraceOriginalPlayer])
				&& unit->hitPoints > 0
				&& unit->hitPoints / 256 != unit->getMaxHpInGame()
				&& unitExt->damageReceiveTimer > 2 * 24)
			{
				s32 regen = 11; // 1 HP per second

				regen *=  1 + 0.1 * unitExt->unitVectors[UnitVectorCrucibles].size();
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
							target->damage_with_custom(damage, DamageType::Explosive, unit, unit->player, 0, 1, false);
						});
						unit->remove();
						bStopHere = true;
					}
				}
			}

			// Terran building burn-down
			if (!bStopHere && !unitExt->values[ValueBlissfulEmbraceOriginalPlayer])
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
	void update_units() // 004ED000
	{
		*interceptorFocusedThisFrame = 0;
		if (helper::is_selected_unit_group_enabled())
		{
			if (*IS_PLACING_BUILDING)
			{
				helper::refresh_layers_3_and_4();
				helper::end_building_placement();
			}
			if (*isTargeting)
			{
				helper::cancel_target_order();
			}
		}
		Unit* first_visible_unit = *firstVisibleUnit;
		if (!--(*orderTimerReassignCount))
		{
			char visible_timer = 0;
			if (*firstVisibleUnit)
			{
				do
				{
					first_visible_unit->orderQueueTimer = visible_timer++;
					if (visible_timer == 8)
					{
						visible_timer = 0;
					}
					first_visible_unit = first_visible_unit->link.next;
				}
				while (first_visible_unit);
				first_visible_unit = *firstVisibleUnit;
			}
			*orderTimerReassignCount = 150;
		}
		if (!--(*secondaryOrderTimerReassignCount))
		{
			for (char i = 0; first_visible_unit; first_visible_unit = first_visible_unit->link.next)
			{
				first_visible_unit->secondaryOrderTimer = i++;
				if (i == 30)
				{
					i = 0;
				}
			}
			*secondaryOrderTimerReassignCount = 300;
		}
		// Now handled in global effect
	//	helper::update_disruption_web_status();
		Unit* first_dying_unit = *firstDyingUnit;
		Unit* next_dying_unit = first_dying_unit->link.next;
		if (*firstDyingUnit)
		{
			do
			{
				next_dying_unit = first_dying_unit->link.next;
				*activeIscriptEntity = first_dying_unit;
				*activeIscriptUnit = first_dying_unit;
				helper::progress_dying_unit_frame(first_dying_unit);
				if (first_dying_unit->sprite)
				{
					if (*visionUpdated)
					{
						if (playerTable[first_dying_unit->player].type == 2) // PLAYER_OCCUPIED
						{
							helper::reveal_sight_at_location(1, playerVision->flags[first_dying_unit->player], first_dying_unit->sprite->position.x, first_dying_unit->sprite->position.y, (first_dying_unit->status >> 2) & 1); // is air unit
						}
					}
					helper::update_visibility(first_dying_unit);
				}
				first_dying_unit = next_dying_unit;
			}
			while (next_dying_unit);
		}
		first_visible_unit = *firstVisibleUnit;
		if (*firstVisibleUnit)
		{
			do
			{
				*activeIscriptEntity = first_visible_unit;
				*activeIscriptUnit = first_visible_unit;
				helper::progress_active_unit_frame(first_visible_unit);
				first_visible_unit = first_visible_unit->link.next;
			}
			while (first_visible_unit);
			first_visible_unit = *firstVisibleUnit;
		}
		Unit* first_scanner_sweep = *firstScannerSweep;
		if (*visionUpdated)
		{
			if (*firstScannerSweep)
			{
				Unit* next_scanner_sweep = first_scanner_sweep->link.next;
				do
				{
					next_scanner_sweep = first_scanner_sweep->link.next;
					*activeIscriptEntity = first_scanner_sweep;
					*activeIscriptUnit = first_scanner_sweep;
					helper::refresh_unit_vision(first_scanner_sweep);
					first_scanner_sweep = next_scanner_sweep;
				}
				while (next_scanner_sweep);
				first_visible_unit = *firstVisibleUnit;
			}
		}
		if (first_visible_unit)
		{
			UnitExt* unitExt = globals.get_unit_ext(first_visible_unit);
			do
			{
				helper::update_visibility(first_visible_unit);
				if (first_visible_unit->status & 0x300 && unitExt->timers[TimerDecloak] == 0)
				{
					first_visible_unit->isCloaked = 0;
					if (first_visible_unit->secondaryOrderTimer)
					{
						first_visible_unit->secondaryOrderTimer = first_visible_unit->secondaryOrderTimer - 1;
					}
					else
					{
						helper::check_unit_visibility(first_visible_unit);
						first_visible_unit->secondaryOrderTimer = 30;
					}
				}
				first_visible_unit = first_visible_unit->link.next;
			}
			while (first_visible_unit);
			first_visible_unit = *firstVisibleUnit;
		}
		if (first_visible_unit)
		{
			Unit* next_visible_unit = first_visible_unit->link.next;
			do
			{
				next_visible_unit = first_visible_unit->link.next;
				*activeIscriptEntity = first_visible_unit;
				*activeIscriptUnit = first_visible_unit;
				helper::progress_unit_frame(first_visible_unit);
				first_visible_unit = next_visible_unit;
			}
			while (next_visible_unit);
		}
		Unit* first_hidden_unit = *firstHiddenUnit;
		Unit* next_hidden_unit = first_hidden_unit->link.next;
		if (*firstHiddenUnit)
		{
			do
			{
				next_hidden_unit = first_hidden_unit->link.next;
				if (first_hidden_unit->status & 0x300)
				{
					first_hidden_unit->isCloaked = 0;
				}
				*activeIscriptEntity = first_hidden_unit;
				*activeIscriptUnit = first_hidden_unit;
				helper::unit_update(first_hidden_unit);
				first_hidden_unit = next_hidden_unit;
			}
			while (next_hidden_unit);
		}
		helper::update_burrowing_cloaking_units();
		helper::update_unit_power();
		s32 lurker_splash_index = (*lurkerSplashListIndex)++ + 1;
		if (*lurkerSplashListIndex == 32)
		{
			lurker_splash_index = 0;
			*lurkerSplashListIndex = 0;
		}
		memset(&lurkerSplashList[32 * lurker_splash_index], 0, 0x80u);
		*lastBulletIndex = 0;
		first_scanner_sweep = *firstScannerSweep;
		if (*firstScannerSweep)
		{
			Unit* next_scanner_sweep_2 = first_scanner_sweep->link.next;
			do
			{
				next_scanner_sweep_2 = first_scanner_sweep->link.next;
				*activeIscriptEntity = first_scanner_sweep;
				*activeIscriptUnit = first_scanner_sweep;
				helper::progress_unit_frame(first_scanner_sweep);
				first_scanner_sweep = next_scanner_sweep_2;
			}
			while (next_scanner_sweep_2);
		}
		*activeIscriptEntity = 0;
		*activeIscriptUnit = 0;
	}

	// Hooked to help with the cloak rewrite - Pr0nogo
	void update_burrowing_cloaking_units() // 00492EB0
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
					do_cloak(v0, SoundId::Terran_PHOENIX_TPhClo01_WAV);
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
	void do_cloak(Unit* unit, s32 soundId) // 0049B440
	{
		Unit* unitVar = unit;
		if (!(unit->status & UnitStatus::Burrowed))
		{
			helper::playSoundFromSource(soundId, unit, 1, 0);
		}
		if (!IS_IN_REPLAY && unitVar->player == *LOCAL_NATION_ID)
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
	void progress_unit_frame(Unit* unit) // 004ECF70
	{
		Unit* unitVar = unit;
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Subunit) && unit->sprite)
		{
			if (!(unit->sprite->flags & 0x20))
			{
				helper::refresh_sprite(unit->sprite, selectionCircleColor[unit->player]);
			}
		}
		update_unit_state(unit);
		helper::progress_order(unit);
		helper::perform_secondary_orders(unit, unit->player); // ECX at the time of calling this function

		if (unit->subunit)
		{
			if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Subunit))
			{
				*activeIscriptUnit = unit->subunit;
				progress_unit_frame(unit->subunit);
				*activeIscriptUnit = unit;
			}
		}
		if (unit->sprite)
		{
			helper::sprite_to_iscript_loop(unit->sprite);
			if (!unit->sprite->images.head)
			{
				unit->sprite = 0;
			}
		}
	}
}