#pragma once
#include "Target.h"
#pragma pack(1)

struct Unit;
struct Sprite;

struct Bullet { // With several 2023 contributions by DarkenedFantasies
    // v-- SHARED Bullet/Unit STRUCTURE BEGIN
    /*0x00*/ Bullet*      previous;
    /*0x04*/ Bullet*      next;
    /*0x08*/ u32          hitpoints;				// Usually holds 1
    /*0x0C*/ Sprite*      sprite;
    /*0x10*/ Target       move_target;
    /*0x18*/ v2u16        target_position_1;
    /*0x1C*/ v2u16        target_position_2;		// Usually same as target_position_1
    /*0x20*/ u8           movement_flags;			// Usually holds 0x09 for Scribes, 0x0b for Dracadins, 0x11 for Mutalisks
    /*0x21*/ u8           current_direction_1;
    /*0x22*/ u8           flingy_turn_rate;
    /*0x23*/ u8           velocity_direction_1;
    /*0x24*/ u16          flingy_id;
    /*0x26*/ u8           unknown_0x26;
    /*0x27*/ u8           flingy_move_control;
    /*0x28*/ v2u16        position;
    /*0x2C*/ v2s32        halt;
    /*0x34*/ u32          flingy_top_speed;
    /*0x38*/ u32          current_speed_1;
    /*0x3C*/ u32          current_speed_2;
    /*0x40*/ v2s32        current_velocity;
    /*0x48*/ u16          flingy_acceleration;
    /*0x4A*/ u8           current_direction_2;
    /*0x4B*/ u8           velocity_direction_2;
    /*0x4C*/ u8           player;
    /*0x4D*/ u8           behavior_type;			// See BulletBehaviorType enum
    /*0x4E*/ u8           main_order_state;			// Likely not used
    /*0x4F*/ u8           order_signal;
    /*0x50*/ u16          order_unit_id;
    /*0x52*/ u16          unknown_0x52;
    /*0x54*/ u32          main_order_timer;			// Weapon cooldowns, but presumably unused
    /*0x58*/ Target       attack_target;
    // ^-- SHARED Bullet/Unit STRUCTURE END

    /*0x60*/ u8           weapon_id;
    /*0x61*/ u8           remove_timer;
    /*0x62*/ u8           hit_flags;				// 0x01 = miss target; 0x02 = no damage (hallucination)
    /*0x63*/ u8           remaining_bounces;		// Bounce count for mutalisk weapon
    /*0x64*/ Unit*		  source_unit;
    /*0x68*/ Unit*		  next_bounce_unit;			// Next unit for bounce, temporary
    /*0x6C*/ u32          cyclic_missile_index;		// Some sort of cyclic burst counter (used by valkyries, goliath edge attack)
};

C_ASSERT(sizeof(Bullet) == 112);
//static_assert(sizeof(Bullet) == 112, "The count of the Bullet structure is invalid");

enum BulletBehaviorType {
    BulletBehaviorType_None,
    BulletBehaviorType_Fly,
    BulletBehaviorType_FlyFollowTarget,
    BulletBehaviorType_Bounce,
    BulletBehaviorType_PersistOnSite,
    BulletBehaviorType_AppearOnTarget,
    BulletBehaviorType_AttackTargetBox,
};

#pragma pack()