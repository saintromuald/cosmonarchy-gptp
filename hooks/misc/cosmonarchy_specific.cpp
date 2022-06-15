#include "hooks/misc/cosmonarchy_specific.h"
#include "globals.h"
#include <cassert>
#include "hooks/orders/unit_making/unit_train.h"
#include "stopwatch/Stopwatch.h"
#include "logger.h"
#include "SCBW/scbwdata.h"
#include <array>

namespace hooks
{
	bool debug_damage_overlay(Sprite* sprite) {
		
		if (sprite->main_graphic == nullptr)
		{
			return false;
		}
		const LO_Header* overlay = reinterpret_cast<LO_Header*>(scbw::get_image_data(ImagesDatEntries::OverlayDamagePtr, sprite->main_graphic->id));
		if (overlay == nullptr)
		{
			return false;
		}
		for (Image* image = sprite->images.head; image; image = image->link.next)
		{
			if (image->id >= ImageId::FirstMinorDamageOverlay
				&& image->id <= ImageId::LastMinorDamageOverlay)
			{
				s32 variation = image->id - ImageId::FirstMinorDamageOverlay;
				if (sprite->main_graphic->frameIndex >= overlay->frameCount)
				{
					scbw::printf("CreateDamageOverlay Bug: incorrect frame %d, image id %d", sprite->main_graphic->frameIndex, sprite->main_graphic->id);
					{
						return false;
					}
				}
			}
		}
		std::array<s32, 22> results;
		auto result_pos = results.begin();
		if (sprite->main_graphic->frameIndex >= overlay->frameCount)
		{
			scbw::printf("CreateDamageOverlay Bug: incorrect frame %d, image id %d", sprite->main_graphic->frameIndex, sprite->main_graphic->id);
			return false;
		}
		return true;
	}

	void create_damage_overlay(Sprite* sprite)
	{
		if (sprite->main_graphic == nullptr)
		{
			return;
		}
		const LO_Header* overlay = reinterpret_cast<LO_Header*>(scbw::get_image_data(ImagesDatEntries::OverlayDamagePtr, sprite->main_graphic->id));
		if (overlay == nullptr)
		{
			return; // return if there's no overlay defined
		}
		for (Image* image = sprite->images.head; image; image = image->link.next)
		{
			if (image->id >= ImageId::FirstMinorDamageOverlay
				&& image->id <= ImageId::LastMinorDamageOverlay)
			{
				s32 variation = image->id - ImageId::FirstMinorDamageOverlay;
				image->free();
				if (sprite->main_graphic->frameIndex >= overlay->frameCount)
				{
					scbw::printf("CreateDamageOverlay Bug: incorrect frame %d, image id %d", sprite->main_graphic->frameIndex, sprite->main_graphic->id);
					{
						return; 
					}
				}
				Point8 offset = overlay->get_offset(sprite->main_graphic->frameIndex, variation);
				sprite->create_top_overlay(ImageId::FirstMajorDamageOverlay + variation, offset.x, offset.y, 0);
			}
		}
		std::array<s32, 22> results;
		auto result_pos = results.begin();
		if (sprite->main_graphic->frameIndex >= overlay->frameCount)
		{
			return;
		}
		for (s32 i = 0; i < 22; i++)
		{
			Point8 offset = overlay->get_offset(sprite->main_graphic->frameIndex, i);
			if (offset.x == 127 && offset.y == 127)
			{
				continue;
			}
			scbw::printf("Overlay: %d", i);
			u32 overlay_id = ImageId::FirstMajorDamageOverlay + i;
			bool found = false;
			for (Image* image = sprite->images.head; image; image = image->link.next)
			{
				if (image->id == overlay_id)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				*result_pos++ = i;
			}
		}
		if (result_pos != results.begin())
		{
			s32 variation = results[scbw::rand_between(0, result_pos - results.begin() - 1)];
			Point8 offset = overlay->get_offset(sprite->main_graphic->frameIndex, variation);
			sprite->create_top_overlay(ImageId::FirstMinorDamageOverlay + variation, offset.x, offset.y, 0);
		}
	}

	u32 real_top_speed(Flingy* flingy, u32 flingy_id)
	{
		u32 speed = scbw::get_flingy_data(FlingyDatEntries::TopSpeed, flingy_id);
		return speed;
	}

	u16 real_acceleration(Flingy* flingy, u32 flingy_id)
	{
		u16 acceleration = scbw::get_flingy_data(FlingyDatEntries::Acceleration, flingy_id);
		return acceleration;
	}

	bool is_high_templar_conduit(u16 unit_id)
	{
		switch (unit_id)
		{
			case UnitId::ProtossAugur:
			{
				return true;
			}
			default:
			{
				break;
			}
		}
		return false;
	}

