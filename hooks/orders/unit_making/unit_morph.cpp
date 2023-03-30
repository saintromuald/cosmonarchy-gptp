#include <SCBW/api.h>
// copied from unhooked
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations/TechId.h>
#include <cassert>
#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "hooks\orders\ai\ai_orders.h"
#include "hooks\unit_stats\max_energy.h"
#include "globals.h"

namespace hooks {
	static void add_unit_to_control_group(ControlGroup* control_group, Unit* unit) {
		for (u32 i = 0; i < SelectionCap; ++i) {
			UnitIndex* index = &control_group->unit_index[i];
			if (control_group->unit_index[i].index) {
				Unit* old_unit = &g_unit_table[index->index - 1];
				if (old_unit->unique_id == index->unique_id) {
					continue;
				}
			}
			index->index = unit->get_unit_index();
			index->unique_id = unit->unique_id;
			break;
		}
	}

	static void add_new_unit_to_control_groups(Unit* original_unit, Unit* new_unit) {
		assert(original_unit->player == new_unit->player);
		u32 player = original_unit->player;
		for (u32 i = 0; i < TotalControlGroups; ++i) {
			for (u32 j = 0; j < SelectionCap; ++j) {
				UnitIndex index = control_group_array->player[player].control_group[i].unit_index[j];
				if (!index.value) {
					break;
				}
				if (index.index) {
					Unit* unit = &g_unit_table[index.index - 1];
					if (unit == original_unit) {
						add_unit_to_control_group(&control_group_array->player[player].control_group[i], new_unit);
					}
				}
			}
		}
	}

	void orders_zerg_birth(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		if (!(unit->order_signal & 4)) {
			Image* current_image = unit->sprite->main_graphic;
			IscriptAnimation::Enum animation = IscriptAnimation::SpecialState1;
			if (unit_ext->values[ValueSpecialSovrolethBirth]) {
				animation = IscriptAnimation::Unused2;
			}
			if (current_image->animation != animation) {
				current_image->play_iscript_animation(animation);
			}
		}

		if (unit->order_signal & 4) {
			unit->order_signal &= ~4;

			Point16 pos = {};
			bool no_two_units = false;
			if (units_dat::base_property[unit->create_origin] & UnitProperty::TwoUnitsIn1Egg // Fixes crash when cancelling a unit morphed from a dual birth source. -Pr0nogo
			||  unit_ext->values[ValueSpecialSovrolethBirth]) {
				no_two_units = true;
			}

			bool offset_unit = true;
			if (units_dat::base_property[unit->id] & UnitProperty::TwoUnitsIn1Egg
				|| unit->previous_unit_type == UnitId::ZergCocoon) {
				offset_unit = false;
			}
			if (unit_ext->values[ValueSpecialSovrolethBirth]) {
				offset_unit = true;
			}
			unit_ext->values[ValueSpecialSovrolethBirth] = 0;

			Unit* factory = 0;
			if ((unit->create_origin == UnitId::ZergLarva && unit->id != UnitId::ZergLarva)
			||  (unit->create_origin == UnitId::ZergOcsalLarva && unit->id != UnitId::ZergOcsalLarva)) {
				factory = unit->connected_unit;
			}

			u32 extra_count = unit_ext->values[ValueSwarmhold];
			unit_ext->values[ValueSwarmhold] = 0;
			if (scbw::unit_id_is_morphed_from_double_unit(unit->id) && unit->create_origin == UnitId::ZergOcsalLarva) { // Kinda stupid. -Pr0nogo
				extra_count++;
			}

			Unit* twin_unit = 0;
			if (units_dat::base_property[unit->id] & UnitProperty::TwoUnitsIn1Egg && !no_two_units) {
				twin_unit = helper::zerg_dual_birth(unit);
				if (twin_unit) {
					twin_unit->rally = unit->rally;
					helper::ai_training_unit(twin_unit, unit);
					add_new_unit_to_control_groups(unit, twin_unit);
				}
				extra_count *= 2;
			}

			for (u32 i = 0; i < extra_count; ++i) {
				Unit* new_unit = scbw::create_unit_in_game(unit->id, unit->player, unit->position.x, unit->position.y, 0, false);
				if (new_unit) {
					new_unit->rally = unit->rally;
					helper::ai_training_unit(new_unit, unit); // We inherit the AI, since it might be specifically assigned to a region or w/e
					helper::order_new_unit_to_rally(new_unit, factory);
					if (*g_active_player_id == *g_local_human_id && unit->sprite->flags & SpriteFlags::Selected) {
						helper::select_new_unit(new_unit);
					}
					add_new_unit_to_control_groups(unit, new_unit);
				}
			}

			if (unit->id != UnitId::ZergDroleth
			&&  unit->id != UnitId::ZergOvileth) {
				helper::ai_training_unit(unit, unit);
			}
			helper::init_building(unit, 0);
			helper::update_unit_stats_and_finish_building(unit);
			helper::update_unit_strength(unit);

			pos.x = unit->sprite->position.x;
			pos.y = unit->sprite->position.y;
			if (offset_unit) {
				if (units_dat::movement_flags[unit->id] == MovementFlags::HoverUnit) {
					pos.y -= 7;
				} else if (unit->status & UnitStatus::InAir) {
					pos.y -= 42;
				}
			}
			helper::fix_target_location(&pos, unit->id);
			if (pos.x != unit->sprite->position.x ||
				pos.y != unit->sprite->position.y) {
				scbw::set_unit_position(unit, pos.x, pos.y);
			}
			if (!scbw::is_zerg_egg_new(unit, true, false)) {
				helper::order_new_unit_to_rally(unit, factory);
				if (twin_unit) {
					helper::order_new_unit_to_rally(twin_unit, factory);
				}
			}
			helper::force_order_done(unit);
			unit->connected_unit = unit_ext->units[UnitMorphConnectedUnit];
		}
	}

