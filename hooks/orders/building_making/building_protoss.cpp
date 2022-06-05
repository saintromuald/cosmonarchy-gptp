#include "building_protoss.h"
#include "building_modifier.h"
#include <SCBW/api.h>
#include <globals.h>

namespace hooks
{
	void orders_probe_build(Unit* unit)
	{
		if (unit->unit_ai != nullptr)
		{
			unit->status |= UnitStatus::IsGathering;
		}

		if (unit->main_order_state == 0)
		{
			if (helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y))
			{
				unit->main_order_state = 1;
			}
		}
		else if (unit->main_order_state == 1)
		{
			u32 distance, distance_wanted;
			distance = scbw::get_distance_fast(unit->halt.x, unit->halt.y, unit->order_target.pt.x * 256, unit->order_target.pt.y * 256);
			if (unit->build_queue[unit->build_queue_slot] == UnitId::ProtossAquifer)
			{
				distance_wanted = 88;
			}
			else
			{
				distance_wanted = 70;
			}

			if (distance / 256 > distance_wanted)
			{
				if (unit->get_movable_state() != 0)
				{
					helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y);
				}
			}
			else // 004E4DA9
			{
				helper::make_to_hold_position(unit);
				unit->main_order_state = 2;
			}
		}
		else if (unit->main_order_state == 2)
		{
			if (!(unit->movement_flags & MovementFlags::Accelerating))
			{
				u32 distance_wanted;
				bool isBuildingAssimilator = (unit->build_queue[unit->build_queue_slot] == UnitId::ProtossAquifer);
				if (isBuildingAssimilator)
				{
					distance_wanted = 88;
				}
				else
				{
					distance_wanted = 70;
				}
				if (!helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, distance_wanted))
				{
					if (unit->get_movable_state() != 0)
					{
						helper::change_movement_target_move_target_2(unit, unit->order_target.pt.x, unit->order_target.pt.y);
					}
				}
				else
				{
					if (helper::begin_building(unit))
					{
						Unit* builtUnit;
						if (isBuildingAssimilator)
						{
							builtUnit = helper::begin_gas_building(unit, UnitId::ProtossAquifer);
						}
						else
						{
							builtUnit = helper::create_unit(unit->build_queue[unit->build_queue_slot], unit->order_target.pt.x, unit->order_target.pt.y, unit->player);
						}

						u32 unit_id = unit->build_queue[unit->build_queue_slot];
						unit->build_queue[unit->build_queue_slot] = UnitId::None;
						if (builtUnit == nullptr)
						{
							helper::refund_unit_train_cost(unit_id, unit->player);
							helper::display_last_network_error_for_player(unit->player);
							unit->order_to_idle();
						}
						else // 004E4E63
						{
							helper::replace_sprite_images(builtUnit->sprite, ImageId::WarpAnchor, 0);
							if (isBuildingAssimilator)
							{
								builtUnit->sprite->create_underlay(ImageId::VespeneGeyser, 0, 0, 0);
							}
							builtUnit->sprite->createTopUnderlay(ImageId::WarpAnchorUnderlay, 0, 0, 0);
							unit->order_target.unit = builtUnit;
							helper::ai_training_overlord(builtUnit, unit);
							builtUnit->order_computer(OrderId::BuildSelf2);
							helper::play_sound_from_unit(SoundId::Protoss_SHUTTLE_pshBld00_wav, unit, 1, 0);
							unit->main_order_state = 3;
						}
					}
					else
					{
						unit->order_to_idle();
					}
				}
			}
		}
		else if (unit->main_order_state == 3)
		{
			Unit* target = unit->order_target.unit;
			Image* current_image;
			if (target == nullptr
				|| target->id != UnitId::ProtossAquifer
				|| unit->order_queue_head != nullptr)
			{
				unit->order_to_idle();
			}
			else
			{
				unit->order_to(OrderId::CanEnterGasMine, target);
			}
			current_image = unit->sprite->images.head;
			while (current_image != nullptr)
			{
				current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
				current_image = current_image->link.next;
			}
		}
	}

	// Protoss self-building
	void orders_build_self_2(Unit* building)
	{
		bool jump_to_E50EE = false;
		if (building->main_order_state > 3)
		{
			jump_to_E50EE = true;
		}
		else if (building->main_order_state == 0)
		{
			if (building->remaining_build_time == 0)
			{
				Image* current_image = building->sprite->images.head;
				while (current_image != nullptr)
				{
					if (current_image->flags & ImageFlags::Flag04)
					{
						const u32* u32_006D1200 = (u32*)0x006D1200;
						current_image->animation = IscriptAnimation::SpecialState1;
						current_image->iscriptOffset = *(u16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x22);
						current_image->wait = 0;
						current_image->unknown14 = 0;
						helper::iscript_opcode_cases(current_image, (u32)&current_image->iscriptHeaderOffset, 0, 0);
					}
					current_image = current_image->link.next;
				}
				helper::play_sound_from_unit(SoundId::Protoss_SHUTTLE_pshBld03_wav, building, 1, 0);
				building->main_order_state = 1;
				jump_to_E50EE = true;
			}
			else
			{
				jump_to_E50EE = true;
			}
		}
		else if (building->main_order_state == 1)
		{
			if (building->order_signal & 1)
			{
				building->order_signal &= ~1;
				helper::replace_sprite_images(building->sprite, scbw::get_sprite_data(SpritesDatEntries::ImageId, building->sprite->sprite_id), 0);
				helper::switch_to_warp_texture(building->sprite->main_graphic);
				building->main_order_state = 3; // DF: Skip state 2 as it's now redundant.
			}
		}
		else if (building->main_order_state == 2) // DF: Could theorically comment out this entire if block, but I haven't extensively checked/tested if it's truly not used by anything else. Your call.
		{
			if (building->order_signal & 1)
			{
				Image* current_image;
				building->order_signal &= ~1;
				helper::replace_sprite_images(building->sprite, scbw::get_sprite_data(SpritesDatEntries::ImageId, building->sprite->sprite_id), 0);
				current_image = building->sprite->images.head;
				while (current_image != nullptr)
				{
					if (current_image->flags & ImageFlags::Flag04)
					{
						const u32* u32_006D1200 = (u32*)0x006D1200;
						current_image->animation = IscriptAnimation::WarpIn;
						current_image->iscriptOffset = *(u16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x32);
						current_image->wait = 0;
						current_image->unknown14 = 0;
						helper::iscript_opcode_cases(current_image, (u32)&current_image->iscriptHeaderOffset, 0, 0);
					}
					current_image = current_image->link.next;
				}
				building->main_order_state = 3;
				building->order_signal |= 1; // DF: This is what skipped the warp fadeout animation.
			}
		}
		else if (building->main_order_state == 3)
		{
			if (building->order_signal & 1)
			{
				Image* current_image; // DF: Copied over from state 2.
				building->order_signal &= ~1;
				helper::update_unit_stats_and_finish_building(building); // DF: Copying over replace_sprite_images is not necessary because of this function.
				helper::update_unit_strength(building);
				helper::check_unstack(building);
				current_image = building->sprite->images.head; // DF: Copied over from state 2.
				while (current_image != nullptr) // DF: Copied over from state 2.
				{
					if (current_image->flags & ImageFlags::Flag04)
					{
						const u32* u32_006D1200 = (u32*)0x006D1200;
						current_image->animation = IscriptAnimation::WarpIn;
						current_image->iscriptOffset = *(u16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x32);
						current_image->wait = 0;
						current_image->unknown14 = 0;
						helper::iscript_opcode_cases(current_image, (u32)&current_image->iscriptHeaderOffset, 0, 0);
					}
					current_image = current_image->link.next;
				}

				if (building->status & UnitStatus::Frozen)
				{
					Image* current_image = building->sprite->images.head;
					while (current_image != nullptr)
					{
						if (current_image->flags & ImageFlags::Flag04)
						{
							const u32* u32_006D1200 = (u32*)0x006D1200;
							current_image->animation = IscriptAnimation::Disable;
							current_image->iscriptOffset = *(u16*)(*u32_006D1200 + current_image->iscriptHeaderOffset + 0x38);
							current_image->wait = 0;
							current_image->unknown14 = 0;
							helper::iscript_opcode_cases(current_image, (u32)&current_image->iscriptHeaderOffset, 0, 0);
						}
						current_image = current_image->link.next;
					}
					jump_to_E50EE = true;
				}
				else
				{
					jump_to_E50EE = true;
				}
			}
		}

		if (jump_to_E50EE)
		{
			u32 hpGain, shield_gain;			
			BuildingModifier modifier = get_building_modifier(building, 0);
			if (building->remaining_build_time != 0)
			{
				if (*CHEAT_STATE & CheatFlags::OperationCwal)
				{
					if (building->remaining_build_time > 16)
					{
						building->remaining_build_time -= 16;
					}
					else
					{
						building->remaining_build_time = 0;
					}
				}
				else
				{
					building->remaining_build_time--;

					if (modifier.decrease)
					{
						if (building->remaining_build_time > modifier.decrease)
						{
							building->remaining_build_time -= modifier.decrease;
						}
						else
						{
							building->remaining_build_time = 0;
						}
					}
				}
			}

			if (*CHEAT_STATE & CheatFlags::OperationCwal)
			{
				hpGain = building->build_repair_hp_gain * 16;
			}
			else
			{
				hpGain = building->build_repair_hp_gain * modifier.multiplier;
			}
			building->setHp(building->hitpoints + hpGain);

			if (*CHEAT_STATE & CheatFlags::OperationCwal)
			{
				shield_gain = building->shield_gain * 16;
			}
			else
			{
				shield_gain = building->shield_gain * modifier.multiplier;
			}

			if (building->shields + shield_gain >= (u32)units_dat::MaxShieldPoints[building->id] * 256)
			{
				building->shields = units_dat::MaxShieldPoints[building->id] * 256;
			}
			else
			{
				building->shields += shield_gain;
			}
		}
	}
}