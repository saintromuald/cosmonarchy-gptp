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

	//Irradiate splash damage does not affect burrowed units
	if (target != unit && target->status & UnitStatus::Burrowed)
	{
		return;
	}

	//Check if the unit is within distance, or is inside the same transport
	if (unit->status & UnitStatus::InTransport
		|| unit->get_distance_to_target(target) <= 32)
	{
		// const s32 damage = weapons_dat::DamageAmount[WeaponId::Irradiate] * 256 / weapons_dat::Cooldown[WeaponId::Irradiate];
		s32 damage = (125 * 256) / 24;
		if (unit != nullptr)
		{
			if (unit->irradiate_timer == 0)
			{
				damage += (125 * 256) % 24;
				//scbw::print("Add additional damage");
			}
		}
		target->damage_with_custom(50.0 * 256.0 / 15.5, DamageType::Concussive, unit->irradiate_caster, unit->irradiate_player_id, 0, 1, true);
	}
}
	
namespace hooks
{
	void do_irradiate_damage(Unit* irradiatedUnit)
	{
		if (irradiatedUnit->status & UnitStatus::Burrowed)
		{
			// No splash if burrowed
			irradiate_proc(irradiatedUnit, irradiatedUnit);
		}
		else if (irradiatedUnit->status & UnitStatus::InTransport)
		{
			// If inside a transport, damage all units loaded within
			Unit* transport = irradiatedUnit->connected_unit;
			if (transport)
			{
				for (s32 i = 0; i < units_dat::SpaceProvided[transport->id]; ++i)
				{
					Unit* loaded_unit = transport->get_loaded_unit(i);
					if (loaded_unit)
						irradiate_proc(irradiatedUnit, loaded_unit);
				}
			}

		}
		else
		{
			// Find and iterate nearby units
			scbw::UnitFinder(
				irradiatedUnit->get_x() - 160,
				irradiatedUnit->get_y() - 160,
				irradiatedUnit->get_x() + 160,
				irradiatedUnit->get_y() + 160
			).for_each([&](Unit* target)
			{
				irradiate_proc(irradiatedUnit, target);
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
		if (!helper::func_dounitscollide(unit, target))
		{
			return false;
		}
		return true;
	}

	void update_status_effects(Unit* unit) // 00492F70
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		if (unit->subunit != nullptr)
		{
			UnitExt* subunitExt = globals.get_unit_ext(unit->subunit);
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

		// Quarry, Iroleth, Pennant resource regeneration aura
		u32 unit_id = unit->id;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Building && unit->building.addon) {
			unit_id = unit->building.addon->id;
		}
		if (unit->sprite && unit->main_order_id != OrderId::Die
			&& (unit_id == UnitId::TerranQuarry || unit_id == UnitId::ZergIroleth || unit_id == UnitId::ProtossPennant))
		{
			u32 radius = 0;
			switch (unit_id) {
				case UnitId::TerranQuarry:
				{
					radius = 320;
				} break;
				case UnitId::ZergIroleth:
				{
					radius = 224;
				} break;
				case UnitId::ProtossPennant:
				{
					radius = 160;
				} break;
			}

			scbw::UnitFinder(
				unit->position.x - radius,
				unit->position.y - radius,
				unit->position.x + radius,
				unit->position.y + radius
			).for_each([&](Unit* target)
			{
				if (unit == target)
				{
					return;
				}
				if (!(unit->status & UnitStatus::Completed))
				{
					return;
				}
				if (units_dat::BaseProperty[target->id] & UnitProperty::ResourceContainer)
				{
					UnitExt* targetExt = globals.get_unit_ext(target);
					targetExt->timers[TimerResourceBoostedRegen] = 24;
					u32 underlay = ImageId::ResourceBoostOverlay_Small + scbw::get_overlay_offset(target->id);
					if (!target->sprite->get_overlay(underlay)) {
						if (scbw::unit_id_is_gas_structure(target->id))
						{
							target->sprite->create_underlay(underlay, -5, 2);
						}
						else
						{
							target->sprite->create_underlay(underlay);
						}
					}
				}
			});
		}

		// Sire Ectomorphic Accelerant - egg morph speed aura
		if (unit->id == UnitId::ZergSire)
		{
			u32 radius = 224;
			scbw::UnitFinder(
				unit->get_x() - radius,
				unit->get_y() - radius,
				unit->get_x() + radius,
				unit->get_y() + radius
			).for_each([&](Unit* target)
			{
				if (unit == target)
				{
					return;
				}
				if (!(unit->status & UnitStatus::Completed))
				{
					return;
				}
				if (scbw::is_allied_to(target->player, unit->player)
					&& (target->id == UnitId::ZergEgg || target->id == UnitId::ZergCocoon))
				{
					UnitExt* targetExt = globals.get_unit_ext(target);
					targetExt->timers[TimerSireEggBoost] = 24;
				}
			});
		}

		// Ghost Somatic Implants behavior
		if (unit->id == UnitId::TerranEidolon)
		{
			if (!(unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection)))
			{
				if (unitExt->values[ValueSomaticImplants] == 1)
				{
					unit->update_speed();
					unitExt->values[ValueSomaticImplants] = 0;
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

		// Why are we updating Vorvrokor / Ovileth speed every frame?
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

		// Redraw console/UI elements
		if (unit == *activePortraitUnit)
		{
			if (unit->is_valid_caster())
			{
				if (!scbw::is_allied_to(*LOCAL_NATION_ID, unit->player))
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
				s32 damage = (weapons_dat::DamageAmount[WeaponId::Plague] * 256) / 30;
				if (!unit->plague_timer)
				{
					damage += (weapons_dat::DamageAmount[WeaponId::Plague] * 256) % 30;
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
			scbw::placeClarionLinks(unit); //clears, then places all clarion links
		}

		if (unit->is_under_storm != 0)
		{
			unit->is_under_storm--;
		}

		// Vithrilisk - Corrosive Touch - update overlays
		if (unitExt->values[ValueCorrosiveTouchStacks] != 0)
		{
			// Calculate the appropriate overlay ID
			u32 acidOverlayId = get_spores_overlay_id(unit);
			if (!unit->get_overlay(acidOverlayId))
			{
				unit->remove_overlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
				if (unit->subunit)
				{
					unit = unit->subunit;
				}
				unit->sprite->create_top_overlay(acidOverlayId);
			}
		}
	}

	void ai_burrower(Unit* unit) // 004A1340
	{
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Burrowable)
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
						bool canBurrow = true;
						auto tile = scbw::get_stupid_reference_to_active_tile_at(unit->position.x, unit->position.y);
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
								canBurrow = false;
							}
						}
						if (canBurrow)
						{
							if (unit->id != UnitId::ZergLakizilisk) {
								EntryArray array = unit_search_radial(unit->position, 10 * 32);
								unit_search_filter(&array, unit, FilterAlly | FilterBuilding);
								if (array.size) {
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
}