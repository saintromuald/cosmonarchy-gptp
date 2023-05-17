#include "update_status_effects.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "hooks/weapons/weapon_spellhit.h"
#include "hooks/weapons/weapon_damage.h"
#include "globals.h"
	
namespace hooks
{
	void do_irradiate_damage(Unit* unit) {
		unit->damage_with_custom(50.0 * 256.0 / 15.5, -1, unit->irradiate_caster, unit->irradiate_player_id);
	}

	static bool safety_unburrow_check(Unit* unit, Unit* target)
	{
		if (!(target->status & UnitStatus::GroundedBuilding))
		{
			return false;
		}
		if (target == unit)
		{
			return false;
		}
		if (!helper::do_units_collide(unit, target))
		{
			return false;
		}
		return true;
	}

	void update_status_effects(Unit* unit) // 00492F70
	{
		UnitExt* unit_ext = get_unit_ext(unit);

		if (unit->subunit != nullptr)
		{
			UnitExt* subunitExt = get_unit_ext(unit->subunit);
			subunitExt->units[UnitTurretParentLink] = unit;
		}

		// Decrement Stasis Field timer (and remove effect if 0)
		if (unit->stasis_timer != 0)
		{
			unit->stasis_timer--;
			if (unit->stasis_timer == 0)
			{
				unit->unfreeze();
			}
		}

		// Forcibly unburrow units that collide with buildings
		if (unit->status & UnitStatus::Burrowed)
		{
			ActiveTile* tile = scbw::get_active_tile_at_coords(unit->position.x, unit->position.y);
			if (tile->currently_occupied)
			{
				// TODO: unit search
				Unit* nearest = scbw::UnitFinder::get_nearest_target(
					unit->get_left() - 32,
					unit->get_top() - 32,
					unit->get_right() + 32,
					unit->get_bottom() + 32,
					unit, [&](Unit* target)
				{
					return safety_unburrow_check(unit, target);
				});
				bool canBurrowAfter = false;
				// from Ai_Burrowable
				if (nearest != nullptr)
				{
					helper::unburrow(unit);
				}
			}
		}

		if (unit->stim_timer != 0)
		{
			unit->stim_timer--;
			if (unit->stim_timer == 0)
			{
				unit->update_speed();
			}
		}

		// TODO: Why are we updating Vorvrokor / Ovileth speed every frame?
		if (unit->id == UnitId::ZergVorvrokor || unit->id == UnitId::ZergOvileth)
		{
			unit->update_speed();
		}

		// Decrement Ensnare timer (and remove effect if 0)
		if (unit->ensnare_timer != 0)
		{
			unit->ensnare_timer--;
			if (unit->ensnare_timer == 0)
			{
				unit->remove_overlay(ImageId::EnsnareOverlay_Small, ImageId::EnsnareOverlay_Large);
				unit->update_speed();
			}
		}

		// Decrement Defensive Matrix timer (and remove effect if 0)
		if (unit->defensive_matrix_timer != 0)
		{
			unit->defensive_matrix_timer--;
			if (unit->defensive_matrix_timer == 0)
			{
				unit->reduce_defensive_matrix_hp(unit->defensive_matrix_hp);
			}
		}

		// Redraw console and UI elements
		if (unit == *g_active_portrait_unit)
		{
			if (unit->is_valid_caster())
			{
				if (!scbw::is_allied_to(*g_local_nation_id, unit->player))
				{
					scbw::refresh_console();
				}
			}
		}

		// Irradiate ability
		if (unit->irradiate_timer != 0)
		{
			unit->irradiate_timer--;
			do_irradiate_damage(unit);
			if (unit->irradiate_timer == 0)
			{
				unit->remove_overlay(ImageId::IrradiateNew_Small, ImageId::IrradiateNew_Large);
				unit->irradiate_caster = 0;
				unit->irradiate_player_id = 8;
			}
		}

		if (unit->lockdown_timer != 0)
		{
			unit->lockdown_timer--;
			if (unit->lockdown_timer == 0)
			{
				unit->remove_lockdown();
			}
		}

		if (unit->maelstrom_timer != 0)
		{
			unit->maelstrom_timer--;
			if (unit->maelstrom_timer == 0)
			{
				unit->remove_maelstrom();
			}
		}

		if (unit->plague_timer)
		{
			unit->plague_timer--;

			if (!(unit->status & UnitStatus::Invincible))
			{
				s32 damage = (weapons_dat::damage_amount[WeaponId::Plague] * 256) / 30;
				if (!unit->plague_timer)
				{
					damage += (weapons_dat::damage_amount[WeaponId::Plague] * 256) % 30;
				}
				if (damage > 0)
				{
					unit->damage_with(WeaponId::Plague, damage);
				}
			}

			if (!unit->plague_timer)
			{
				unit->remove_overlay(ImageId::PlagueOverlay_Small, ImageId::PlagueOverlay_Large);
			}

		}

		if (unit->id == UnitId::ProtossClarion && scbw::get_aise_value(unit, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
		{
			scbw::place_clarion_links(unit); // Clears, then places all Clarion links
		}

		if (unit->is_under_storm != 0)
		{
			unit->is_under_storm--;
		}

		// Vithrilisk - Corrosive Touch - update overlays
		if (unit_ext->values[ValueCorrosiveTouchStacks] != 0)
		{
			// Calculate the appropriate overlay ID
			u32 acid_overlay_id = get_spores_overlay_id(unit);
			if (!unit->get_overlay(acid_overlay_id))
			{
				unit->remove_overlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
				if (unit->subunit)
				{
					unit = unit->subunit;
				}
				unit->sprite->create_top_overlay(acid_overlay_id);
			}
		}
	}

	void ai_burrower(Unit* unit) { // 004A1340
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueDisableAutoBurrow]) {
			return;
		}

		if (units_dat::base_property[unit->id] & UnitProperty::Burrowable) {
			if (!(unit->status & UnitStatus::Burrowed) && unit->id != UnitId::ZergDroleth) {
				if (unit->main_order_id == OrderId::ComputerAI || unit->main_order_id == OrderId::Guard) {
					if (unit->move_target.pt.x == unit->sprite->position.x
						&& unit->move_target.pt.y == unit->sprite->position.y
						&& ((unit->status & UnitStatus::Unmovable) != 0) != -1
						// Originally checked for burrow tech here
						) {
						bool can_burrow = true;
						ActiveTile* tile = scbw::get_active_tile_at_coords(unit->position.x, unit->position.y);
						if (tile->currently_occupied) {
							Unit* nearest = scbw::UnitFinder::get_nearest_target(
								unit->get_left() - 32,
								unit->get_top() - 32,
								unit->get_right() + 32,
								unit->get_bottom() + 32,
								unit, [&](Unit* target) {
								return safety_unburrow_check(unit, target);
							});
							if (nearest) {
								can_burrow = false;
							}
						}
						if (can_burrow) {
							if (unit->id != UnitId::ZergLakizilisk) {
								EntryArray array = {};
								unit_search_radial(&array, unit->position, 10 * 32);
								unit_search_filter(&array, unit, FilterAlly | FilterBuilding);
								if (array.count) {
									return; // Disable idle burrowing close to infrastructure
								}
							}
							helper::issue_order_targeting_nothing(unit, OrderId::Burrow);
						}
					}
				}
			}
		}
	}