	u32 weapon_flingy(u32 weaponId, u8 player_id)
	{
		globals.flingy_init_bullet_id = weaponId;
		globals.flingy_init_player_id = player_id;
		return weapons_dat::FlingyId[weaponId];
	}

	u32 get_flingy_halt_distance(Flingy* flingy)
	{
		auto speed = flingy->current_speed2;
		if (speed == 0 || flingy->flingy_movement_type != 0)
		{
			return 0;
		}
		auto flingy_id = flingy->flingy_id;
		if (speed != real_top_speed(flingy, flingy_id) || flingy->flingy_acceleration != real_acceleration(flingy, flingy_id))
		{
			return speed * speed / (2 * (u16)flingy->flingy_acceleration);
		}
		else
		{
			return scbw::get_flingy_data(FlingyDatEntries::HaltDistance, flingy_id);
		}
	}

	bool is_unused_order(u8 order_id)
	{
		switch (order_id)
		{
			case OrderId::GuardianAspect:
			case OrderId::PaladinSkyfall: // OrderId::AttackTile
			case OrderId::Hover:
			case OrderId::Warpin:
			case OrderId::CTFCOP2:
			{
				return true; 
			}
			default:
			{
			} break;
		}
		return false;
	}

	void set_up_spider_mines(Unit* unit)
	{
		if (unit->id == UnitId::TerranVulture)
		{
			unit->mine_carrier.mine_count = 3;
		}
		else if (unit->id == UnitId::TerranEidolon)
		{
			unit->mine_carrier.mine_count = 2;
		}
	}

	u32 get_mine_id(Unit* unit)
	{
		if (unit->id == UnitId::TerranEidolon)
		{
			return UnitId::TerranLobotomyMine;
		}
		return UnitId::TerranSpiderMine;
	}
	u32 extend_tech_target_check(u32 orig_result, Unit* caster, Unit* target, u8 techId)
	{
		if (orig_result == 0)
		{ // Valid Target
			scbw::print("Can cast");
			if (techId == TechId::Lockdown)
			{
				scbw::printf("Lockdown %d %d", caster->order_target.pt.x, caster->order_target.pt.y);
			}
		}
		return orig_result;
	}

	// replaces CTFCop2 order
	void order_phase_link(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		auto order_state = unitExt->values[ValueProtossSwitchOrderState];
		u32 status = 0;
		IscriptAnimation::Enum anim = IscriptAnimation::CastSpell;

		scbw::decloak_proc(unit);
		if (unit->id == UnitId::ProtossStarSovereign)
		{
			if (order_state == 0)
			{
				if (unit->movement_flags & MovementFlags::Accelerating)
				{
					scbw::make_to_hold_position(unit);
				}
				unitExt->values[ValueProtossSwitchOrderState] = 1;
			}
			else if (order_state == 1)
			{
				if (!(unit->movement_flags & MovementFlags::Accelerating)
					&& unit->sprite->main_graphic->animation != IscriptAnimation::Walking
					&& unit->sprite->main_graphic->animation != IscriptAnimation::WalkingToIdle
					&& unitExt->timers[TimerPlanetCrackerDelay] == 0)
				{
					unitExt->timers[TimerPlanetCrackerDelay] = 24;
					unit->status |= UnitStatus::CanNotReceiveOrders;
					unit->status |= UnitStatus::CanNotAttack;
					unitExt->values[ValueProtossSwitchOrderState] = 2;
				}
			}
			else if (order_state == 2)
			{
				unit->play_iscript_animation(anim);
				unitExt->timers[TimerPlanetCrackerChannel] = 121;
				unitExt->values[ValueProtossSwitchOrderState] = 3;
			}
			else if (order_state == 3)
			{
				if (unitExt->timers[TimerPlanetCrackerChannel] != 0 && unitExt->timers[TimerPlanetCrackerDelay] == 0 && unit->sprite->main_graphic->animation != IscriptAnimation::CastSpell)
				{
					unit->play_iscript_animation(anim);
					unitExt->timers[TimerPlanetCrackerChannel] = 121;
				}
				else if (unit->order_signal & 4)
				{
					unit->order_signal &= ~0x4;
					unit->status &= ~UnitStatus::CanNotReceiveOrders;
					unit->status &= ~UnitStatus::CanNotAttack;
					unitExt->values[ValueProtossSwitchOrderState] = 0;
					unit->order_to_idle();
				}
			}
			return;
		}

		if (order_state == 0)
		{
			if (unit->movement_flags & MovementFlags::Accelerating)
			{
				scbw::make_to_hold_position(unit);
			}
			unitExt->values[ValueProtossSwitchOrderState] = 1;
		}
		else if (order_state == 1)
		{
			if (!(unit->movement_flags & MovementFlags::Accelerating))
			{
				unit->play_iscript_animation(anim);
				unitExt->timers[TimerSwitchModeGimmick] = 5;
				unitExt->values[ValueProtossSwitchOrderState] = 2;
			}
		}
		else if (order_state == 2)
		{
			if (unitExt->timers[TimerSwitchModeGimmick] == 0)
			{
				unit->order_signal &= ~0x4;
				unitExt->values[ValueProtossSwitchOrderState] = 0;
				if (unit->id == UnitId::ProtossClarion)
				{
					if (status == 0)
					{
						scbw::place_clarion_links(unit);
					}
					else
					{
						scbw::place_clarion_links(unit);
						scbw::clear_clarion_links(unit);
					}
				}
				unit->update_speed();
				unit->order_to_idle(); // do next queued order
			}
		}
	}

