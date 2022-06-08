//The source file for the Weapon/Spell Hit hook module.
#include "weapon_spellhit.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "logger.h"
#include "globals.h"
#include <algorithm>
#include <hooks/byte_operations_utils.h>
#include <cassert>

namespace hooks
{
	// Hooked to allow for replacing proc functions - Pr0nogo
	Unit* iterateUnitsAtLocationTargetProc(Box16* location, Unit*(__fastcall* proc)(Unit*, Unit*), Unit* unit) // 004E8280
	{
		Unit** unitsInBounds = helper::get_all_units_in_bounds(location);
		Unit** unitsInBounds2 = unitsInBounds;
		Unit* aUnitInBound;
		if (proc || (aUnitInBound = *unitsInBounds) == 0)
		{
			aUnitInBound = *unitsInBounds;
			if (*unitsInBounds)
			{
				do
				{
					if (proc(aUnitInBound, unit))
					{
						break;
					}
					aUnitInBound = unitsInBounds2[1];
					++unitsInBounds2;
				}
				while (aUnitInBound);
			}
		}
		s32 index = tempUnitsListArraysCountsListLastIndex[0] - 1;
		tempUnitsListArraysCountsListLastIndex[0] = index;
		*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsList[index];
		return aUnitInBound;
	}

	// Hooked to clean up lardcode - Pr0nogo
	// Seems to be double-called from Maelstrom, no idea why
	char disableUnit(Unit* unit) // 00492CC0
	{
		Unit* subunit = unit->subunit;
		if (subunit)
		{
			subunit->status |= UnitStatus::Disabled;
		}
		helper::set_resource_target(unit);
		s16 unit_id = unit->id;
		if (scbw::unit_can_nuke(unit))
		{
			Unit* nuke = unit->connected_unit;
			if (nuke)
			{
				if (nuke->id == UnitId::TerranNuclearMissile)
				{
					unit->connected_unit = 0;
					nuke->connected_unit = 0;
				}
			}
		}
		if (unit_id == UnitId::ProtossSolarion)
		{
			helper::return_all_interceptors(unit);
		}
		u8 orderId = unit->main_order_id;
		if (orderId == OrderId::Repair1 || orderId == OrderId::DroneLand)
		{
			unit->main_order_state = 0;
		}
		if (!orders_dat::DisablingKeepsTarget[orderId])
		{
			unit->order_target.unit = 0;
		}
		char result;
		result = helper::ignore_all_scripts_and_go_to_idle(unit);
		Sprite* sprite = unit->sprite;
		if (sprite->flags & 8)
		{
			if (*clientSelectionCount > 1u)
			{
				result = helper::updateBuildingLandUnitSelection(*(u32*)sprite, unit);
			}
		}
		unit->current_button_set = 228;
		*control_under_mouse = 0;
		*control_under_mouse_value = 0;
		*force_button_refresh = 1;
		portrait_redraw[0] = 1;
		*status_screen_redraw = 1;
		return result;
	}

	void IrradiateHit(Unit* attacker, Point16 target, u8 attackingPlayerId)
	{
		globals.create_effect(EffectId_Irradiate, 10 * 24, {target.x, target.y}, 1.5 * 32, SpriteId::IrradiateNew_Large, 5, attackingPlayerId, attacker);
	}

	// bullet is not normally part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void OpticalFlareHit(Unit* unused, u32 attackingPlayerId, Bullet* bullet)
	{
		if (!bullet)
		{
			scbw::print("no");
			return;
		}

		scbw::print("fire");
		const u32 radius = 2 * 32;
		helper::play_sound_from_unit(SoundId::Terran_Medic_Miopia1_wav, (Unit*)bullet, 1, 0);
		scbw::UnitFinder(
			bullet->position.x - radius,
			bullet->position.y - radius,
			bullet->position.x + radius,
			bullet->position.y + radius
		).for_each([&](Unit* target)
		{
			u32 overlay = ImageId::OpticalFlareHit_Small + scbw::get_overlay_offset(target->id);
			Unit* overlayUnit = target;
			UnitExt* targetExt = globals.get_unit_ext(target);
			if (target->subunit != nullptr)
			{
				targetExt = globals.get_unit_ext(target->subunit);
				overlayUnit = target->subunit;
			}
			overlayUnit->sprite->create_top_overlay(overlay, 0, 0, 0);
			targetExt->timers[TimerAuralAssaultBlind] = 2 * 24 + 1;
		});
	}

