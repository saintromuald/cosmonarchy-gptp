#include "update_status_effects.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "hooks/weapons/weapon_spellhit.h"
#include "hooks/weapons/weapon_damage.h"
#include "globals.h"

void irradiate_proc(Unit* unit, Unit* target)
{
	if (!unit)
	{
		return;
	}

	// Check if the unit is within distance, or is inside the same transport
	if (unit->status & UnitStatus::InTransport
	||  unit->get_distance_to_target(target) <= 32)
	{
		// const s32 damage = weapons_dat::damage_amount[WeaponId::Irradiate] * 256 / weapons_dat::cooldown[WeaponId::Irradiate];
		s32 damage = (125 * 256) / 24;
		if (unit != nullptr)
		{
			if (unit->irradiate_timer == 0)
			{
				damage += (125 * 256) % 24;
				//scbw::print("Add additional damage");
			}
		}
		target->damage_with_custom(50.0 * 256.0 / 15.5, -1, unit->irradiate_caster, unit->irradiate_player_id);
	}
}
	
namespace hooks
{
	void do_irradiate_damage(Unit* unit)
	{
		if (unit->status & UnitStatus::InTransport)
		{
			// If inside a transport, damage all units loaded within
			Unit* transport = unit->connected_unit;
			if (transport)
			{
				for (s32 i = 0; i < units_dat::space_provided[transport->id]; ++i)
				{
					Unit* loaded_unit = transport->get_loaded_unit(i);
					if (loaded_unit) {
						irradiate_proc(unit, loaded_unit);
					}
				}
			}
		}
		else
		{
			// Find and iterate nearby units
			// TODO: unit search radial
			scbw::UnitFinder(
				unit->get_x() - 160,
				unit->get_y() - 160,
				unit->get_x() + 160,
				unit->get_y() + 160
			).for_each([&](Unit* target)
			{
				irradiate_proc(unit, target);
			});
		}
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
			auto tile = scbw::get_stupid_reference_to_active_tile_at(unit->position.x, unit->position.y);
			if (tile.currently_occupied)
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
				unit->irradiate_caster = nullptr;
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

	void ai_burrower(Unit* unit) // 004A1340
	{
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueDisableAutoBurrow]) {
			return;
		}

		if (units_dat::base_property[unit->id] & UnitProperty::Burrowable)
		{
			if (!(unit->status & UnitStatus::Burrowed) && unit->id != UnitId::ZergDroleth)
			{
				if (unit->main_order_id == OrderId::ComputerAI || unit->main_order_id == OrderId::Guard)
				{
					if (unit->move_target.pt.x == unit->sprite->position.x
						&& unit->move_target.pt.y == unit->sprite->position.y
						&& ((unit->status & UnitStatus::Unmovable) != 0) != -1
						// Originally checked for burrow tech here
						)
					{
						bool can_burrow = true;
						ActiveTile tile = scbw::get_stupid_reference_to_active_tile_at(unit->position.x, unit->position.y);
						if (tile.currently_occupied)
						{
							Unit* nearest = scbw::UnitFinder::get_nearest_target(
								unit->get_left() - 32,
								unit->get_top() - 32,
								unit->get_right() + 32,
								unit->get_bottom() + 32,
								unit,
								[&](Unit* target)
							{
								return safety_unburrow_check(unit, target);
							}
							);
							if (nearest)
							{
								can_burrow = false;
							}
						}
						if (can_burrow)
						{
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
}