#include "trigger_spawn.h"
#include "hooks/unit_destructor_special.h"	

u32 spawn_sequence_count;
SpawnSequence spawn_sequences[SpawnSequenceCap];

void init_spawn() {
	spawn_sequence_count = 0;
	memset(spawn_sequences, 0, sizeof(spawn_sequences));
}

void update_spawn() {
	for (u32 i = 0; i < spawn_sequence_count; ++i) {
		SpawnSequence* seq = &spawn_sequences[i];
		if (seq->delay) {
			seq->delay--;
			continue;
		}

		switch (seq->type) {
			case SpawnType_Augur: {
				Thingy* thingy = helper::create_thingy(SpriteId::AugurDeathExplosion, seq->source_pos.x, seq->source_pos.y, seq->player);
				u32 sound_id = bw::rand_between(SoundId::AugurDeath01, SoundId::AugurDeath04);
				helper::play_sound_from_unit(sound_id, (Unit*)thingy, 1, 0);
			} break;
			case SpawnType_AugurDamage: {
				Unit* augur = bw::create_unit_in_game(UnitId::ProtossAugur, seq->player, seq->source_pos.x, seq->source_pos.y);
				if (augur) {
					augur->user_action_flags |= UserActionFlags_SilentRemove;
					augur_sacrifice(augur, 10 * 32);
				}
			} break;
			case SpawnType_Teleport:
			{
				Thingy* thingy = helper::create_thingy(SpriteId::LegionnaireTeleportEffect, seq->source_pos.x, seq->source_pos.y, seq->player);
			} break;
		}

		Unit* unit = bw::create_unit_in_game(seq->unit_id, seq->player, seq->source_pos.x, seq->source_pos.y);
		if (unit) {
			if (seq->type == SpawnType_Burrow) {
				u8 flag = 0;
				flag |= 0x2;
				helper::chk_unit_apply_burrow_flag(unit, flag, true);
			}

			if (!(units_dat::base_property[unit->id] & UnitProperty::Building) && (seq->move_pos.x || seq->move_pos.y)) {
				if (unit->unit_ai) {
					unit->order_to(OrderId::AttackMoveComputer, seq->move_pos.x, seq->move_pos.y);
				} else {
					unit->order_to(OrderId::AttackMove, seq->move_pos.x, seq->move_pos.y);
				}
			}
			if (units_dat::base_property[unit->id] & UnitProperty::CreepBuilding || unit->id == UnitId::ZergHachirosk || unit->id == UnitId::ZergCaphrolosk || unit->id == UnitId::ZergGathtelosk) {
				helper::update_new_unit_vision(unit->id, unit->position.x, unit->position.y);
			}
		}

		switch (seq->type) {
			case SpawnType_Terran: {
				u32 rubble = SpriteId::Terran_Building_Rubble_Small;
				u32 explo = SpriteId::ExplosionMedium;
				if (units_dat::base_property[seq->unit_id] & UnitProperty::LargeOverlay) {
					rubble = SpriteId::Terran_Building_Rubble_Large;
					explo = SpriteId::Building_Explosion_Large;
				}
				bw::create_and_init_thingy(rubble, seq->source_pos.x, seq->source_pos.y, seq->player, 0);
				bw::create_and_init_thingy(explo, seq->source_pos.x, seq->source_pos.y, seq->player, unit->sprite->elevation + 1);
			} break;
			case SpawnType_Zerg: {
				u32 rubble = SpriteId::Zerg_Building_Rubble_Small;
				u32 explo = SpriteId::ZergAirExplosionSmall;
				if (units_dat::base_property[seq->unit_id] & UnitProperty::LargeOverlay) {
					rubble = SpriteId::Zerg_Building_Rubble_Large;
					explo = SpriteId::ZergAirExplosionLarge;
				}
				bw::create_and_init_thingy(rubble, seq->source_pos.x, seq->source_pos.y, seq->player, 0);
				bw::create_and_init_thingy(explo, seq->source_pos.x, seq->source_pos.y, seq->player, unit->sprite->elevation + 1);
				helper::play_sound_at_pos(SoundId::Zerg_Bldg_ZBldgDth_WAV, seq->source_pos, 1, 0);
			} break;
		}

		if (seq->stagger) {
			seq->delay = seq->stagger;
		}

		seq->count--;
		if (!seq->count) {
			spawn_sequences[i--] = spawn_sequences[--spawn_sequence_count];
		}
	}
}