	// Hooked to reveal Lakizilisks like cloaked units. -Pr0nogo
	bool unit_is_always_detected(Unit* unit) { // 00491810
		UnitExt* unit_ext = get_unit_ext(unit);
		return unit->lockdown_timer
			|| unit->maelstrom_timer
			|| unit->irradiate_timer
			|| unit->ensnare_timer
			|| unit->stasis_timer
			|| unit->plague_timer
			|| unit_ext->timers[TimerDecloakBurrow];
	}

	// Hooked to disallow lifted Terran structures from being detectors. -Pr0nogo
	b32 unit_is_clean_detector(Unit* unit) // 00403430
	{
		if ((units_dat::base_property[unit->id] & UnitProperty::Detector) && unit->status & UnitStatus::Completed)
		{
			if (!(units_dat::base_property[unit->id] & UnitProperty::Building)
			||   (units_dat::base_property[unit->id] & UnitProperty::Building && !(unit->status & UnitStatus::InAir))) // is not a building OR is a building and is not in air
			{
				if (!unit->is_frozen() && !unit->is_blind)
				{
					return true;
				}
			}
		}
		return false;
	}

	// Hooked to hide new resource overlays when burrowed. -Pr0nogo
	void refresh_carryable_sprite(Sprite* sprite) // 00497750
	{
		Image* overlay = scbw::get_powerup_overlay(sprite);
		if (overlay) {
			if (!(overlay->flags & ImageFlags::Invisible))
			{
				if (!(g_game_layer->has_been_refreshed & 1))
				{
					helper::refresh_image(overlay);
				}
				overlay->flags |= ImageFlags::Invisible;
			}
		}
	}

	// Hooked to show new resources after unburrowing. -Pr0nogo
	void update_carryable_sprite_flags(Sprite* sprite) // 004981B0
	{
		Image* overlay = scbw::get_powerup_overlay(sprite);
		if (overlay) {
			if (overlay->flags & ImageFlags::Invisible)
			{
				overlay->flags = overlay->flags & 0xFFBF | 1;
			}
		}
	}
}