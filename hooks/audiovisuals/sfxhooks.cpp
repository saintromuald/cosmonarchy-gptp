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

		u32 what_start = units_dat::sound_what_first[unit_id];
		u32 what_end   = units_dat::sound_what_last [unit_id];
		return helper::get_random_sfx_between(what_start, what_end);
	}
	
	// Hooked to allow for aggressive order unit responses. -Pr0nogo
	void play_yes_unit_response(Unit* self) { // 0048F4D0
		if (!*use_quiet_sounds) {
			*g_place_building_parent_unit = 0;
			if (!*g_is_in_replay && self->player == *g_local_nation_id) {
				if (!self->is_frozen()) {
					u32 last_rand = *g_last_random_sfx_id;
					u32 rand_response = get_random_yes_unit_response(self->id);
					bool can_play = (g_options_list->flags >> 1) & 1;
					if (helper::play_sound_from_unit(rand_response, self, can_play, 99)) {
						helper::do_unit_event_notification(self, 17, sfx_dat::portrait_duration[rand_response] + 1000 * (sfx_dat::sound_data[rand_response].duration_ms / 1000), 0, rand_response);
					} else {
						*g_last_random_sfx_id = last_rand;
					}
				}
			}
		}
	}

	// Written for aggressive order unit responses. -Pr0nogo
	// We should probably remove the context parameter, maybe pass is_aggressive bool or something for less complexity? -Veeq7
	u32 get_random_yes_unit_response_internal(Unit* unit, UnitResponseContext context) {
		// Technically this will not work if a unit has attack sounds but no order passive sounds... -Pr0nogo
		UnitExt* unit_ext = get_unit_ext(unit);
		bool aggressive_response = false;
		if (!units_dat::group_flags[unit->id].is_building) { // We might not give a shit about this one anymore tbh
			switch (context) {
				case URC_Target: {
					if (unit_ext->units[UnitResponseTarget] && helper::unit_is_enemy(unit_ext->units[UnitResponseTarget])) {
						aggressive_response = true;
					}
				} break;
				case URC_OrderId: {
					if (scbw::order_id_is_aggressive(unit_ext->values[ValueUnitResponseOrder])) {
						aggressive_response = true;
					}
				} break;
			}
		}
		if (aggressive_response) {
			switch (unit->id) {
				case UnitId::TerranCyprian: {
					return helper::get_random_sfx_between(SoundId::CyprianOrderAggressive01, SoundId::CyprianOrderAggressive04);
				} break;
				case UnitId::TerranMadcap: {
					return helper::get_random_sfx_between(SoundId::MadcapOrderAggressive01, SoundId::MadcapOrderAggressive04);
				} break;
				case UnitId::TerranApostle: {
					return helper::get_random_sfx_between(SoundId::ApostleOrderAggressive01, SoundId::ApostleOrderAggressive04);
				} break;
				case UnitId::TerranBlackjack: {
					return helper::get_random_sfx_between(SoundId::BlackjackAggressive01, SoundId::BlackjackAggressive04);
				} break;
				case UnitId::TerranMatador: {
					return helper::get_random_sfx_between(SoundId::MatadorOrderAggressive01, SoundId::MatadorOrderAggressive04);
				} break;
				case UnitId::TerranMadrigal: {
					return helper::get_random_sfx_between(SoundId::MadrigalOrderAggressive01, SoundId::MadrigalOrderAggressive04);
				} break;
				case UnitId::TerranPaladin: {
					return helper::get_random_sfx_between(SoundId::PaladinOrderAggressive01, SoundId::PaladinOrderAggressive04);
				} break;
				case UnitId::TerranAion: {
					return helper::get_random_sfx_between(SoundId::AionOrderAggressive01, SoundId::AionOrderAggressive04);
				} break;
				case UnitId::TerranWyvern: {
					return helper::get_random_sfx_between(SoundId::WyvernOrderAggressive01, SoundId::WyvernOrderAggressive04);
				} break;
				case UnitId::TerranSundog: {
					return helper::get_random_sfx_between(SoundId::SundogOrderAggressive01, SoundId::SundogOrderAggressive04);
				} break;
				case UnitId::TerranSalamander: {
					return helper::get_random_sfx_between(SoundId::SalamanderOrderAggressive01, SoundId::SalamanderOrderAggressive04);
				} break;
				case UnitId::TerranAzazel: {
					return helper::get_random_sfx_between(SoundId::AzazelOrderAggressive01, SoundId::AzazelOrderAggressive04);
				} break;
				case UnitId::TerranPhobos: {
					return helper::get_random_sfx_between(SoundId::PhobosOrderAggressive01, SoundId::PhobosOrderAggressive04);
				} break;
				case UnitId::ProtossEcclesiast: {
					return helper::get_random_sfx_between(SoundId::EcclesiastOrderAggressive01, SoundId::EcclesiastOrderAggressive04);
				} break;
				case UnitId::ProtossLegionnaire: {
					return helper::get_random_sfx_between(SoundId::LegionnaireOrderAggressive01, SoundId::LegionnaireOrderAggressive04);
				} break;
				case UnitId::ProtossHierophant: {
					return helper::get_random_sfx_between(SoundId::HierophantOrderAggressive01, SoundId::HierophantOrderAggressive04);
				} break;
				case UnitId::ProtossPariah: {
					return helper::get_random_sfx_between(SoundId::PariahOrderAggressive01, SoundId::PariahOrderAggressive04);
				} break;
				case UnitId::ProtossMagister: {
					return helper::get_random_sfx_between(SoundId::MagisterOrderAggressive01, SoundId::MagisterOrderAggressive04);
				} break;
				case UnitId::ProtossStarSovereign: {
					return helper::get_random_sfx_between(SoundId::StarSovereignOrderAggressive01, SoundId::StarSovereignOrderAggressive04);
				} break;
			}
		}
		return get_random_yes_unit_response(unit->id);
	}

	// Written for aggressive order unit responses. -Pr0nogo
	void play_yes_unit_response_internal(Unit* self, UnitResponseContext context) {
		if (!*use_quiet_sounds) {
			*g_place_building_parent_unit = 0;
			if (!*g_is_in_replay && self->player == *g_local_nation_id) {
				if (!self->is_frozen()) {
					u32 last_rand = *g_last_random_sfx_id;
					u32 rand_response = get_random_yes_unit_response_internal(self, context);
					bool can_play = (g_options_list->flags >> 1) & 1;
					if (helper::play_sound_from_unit(rand_response, self, can_play, 99)) {
						helper::do_unit_event_notification(self, 17, sfx_dat::portrait_duration[rand_response] + 1000 * (sfx_dat::sound_data[rand_response].duration_ms / 1000), 0, rand_response);
					}
					else {
						*g_last_random_sfx_id = last_rand;
					}
				}
			}
		}
	}

	// Hooked to allow more unit ids to process passive order unit responses. -Pr0nogo
	// Can disable injection once the internal version is working.
	u32 get_random_yes_unit_response(u32 unit_id) { // 0048EAB0
		if (!(units_dat::group_flags[unit_id].is_building)) {
			u32 yes_first = units_dat::sound_yes_first[unit_id];
			u32 yes_last = units_dat::sound_yes_last[unit_id];
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
				u32 ready_sound = units_dat::sound_ready[unit_id];
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
					case UnitId::TerranBlackjack: {
						ready_sound = helper::get_random_sfx_between(SoundId::BlackjackReady01, SoundId::BlackjackReady03);
					} break;
					case UnitId::TerranMatador: {
						ready_sound = helper::get_random_sfx_between(SoundId::MatadorReady01, SoundId::MatadorReady03);
					} break;
					case UnitId::TerranPhalanxTankMode:
					case UnitId::TerranPhalanxTankModeTurret:
					case UnitId::TerranPhalanxSiegeMode:
					case UnitId::TerranPhalanxSiegeModeTurret: {
						ready_sound = helper::get_random_sfx_between(SoundId::PhalanxReady01, SoundId::PhalanxReady02);
					} break;
					case UnitId::TerranCataphract: // TODO: Own lines
					case UnitId::TerranPaladin: {
						ready_sound = helper::get_random_sfx_between(SoundId::PaladinReady01, SoundId::PaladinReady03);
					} break;
					case UnitId::TerranMadrigal: {
						ready_sound = helper::get_random_sfx_between(SoundId::MadrigalReady01, SoundId::MadrigalReady03);
					} break;
					case UnitId::TerranAion: {
						ready_sound = helper::get_random_sfx_between(SoundId::AionReady01, SoundId::AionReady03);
					} break;
					case UnitId::TerranWyvern: {
						ready_sound = helper::get_random_sfx_between(SoundId::WyvernReady01, SoundId::WyvernReady03);
					} break;
					case UnitId::TerranSundog: {
						ready_sound = helper::get_random_sfx_between(SoundId::SundogReady01, SoundId::SundogReady03);
					} break;
					case UnitId::TerranSalamander: {
						ready_sound = helper::get_random_sfx_between(SoundId::SalamanderReady01, SoundId::SalamanderReady03);
					} break;
					case UnitId::TerranAncile: // TODO: Own lines
					case UnitId::TerranAzazel: {
						ready_sound = helper::get_random_sfx_between(SoundId::AzazelReady01, SoundId::AzazelReady03);
					} break;
					case UnitId::TerranPhobos: {
						ready_sound = helper::get_random_sfx_between(SoundId::PhobosReady01, SoundId::PhobosReady03);
					} break;
					case UnitId::TerranCentaur:
					case UnitId::TerranMagnetar: // TODO: Own lines
					case UnitId::TerranHippogriff: // TODO: Own lines
					{
						ready_sound = helper::get_random_sfx_between(SoundId::CentaurReady01, SoundId::CentaurReady03);
					} break;
					case UnitId::ProtossZealot: {
						ready_sound = helper::get_random_sfx_between(SoundId::ZealotReady01, SoundId::ZealotReady02);
					} break;
					case UnitId::ProtossEcclesiast:
					case UnitId::ProtossPatriarch: {
						ready_sound = helper::get_random_sfx_between(SoundId::EcclesiastReady01, SoundId::EcclesiastReady03);
					} break;
					case UnitId::ProtossLegionnaire: {
						ready_sound = helper::get_random_sfx_between(SoundId::LegionnaireReady01, SoundId::LegionnaireReady03);
					} break;
					case UnitId::ProtossHierophant: {
						ready_sound = helper::get_random_sfx_between(SoundId::HierophantReady01, SoundId::HierophantReady03);
					} break;
					case UnitId::ProtossPariah: {
						ready_sound = helper::get_random_sfx_between(SoundId::PariahReady01, SoundId::PariahReady03);
					} break;
					case UnitId::ProtossMagister: {
						ready_sound = helper::get_random_sfx_between(SoundId::MagisterReady01, SoundId::MagisterReady03);
					} break;
					case UnitId::ProtossStarSovereign: {
						ready_sound = helper::get_random_sfx_between(SoundId::StarSovereignReady01, SoundId::StarSovereignReady03);
					} break;
				}
				helper::play_sound_from_unit(ready_sound, unit, g_options_list->flags & 1, 99);
				helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[ready_sound] + 1000 * (sfx_dat::sound_data[ready_sound].duration_ms / 1000), 1, ready_sound);
			}
		}
	}
}