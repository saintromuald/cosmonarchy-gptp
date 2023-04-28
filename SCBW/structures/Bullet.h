#pragma once
#include "Target.h"
#pragma pack(1)

struct Unit;
struct Sprite;

struct Bullet // With several 2023 contributions by DarkenedFantasies
{
	// v-- SHARED Bullet/Unit STRUCTURE BEGIN
	/*0x00*/ Bullet*      previous;
	/*0x04*/ Bullet*      next;
	/*0x08*/ u32          hitpoints;				// Usually holds 1
	/*0x0C*/ Sprite*      sprite;
	/*0x10*/ Target       move_target;
	/*0x18*/ Point16      target_position;
	/*0x1C*/ Point16      target_position_2;		// Usually same as target_position
	/*0x20*/ u8           movement_flags;			// Usually holds 0x09 for probes, 0x0b for dragoons, 0x11 for mutas
	/*0x21*/ u8           current_direction_1;
	/*0x22*/ u8           flingy_turn_rate;
	/*0x23*/ u8           velocity_direction_1;
	/*0x24*/ u16          flingy_id;
	/*0x26*/ u8           unknown_0x26;
	/*0x27*/ u8           flingy_move_control;
	/*0x28*/ Point16      position;
	/*0x2C*/ Vector32     halt;
	/*0x34*/ u32          flingy_top_speed;
	/*0x38*/ u32          current_speed_1;
	/*0x3C*/ u32          current_speed_2;
	/*0x40*/ Vector32     current_velocity;
	/*0x48*/ u16          flingy_acceleration;
	/*0x4A*/ u8           current_direction;
	/*0x4B*/ u8           velocity_direction;
	/*0x4C*/ u8           player;
	/*0x4D*/ u8           behavior_type_internal;	// 0 = none
													// 1 = fly
													// 2 = fly & follow target
													// 3 = bounce
													// 4 = persist on site
													// 5 = appear on target/instant
													// 6 = attack target box
	/*0x4E*/ u8           main_order_state;			// Likely not used
	/*0x4F*/ u8           order_signal;
	/*0x50*/ u16          order_unit_id;
	/*0x52*/ u16          unknown_0x52;
	/*0x54*/ u32          main_order_timer;			// Weapon cooldowns, but presumably unused
	/*0x58*/ Target       attack_target;
	// ^-- SHARED Bullet/Unit STRUCTURE END

	/*0x60*/ u8           weapon_id;
	/*0x61*/ u8           time_remaining;
	/*0x62*/ u8           hit_flags;				// 0x01 = miss target; 0x02 = no damage (hallucination)
	/*0x63*/ u8           remaining_bounces;		// Bounce count for mutalisk weapon
	/*0x64*/ Unit*		  source_unit;
	/*0x68*/ Unit*		  next_bounce_unit;			// Next unit for bounce, temporary
	/*0x6C*/ u32          cyclic_missile_index;		// Some sort of cyclic burst counter (used by valkyries, goliath air attack)
};

C_ASSERT(sizeof(Bullet) == 112);
//static_assert(sizeof(Bullet) == 112, "The count of the Bullet structure is invalid");

#pragma pack()