	bool can_attack_target_end_chunk(Unit* source, u32 target_flags)
	{
		if (source->id == UnitId::ProtossClarion && scbw::get_aise_value(source, nullptr, AiseId::SendClarionValue, 0, 0) == 1)
		{
			return false;
		}
		if (target_flags & UnitStatus::InAir)
		{
			if (source->get_air_weapon() != WeaponId::None)
			{
				return true;
			}
		}
		else
		{
			if (source->get_ground_weapon() != WeaponId::None)
			{
				return true;
			}
		}
		return false;
	}

	s32 game_loop()
	{ // 004D94B0 - hooked for lag testing - Pr0nogo
		bool v0;
		win32::Stopwatch sw;
		sw.Start();
		f64 aiLoop, unitLoop, imageThingyLoop, tileLoop;

		scbw::set_ingame_loop_state(1);
		helper::initialize_randomizer_info();
		helper::ai_loop();
		aiLoop = sw.ElapsedMilliseconds();
		sw.Stop();

		if (!visionUpdateCount)
		{
			*visionUpdateCount = 100;
		}
		v0 = *visionUpdateCount == 1;
		*visionUpdateCount -= 1;
		*visionUpdated = v0;
		sw.Start();
		if (v0)
		{
			helper::update_active_tile_info();
			helper::remove_fow_cheat();
		}
		sw.Stop();
		tileLoop = sw.ElapsedMilliseconds();
		sw.Start();
		helper::update_units();
		sw.Stop();
		sw.Start();
		unitLoop = sw.ElapsedMilliseconds();
		helper::image_drawing_bullet_drawing();
		helper::update_images();
		helper::update_thingies();
		sw.Stop();
		imageThingyLoop = sw.ElapsedMilliseconds();
		GPTP::logger << "aiLoop: " << aiLoop << ", unitLoop: " << unitLoop << ", " << "tile loop: " << tileLoop << ", thingy/bullet/imageLoop: " << imageThingyLoop << '\n';
		return scbw::set_ingame_loop_state(0);
	}

	// 004A09D0 - hooked to add ccmu cases (protect against garbage pointers) - Pr0nogo
	// Now used to set starting energy to 100 - Veeq7
	Unit* create_unit(u16 unit_id, s32 x, s32 y, s32 player) 
	{
		u32 randomNumber = 0;
		if (*is_in_game_loop)
		{
			++randomCounts[14];
			*last_random_number = 0x15A4E35 * *last_random_number + 1;
			++(*randomCountsTotal);
			randomNumber = (*last_random_number >> 16) & 0x7FFF;
		}
		Unit* unit = helper::unit_constructor(x, y, unit_id, player, randomNumber);
		if (!unit)
		{
			return 0;
		}
		if (units_dat::BaseProperty[unit_id] & UnitProperty::Spellcaster) {
			unit->energy = 100 * 256; // New starting amount
		} else {
			unit->energy = 0;
		}
		if (unit->status & UnitStatus::GroundedBuilding)
		{
			helper::list_push_back_used_unit_entry(unit);
		}
		else
		{
			helper::list_push_back_hidden_unit_entry(unit);
		}

		if (unit_id > UnitId::Spell_DisruptionWeb || units_dat::SubUnit[unit_id] == UnitId::None)
		{
			unit->subunit = 0;
			return unit;
		}

		Unit* subunit = helper::unit_constructor(x, y, units_dat::SubUnit[unit_id], player, randomNumber);
		if (!subunit)
		{
			helper::unit_destructor(unit);
			return 0;
		}

		unit->subunit = subunit;
		subunit->subunit = unit;
		u32 attack_frame = 0;
		helper::get_image_attack_frame(&attack_frame, unit->sprite, 0, 2u);
		if (!subunit) {} // debug builds will crash without this line, because XD
		helper::set_sprite_main_image_offset(subunit->sprite, attack_frame, randomNumber);
		return unit;
	}