	// bullet is not normally part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void RestoreHit(Unit* target, Bullet* bullet)
	{
		if (target->status & UnitStatus::IsHallucination)
		{
			target->remove();
		}
		else
		{
			// create impact overlay
			scbw::restore_func(target);
		}
	}

	// bullet is not normally part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	// the timer is reset before this call 2 times out of 3 (with previousLockdownTimer holding the pre-reset value), and the 3rd case, previousLockdownTimer is set to 131 (0x83)
	void LockdownHit(Unit* target, u8 previousLockdownTimer, Bullet* bullet)
	{
		// add lockdown overlay if no ongoing lockdown already
		if (target->lockdown_timer == 0)
		{
			u32 overlayImageId = ImageId::LockdownField_Small + scbw::get_overlay_offset(target->id);
			Unit* overlayTargetUnit;
			if (target->subunit != nullptr)
			{
				overlayTargetUnit = target->subunit;
			}
			else
			{
				overlayTargetUnit = target;
			}
			(overlayTargetUnit->sprite)->create_top_overlay(overlayImageId, 0, 0, 0);
		}

		// this, added to the fact that target->lockdown_timer is set to 0 before this function, probably prevents lockdown to reset the duration of a previous lock
		if (previousLockdownTimer > target->lockdown_timer)
		{
			target->lockdown_timer = previousLockdownTimer;
		}

		// Should stop movements/orders/scripts of the unit, give it the status DoodadStatesThing.
		// May have an effect on selection. Performs scbw::refresh_console() code at the end
		disableUnit(target);
	}

	void WeaponBulletHit(Bullet* bullet, Unit* target, u32 damageDivisor)
	{
		u32 base_damage = helper::get_unit_bullet_damage(bullet, target);
		if (bullet->hitFlags & 2)
		{
			if (bullet->sourceUnit != nullptr)
			{
				target->damage_with(bullet->weaponId, base_damage, bullet->sourceUnit, bullet->srcPlayer, bullet->direction1, damageDivisor);
			}
			else
			{
				helper::attack_overlay_and_notify(bullet->sourceUnit, target, bullet->weaponId, bullet->direction1); // no damage, but still ask for retaliation
			}
		}
		else
		{
			target->damage_with(bullet->weaponId, base_damage, bullet->sourceUnit, bullet->srcPlayer, bullet->direction1, damageDivisor);
		}
	}

	u16 GetUnitWidth(Unit* unit)
	{
		return unit->get_right() - unit->get_left();
	}
	u16 GetUnitHeight(Unit* unit)
	{
		return unit->get_bottom() - unit->get_top();
	}

