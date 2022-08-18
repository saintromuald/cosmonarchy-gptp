#include "sfxhooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <hooks/byte_operations_utils.h>
#include <hooks/limits/sfx_extender.h>

namespace hooks {
	// Hooked to allow more unit ids to have "annoyed" lines.
	// Updated 22 July 2022 to remove "annoyed" lines, as they are now used for "under attack" lines. -Pr0nogo
	u32 get_unit_response_id(s32 unit_id, s32 last_sound_id) { // 0048EA20
		/*
		if (!(units_dat::group_flags[unit_id].is_building)) { // original check was unit_id <= DisruptionWeb, now checks for existance of annoyed lines
			u32 sound = scbw::get_unit_data(UnitsDatEntries::SndPssFirst, unit_id);
			if (sound) {
				if (last_sound_id <= scbw::get_unit_data(UnitsDatEntries::SndPssLast, unit_id) - sound + 1) {
					if (last_sound_id) {
						return sound + last_sound_id - 1;
					}
				} else {
					*selection_sound_counter = 0;
				}
			}
		}
		*/

		u32 what_start = scbw::get_unit_data(UnitsDatEntries::SndWhtFirst, unit_id);
		u32 what_end = scbw::get_unit_data(UnitsDatEntries::SndWhtLast, unit_id);
		return helper::get_random_sfx_between(what_start, what_end);
	}
	
	// Hooked to allow for aggressive order unit responses. -Pr0nogo
	// TODO: Parse context of unit order to accurately handle aggressive orders. Order ids are not assigned until after this function is called...
	void play_yes_unit_response(Unit* self) { // 0048F4D0
		if (!*use_quiet_sounds) {
			*g_place_building_parent_unit = 0;
			if (!*g_is_in_replay && self->player == *g_local_nation_id) {
				if (!self->is_frozen()) {
					u32 last_rand = *g_last_random_sfx_id;
					u32 rand_response = get_random_yes_unit_response_internal(self->id, self->main_order_id); // New second arg
					bool can_play = (g_options_list->flags >> 1) & 1;
					if (helper::play_sound_from_unit(rand_response, self, can_play, 99)) {
						helper::do_unit_event_notification(self, 17, sfx_dat::PortraitDuration[rand_response] + 1000 * (SFXDAT_SoundData[rand_response].duration_ms / 1000), 0, rand_response);
					} else {
						*g_last_random_sfx_id = last_rand;
					}
				}
			}
		}
	}

	// Written for aggressive order unit responses. -Pr0nogo
	u32 get_random_yes_unit_response_internal(u32 unit_id, u32 order_id) {
		// Technically this will not work if a unit has attack sounds but no order passive sounds... -Pr0nogo
		if (!(units_dat::group_flags[unit_id].is_building)) {
			if (scbw::order_id_is_aggressive(order_id)) {
				switch (unit_id) {
					case UnitId::ProtossHierophant: {
						return helper::get_random_sfx_between(SoundId::HierophantOrderAggressive01, SoundId::HierophantOrderAggressive04);
					} break;
				}
			}
		}
		return get_random_yes_unit_response(unit_id);
	}

	// Hooked to allow more unit ids to process passive order unit responses. -Pr0nogo
	// Can disable injection once the internal version is working.
	u32 get_random_yes_unit_response(u32 unit_id) { // 0048EAB0
		if (!(units_dat::group_flags[unit_id].is_building)) {
			u32 yes_first = scbw::get_unit_data(UnitsDatEntries::SndYesFirst, unit_id);
			u32 yes_last = scbw::get_unit_data(UnitsDatEntries::SndYesLast, unit_id);
			if (yes_first && yes_last) {
				return helper::get_random_sfx_between(yes_first, yes_last);
			}
		}
		return 0;
	}

	// Hooked to allow more ready unit responses. -Pr0nogo
	void play_unit_ready_sound(Unit* unit) { // 0048F440
		if (!*g_is_in_replay && unit->player == *g_local_nation_id && !*use_quiet_sounds) {
			u32 unit_id = unit->id;
			if (unit_id != UnitId::ProtossSteward && unit_id != UnitId::ProtossPlasmaShell) {
				u32 ready_sound = scbw::get_unit_data(UnitsDatEntries::SndReady, unit_id);
				switch (unit_id) {
					// TODO: Maybe use random method in get_unit_response_id to avoid the same ready line back to back multiple times? -Pr0nogo
					case UnitId::TerranCyprian: {
						ready_sound = helper::get_random_sfx_between(SoundId::CyprianReady01, SoundId::CyprianReady03);
					} break;
					case UnitId::TerranShaman: {
						ready_sound = helper::get_random_sfx_between(SoundId::ShamanReady01, SoundId::ShamanReady03);
					} break;
					case UnitId::TerranApostle: {
						ready_sound = helper::get_random_sfx_between(SoundId::ApostleReady01, SoundId::ApostleReady03);
					} break;
					case UnitId::TerranPhalanxTankMode:
					case UnitId::TerranPhalanxTankModeTurret:
					case UnitId::TerranPhalanxSiegeMode:
					case UnitId::TerranPhalanxSiegeModeTurret: {
						ready_sound = helper::get_random_sfx_between(SoundId::PhalanxReady01, SoundId::PhalanxReady02);
					} break;
					case UnitId::TerranPaladin: {
						ready_sound = helper::get_random_sfx_between(SoundId::PaladinReady01, SoundId::PaladinReady03);
					} break;
					case UnitId::TerranWyvern: {
						ready_sound = helper::get_random_sfx_between(SoundId::WyvernReady01, SoundId::WyvernReady03);
					} break;
					case UnitId::ProtossZealot: {
						ready_sound = helper::get_random_sfx_between(SoundId::ZealotReady01, SoundId::ZealotReady02);
					} break;
					case UnitId::ProtossEcclesiast: {
						ready_sound = helper::get_random_sfx_between(SoundId::EcclesiastReady01, SoundId::EcclesiastReady03);
					} break;
					case UnitId::ProtossLegionnaire: {
						ready_sound = helper::get_random_sfx_between(SoundId::LegionnaireReady01, SoundId::LegionnaireReady03);
					} break;
					case UnitId::ProtossHierophant: {
						ready_sound = helper::get_random_sfx_between(SoundId::HierophantReady01, SoundId::HierophantReady03);
					} break;
					case UnitId::ProtossStarSovereign: {
						ready_sound = helper::get_random_sfx_between(SoundId::StarSovereignReady01, SoundId::StarSovereignReady03);
					} break;
					case UnitId::ProtossPariah: {
						ready_sound = helper::get_random_sfx_between(SoundId::PariahReady01, SoundId::PariahReady03);
					} break;
				}
				helper::play_sound_from_unit(ready_sound, unit, g_options_list->flags & 1, 99);
				helper::do_unit_event_notification(unit, 17, sfx_dat::PortraitDuration[ready_sound] + 1000 * (SFXDAT_SoundData[ready_sound].duration_ms / 1000), 1, ready_sound);
			}
		}
	}
}