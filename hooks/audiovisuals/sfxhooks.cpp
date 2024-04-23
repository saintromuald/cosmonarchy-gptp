#include "sfxhooks.h"
#include <BW/api.h>
#include <BW/structures.h>
#include <BW/bwdata.h>
#include <hooks/byte_operations_utils.h>
#include <hooks/limits/sfx_extender.h>

namespace hooks {
	// Hooked to allow more unit ids to have "annoyed" lines.
	// Updated 22 July 2022 to remove "annoyed" lines, as they are now used for "under attack" lines. -Pr0nogo
	u32 get_unit_response_id(s32 unit_id, s32 last_sound_id) { // 0048EA20
		return helper::get_random_unit_response(units_dat::sound_what_first[unit_id], units_dat::sound_what_last[unit_id]);
	}
	
	// Hooked to allow for aggressive order unit responses. -Pr0nogo
	void play_yes_unit_response(Unit* unit) { // 0048F4D0
		if (!*g_use_quiet_sounds) {
			*g_place_building_parent_unit = 0;
			if (!*g_is_in_replay && unit && unit->player == *g_local_nation_id) { // unit nullcheck is new, resolves rare minimap crash
				if (!unit->is_frozen()) {
					u32 last_rand = *g_last_random_sfx_id;
					u32 sound_id = helper::get_random_yes_unit_response_local(unit->id);
					if (helper::play_sound_from_unit(sound_id, unit, (g_options_list->flags >> 1) & 1, 99)) {
						helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[sound_id] + 1000 * (sfx_dat::sound_data[sound_id].duration_ms / 1000), 0, sound_id);
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
					if (bw::order_id_is_aggressive(unit_ext->values[ValueUnitResponseOrder])) {
						aggressive_response = true;
					}
				} break;
			}
		}
		if (aggressive_response) {
			switch (unit->id) {
				case UnitId::TerranTalos: {
					return helper::get_random_unit_response(SoundId::TalosOrderAggressive01, SoundId::TalosOrderAggressive04);
				} break;
				case UnitId::TerranHotshot: {
					return helper::get_random_unit_response(SoundId::HotshotOrderAggressive01, SoundId::HotshotOrderAggressive04);
				} break;
				case UnitId::TerranCyprian: {
					return helper::get_random_unit_response(SoundId::CyprianOrderAggressive01, SoundId::CyprianOrderAggressive04);
				} break;
				case UnitId::TerranSavant: {
					return helper::get_random_unit_response(SoundId::SavantOrderAggressive01, SoundId::SavantOrderAggressive04);
				} break;
				case UnitId::TerranMadcap: {
					return helper::get_random_unit_response(SoundId::MadcapOrderAggressive01, SoundId::MadcapOrderAggressive04);
				} break;
				case UnitId::TerranHeracles: {
					return helper::get_random_unit_response(SoundId::HeraclesOrderAggressive01, SoundId::HeraclesOrderAggressive04);
				} break;
				case UnitId::TerranApostle: {
					return helper::get_random_unit_response(SoundId::ApostleOrderAggressive01, SoundId::ApostleOrderAggressive04);
				} break;
				case UnitId::TerranSiren: {
					return helper::get_random_unit_response(SoundId::SirenOrderAggressive01, SoundId::SirenOrderAggressive04);
				} break;
				case UnitId::TerranStriga: {
					return helper::get_random_unit_response(SoundId::StrigaOrderAggressive01, SoundId::StrigaOrderAggressive04);
				} break;
				case UnitId::TerranBlackjack: {
					return helper::get_random_unit_response(SoundId::BlackjackAggressive01, SoundId::BlackjackAggressive04);
				} break;
				case UnitId::TerranMatadorTankMode: {
					return helper::get_random_unit_response(SoundId::MatadorOrderAggressive01, SoundId::MatadorOrderAggressive04);
				} break;
				case UnitId::TerranMadrigalImperiosoMode: {
					return helper::get_random_unit_response(SoundId::MadrigalOrderAggressive01, SoundId::MadrigalOrderAggressive04);
				} break;
				case UnitId::TerranPaladin: {
					return helper::get_random_unit_response(SoundId::PaladinOrderAggressive01, SoundId::PaladinOrderAggressive04);
				} break;
				case UnitId::TerranAnticthon: {
					return helper::get_random_unit_response(SoundId::AnticthonOrderAggressive01, SoundId::AnticthonOrderAggressive04);
				} break;
				case UnitId::TerranAion: {
					return helper::get_random_unit_response(SoundId::AionOrderAggressive01, SoundId::AionOrderAggressive04);
				} break;
				case UnitId::TerranWyvern: {
					return helper::get_random_unit_response(SoundId::WyvernOrderAggressive01, SoundId::WyvernOrderAggressive04);
				} break;
				case UnitId::TerranSundog: {
					return helper::get_random_unit_response(SoundId::SundogOrderAggressive01, SoundId::SundogOrderAggressive04);
				} break;
				case UnitId::TerranSalamander: {
					return helper::get_random_unit_response(SoundId::SalamanderOrderAggressive01, SoundId::SalamanderOrderAggressive04);
				} break;
				case UnitId::TerranAzazel: {
					return helper::get_random_unit_response(SoundId::AzazelOrderAggressive01, SoundId::AzazelOrderAggressive04);
				} break;
				case UnitId::TerranPhobos: {
					return helper::get_random_unit_response(SoundId::PhobosOrderAggressive01, SoundId::PhobosOrderAggressive04);
				} break;
				case UnitId::ProtossDracadin: {
					return helper::get_random_unit_response(SoundId::DracadinOrderAggressive01, SoundId::DracadinOrderAggressive04);
				} break;
				case UnitId::ProtossEcclesiast: {
					return helper::get_random_unit_response(SoundId::EcclesiastOrderAggressive01, SoundId::EcclesiastOrderAggressive04);
				} break;
				case UnitId::ProtossLegionnaire: {
					return helper::get_random_unit_response(SoundId::LegionnaireOrderAggressive01, SoundId::LegionnaireOrderAggressive04);
				} break;
				case UnitId::ProtossHierophant: {
					return helper::get_random_unit_response(SoundId::HierophantOrderAggressive01, SoundId::HierophantOrderAggressive04);
				} break;
				case UnitId::ProtossVagrant: {
					return helper::get_random_unit_response(SoundId::VagrantOrderAggressive01, SoundId::VagrantOrderAggressive04);
				} break;
				case UnitId::ProtossVagrantShade: {
					return helper::get_random_unit_response(SoundId::VagrantShadeOrderAggressive01, SoundId::VagrantShadeOrderAggressive04);
				} break;
				case UnitId::ProtossPariah: {
					return helper::get_random_unit_response(SoundId::PariahOrderAggressive01, SoundId::PariahOrderAggressive04);
				} break;
				case UnitId::ProtossOptecton: {
					return helper::get_random_unit_response(SoundId::OptectonOrderAggressive01, SoundId::OptectonOrderAggressive04);
				} break;
				case UnitId::ProtossGolem: {
					return helper::get_random_unit_response(SoundId::GolemOrderAggressive01, SoundId::GolemOrderAggressive02);
				} break;
				case UnitId::ProtossEmpress: {
					return helper::get_random_unit_response(SoundId::EmpressOrderAggressive01, SoundId::EmpressOrderAggressive04);
				} break;
				case UnitId::ProtossMagister: {
					return helper::get_random_unit_response(SoundId::MagisterOrderAggressive01, SoundId::MagisterOrderAggressive04);
				} break;
				case UnitId::ProtossStarSovereign: {
					return helper::get_random_unit_response(SoundId::StarSovereignOrderAggressive01, SoundId::StarSovereignOrderAggressive04);
				} break;
			}
		}
		return get_random_yes_unit_response_local(unit->id);
	}