	// Hooked to set starting energy to 100 - Veeq7
	void transform_unit_main(Unit* unit, u32 unit_id) // 0049F170
	{
		u32 hp = unit->hitpoints;
		u32 order_type = unit->main_order_id;
		u32 order_state = unit->main_order_state;
		Target order_target = unit->order_target;
		unit->previous_unit_type = unit->id;
		unit->previous_hitpoints = hp / 256;
		if (!unit->previous_hitpoints)
		{
			unit->previous_hitpoints = 1;
		}
		Sprite* sprite = unit->sprite;
		u32 player_id = sprite->player;
		u32 x = sprite->position.x;
		u32 y = sprite->position.y;
		if (sprite->images.head)
		{
			helper::sprite_destructor(sprite);
			unit->sprite = 0;
		}
		if (unit->path)
		{
			helper::reassign_path(unit);
		}
		helper::initialize_unit(unit, y, unit_id, x, unit->player);
		if (units_dat::BaseProperty[unit_id] & UnitProperty::Spellcaster) {
			unit->energy = 100 * 256; // New starting amount
		} else {
			unit->energy = 0;
		}
		unit->status = unit->status & ~UnitStatus::Frozen;
		u32 new_hp = (hp * units_dat::MaxHitPoints[unit->id]) / 256;
		u32 max_hp = units_dat::MaxHitPoints[unit->previous_unit_type] / 256;
		u32 final_hp = 1;
		if (max_hp)
		{
			final_hp = (new_hp / max_hp) / 256;
		}
		unit->hitpoints = final_hp * 256;
		if (!unit->hitpoints)
		{
			unit->hitpoints = 1;
		}
		unit->air_strength = helper::get_unit_strength(unit, max_hp, false);
		unit->ground_strength = helper::get_unit_strength(unit, max_hp, true);
		unit->status &= ~(UnitStatus::CanNotReceiveOrders | UnitStatus::NoBrkCodeStart);
		if (!orders_dat::CanBeInterrupted[order_type]) {
			unit->status |= UnitStatus::CanNotReceiveOrders;
		}
		if (sprite->player != player_id )
		{
			sprite->player = player_id;
			for (Image* image = sprite->images.head; image; image->link.next)
			{
				//image->flags |= ImageFlags::Redraw;
			}
		}
		if (unit->status & UnitStatus::GroundedBuilding)
		{
			unit->build_queue[unit->build_queue_slot] = UnitId::None;
		}
		unit->main_order_id = order_type;
		unit->main_order_state = order_state;
		unit->order_target = order_target;

		u32 temp_unit_id = unit_id;
		if (!(unit->status & UnitStatus::Completed))
		{
			temp_unit_id = UnitId::None;
		}
		if (!(unit->status & UnitStatus::Frozen) && !unit->lockdown_timer && !unit->stasis_timer && !unit->maelstrom_timer
			|| units_dat::BaseProperty[unit->id] & UnitProperty::Building
			|| temp_unit_id == UnitId::None)
		{
			unit->current_buttonset = temp_unit_id;
		}

		*force_button_refresh = 1;
		portrait_redraw[0] = 1;
		*status_screen_redraw = 1;
		*control_under_mouse = 0;
		*control_under_mouse_value = 0;
	}

	// Hooked to add nullcheck - Pr0nogo
	char ignore_iscript_and_go_to_idle(Unit* unit) // 00476550
	{
		if (!unit)
		{
			return 0;
		}
		Sprite* unitSprite = unit->sprite;
		if (!unitSprite)
		{
			return 0;
		}

		unit->status &= 0xFFFFDFFF;
		Unit* activeEntity = *activeIscriptEntity;
		Unit* activeSubunit = *activeIscriptUnit;
		unitSprite->flags &= 0x7Fu;
		*activeIscriptUnit = unit;
		*activeIscriptEntity = unit;
		helper::play_idle_iscript(unitSprite);
		char unitMovementFlags = unit->movement_flags;
		*activeIscriptUnit = activeSubunit;

		char result = unitMovementFlags & 0xF7;
		*activeIscriptEntity = activeEntity;
		unit->movement_flags = result;
		return result;
	}

	// Hooked to investigate startup crash. -Pr0nogo
	void init_damage_overlay_counts() // 004D5C20
	{
		s32 index = 999;
		s32 offset = 0;
		do
		{
			--index;
			u32 max_overlay_state = 0;
			building_overlay_state_max[index] = 0;
			GrpHead* overlay = images_damage_overlay_graphic[index];
			if (overlay)
			{
				offset = 44;
				do
				{
					offset -= 2;
					GrpFrame* overlay_frame = 0; // (overlay + offset + *&overlay->frames[0].width);
					if (overlay_frame->x != 127 || overlay_frame->y != 127)
					{
						++max_overlay_state;
					}
				} while (offset);
				if (max_overlay_state)
				{
					building_overlay_state_max[index] = 2 * max_overlay_state;
				}
			}
		} while (index);
	}
}