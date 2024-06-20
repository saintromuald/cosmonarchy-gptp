#include "siege_transform.h"
#include <BW/api.h>
#include <cassert>
#include <hooks/limits/sfx_extender.h>

namespace hooks
{
	void orders_tank_mode(Unit* unit) { // 00464AE0
		u16 unit_id = unit->id;
		if (!unit->main_order_state) {
			if (!unit_id_is_siege_mode(unit_id)) {
				unit->order_to_idle();
				return;
			}

			bw::decloak_passive_proc(unit);
			Unit* subunit = unit->subunit;
			if (unit->status & UnitStatus::NoBrkCodeStart || (subunit && subunit->status & UnitStatus::NoBrkCodeStart)) {
				return;
			}

			if (subunit) {
				subunit->order_to(OrderId::Nothing3, unit);
				helper::play_subunit_iscript(unit, IscriptAnimation::SpecialState1);
				bw::decloak_passive_proc(subunit);
			}
			for (Image* image = unit->sprite->images.head; image; image = image->link.next) {
				image->play_iscript_animation(IscriptAnimation::SpecialState2);
			}
			if (subunit) {
				helper::play_subunit_iscript(unit, IscriptAnimation::SpecialState2);
			}
			unit->order_signal &= ~1;
			++unit->main_order_state;

			if (unit->player == g_local_nation_id && !g_is_in_replay) {
				u32 action_sound = 0;
				switch (unit_id) {
					case UnitId::TerranShamanVisionaryMode: {
						action_sound = SoundId::ShamanSupport01;
					} break;
					case UnitId::TerranPhalanxSiegeMode: {
						action_sound = SoundId::Terran_TANK_TTaYes00_WAV;
					} break;
				}
			
				if (action_sound) {
					bw::play_action_sound(action_sound, unit);
				}
			}
		}

		if (unit->main_order_state && unit->order_signal & 1) {
			unit->order_signal &= ~1;

			// Transform unit id
			switch (unit_id) {
				case UnitId::TerranShamanVisionaryMode: {
					unit_id = UnitId::TerranShamanSupportMode;
				} break;
				case UnitId::TerranMatadorCinderMode: {
					unit_id = UnitId::TerranMatadorTankMode;
				} break;
				case UnitId::TerranPhalanxSiegeMode: {
					unit_id = UnitId::TerranPhalanxTankMode;
				} break;
				case UnitId::TerranMadrigalFuriaMode: {
					unit_id = UnitId::TerranMadrigalImperiosoMode;
				} break;
			}

			bw::transform_unit(unit, unit_id);
			helper::order_return_to_idle(unit);
			if (unit->subunit) {
				helper::order_return_to_idle(unit->subunit);
			}
		}
	}
	
	void orders_siege_mode(Unit* unit) { // 00464BD0
		u16 unit_id = unit->id;
		switch (unit->main_order_state) {
			case 0: {
				if (!unit_id_is_tank_mode(unit_id)) {
					unit->order_to_idle();
					return;
				}
				if (unit->movement_flags & MovementFlags_Accelerating) {
					helper::make_to_hold_position(unit);
				}
				++unit->main_order_state;
			} // fallthrough
			case 1: {
				if (unit->movement_flags & MovementFlags_Accelerating) {
					return;
				}
				Unit* subunit = unit->subunit;
				if (subunit && (subunit->movement_flags & MovementFlags_Accelerating || subunit->status & UnitStatus::NoBrkCodeStart)) {
					return;
				}
				if (subunit) {
					subunit->order_to(OrderId::Nothing3, unit);
					helper::play_subunit_iscript(unit, IscriptAnimation::SpecialState1);
				}
				++unit->main_order_state;
				if (helper::check_for_close_unit(unit)) {
					return;
				}
			} // fallthrough
			case 2: {
				if (unit->player == g_local_nation_id && !g_is_in_replay) {
					u32 action_sound = 0;
					switch (unit_id) {
						case UnitId::TerranShamanSupportMode: {
							action_sound = bw::rand_between_local(SoundId::ShamanVisionary01, SoundId::ShamanVisionary02);
						} break;
						case UnitId::TerranMatadorTankMode: {
							// TODO: Unit responses
						} break;
						case UnitId::TerranPhalanxTankMode: {
							action_sound = SoundId::Terran_TANK_TTaPss00_WAV;
						} break;
						case UnitId::TerranMadrigalImperiosoMode: {
							// TODO: Unit responses
						} break;
					}
					if (action_sound) {
						bw::play_action_sound(action_sound, unit);
					}
				}

				// Transform unit ID
				switch (unit_id) {
					case UnitId::TerranShamanSupportMode: {
						unit_id = UnitId::TerranShamanVisionaryMode;
					} break;
					case UnitId::TerranMatadorTankMode: {
						unit_id = UnitId::TerranMatadorCinderMode;
					} break;
					case UnitId::TerranPhalanxTankMode: {
						unit_id = UnitId::TerranPhalanxSiegeMode;
					} break;
					case UnitId::TerranMadrigalImperiosoMode: {
						unit_id = UnitId::TerranMadrigalFuriaMode;
					} break;
				}

				bw::transform_unit(unit, unit_id);
				++unit->main_order_state;
			} // fallthrough
			case 3: {
				if (unit->order_signal & 1) {
					Order* order_queue_head = unit->order_queue_head;
					unit->order_signal &= ~1;
					if (order_queue_head && !(order_queue_head->order_id == OrderId::WatchTarget)) {
						unit->order_computer(units_dat::return_to_idle_order[unit_id]);
						helper::force_order_done(unit);
					} else {
						helper::order_return_to_idle(unit);
					}

					if (unit->subunit) {
						helper::order_return_to_idle(unit->subunit);
					}
				}
				return;
			} break;
		}
	}

	// Hooked to make invalid Phalanxes undeploy instead of self-destructing. -Pr0nogo
	s32 __fastcall siege_tank_self_destruct_proc(Unit* other, Unit* self) { // 00463F90
		if (other == self) {
			return 0;
		}
		if (!(other->status & UnitStatus::GroundedBuilding) || other->status & UnitStatus::NoCollide) {
			return 0;
		}
		self->order_to(OrderId::TankMode);
		return 1;
	}
}