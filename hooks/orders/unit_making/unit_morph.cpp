#include <SCBW/api.h>
//copied from unhooked
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations/TechId.h>
#include <cassert>
#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "hooks\orders\ai\ai_orders.h"
#include "globals.h"

namespace hooks
{
	void orders_zerg_birth(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		if (!(unit->orderSignal & 4))
		{
			Image* current_image = unit->sprite->mainGraphic;

			IscriptAnimation::Enum animation = IscriptAnimation::SpecialState1;
			if (unitExt->values[ValueSpecialSovrolethBirth]) {
				animation = IscriptAnimation::Unused2;
			}
			if (current_image->animation != animation)
			{
				current_image->play_iscript_animation(animation);
			}
		}

		if (unit->orderSignal & 4)
		{
			bool bTwinEggOrCocoon = true;
			Point16 pos;

			unit->orderSignal &= ~4;
			bool no_two_units = false;
			auto origin = unit->eggOrigin;
			if (origin == UnitId::ZergZethrokor || unitExt->values[ValueSpecialSovrolethBirth])
			{
				no_two_units = true;
			}
			if (units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg
				|| unit->previousUnitType == UnitId::ZergCocoon)
			{
				bTwinEggOrCocoon = false;
			}
			if (unitExt->values[ValueSpecialSovrolethBirth]) {
				bTwinEggOrCocoon = true;
			}
			unitExt->values[ValueSpecialSovrolethBirth] = 0;
			Unit* unit2 = nullptr;
			if (units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg && !no_two_units)
			{
				unit2 = helper::zerg_dual_birth(unit); // zerg dual birth
				if (unit2)
				{
					helper::ai_training_unit(unit2, unit);
				}
			}
			if (unit->id != UnitId::ZergDroleth
				&& unit->id != UnitId::ZergOvileth)
			{
				helper::ai_training_unit(unit, unit);
			}
			helper::init_building(unit, 0);
			helper::update_unit_stats_and_finish_building(unit);
			helper::update_unit_strength(unit);
			pos.x = unit->sprite->position.x;
			pos.y = unit->sprite->position.y;

			if (bTwinEggOrCocoon)
			{
				if (units_dat::MovementFlags[unit->id] == MovementFlags::HoverUnit)
				{
					pos.y -= 7;
				}
				else if (unit->status & UnitStatus::InAir)
				{
					pos.y -= 42;
				}
			}
			helper::fix_target_location(&pos, unit->id);
			if (pos.x != unit->sprite->position.x ||
				pos.y != unit->sprite->position.y)
			{
				scbw::set_unit_position(unit, pos.x, pos.y);
			}
			if (!scbw::is_zerg_egg_new(unit, true, false, unit->previousUnitType))
			{
				if (unit->eggOrigin == UnitId::ZergLarva)
				{
					helper::order_new_unit_to_rally(unit, unit->connectedUnit);
					if (unit2 != nullptr)
					{
						helper::order_new_unit_to_rally(unit2, unit->connectedUnit);
					}
				}
			}
			helper::force_order_done(unit);
			unit->connectedUnit = nullptr;
		}
	}

	bool advance_remaining_build_time_custom(Unit* unit) {
		if (!unit->remainingBuildTime) {
			return 1;
		}

		if (*CHEAT_STATE & CheatFlags::OperationCwal) {
			unit->remainingBuildTime -= 16;
		} else {
			unit->remainingBuildTime -= 1;
		}

		if (!scbw::is_larval_egg(unit) &&
			scbw::get_active_tile_at(unit->position.x, unit->position.y).hasCreep) {
			unit->remainingBuildTime -= 1;
		}

		if (unit->remainingBuildTime < 0) {
			unit->remainingBuildTime = 0;
		}
		return 0;
	}