	bool advance_remaining_build_time_custom(Unit* unit) {
		if (!unit->remaining_build_time) {
			return 1;
		}
		if (*g_cheat_state & CheatFlags::OperationCwal) {
			unit->remaining_build_time -= 16;
		} else {
			unit->remaining_build_time -= 1;
		}
		if (unit->remaining_build_time < 0) {
			unit->remaining_build_time = 0;
		}
		return 0;
	}

	void orders_morph_1(Unit* unit)
	{
		UnitExt* unitExt = get_unit_ext(unit);
		if (unit->main_order_state == 0)
		{
			if (scbw::is_conventional_unit_morph_source(unit->id, true, true))
			{
				if (!helper::has_supplies_for_unit(unit->build_queue[unit->build_queue_slot], unit->player, 1))
				{
					helper::refund_all_queue_slots(unit);
					unit->order_computer(units_dat::return_to_idle_order[unit->id]);
				}
				else
				{
					u16 eggId = UnitId::None;
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
						case UnitId::ZergOcsalLarva:
						case UnitId::ZergDroleth:
						case UnitId::ZergZethrokor:
						case UnitId::ZergQuazilisk:
						case UnitId::ZergCikralisk:
						case UnitId::ZergProtathalor:
						case UnitId::ZergAxitrilisk:
						case UnitId::ZergMortothrokor:
						case UnitId::ZergHydralisk:
						case UnitId::ZergIzirokor:
						case UnitId::ZergKagralisk:
						case UnitId::ZergEvigrilisk:
						case UnitId::ZergKeskathalor:
						case UnitId::ZergZoryusthaleth:
						case UnitId::ZergLakizilisk:
						case UnitId::ZergKonvilisk:
						case UnitId::ZergVilgorokor:
						case UnitId::ZergIsthrathaleth:
						case UnitId::ZergElthrazalisk:
						{
							eggId = UnitId::ZergEgg;
						} break;
						case UnitId::ZergOvileth:
						case UnitId::ZergOthstoleth:
						case UnitId::ZergNathrokor:
						case UnitId::ZergMutalisk:
						case UnitId::ZergSkithrokor:
						case UnitId::ZergAlmaksalisk:
						case UnitId::ZergGeszithalor:
						case UnitId::ZergGorgrokor:
						case UnitId::ZergMatraleth:
						{
							eggId = UnitId::ZergCocoon;
						} break;
						default:
						{
							return;
						}  break;
					}
					if (units_dat::base_property[unit->id] & UnitProperty::Spellcaster)
					{
						unitExt->values[ValuePreMorphEnergy] = unit->energy;
					} else
					{
						unitExt->values[ValuePreMorphEnergy] = hooks::get_unit_max_energy(unit); // New starting amount
					}
					unit->create_origin = unit->id;
					unitExt->units[UnitMorphConnectedUnit] = 0;
					if (unit->unit_ai && unit->unit_ai->type == AiType::Worker) {
						AiTown* town = unit->unit_ai->worker_ai.town;
						helper::remove_worker_or_building_ai(unit, false);
						helper::replace_unit_with_id(unit, eggId); // morph into egg from source unit
						if (eggId == UnitId::ZergEgg) {
							helper::add_worker_structure_ai(unit, town);
						}
						else {
							scbw::printf("There is no code for cocoon building AI in internal game code, this shouldn't be called!");
						}
					}
					else {
						helper::replace_unit_with_id(unit, eggId); // morph into egg from source unit
					}
					helper::change_unit_buttonset(unit, unit->id);
					
					unit->remaining_build_time = units_dat::time_cost[unit->build_queue[unit->build_queue_slot]];
					unit->main_order_state = 1;
					auto origin = unit->create_origin;
					IscriptAnimation::Enum animation = IscriptAnimation::Init;
					switch (origin)
					{
						case UnitId::ZergLarva:
						case UnitId::ZergOcsalLarva:
						{
							animation = IscriptAnimation::Walking;
						} break;
						case UnitId::ZergDroleth:
						{
							animation = IscriptAnimation::AirAttkInit;
						} break;
						case UnitId::ZergZethrokor:
						case UnitId::ZergNathrokor:
						{
							animation = IscriptAnimation::Unused1;
						} break;
						case UnitId::ZergQuazilisk:
						{
							animation = IscriptAnimation::GndAttkToIdle;
						} break;
						case UnitId::ZergZoryusthaleth:
						{
							animation = IscriptAnimation::Built;
						} break;
						case UnitId::ZergUltrokor:
						case UnitId::ZergOvileth:
						{
							animation = IscriptAnimation::AlmostBuilt;
						} break;
						case UnitId::ZergKonvilisk:
						{
							animation = IscriptAnimation::Landing;
						} break;
						case UnitId::ZergKeskathalor:
						{
							animation = IscriptAnimation::LiftOff;
						} break;
						default:
						{
							animation = IscriptAnimation::Init;
						} break;
					}
					unit->sprite->play_iscript_animation(animation, true);
				}
			}
		}
		else if (unit->main_order_state == 1)
		{
			if (unit->id == UnitId::ZergEgg && unit->remaining_build_time < 19)
			{
				unit->play_iscript_animation(IscriptAnimation::GndAttkInit);
				unit->remaining_build_time = 0;
			}
			if (unit->id == UnitId::ZergCocoon && unit->remaining_build_time < 12)
			{
				unit->play_iscript_animation(IscriptAnimation::SpecialState1);
				unit->remaining_build_time = 0;
			}
			if (advance_remaining_build_time_custom(unit))
			{
				unit->main_order_state = 2;
			}
		}
		else if (unit->main_order_state == 2)
		{
			if (unit->order_signal & 4)
			{
				unit->order_signal &= ~4;
				helper::replace_unit_with_id(unit, unit->build_queue[unit->build_queue_slot]);
				if (units_dat::base_property[unit->id] & UnitProperty::Spellcaster) {
					unit->energy = unitExt->values[ValuePreMorphEnergy];
				} else {
					unit->energy = 0;
				}
				if (unit->id == UnitId::ZergDroleth
				||  unit->id == UnitId::ZergOvileth)
				{
					helper::ai_training_unit(unit, unit);
				}
				helper::play_morphing_complete_sound(unit);
				unit->build_queue[unit->build_queue_slot] = UnitId::None;
				if (units_dat::construction_graphic[unit->id] != 0)
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
				else
				{
					helper::update_unit_stats_and_finish_building(unit);
					helper::update_unit_strength(unit);
					if (unit->id == UnitId::ZergEgg && (unit->create_origin == UnitId::ZergLarva || unit->create_origin == UnitId::ZergOcsalLarva)) {
						helper::order_new_unit_to_rally(unit, unit->connected_unit);
					}
				}
			}
		}
	}

	bool unit_can_morph(Unit* unit, u16 morph_unit_id)
	{
		switch (unit->id)
		{
			case UnitId::ZergLarva:
			case UnitId::ZergOcsalLarva:
			{
				if (unit->can_make_unit(morph_unit_id, *g_active_nation_id) == 1)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergDroleth:
			{
				if (morph_unit_id == UnitId::ZergGosvileth)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergOvileth:
			{
				if (morph_unit_id == UnitId::ZergIroleth)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergOthstoleth:
			{
				if (morph_unit_id == UnitId::ZergAlaszileth)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergZethrokor:
			{
				if (morph_unit_id == UnitId::ZergVorvrokor)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergQuazilisk:
			{
				if (morph_unit_id == UnitId::ZergLiiralisk)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergNathrokor:
			{
				if (morph_unit_id == UnitId::ZergKalkalisk)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergCikralisk:
			{
				if (morph_unit_id == UnitId::ZergGhitorokor)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergProtathalor:
			{
				if (morph_unit_id == UnitId::ZergSovroleth)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergMutalisk:
			{
				if (morph_unit_id == UnitId::ZergTosgrilisk)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergAxitrilisk:
			{
				if (morph_unit_id == UnitId::ZergBegrothilisk)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergMortothrokor:
			{
				if (morph_unit_id == UnitId::ZergOztukrokor)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergHydralisk:
			{
				if (morph_unit_id == UnitId::ZergBactalisk)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergIzirokor:
			{
				if (morph_unit_id == UnitId::ZergTethzorokor)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergSkithrokor:
			{
				if (morph_unit_id == UnitId::ZergVithrilisk)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergKagralisk:
			{
				if (morph_unit_id == UnitId::ZergCherbrathalor)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergEvigrilisk:
			{
				if (morph_unit_id == UnitId::ZergOkrimnilisk)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergAlmaksalisk:
			{
				if (morph_unit_id == UnitId::ZergAlgoztalisk)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergKeskathalor:
			{
				if (morph_unit_id == UnitId::ZergKethrostalor)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergGeszithalor:
			{
				if (morph_unit_id == UnitId::ZergAlkajelisk)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergZoryusthaleth:
			{
				if (morph_unit_id == UnitId::ZergUltrokor)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergLakizilisk:
			{
				if (morph_unit_id == UnitId::ZergZobriolisk)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergGorgrokor:
			{
				if (morph_unit_id == UnitId::ZergZarcavrokor)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergKonvilisk:
			{
				if (morph_unit_id == UnitId::ZergTetcaerokor)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergVilgorokor:
			{
				if (morph_unit_id == UnitId::ZergSelkerilisk)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergMatraleth:
			{
				if (morph_unit_id == UnitId::ZergAkistrokor)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergIsthrathaleth:
			{
				if (morph_unit_id == UnitId::ZergOgroszaleth)
				{
					return true;
				}
				return false;
			} break;
			case UnitId::ZergElthrazalisk:
			{
				if (morph_unit_id == UnitId::ZergRakzilisk)
				{
					return true;
				}
				return false;
			} break;
			default:
			{
				return false;
			}
		}
	}

	bool is_rallyable_egg_unit(u16 unit_id) {
		if (scbw::is_zerg_egg_new(0, false, false, unit_id)) {
			return false;
		}
		return true;
	}

	// Return the ID of the egg unit to use when morphing @p unit_id. If the unit cannot morph, return UnitId::None.
	u16 get_unit_morph_egg_id(u16 unit_id, Unit* unit, u16 resultId)
	{
		if (units_dat::base_property[unit_id] & UnitProperty::Flyer)
		{
			return UnitId::ZergCocoon;
		}
		else
		{
			return UnitId::ZergEgg;
		}
		return UnitId::None;
	}

	// Determine the type (unit ID) of the unit to revert to when cancelling an @p egg_unit while it is morphing.
	u16 get_cancel_morph_revert_id(Unit* egg_unit)
	{
		auto source = egg_unit->create_origin;
		if (egg_unit->get_race() == RaceId::Zerg)
		{
			if (!(units_dat::base_property[egg_unit->id] & UnitProperty::Building))
			{
				return source;
			}
		}
		return UnitId::None;
	}

	// Determines the vertical (Y) offset by which the @p unit will be shifted to when it finishes morphing.
	s16 get_unit_vertical_offset_on_birth(Unit* unit)
	{
		// No offset, birth offset is handled elsewhere
		if (units_dat::base_property[unit->id] & UnitProperty::TwoUnitsIn1Egg)
		{
			return 0;
		}
		// No offset, since the morphed unit should stay where it is
		if (unit->previous_unit_type == UnitId::ZergCocoon)
		{
			return 0;
		}
		// Hovering units (?) f32 7 pixels above ground
		if (units_dat::movement_flags[unit->id] == MovementFlags::HoverUnit)
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
			|| unit->id == UnitId::ProtossAccantorOld
			|| scbw::is_larval_egg(unit)) // just egg
		{
			if (helper::has_supplies_for_unit(unit_id, unit->player, notify_error))
			{
				Unit* result = helper::create_unit(unit_id, unit->get_x(), unit->get_y(), unit->player);
				if (result)
				{
					return result;
				}
				helper::display_last_network_error_for_player(unit->player);
			}
		}
		return 0;
	}
}