	// Written for aggressive order unit responses. -Pr0nogo
	void play_yes_unit_response_internal(Unit* self, UnitResponseContext context) {
		if (!*g_use_quiet_sounds) {
			*g_place_building_parent_unit = 0;
			if (!*g_is_in_replay && self->player == *g_local_nation_id) {
				if (!self->is_frozen()) {
					u32 last_rand = *g_last_random_sfx_id;
					u32 rand_response = get_random_yes_unit_response_internal(self, context);
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

	// Hooked to allow more unit ids to process passive order unit responses. -Pr0nogo
	// Can disable injection once the internal version is working.
	u32 get_random_yes_unit_response_local(u32 unit_id) { // 0048EAB0
		if (!(units_dat::group_flags[unit_id].is_building)) {
			u32 yes_first = units_dat::sound_yes_first[unit_id];
			u32 yes_last = units_dat::sound_yes_last[unit_id];
			if (yes_first && yes_last) {
				return helper::get_random_unit_response(yes_first, yes_last);
			}
		}
		return 0;
	}

	// Hooked to allow more ready unit responses. -Pr0nogo
	void play_unit_ready_sound(Unit* unit) { // 0048F440
		if (!*g_is_in_replay && unit->player == *g_local_nation_id && !*g_use_quiet_sounds) {
			u32 unit_id = unit->id;
			if (unit_id != UnitId::ProtossSteward && unit_id != UnitId::ProtossPlasmaShell) {
				u32 ready_sound = units_dat::sound_ready[unit_id];
				switch (unit_id) {
					case UnitId::TerranMorningstar:
					{
						ready_sound = bw::get_advisor_sound_from_event_type(*g_local_nation_id, AdvisorEvent_MissileArmed);
					} break;
					case UnitId::TerranTalos: {
						ready_sound = helper::get_random_unit_response(SoundId::TalosReady01, SoundId::TalosReady03);
					} break;
					case UnitId::TerranHotshot: {
						ready_sound = helper::get_random_unit_response(SoundId::HotshotReady01, SoundId::HotshotReady03);
					} break;
					case UnitId::TerranCyprian: {
						ready_sound = helper::get_random_unit_response(SoundId::CyprianReady01, SoundId::CyprianReady03);
					} break;
					case UnitId::TerranShamanSupportMode: {
						ready_sound = helper::get_random_unit_response(SoundId::ShamanReady01, SoundId::ShamanReady03);
					} break;
					case UnitId::TerranSavant: {
						ready_sound = helper::get_random_unit_response(SoundId::SavantReady01, SoundId::SavantReady03);
					} break;
					case UnitId::TerranMadcap: {
						ready_sound = helper::get_random_unit_response(SoundId::MadcapReady01, SoundId::MadcapReady03);
					} break;
					case UnitId::TerranHeracles: {
						ready_sound = helper::get_random_unit_response(SoundId::HeraclesReady01, SoundId::HeraclesReady03);
					} break;
					case UnitId::TerranApostle: {
						ready_sound = helper::get_random_unit_response(SoundId::ApostleReady01, SoundId::ApostleReady03);
					} break;
					case UnitId::TerranSiren: {
						ready_sound = helper::get_random_unit_response(SoundId::SirenReady01, SoundId::SirenReady03);
					} break;
					case UnitId::TerranStriga: {
						ready_sound = helper::get_random_unit_response(SoundId::StrigaReady01, SoundId::StrigaReady03);
					} break;
					case UnitId::TerranBlackjack: {
						ready_sound = helper::get_random_unit_response(SoundId::BlackjackReady01, SoundId::BlackjackReady03);
					} break;
					case UnitId::TerranMatadorTankMode: {
						ready_sound = helper::get_random_unit_response(SoundId::MatadorReady01, SoundId::MatadorReady03);
					} break;
					case UnitId::TerranPhalanxTankMode:
					case UnitId::TerranPhalanxTankModeTurret:
					case UnitId::TerranPhalanxSiegeMode:
					case UnitId::TerranPhalanxSiegeModeTurret: {
						ready_sound = helper::get_random_unit_response(SoundId::PhalanxReady01, SoundId::PhalanxReady02);
					} break;
					case UnitId::TerranCataphract: // TODO: Own lines
					case UnitId::TerranPaladin: {
						ready_sound = helper::get_random_unit_response(SoundId::PaladinReady01, SoundId::PaladinReady03);
					} break;
					case UnitId::TerranMadrigalImperiosoMode: {
						ready_sound = helper::get_random_unit_response(SoundId::MadrigalReady01, SoundId::MadrigalReady03);
					} break;
					case UnitId::TerranAnticthon: {
						ready_sound = helper::get_random_unit_response(SoundId::AnticthonReady01, SoundId::AnticthonReady03);
					} break;
					case UnitId::TerranAion: {
						ready_sound = helper::get_random_unit_response(SoundId::AionReady01, SoundId::AionReady03);
					} break;
					case UnitId::TerranAncile: {
						ready_sound = helper::get_random_unit_response(SoundId::AncileReady01, SoundId::AncileReady01);
					} break;
					case UnitId::TerranWyvern: {
						ready_sound = helper::get_random_unit_response(SoundId::WyvernReady01, SoundId::WyvernReady03);
					} break;
					case UnitId::TerranSundog: {
						ready_sound = helper::get_random_unit_response(SoundId::SundogReady01, SoundId::SundogReady03);
					} break;
					case UnitId::TerranSalamander: {
						ready_sound = helper::get_random_unit_response(SoundId::SalamanderReady01, SoundId::SalamanderReady03);
					} break;
					case UnitId::TerranAzazel: {
						ready_sound = helper::get_random_unit_response(SoundId::AzazelReady01, SoundId::AzazelReady03);
					} break;
					case UnitId::TerranPhobos: {
						ready_sound = helper::get_random_unit_response(SoundId::PhobosReady01, SoundId::PhobosReady03);
					} break;
					case UnitId::TerranCentaur:
					case UnitId::TerranMagnetar: // TODO: Own lines
					case UnitId::TerranHippogriff: // TODO: Own lines
					{
						ready_sound = helper::get_random_unit_response(SoundId::CentaurReady01, SoundId::CentaurReady03);
					} break;
					case UnitId::ProtossZealot: {
						ready_sound = helper::get_random_unit_response(SoundId::ZealotReady01, SoundId::ZealotReady02);
					} break;
					case UnitId::ProtossEcclesiast:
					case UnitId::ProtossPatriarch: { // TODO: Own lines
						ready_sound = helper::get_random_unit_response(SoundId::EcclesiastReady01, SoundId::EcclesiastReady03);
					} break;
					case UnitId::ProtossDracadin: {
						ready_sound = helper::get_random_unit_response(SoundId::DracadinReady01, SoundId::DracadinReady03);
					} break;
					case UnitId::ProtossLegionnaire: {
						ready_sound = helper::get_random_unit_response(SoundId::LegionnaireReady01, SoundId::LegionnaireReady03);
					} break;
					case UnitId::ProtossHierophant: {
						ready_sound = helper::get_random_unit_response(SoundId::HierophantReady01, SoundId::HierophantReady03);
					} break;
					case UnitId::ProtossVagrant: {
						ready_sound = helper::get_random_unit_response(SoundId::VagrantReady01, SoundId::VagrantReady03);
					} break;
					case UnitId::ProtossVagrantShade: {
						ready_sound = helper::get_random_unit_response(SoundId::VagrantShadeReady01, SoundId::VagrantShadeReady04);
					} break;
					case UnitId::ProtossPariah: {
						ready_sound = helper::get_random_unit_response(SoundId::PariahReady01, SoundId::PariahReady03);
					} break;
					case UnitId::ProtossOptecton: {
						ready_sound = helper::get_random_unit_response(SoundId::OptectonReady01, SoundId::OptectonReady03);
					} break;
					case UnitId::ProtossGolem: {
						ready_sound = helper::get_random_unit_response(SoundId::GolemReady01, SoundId::GolemReady02);
					} break;
					case UnitId::ProtossEmpress: {
						ready_sound = helper::get_random_unit_response(SoundId::EmpressReady01, SoundId::EmpressReady03);
					} break;
					case UnitId::ProtossMagister: {
						ready_sound = helper::get_random_unit_response(SoundId::MagisterReady01, SoundId::MagisterReady03);
					} break;
					case UnitId::ProtossStarSovereign: {
						ready_sound = helper::get_random_unit_response(SoundId::StarSovereignReady01, SoundId::StarSovereignReady03);
					} break;
				}
				helper::play_sound_from_unit(ready_sound, unit, g_options_list->flags & 1, 99);
				helper::do_unit_event_notification(unit, 17, sfx_dat::portrait_duration[ready_sound] + 1000 * (sfx_dat::sound_data[ready_sound].duration_ms / 1000), 1, ready_sound);
			}
		}
	}
}