	void orders_morph_1(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		if (unit->mainOrderState == 0)
		{
			if (scbw::is_conventional_unit_morph_source(unit->id, true, true))
			{
				if (!helper::has_supplies_for_unit(unit->buildQueue[unit->buildQueueSlot], unit->playerId, 1))
				{
					helper::refund_all_queue_slots(unit);
					unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
				}
				else
				{
					u16 eggId;
					switch (unit->id)
					{
						case UnitId::ZergOvileth:
						case UnitId::ZergIroleth:
						case UnitId::ZergOthstoleth:
						{
							break;
						}
						default:
						{
							helper::increment_unit_scores_ex(unit, -1, 0);
						} break;
					}

					if (unit->status & UnitStatus::Completed)
					{
						unit->status -= UnitStatus::Completed;
					}
					switch (unit->id)
					{
						case UnitId::ZergLarva:
						case UnitId::ZergZethrokor:
						case UnitId::ZergQuazilisk:
						case UnitId::ZergHydralisk:
						case UnitId::ZergZoryusthaleth:
						case UnitId::ZergKonvilisk:
						case UnitId::ZergKeskathalor:
						{
							eggId = UnitId::ZergEgg;
						} break;
						case UnitId::ZergOvileth:
						case UnitId::ZergIroleth:
						case UnitId::ZergOthstoleth:
						case UnitId::ZergNathrokor:
						case UnitId::ZergSkithrokor:
						case UnitId::ZergMutalisk:
						case UnitId::ZergMatraleth:
						{
							eggId = UnitId::ZergCocoon;
						} break;
						default:
						{
							if (unit->id != UnitId::ProtossCantavis)
							{
								return;
							}
						}  break;
					}

					if (units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster) {
						unitExt->values[ValuePreMorphEnergy] = unit->energy;
					} else {
						unitExt->values[ValuePreMorphEnergy] = 100 * 256; // New starting amount
					}
					if (unit->id == UnitId::ProtossCantavis)
					{
						helper::replace_unit_with_type(unit, UnitId::ProtossAugur);
						unit->sprite->flags &= ~SpriteFlags::IscriptCode;
						if (unit->sprite->images.head != nullptr)
						{
							Image* current_image = unit->sprite->images.head;
							do
							{
								current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
								current_image = current_image->link.next;
							}
							while (current_image != nullptr);
						}
						helper::change_unit_buttonset(unit, UnitId::None);
						scbw::refresh_console();
						unit->status &= ~UnitStatus::IsBuilding; // disable the flag if set
						helper::set_archon_pathing(unit);
						unit->order_computer(OrderId::CompletingArchonSummon);
						return;
					}
					else
					{
						unit->eggOrigin = unit->id;
						// morph into egg from source unit
						helper::replace_unit_with_type(unit, eggId);
					}
					helper::change_unit_buttonset(unit, unit->id);
					
					unit->remainingBuildTime = units_dat::TimeCost[unit->buildQueue[unit->buildQueueSlot]];
					unit->mainOrderState = 1;
					auto origin = unit->eggOrigin;
					IscriptAnimation::Enum animation = IscriptAnimation::Unused1;
					if (origin == UnitId::ZergZethrokor
						|| origin == UnitId::ZergNathrokor)
					{
						animation = IscriptAnimation::Unused1;
					}
					else if (origin == UnitId::ZergUltrokor
						|| origin == UnitId::ZergOvileth)
					{
						animation = IscriptAnimation::AlmostBuilt;
					}
					else if (origin == UnitId::ZergQuazilisk)
					{
						animation = IscriptAnimation::GndAttkToIdle;
					}
					else if (origin == UnitId::ZergLarva)
					{
						animation = IscriptAnimation::Walking;
					}
					unit->sprite->play_iscript_animation(animation, true);
				}
			}
		}
		else if (unit->mainOrderState == 1)
		{
			// advance
			if (advance_remaining_build_time_custom(unit))
			{
				auto anim = IscriptAnimation::SpecialState1;
				unit->sprite->play_iscript_animation(anim, true);
				unit->mainOrderState = 2;
			}
		}
		else if (unit->mainOrderState == 2)
		{
			if (unit->orderSignal & 4)
			{
				unit->orderSignal -= 4;
				helper::replace_unit_with_type(unit, unit->buildQueue[unit->buildQueueSlot]);
				if (units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster) {
					unit->energy = unitExt->values[ValuePreMorphEnergy];
				} else {
					unit->energy = 0;
				}
				if (unit->id == UnitId::ZergDroleth
					|| unit->id == UnitId::ZergOvileth)
				{
					helper::ai_training_unit(unit, unit);
				}
				helper::play_morphing_complete_sound(unit);
				unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
				if (units_dat::ConstructionGraphic[unit->id] != 0)
				{
					Image* current_image;
					helper::init_building(unit, 1);
					current_image = unit->sprite->images.head;
					while (current_image != nullptr)
					{
						auto anim = IscriptAnimation::SpecialState1;
						current_image->play_iscript_animation(anim);
						current_image = current_image->link.next;
					}
					unit->order_computer(OrderId::ZergBirth);
				}
				else // 0045DF60
				{
					helper::update_unit_stats_and_finish_building(unit);
					helper::update_unit_strength(unit);
					if (unit->id == UnitId::ZergEgg && unit->eggOrigin == UnitId::ZergLarva)
					{
						helper::order_new_unit_to_rally(unit, unit->connectedUnit);
					}
				}
			}
		}
	}