	bool IsInRange(Unit* source, Unit* target, u32 range)
	{
		u16 sourceX = source->position.x;
		u16 sourceY = source->position.y;
		u16 targetX = target->position.x;
		u16 targetY = target->position.y;

		u16 horizontalDifference = abs(sourceX - targetX);
		u16 verticalDifference = abs(sourceY - targetY);
		u16 horizontalRange = range + GetUnitWidth(source) / 2 + GetUnitWidth(target) / 2 + 1;
		u16 verticalRange = range + GetUnitHeight(source) / 2 + GetUnitHeight(target) / 2 + 1;

		if (horizontalDifference > horizontalRange
			|| verticalDifference > verticalRange)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	void MeleeAttackHit(Unit* attacker)
	{
		UnitExt* attackerExt = globals.get_unit_ext(attacker);
		Unit* target = attacker->order_target.unit;
		if (target != nullptr)
		{
			u8 weaponId;
			// Lakizilisks can only attack while burrowed even if given a melee attack, according to this code
			if (attacker->id != UnitId::ZergLakizilisk
				|| (attacker->status & UnitStatus::Burrowed
					|| attacker->status & UnitStatus::InBuilding)) // Attempt to allow Lakizilisks to attack from within Bunkers, didn't work, but may still be part of the solution
			{
				weaponId = units_dat::GroundWeapon[attacker->id];
			}
			else
			{
				weaponId = WeaponId::None;
			}
			attackerExt->values[ValueAI_KitingSignal] += (1 << 8) / weapons_dat::DamageFactor[weaponId] + 1;
			attackerExt->values[ValueAI_PreventTargeting] = 0;

			if (attacker->status & UnitStatus::IsHallucination)
			{
				if (attacker != nullptr)
				{
					u32 damage = helper::get_unit_damage_bonus(attacker, weaponId);
					damage = ((u16)damage + weapons_dat::DamageAmount[weaponId]) * 256;
					if (attackerExt->values[ValueDeconstruction])
					{
						scbw::do_deconstruction_damage(attacker, weaponId);
					}
					target->damage_with(
						weaponId,
						damage,
						attacker,
						attacker->player,
						attacker->current_direction_1,
						1);
				}
				else
				{
					helper::attack_overlay_and_notify(attacker, target, weaponId, attacker->current_direction_1); // no damage, but still ask for retaliation
				}
			}
			else
			{
				u32 damage = helper::get_unit_damage_bonus(attacker, weaponId);
				damage = ((u16)damage + weapons_dat::DamageAmount[weaponId]) * 256;
				if (attackerExt->values[ValueDeconstruction])
				{
					scbw::do_deconstruction_damage(attacker, weaponId);
				}
				target->damage_with(
					weaponId,
					damage,
					attacker,
					attacker->player,
					attacker->current_direction_1,
					1);
			}

			// Cloaked Units - Decloak
			// Cabalist uses silent strike, so the check has to be in weapon damage
			if (attacker->id != UnitId::ProtossCabalist)
			{
				scbw::decloak_proc(attacker);
			}
		}
	}

	// bullet is not normally part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	// In the original code, the parameters are gotten from Bullet as:
	//		attacker from bullet->sourceUnit
	//		x and y from bullet->sprite->position
	void EMPShockwaveHit(Unit* attacker, s32 x, s32 y, Bullet* bullet)
	{
		Thingy* emp_sprite = helper::create_thingy(371, x, y, 0);
		if (emp_sprite != nullptr)
		{
			emp_sprite->sprite->elevationLevel = 19; // 0x13
			helper::set_thingy_visibility_flags(emp_sprite);
		}
		static u16* const maxBoxRightValue = (u16*)0x00628450;	// should usually be map_tile_size->width * 32
		static u16* const maxBoxBottomValue = (u16*)0x006284B4;	// should usually be map_tile_size->height * 32
		Box16 area_of_effect;
		Unit** unitsInAreaOfEffect;
		Unit* current_unit;

		// define the base area of effect
		area_of_effect.left = (u16)x - weapons_dat::InnerSplashRadius[WeaponId::EMP_Shockwave];
		area_of_effect.right = (u16)x + weapons_dat::InnerSplashRadius[WeaponId::EMP_Shockwave];
		area_of_effect.top = (u16)y - weapons_dat::InnerSplashRadius[WeaponId::EMP_Shockwave];
		area_of_effect.bottom = (u16)y + weapons_dat::InnerSplashRadius[WeaponId::EMP_Shockwave];

		// check and fix effect if beyond width of map
		if (area_of_effect.left < 0)
		{
			area_of_effect.left = 0;
		}
		else if (area_of_effect.right > *maxBoxRightValue)
		{
			area_of_effect.right = *maxBoxRightValue;
		}
		// check and fix effect if beyond height of map
		if (area_of_effect.top < 0)
		{
			area_of_effect.top = 0;
		}
		else if (area_of_effect.bottom > *maxBoxBottomValue)
		{
			area_of_effect.bottom = *maxBoxBottomValue;
		}

		// find all units in area of effect and pick the first
		unitsInAreaOfEffect = helper::get_all_units_in_bounds(&area_of_effect);
		current_unit = *unitsInAreaOfEffect;
		while (current_unit != nullptr)
		{
			if (current_unit != attacker // EMP doesn't affect the attacker
				&& (attacker == nullptr || current_unit != attacker->subunit))
			{
				if (current_unit->status & UnitStatus::IsHallucination)
				{
					current_unit->remove();
				}
				else if (current_unit->stasis_timer == 0) // don't affect units in stasis
				{
					current_unit->energy -= 100;
					current_unit->shields -= 100;
					if (current_unit->sprite->flags & SpriteFlags::Selected
						&& current_unit->sprite != nullptr) // Follows original code, though this order of tests doesn't make sense
					{
						helper::setAllImageGroupFlagsPal11(current_unit->sprite); // redraw the unit
					}
				}
			}
			unitsInAreaOfEffect++; // go on next unit of the list (or null)
			current_unit = *unitsInAreaOfEffect;
		}
		// reload the previous temporary unit list from before the call to get_all_units_in_bounds
		*tempUnitsListArraysCountsListLastIndex = *tempUnitsListArraysCountsListLastIndex - 1;
		*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsList[*tempUnitsListArraysCountsListLastIndex];
	}

	// bullet is not normally part of the function, so if the function is called in an unexpected way, it may not contain the correct value
	void CorrosiveAcidHit(u32 attackingPlayerId, s32 x, s32 y, Bullet* bullet)
	{
		Box16 coords;

		coords.left = x - 64;
		coords.bottom = y + 64;
		coords.top = y - 64;
		coords.right = x + 64;

		// IterateUnitsAtLocationTargetProc_CorrosiveAcid(attackingPlayerId,&coords);
		// spreads originally
		if (bullet->attackTarget.unit != nullptr)
		{
			auto vec = scbw::get_clarion_links(bullet->attackTarget.unit);
			if (vec.size() != 0)
			{
				for (auto unit : vec)
				{
					helper::ApplySpores(unit);
				}
			}
			else
			{
				helper::ApplySpores(bullet->attackTarget.unit);
			}
		}
	}

	// no bullet because the code normally calling this function lose the easily available information (and getting it the hard way seems overkill)
	void ConsumeHit(Unit* attacker, Unit* target)
	{
		if (attacker == nullptr
			|| target == nullptr)
		{
			return;
		}

		if (target != nullptr
			&& !(target->status & UnitStatus::Invincible)
			&& helper::get_spell_stat_string(target, TechId::Consume, attacker->player) == 0) // returning a value != 0 means there was an error
		{
			helper::increment_unit_death_scores(target, attacker->player);
			if (!(target->status & UnitStatus::IsHallucination))
			{
				EntryArray units = unit_search_radial(attacker->position, 3 * 32);
				unit_search_filter(&units, attacker, FilterAlly | FilterIncludingSelf);
				for (u32 i = 0; i < units.size; ++i) {
					Unit* unit = units.units[i];

					u32 power = target->hitpoints;
					u32 energyMax = unit->get_max_energy();
					u32 diff = energyMax - unit->energy;
					if (power > diff)
					{
						unit->energy = energyMax;
						power -= diff;
						unit->heal_overflow(power, attacker);
					}
					else
					{
						unit->heal_energy(power, attacker);
					}
					unit->sprite->create_underlay(ImageId::SmallExplosion_Unused);
				}
			}
			target->remove();
		}
	}

	// bullet is not normally part of the function, so if the function is called
	// in an unexpected way, it may not contain the correct value.
	void EnsnareHit(Unit* attacker, s32 x, s32 y, Bullet* bullet)
	{
		Thingy* ensnare_effect_sprite = helper::create_thingy(SpriteId::Ensnare, x, y, 0);
		if (ensnare_effect_sprite != nullptr)
		{
			ensnare_effect_sprite->sprite->elevationLevel = 19; // 0x13
			helper::set_thingy_visibility_flags(ensnare_effect_sprite);
		}
		Box16 coords;
		coords.top = y - 64;
		coords.right = x + 64;
		coords.left = x - 64;
		coords.bottom = y + 64;
		helper::IterateUnitsAtLocationTargetProc_Ensnare(attacker, &coords);
	}

	s32 ensnareProc(Unit* target, Unit* attacker)
	{
		if (target != attacker
			&& !(units_dat::BaseProperty[target->id] & UnitProperty::Building)
			&& target->id != UnitId::ZergInfestedTerran)
		{
			if (!(target->status & UnitStatus::Invincible) && !(target->status & UnitStatus::Burrowed))
			{
				helper::createEnsnareOverlay(target);
			}
		}
		return 0;
	}

	// bullet is not normally part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void DarkSwarmHit(s32 x, s32 y, u32 attackingPlayerId, Bullet* bullet)
	{
		Point16 coords;
		Unit* dark_swarm_unit;
		coords.x = x;
		coords.y = y;

		// adjust location where Dark Swarm will be created if necessary (by modifying coords)
		helper::fix_target_location(&coords, UnitId::Spell_DarkSwarm);

		// the dark swarm is created for player 11 (neutral), not for the owner
		dark_swarm_unit = helper::create_unit(UnitId::Spell_DarkSwarm, coords.x, coords.y, 11);

		if (dark_swarm_unit == nullptr)
		{
			helper::display_last_network_error_for_player(attackingPlayerId);
		}
		else
		{
			dark_swarm_unit->status |= UnitStatus::NoCollide;
			dark_swarm_unit->sprite->elevationLevel = 11;

			// update various stuff (set hp, set shield...) not finished on create_unit
			helper::update_unit_stats_and_finish_building(dark_swarm_unit);
			helper::update_unit_strength(dark_swarm_unit);
			if (dark_swarm_unit->remove_timer == 0
				|| dark_swarm_unit->remove_timer > 900)
			{
				dark_swarm_unit->remove_timer = 900;	// 0x0384
			}
		}
	}

	// bullet is not normally part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void BroodlingHit(Unit* attacker, Unit* target, Bullet* bullet)
	{
		u32 broodlings_to_spawn = 3;
		ActiveTile tile = scbw::get_stupid_reference_to_active_tile_at(bullet->sprite->position.x, bullet->sprite->position.y);
		if (tile.has_creep || tile.creep_receeding)
		{
			broodlings_to_spawn = 5;
		}
		s32 current_x = bullet->sprite->position.x - 2;
		s32 current_y = bullet->sprite->position.y - 2;
		while (broodlings_to_spawn > 0)
		{
			bool jump_to_end_of_loop = false; // 004F4A82
			// check various conditions before attempting to create a broodling
			if (helper::can_unit_id_fit_at_xy(current_x, current_y, UnitId::ZergRilirokor))
			{
				Point16 inPos, outPos;
				Box16 moveArea;

				inPos.y = current_y;
				moveArea.top = current_y - 32;
				moveArea.bottom = current_y + 32;
				moveArea.left = current_x - 32;
				inPos.x = current_x;
				moveArea.right = current_x + 32;

				// if impossible to avoid a collision even within moveArea,
				// skip the broodling creation code for the current position
				jump_to_end_of_loop = !scbw::check_unit_collision_pos(attacker, &inPos, &outPos, &moveArea, false, 0);

				if (!jump_to_end_of_loop)
				{
					current_y = outPos.y;
					current_x = outPos.x;
				}
			}

			jump_to_end_of_loop = false; // <- allows to target air units, and presumably terrain, if only the function would actually be called
			if (!jump_to_end_of_loop)
			{ // 004F49E2
				Point16 pos;
				Unit* created_broodling;
				pos.x = current_x;
				pos.y = current_y;

				// adjust the position to welcome the unit...I think
				helper::fix_target_location(&pos, UnitId::ZergRilirokor);

				// create the broodling
				created_broodling = helper::create_unit(UnitId::ZergRilirokor, pos.x, pos.y, attacker->player);
				UnitExt* broodlingExt = globals.get_unit_ext(created_broodling);
				broodlingExt->units[UnitBroodlingMother] = attacker;

				if (created_broodling == nullptr)
				{
					helper::display_last_network_error_for_player(attacker->player);
					jump_to_end_of_loop = true;
				}
				else
				{ // 004F4A26
					helper::play_sound_from_unit(SoundId::Zerg_BROODLING_ZBrRdy00_WAV, created_broodling, 1, 0);
					u32 timerBeforeDisappearance;

					// update various stuff (set hp, set shield...) not finished on create_unit
					helper::update_unit_stats_and_finish_building(created_broodling);
					helper::update_unit_strength(created_broodling);
					timerBeforeDisappearance = 720;	// 0x708
					if (created_broodling->remove_timer == 0
						|| created_broodling->remove_timer > timerBeforeDisappearance)
					{
						created_broodling->remove_timer = timerBeforeDisappearance;
					}

					if (attacker->unit_ai != nullptr)
					{
						helper::ai_training_unit(created_broodling, attacker);
					}
				}
			}
			// 004F4A82 (end of loop)
			current_y += 4;
			current_x += 4;
			broodlings_to_spawn--;
		}
	}

	// bullet is not normally part of the function, so if the function is called in an unexpected way, it may not contain the correct value.
	void PlagueHit(Unit* attacker, s32 x, s32 y, Bullet* bullet)
	{
		if (attacker) {
			globals.create_effect(EffectId_PlagueField, 10 * 32, {x, y}, 3 * 32, SpriteId::PlagueField, 5, attacker->player, attacker);
		}
		//if (attacker) {
		//	u32 distance = 64;
		//	Box16 coords = {};
		//	coords.left   = x - distance;
		//	coords.top    = y - distance;
		//	coords.right  = x + distance;
		//	coords.bottom = y + distance;
		//	helper::iterate_units_at_location_target_proc_plague(attacker, &coords);
		//
		//	UnitExt* attackerExt = globals.get_unit_ext(attacker);
		//	scbw::UnitFinder(
		//		x - distance,
		//		y - distance,
		//		x + distance,
		//		y + distance
		//	).for_each([&](Unit* unit) {
		//		if (attacker && attackerExt->timers[TimerEnsnaringBrood] > 0
		//			&& !(units_dat::BaseProperty[unit->id] & UnitProperty::Building))
		//		{
		//			scbw::apply_ensnare(unit);
		//		}
		//	});
		//}
	}

	// bullet is not normally part of the function, so if the function is called
	// in an unexpected way, it may not contain the correct value.
	void MaelstromHit(Unit* attacker, s32 x, s32 y, Bullet* bullet)
	{
		Thingy* maelstrom_effect_sprite = helper::create_thingy(SpriteId::Maelstrom_Hit, x, y, 0);
		if (maelstrom_effect_sprite != nullptr)
		{
			maelstrom_effect_sprite->sprite->elevationLevel = 17;
			helper::set_thingy_visibility_flags(maelstrom_effect_sprite);
		}
		helper::play_sound_from_unit(SoundId::Protoss_Darchon_Parahit_wav, attacker, 1, 0);

		const u32 radius = 48;
		scbw::UnitFinder(
			x - radius,
			y - radius,
			x + radius,
			y + radius
		).for_each([attacker](Unit* unit)
		{
			maelstromProc(unit, attacker);
		});
	}

	// Hooked to modify maelstrom timer and conditions; however, this hook is not called yet, we may need to hook iterateUnitsAtLocationTargetProc - 4E8280
	bool maelstromProc(Unit* target, Unit* source) // 004F6760, always returns 0
	{
		s32 unitStatus = target->status;
		if (unitStatus & UnitStatus::IsHallucination)
		{
			helper::orders_self_destructing(target);
		}
		else
		{
			if (target != source && !(unitStatus & UnitStatus::Invincible))
			{
				s32 unitProperty = units_dat::BaseProperty[target->id];
				if (!(unitStatus & UnitStatus::Burrowed))
				{
					helper::set_maelstrom_timer(target, MindTyrantMaelstromTimer);
				}
			}
		}
		return 0;
	}

	// no bullet because this function is called in another context without bullet
	void MindControlHitHook(Unit* attacker, Unit* target)
	{
		scbw::decloak_proc(attacker);
		if (target->status & UnitStatus::IsHallucination)
		{
			target->remove();
		}
		else
		{
			scbw::mind_control(attacker, target);
		}
	}

	// bullet is not normally part of the function, so if the function is called
	// in an unexpected way, it may not contain the correct value.
	void StasisFieldHit(Unit* attacker, s32 x, s32 y, Bullet* bullet)
	{
		Thingy* stasisfield_effect_sprite = helper::create_thingy(SpriteId::Stasis_Field_Hit, x, y, 0);
		if (stasisfield_effect_sprite != nullptr)
		{
			stasisfield_effect_sprite->sprite->elevationLevel = 17; // 0x11
			helper::set_thingy_visibility_flags(stasisfield_effect_sprite);
		}
		Box16 coords;
		coords.top = y - 48;
		coords.right = x + 48;
		coords.left = x - 48;
		coords.bottom = y + 48;
		helper::IterateUnitsAtLocationTargetProc_StasisField(attacker, &coords);
	}

	// bullet is not normally part of the function, so if the function is called
	// in an unexpected way, it may not contain the correct value.
	Unit* createDisruptionWeb(u32 attackingPlayerId, s32 x, s32 y)
	{
		Point16 coords;
		Unit* disruption_web_unit;
		coords.x = x;
		coords.y = y;

		// adjust location where Disruption Web will be created if necessary (by modifying coords)
		helper::fix_target_location(&coords, UnitId::Spell_DisruptionWeb);
		disruption_web_unit = helper::create_unit(UnitId::Spell_DisruptionWeb, coords.x, coords.y, 11);
		if (disruption_web_unit == nullptr)
		{
			helper::display_last_network_error_for_player(attackingPlayerId);
		}
		else
		{
			disruption_web_unit->status |= UnitStatus::NoCollide;
			disruption_web_unit->sprite->elevationLevel = 11;
			// update various stats (set hp, set shield...) not finished on create_unit
			helper::update_unit_stats_and_finish_building(disruption_web_unit);
			helper::update_unit_strength(disruption_web_unit);
			if (disruption_web_unit->remove_timer == 0
				|| disruption_web_unit->remove_timer > 360)
			{
				disruption_web_unit->remove_timer = 360; // 0x0168
			}
		}
		return disruption_web_unit;
	}

	void DisruptionWebHit(u32 attackingPlayerId, s32 x, s32 y, Bullet* bullet)
	{
		auto unit = createDisruptionWeb(attackingPlayerId, x, y);
	}

	u8 stasisFieldOverlay(Unit* unit, u8 timer)
	{
		if (!unit->stasis_timer)
		{
			Unit* finalUnit = unit->subunit;
			if (!finalUnit)
			{
				finalUnit = unit;
			}
			u16 overlayImageId = ImageId::StasisField_Small + scbw::get_overlay_offset(unit->id);
			unit->sprite->create_overlay(overlayImageId, 0, 0, 0);
			unit->status |= UnitStatus::Invincible; // remove
		}
		u8 finalTimer = unit->stasis_timer;
		if (timer > finalTimer)
		{
			finalTimer = timer;
		}
		unit->stasis_timer = finalTimer;
		return disableUnit(unit);
	}

	void createScanner(Unit* unit) // 00464E40
	{
		Unit* v1; // esi
		u16 v2; // ax
		bool v3; // zf
		GroupFlag targetsFound2; // al
		u8 iterate; // al
		s16 v6; // cx
		s16 v7; // ax
		const char* v8; // eax
		s16 v9; // cx
		Unit* v10; // eax
		Unit* v11; // ebx
		s16 v12; // ax
		Point16 a2; // [esp+4h] [ebp-4h]

		v1 = unit;
		if (!(*bw_cheat_state & 0x800))
		{
			v2 = techdata_dat::EnergyCost[4] * 256;
			v3 = v1->energy >= v2;
			//			a2 = (Point32)(v1->energy < v2);		// check this out
			if (!v3)
			{
				targetsFound2 = units_dat::GroupFlags[(u16)v1->id];
				if (targetsFound2.is_zerg)
				{
					iterate = 0;
				}
				else if (targetsFound2.is_terran)
				{
					iterate = 1;
				}
				else
				{
					iterate = (targetsFound2.is_protoss) != 0 ? 2 : 4;
				}
				v6 = iterate + 864;
				v7 = iterate + 863;
				if (!v6)
				{
					goto LABEL_15;
				}
				if (v7 < *statTxtTbl)
				{
					v8 = (char*)statTxtTbl + statTxtTbl[v7 + 1];
					if (!v8)
					{
LABEL_15:
						if (v1->order_queue_head)
						{
							v1->user_action_flags |= 1u;
							helper::orders_nothing_2(v1);
						}
						else if (v1->unit_ai)
						{
							helper::issue_order_targeting_nothing(v1, 0x9Cu);
						}
						else
						{
							helper::issue_order_targeting_nothing(v1, units_dat::ReturnToIdleOrder[(u16)v1->id]);
						}
						return;
					}
				}
				else
				{
					v8 = "";
				}
				if ((u8)v1->player == *LOCAL_NATION_ID && *ACTIVE_PLAYER_ID == *LOCAL_HUMAN_ID)
				{
					helper::setUnitStatTxtErrorMsg((s32)v8);
				}
				goto LABEL_15;
			}
		}
		v9 = v1->order_target.pt.x;
		a2.y = v1->order_target.pt.y;
		a2.x = v9;
		helper::fix_target_location(&a2, UnitId::Spell_ScannerSweep);
		v10 = helper::create_unit(0x21u, a2.x, a2.y, (u8)v1->player);
		v11 = v10;
		if (v10)
		{
			helper::update_unit_stats_and_finish_building(v10);
			helper::update_unit_strength(v11);
			helper::play_sound_from_unit(388, v1, 1, 0);
			if (!(*bw_cheat_state & 0x800))
			{
				v1->spend_unit_energy(techdata_dat::EnergyCost[4] * 256);
			}
			helper::to_idle(v1);
		}
		else
		{
			helper::display_last_network_error_for_player(v1->player);
			helper::to_idle(v1);
		}
	}

	// Hooked to add Nydus handling, except it already existed - Pr0nogo
	b32 giveUnitToPlayer(Unit* unit, u32 playerId) // 004C8040
	{
		u32 playerIdVar = playerId;
		if (playerId == 13)
		{
			playerIdVar = *currentTriggerPlayerId;
		}
		if (!(playerIdVar < 0xC ? -(unit != 0) : 0))
		{
			helper::SErrSetLastError(87);
			return 0;
		}
		if (unit->status & UnitStatus::IsHallucination)
		{
			return 0;
		}
		s16 unit_id = unit->id;
		if (unit_id == UnitId::ResourceMineralField
		  || unit_id == UnitId::ResourceMineralFieldType2
		  || unit_id == UnitId::ResourceMineralFieldType3
		  || unit_id == UnitId::ResourceVespeneGeyser
		  || unit_id == UnitId::ProtossSteward
		  || unit_id == UnitId::ProtossPlasmaShell
		  || units_dat::BaseProperty[unit_id] & UnitProperty::Addon
		  || (u8)unit->player == playerIdVar)
		{
			return 0;
		}
		if (helper::unit_is_geyser(unit))
		{
			Unit* hiddenMiner = helper::getHiddenMiner(unit);
			if (hiddenMiner)
			{
				helper::giveAllUnitPropertiesToPlayer(hiddenMiner, playerIdVar);
			}
		}
		helper::giveAllUnitPropertiesToPlayer(unit, playerIdVar);
		const u32 giveUnitsProc = 0x004C7CE0;
		if (helper::unit_is_active_transport(unit))
		{
			helper::iterateLoadedUnits(unit, (Unit*(__fastcall*)(Unit*, Unit*))giveUnitsProc, playerIdVar);
		}
		if (helper::unitHasHanger(unit))
		{
			helper::forHangarUnits(unit, (Unit*(__fastcall*)(Unit*, Unit*))giveUnitsProc, playerIdVar);
		}
		if (unit->status & UnitStatus::GroundedBuilding)
		{
			Unit* connected_unit = unit->building.addon;
			if (connected_unit)
			{
				helper::giveAllUnitPropertiesToPlayer(connected_unit, playerIdVar);
			}
			if (helper::is_constructing_addon(unit))
			{
				Unit* creatingUnit = helper::getCreatingUnit(unit);
				helper::giveAllUnitPropertiesToPlayer(creatingUnit, playerIdVar);
			}
			if (unit->id == UnitId::ZergNydusCanal)
			{
				Unit* nydus_exit = unit->building.nydus_exit;
				if (nydus_exit)
				{
					helper::giveAllUnitPropertiesToPlayer(nydus_exit, playerIdVar);
				}
			}
		}
		return 1;
	}


}