	bool unit_can_morph(Unit* unit, u16 morphUnitId)
	{
		// Default StarCraft behavior
		if (unit->id == UnitId::ZergZethrokor)
		{
			if (morphUnitId == UnitId::ZergVorvrokor)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergQuazilisk)
		{
			if (morphUnitId == UnitId::ZergLiiralisk)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergHydralisk)
		{
			if (morphUnitId == UnitId::ZergBactalisk)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergSkithrokor)
		{
			if (morphUnitId == UnitId::ZergGorgrokor)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergMutalisk)
		{
			if (morphUnitId == UnitId::ZergVithrilisk)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergNathrokor)
		{
			if (morphUnitId == UnitId::ZergKalkalisk)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergZoryusthaleth)
		{
			if (morphUnitId == UnitId::ZergUltrokor)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergMatraleth)
		{
			if (morphUnitId == UnitId::ZergAkistrokor)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergKonvilisk)
		{
			if (morphUnitId == UnitId::ZergAlmaksalisk)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergKeskathalor)
		{
			if (morphUnitId == UnitId::ZergAlkajelisk)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergOvileth)
		{
			if (morphUnitId == UnitId::ZergIroleth)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergIroleth)
		{
			if (morphUnitId == UnitId::ZergOthstoleth)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergOthstoleth)
		{
			if (morphUnitId == UnitId::ZergAlaszileth)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergLarva)
		{
			if (unit->can_make_unit(morphUnitId, *ACTIVE_NATION_ID) == 1)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ProtossCantavis)
		{
			if (morphUnitId == UnitId::ProtossAugur)
			{
				return true;
			}
		}
		return false;
	}

	bool is_rallyable_egg_unit(u16 unitId)
	{
		if (scbw::is_zerg_egg_new(nullptr, false, false, unitId))
		{
			return false;
		}
		return true;
	}

	// Return the ID of the egg unit to use when morphing @p unitId. If the unit cannot morph, return UnitId::None.
	u16 get_unit_morph_egg_id(u16 unitId, Unit* unit, u16 resultId)
	{
		if (units_dat::BaseProperty[unitId] & UnitProperty::Flyer)
		{
			return UnitId::ZergCocoon;
		}
		else
		{
			return UnitId::ZergEgg;
		}
		return UnitId::None;
	}

	// Determine the type (unit ID) of the unit to revert to when cancelling an @p eggUnit while it is morphing.
	u16 get_cancel_morph_revert_id(Unit* eggUnit)
	{
		auto source = eggUnit->eggOrigin;
		if (eggUnit->getRace() == RaceId::Zerg
			&& eggUnit->id != UnitId::ZergInfestedTerran)
		{
			if (!(units_dat::BaseProperty[eggUnit->id] & UnitProperty::Building)
				&& source != UnitId::ZergLarva)
			{
				return source;
			}
		}
		return UnitId::None; // Default (no revert for larvae)
	}

	// Determines the vertical (Y) offset by which the @p unit will be shifted to when it finishes morphing.
	s16 get_unit_vertical_offset_on_birth(Unit* unit)
	{
		// No offset, birth offset is handled elsewhere
		if (units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg)
		{
			return 0;
		}
		// No offset, since the morphed unit should stay where it is
		if (unit->previousUnitType == UnitId::ZergCocoon)
		{
			return 0;
		}
		// Hovering units (?) f32 7 pixels above ground
		if (units_dat::MovementFlags[unit->id] == MovementFlags::HoverUnit)
		{
			return -7;
		}
		// Air units f32 42 pixels above ground
		if (unit->status & UnitStatus::InAir)
		{
			return -42;
		}
		// Default for ground units
		return 0;
	}

	Unit* begin_train(Unit* unit, u16 unit_id, bool notify_error)
	{
		if (unit->status & UnitStatus::GroundedBuilding
			|| unit->id == UnitId::ProtossSolarion
			|| unit->id == UnitId::ProtossAccantor
			|| scbw::is_larval_egg(unit)) // just egg
		{
			if (helper::has_supplies_for_unit(unit_id, unit->playerId, notify_error))
			{
				Unit* result = helper::create_unit(unit_id, unit->getX(), unit->getY(), unit->playerId);
				if (result)
				{
					return result;
				}
				helper::display_last_network_error_for_player(unit->playerId);
			}
		}
		return 0;
	}
}