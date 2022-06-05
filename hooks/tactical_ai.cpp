#include "hooks\tactical_ai.h"
#include "SCBW/api.h"
#include "SCBW/scbwdata.h"
#include "unit_constants.h"
#include "math_utils.h"
#include "globals.h"
#include "hooks/weapons/weapon_cooldown.h"
#include "hooks/weapons/weapon_damage.h"

/* DONE
Apostle Lazarus Agent: If 5 or more organic nearby_allies (that don't have the buff) are nearby,
cast the spell. **DONE**
AI should also try to position Apostles near organics.

Matraleth Parasite: When 5 or more supply's worth of units (of any owner) are within 128px of each other,
target the group. **DONE**

Star Sovereign Grief of All Gods: When at least 2000 HP+shields worth of enemy units are within 192px of the Star Sovereign,
and there are no allied town halls and less than 2000 HP+shields worth of allied units within the same radius,
and the Star Sovereign has more than 50% of HP+shields combined,
channel the ability. **DONE**
Attempt to protect the Star Sovereign with low-cost/low-HP units.

Lanifect Disruption Field:
When there are at least 3 nearby_allies within 128px, and there are at least 2 enemies within 128px,
and there are no activated Disruption Fields within 96px,
and the Lanifect is above 50% total HP and shields,
move towards the most significant enemy threat within 128px (in terms of enemy unit count).
Recheck conditions to disable the behavior (besides HP/shield checks). **DONE**
*/

/* TODO
Eidolon Lobotomy Mine: I think all mines should be reworked in terms of how the AI places them,
I would suggest dropping them in positions where enemy ground units
have been spotted/attacked/killed within the last few minutes.
That way it's like they're responding to ground paths taken by enemies.
Vulture Spider Mines could be done in similar ways.

Madrigal Tempo Change: When 3 or more hostiles within 4 weapon distance,
switch to Tempo Up (Furia);
else, switch to Tempo Down (Imperioso)

Paladin Skyfall: If 6 or more enemies are within 64px of each other while being within distance,
target terrain between the Paladin and the target group, 64px away from the group.

Science Vessel Observance: If 3 or more enemies are within 96px of each other, and the Vessel has more than 50% HP, target
the group and stay within distance while attempting to avoid damage.

Azazel Sublime Shepherd: Toggle while 2 or more allied units are within 96px of the Azazel, and stay within 96px of as many
nearby_allies as possible.

Didact Recall: "Offensive" cast when the Didact is added to an attack and a group of 6 or more units are within 160px of
one another. Attempt to group as many units as possible during the channel.
*/

#define ENABLE_KITE_DEBUG_MESSAGES 0

#if ENABLE_KITE_DEBUG_MESSAGES
#define KITE_DEBUG(...) scbw::printf(__VA_ARGS__)
#else
#define KITE_DEBUG(...)
#endif

#define MoveRadius (3 * 32)
#define LocalGroupRadius (6 * 32)
#define GroupRadius (12 * 32)
#define CrowdCalcRadius (2 * 32)
#define KiteZoneCount 9

const Vector32f KiteOffsets[KiteZoneCount] = {
	{-0.7, -0.7}, { 0, -1.0}, {+0.7, -0.7},
	{-1.0,    0}, { 0,    0}, {+1.0,    0},	
	{-0.7, +0.7}, { 0, +1.0}, {+0.7, +0.7},
};

struct KitePositions {
	bool targets_found;
	bool forward_found;
	bool backward_found;

	u32 distance;
	u32 ally_strength;
	u32 hostile_strength;

	f32 hostile_biases[KiteZoneCount];
	f32 forward_biases[KiteZoneCount];
	f32 backward_biases[KiteZoneCount];
	f32 effect_biases[KiteZoneCount];
	u32 forward_index;
	u32 backward_index;
	Point16 forward_pos;
	Point16 backward_pos;
};

static u32 get_max_range(u32 unit_id) {
	if (unit_id == UnitId::ProtossSolarion) {
		return 8 * 32;
	}
	if (unit_id == UnitId::ProtossAccantor) {
		return 8 * 32;
	}

	u32 ground_weapon = units_dat::GroundWeapon[unit_id];
	u32 ground_range = ground_weapon != WeaponId::None ? weapons_dat::MaxRange[ground_weapon] : 0;

	u32 air_weapon = units_dat::AirWeapon[unit_id];
	u32 air_range = air_weapon != WeaponId::None ? weapons_dat::MaxRange[air_weapon] : 0;

	return std::max(ground_range, air_range);
}

static bool is_melee_attacker(u32 unit_id) {
	return get_max_range(unit_id) <= 32;
}

static void adjust_biases(f32* biases, Unit* unit, Point16 position, s32 value) {
	Vector32f delta = {};
	delta.x = position.x - unit->position.x;
	delta.y = position.y - unit->position.y;
	delta = scbw::get_normalized_vector(delta);
	for (u32 i = 0; i < KiteZoneCount; ++i) {
		biases[i] += scbw::dot_product(delta, KiteOffsets[i]) * value;
	}
}

static bool unit_has_min_range(Unit* unit) {
	if (unit->id == UnitId::ZergGeszithalor
		|| unit->id == UnitId::ZergBactalisk
		|| unit->id == UnitId::ProtossArchitect) {
		return true;
	} else {
		Unit* context = unit;
		if (context->subunit) {
			context = context->subunit;
		}
		u32 weapon_id = WeaponId::None;
		if (unit->status & UnitStatus::InAir) {
			weapon_id = context->get_air_weapon();
		} else {
			weapon_id = context->get_ground_weapon();
		}
		if (weapons_dat::MinRange[weapon_id] || weapons_dat::ExplosionType[weapon_id] == WeaponEffect::SplashRadial) {
			return true;
		}
	}
	return false;
}

static u8 get_current_weapon_cooldown(Unit* unit) {
	if (unit->id == UnitId::ProtossSolarion || unit->id == UnitId::ProtossAccantor) {
		if (unit->main_order_timer == 0 && (unit->unit_with_hangar.in_hangar_count || (*elapsedTimeFrames) % 24 == 0)) {
			return 0;
		}
		return 2;
	}
	return std::max(unit->ground_weapon_cooldown, unit->air_weapon_cooldown);
}

static u8 get_max_weapon_cooldown(Unit* unit) {
	if (unit->id == UnitId::ProtossSolarion || unit->id == UnitId::ProtossAccantor) {
		return 1;
	}

	u32 ground_weapon = unit->get_ground_weapon();
	u32 ground_cooldown = 0;
	if (ground_weapon != WeaponId::None) {
		ground_cooldown = hooks::get_modified_weapon_cooldown(unit, ground_weapon);
	}

	u32 air_weapon = unit->get_air_weapon();
	u32 air_cooldown = 0;
	if (air_weapon != WeaponId::None) {
		air_cooldown = hooks::get_modified_weapon_cooldown(unit, air_weapon);
	}

	return std::max(ground_cooldown, air_cooldown);
}

static bool is_channeling(Unit* unit) {
	if (unit->main_order_id == OrderId::NukeTrack) {
		return true;
	}
	if (unit->main_order_id == OrderId::Plague && unit->sprite->main_graphic->animation == IscriptAnimation::Unused2) {
		return true;
	}
	if (unit->main_order_id == OrderId::FireYamatoGun1 && unit->sprite->main_graphic->animation == IscriptAnimation::CastSpell) {
		return true;
	}
	if (unit->main_order_id == OrderId::PaladinSkyfall && unit->sprite->main_graphic->animation == IscriptAnimation::CastSpell) {
		return true;
	}
	if (unit->id == UnitId::ProtossStarSovereign && unit->main_order_id == OrderId::CTFCOP2 && unit->sprite->main_graphic->animation == IscriptAnimation::CastSpell) {
		return true;
	}
	if (unit->id == UnitId::ProtossAnthelion && unit->main_order_id == OrderId::Move && unit->sprite->main_graphic->animation == IscriptAnimation::Walking) {
		return true;
	}
	return false;
}

static s32 get_target_cost(Unit* source, Unit* target) {
	// Value attacking wounded units first
	s32 cost = target->get_current_life_ingame() / 10;

	// Value attacking more threatning units first
	if (units_dat::BaseProperty[target->id] & UnitProperty::Spellcaster
		&& target->id != UnitId::TerranComsatStation
		&& target->id != UnitId::ProtossShieldBattery) {
		cost -= (target->energy / 256 / 50) * 20;
	}

	// Value channeling units more
	if (is_channeling(target)) {
		cost -= 50;
	}

	// Value big boys more
	switch (target->id) {
		case UnitId::ProtossAnthelion: {
			cost -= 50;
		} break;
		case UnitId::SharedAnticthon:
		case UnitId::SharedEmpress:
		case UnitId::ProtossDemiurge:
		case UnitId::ProtossClarion: {
			cost -= 25;
		} break;
	}

	// Value splitting simulacra more
	UnitExt* target_ext = globals.get_unit_ext(target);
	if (target_ext->timers[TimerSimulacrumClone]) {
		cost -= 5;
	}

	// Value hallucinating enemies more
	if (target_ext->timers[TimerHallucinating]) {
		cost -= 25;
	}

	// Value detected hallucinated enemies less
	if (helper::get_detecting_players(target) & 1 << target->player && target->status & UnitStatus::IsHallucination) {
		cost += 25;
	}

	if (target->id == UnitId::ProtossAccantor) {
		cost -= (weapons_dat::DamageAmount[WeaponId::Scarab] /* / INSERT ACCANTOR WEAPON COOLDOWN HERE / 24.0f*/)
			* 1.5; // aoe multiplier
	} else if (target->id == UnitId::ProtossSolarion) {
		cost -= (weapons_dat::DamageAmount[WeaponId::PulseCannon] /* / INSERT STEWARD WEAPON COOLDOWN HERE / 24.0f*/)
			* (target->unit_with_hangar.in_hangar_count + target->unit_with_hangar.out_hangar_count)
			* 1.5; // additional bias to encourage targeting solarions more
	} else {
		u32 weapon_id = WeaponId::None;
		if (source->status & UnitStatus::InAir) {
			weapon_id = target->get_air_weapon();
		} else {
			weapon_id = target->get_ground_weapon();
		}
		f32 multiplier = weapons_dat::InnerSplashRadius[weapon_id] ? 1.5 : 1;
		cost -= hooks::getCurrentDPS(target, weapon_id) * multiplier;
	}

	// Value attacking units with smaller pathing cost more
	s32 dist = (s32)scbw::get_distance_fast(source->position, target->position) - get_max_range(source->id);
	if (dist > 0) {
		cost += dist / 32.0;
	}

	// Value attacking strategically important targets
	if (target->id == UnitId::ZergNydusCanal
		|| (units_dat::BaseProperty[target->id] & UnitProperty::Worker
			&& target->main_order_id == OrderId::ConstructingBuilding)
		|| (units_dat::BaseProperty[target->id] & UnitProperty::ResourceDepot
			&& !(target->status & UnitStatus::Completed))) {
		cost -= 50;
	}
	else if (target->id == UnitId::ProtossPylon
		|| units_dat::BaseProperty[target->id] & UnitProperty::ResourceDepot
		|| units_dat::BaseProperty[target->id] & UnitProperty::Worker
		|| units_dat::GroupFlags[target->id].is_factory
		|| scbw::unit_id_is_tech(target->id)) {
		cost -= 25;
	}

	{
		u32 weapon_id = 0;
		if (target->status & UnitStatus::InAir) {
			weapon_id = source->get_air_weapon();
		} else {
			weapon_id = source->get_ground_weapon();
		}

		// Value dark swarm immune units way less
		if (darkswarm_blocks_damage(target, weapon_id, 1)) {
			cost += 1000;
		}
	}

	return cost;
}

enum KiterType {
	KiterType_TargetingOnly,
	KiterType_Melee,
	KiterType_Ranged,
	KiterType_Artillery,
	KiterType_Flagship,

	KiterType_Count,
};

static KiterType get_kiter_type(u32 unit_id) {
	if (units_dat::BaseProperty[unit_id] & UnitProperty::Building) {
		// No kiting; Just smart targeting
		return KiterType_TargetingOnly;
	}

	switch(unit_id) {
		case UnitId::ZergLakizilisk:
		case UnitId::TerranPhalanxSiegeMode: {
			// No kiting; Just smart targeting
			return KiterType_TargetingOnly;
		} break;

		case UnitId::ProtossSolarion:
		case UnitId::ProtossArchitect:
		case UnitId::ProtossAccantor:
		case UnitId::ZergKeskathalor:
		case UnitId::ZergGeszithalor:
		case UnitId::ZergBactalisk:
		case UnitId::TerranPaladin: {
			// Stay back
			return KiterType_Artillery;
		} break;

		case UnitId::TerranMinotaur:
		case UnitId::TerranCentaur:
		case UnitId::TerranPhobos:
		case UnitId::ProtossEmpyrean:
		case UnitId::ProtossStarSovereign: {
			// Advantage / Disadvantage kiting
			return KiterType_Flagship;
		} break;
	}

	if (is_melee_attacker(unit_id)) {
		// Minor adjustments, while in melee distance
		return KiterType_Melee;
	}
	// Default kiting
	return KiterType_Ranged;
}

static s32 get_effect_value(Unit* unit, Effect* effect) {
	UnitExt* unit_ext = globals.get_unit_ext(unit);
	s32 value = 0;

	bool flyer = unit->status & UnitStatus::InAir;
	u32 total_hp = unit->get_total_hp_plus_shields();
	f32 hp_percentage = total_hp / (f32)(unit->get_max_hp_ingame() + unit->get_max_shields_in_game());

	KiterType type = get_kiter_type(unit->id);
	switch (effect->id) {
		case EffectId_SalamanderFire: {
			if (!flyer) {
				value -= 50;
			}
		} break;
		case EffectId_StarSovereignTheGriefOfAllGods: {
			if (!flyer) {
				value -= 1000;
			}
		} break;
		case EffectId_IonField: {
			if (!flyer) {
				if (total_hp > 50 && hp_percentage > 0.5 && unit_ext->timers[TimerIonFieldSpeed] < 1 * 24) {
					value += 100;
				} else {
					value -= 100;
				}
			}
		} break;
		case EffectId_Criticality: {
			if (!flyer && hp_percentage < 1) {
				value += 100;
			}
		} break;
		case EffectId_CausticDischarge: {
			if (!flyer) {
				if (total_hp > 50 && hp_percentage > 0.5 && type == KiterType_Melee && unit_ext->timers[TimerCausticDischargeSpeed] < 1 * 24) {
					value += 100;
				} else {
					value -= 100;
				}
			}
		} break;
		case EffectId_Irradiate: {
			value -= 100;
		} break;
		case EffectId_Hallucination: {
			value -= 100;
		} break;
		case EffectId_GroundZero: {
			value -= 100;
		} break;
		case EffectId_DisruptionField: {
			if (unit->has_weapon()) {
				value -= 100;
			}
		} break;
		case EffectId_PlagueField: {
			value -= 100;
		} break;
	}
	return value;
}

static bool should_kite_back_against(Unit* unit, Unit* target) {
	if (target && target->canAttackTarget(unit)) { // This formula was found by iquare
		f32 factor = 1.0 / 343.0;
		f32 kiter_speed = unit->flingy_top_speed;
		f32 target_speed = target->flingy_top_speed;
		f32 kiting_time = (unit->flingy_acceleration + unit->flingy_turn_speed) * 2 * factor;
		if (kiter_speed > target_speed && get_max_range(target->id) > get_max_range(unit->id) + kiting_time) {
			return true;
		}
	}
	return false;
}

static bool unit_in_combat(Unit* unit, bool strict) {
	UnitExt* unit_ext = globals.get_unit_ext(unit);
	if (unit_ext->timers[TimerAI_KitingCombat]
		|| unit_ext->damage_receive_timer < 4 * 24
		|| unit_ext->damage_dealt_timer < 4 * 24
		|| unit->main_order_id == OrderId::Patrol
		|| unit->main_order_id == OrderId::AttackMove
		|| unit->main_order_id == OrderId::AttackMoveComputer
		|| unit->main_order_id == OrderId::AttackUnit
		|| unit->main_order_id == OrderId::CarrierFight
		|| unit->main_order_id == OrderId::ReaverFight) {
		return true;
	}

	if (!strict) {
		if (unit->main_order_id == OrderId::Move
			|| unit->main_order_id == OrderId::MoveToMinerals
			|| unit->main_order_id == OrderId::HarvestGas1
			|| unit->main_order_id == OrderId::ComputerAI
			|| unit->main_order_id == OrderId::ComputerReturn) {
			return true;
		}
	}

	return false;
}

static KitePositions get_kite_positions(Unit* unit) {
	KitePositions result = {};
	result.targets_found = false;
	result.distance = get_max_range(unit->id);

	EntryArray hostiles = unit_search_square(unit->position, GroupRadius);
	unit_search_filter(&hostiles, unit, FilterEnemy | FilterCanAttackOrCast);
	for (u32 i = 0; i < hostiles.size; ++i) {
		Unit* hostile = hostiles.units[i];

		s32 strength = 0;
		if (hostile->canAttackTarget(unit, true)) {
			if (unit->status & UnitStatus::InAir) {
				strength = hostile->air_strength;
			} else {
				strength = hostile->ground_strength;
			}
		}
		result.hostile_strength += strength; 

		u32 distance = scbw::get_distance_fast(unit->position, hostile->position);
		if (distance < result.distance) {
			result.distance = distance;
		}

		if (strength) {
			result.targets_found = true;
			if (units_dat::BaseProperty[hostile->id] & UnitProperty::Building) {
				adjust_biases(result.forward_biases, unit, hostile->position, strength);
			} else {
				adjust_biases(result.hostile_biases, unit, hostile->position, strength);
				if (should_kite_back_against(unit, hostile)) {
					result.backward_found = true;
					adjust_biases(result.backward_biases, unit, hostile->position, strength);
				}
			}
			if (unit_has_min_range(unit)) {
				result.forward_found = true;
				adjust_biases(result.forward_biases, unit, hostile->position, strength);
			}
		}
	}

	EntryArray allies = unit_search_square(unit->position, GroupRadius);
	unit_search_filter(&allies, unit, FilterAlly | FilterCanAttackOrCast);
	for (u32 i = 0; i < allies.size; ++i) {
		Unit* ally = allies.units[i];

		u32 strength = 0;
		if (ally->air_strength > ally->ground_strength) {
			strength = ally->air_strength;
		} else {
			strength = ally->ground_strength;
		}
		result.ally_strength += strength; 
	}

	EntryArray effects = effect_search_square(unit->position, GroupRadius);
	for (u32 i = 0; i < effects.size; ++i) {
		Effect* effect = effects.effects[i];
		s32 value = get_effect_value(unit, effect);
		if (value) {
			result.targets_found = true;
			Point16 position = {};
			position.x = effect->position.x;
			position.y = effect->position.y;
			adjust_biases(result.effect_biases, unit, position, value);
		}
	}

	if (result.targets_found) {
		// Default to center region
		result.forward_index = KiteZoneCount / 2;
		result.backward_index = KiteZoneCount / 2;

		s32 forward_scores[KiteZoneCount] = {};
		s32 backward_scores[KiteZoneCount] = {};

		for (u32 i = 0; i < KiteZoneCount; i++) {
			s32 forward_score = result.effect_biases[i] + result.hostile_biases[i] + result.forward_biases[i];
			if (forward_score > forward_scores[result.forward_index]) {
				result.forward_index = i;
				forward_scores[i] = forward_score;
			}

			s32 backward_score = result.effect_biases[i] - result.hostile_biases[i] + result.backward_biases[i];
			if (backward_score > backward_scores[result.backward_index]) {
				result.backward_index = i;
				backward_scores[i] = backward_score;
			}
		}

		result.forward_pos = unit->position;
		result.forward_pos.x += KiteOffsets[result.forward_index].x * MoveRadius;
		result.forward_pos.y += KiteOffsets[result.forward_index].y * MoveRadius;
		result.forward_pos = scbw::clamp_point(result.forward_pos);

		result.backward_pos = unit->position;
		result.backward_pos.x += KiteOffsets[result.backward_index].x * MoveRadius;
		result.backward_pos.y += KiteOffsets[result.backward_index].y * MoveRadius;
		result.backward_pos = scbw::clamp_point(result.backward_pos);
	}
	return result;
}

static bool unit_could_kite(Unit* unit) {
	UnitExt* unit_ext = globals.get_unit_ext(unit);
	bool in_combat = unit_in_combat(unit, false);

	if (unit->id == UnitId::ZergLakizilisk) {
		if (!(unit->status & UnitStatus::Burrowed)) {
			return false;
		}
	} else {
		if (unit->status & UnitStatus::Burrowed) {
			return false;
		}
	}

	return in_combat
		&& unit->has_weapon()
		&& !unit->is_frozen()
		&& unit->status & UnitStatus::Completed
		&& !(unit->movement_flags & MovementFlags::StartingAttack)	
		// Special units
		&& unit->id != UnitId::ProtossSteward
		&& unit->id != UnitId::ProtossPlasmaShell
		//&& unit->id != UnitId::ProtossAccantor // currently disabled
		&& unit->id != UnitId::TerranNuclearMissile
		&& unit->id != UnitId::ProtossAnthelion
		// Other rules
		&& unit->main_order_id != OrderId::BuildTerran
		&& unit->main_order_id != OrderId::ConstructingBuilding
		&& unit->main_order_id != OrderId::BuildProtoss1
		&& unit->main_order_id != OrderId::BuildProtoss2
		&& unit->main_order_id != OrderId::DroneLand
		&& unit->main_order_id != OrderId::DroneStartBuild
		&& unit->main_order_id != OrderId::DroneBuild
		&& !unit_ext->values[ValueDisruptionFieldOn];
}

static bool unit_can_kite(Unit* unit) {
	UnitExt* unit_ext = globals.get_unit_ext(unit);
	if (unit_ext->values[ValueAI_KitingSignal] < 256
		&& unit->id != UnitId::ProtossAccantor
		&& unit->id != UnitId::ProtossSolarion) {
		return false;
	}
	// ComputerReturn was patched away, still keeping the unreliable hardcode just in case
	return get_kiter_type(unit->id) != KiterType_TargetingOnly && (unit_ext->timers[TimerAI_KitingCooldown] == 0 || unit->main_order_id == OrderId::ComputerReturn);
}

static bool unit_kiting_can_attack(Unit* unit) {
	UnitExt* unit_ext = globals.get_unit_ext(unit);
	if (unit_ext->values[ValueAI_PreventTargeting]) {
		return false;
	}
	if (unit->id == UnitId::ProtossSolarion || unit->id == UnitId::ProtossAccantor) {
		return get_current_weapon_cooldown(unit) <= get_max_weapon_cooldown(unit);
	}
	if (unit->movement_flags & MovementFlags::Accelerating || unit->flingy_turn_speed < 35) {
		return get_current_weapon_cooldown(unit) <= get_max_weapon_cooldown(unit) * 0.75;
	}
	return get_current_weapon_cooldown(unit) <= get_max_weapon_cooldown(unit) * 0.25;
}

void kiting_ai(Unit* unit, UnitExt* unit_ext) {
	// Kiting AI
	if (unit_could_kite(unit) && unit_ext->timers[TimerAI_KitingLeash] == 1) {
		unit_ext->values[ValueAI_PreventTargeting] = 0;
		unit_ext->timers[TimerAI_KitingCooldown] = 1 * 24;
		unit->order_to(OrderId::Move, unit_ext->ai_kiting_origin.x, unit_ext->ai_kiting_origin.y);
		unit->issue_queued_order_target(OrderId::ComputerAI, 0, false);
	}

	if (unit_could_kite(unit)) {
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker && !unit_ext->timers[TimerAI_KitingCombat]) {
			EntryArray allies = unit_search_square(unit->position, GroupRadius);
			unit_search_filter(&allies, unit, FilterAlly | FilterCanAttackOrCast);
			u32 allied_combat_strength = 0;
			for (u32 i = 0; i < allies.size; ++i) {
				Unit* ally = allies.units[i];
				if (unit_in_combat(ally, true)) {
					u32 strength = 0;
					if (ally->air_strength > ally->ground_strength) {
						strength = ally->air_strength;
					} else {
						strength = ally->ground_strength;
					}
					allied_combat_strength += strength; 
				}
			}

			EntryArray hostiles = unit_search_square(unit->position, GroupRadius);
			unit_search_filter(&hostiles, unit, FilterEnemy | FilterCanAttackOrCast);
			u32 hostile_combat_strength = 0;
			for (u32 i = 0; i < hostiles.size; ++i) {
				Unit* hostile = hostiles.units[i];
				if (unit_in_combat(hostile, true)) {
					u32 strength = 0;
					if (hostile->air_strength > hostile->ground_strength) {
						strength = hostile->air_strength;
					} else {
						strength = hostile->ground_strength;
					}
					hostile_combat_strength += strength; 
				}
			}

			if (allied_combat_strength * 0.75 >= hostile_combat_strength) {
				unit_ext->values[ValueAI_PreventTargeting] = 0;
				return;
			}
		}

		if (unit_kiting_can_attack(unit)) {
			if (get_kiter_type(unit->id) != KiterType_TargetingOnly) {
				if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker) {
					unit_ext->timers[TimerAI_KitingLeash] = 6 * 24;
				} else {
					unit_ext->timers[TimerAI_KitingLeash] = 24 * 24;
				}
				unit_ext->ai_kiting_origin = unit->position;
			}

			auto target_match = cosmonarchy_finder_match(unit, Flags::Enemy, {});
			Unit* best_target = 0;
			s32 best_cost = INT_MAX;
			scbw::UnitFinder(
				unit->get_x() - GroupRadius,
				unit->get_y() - GroupRadius,
				unit->get_x() + GroupRadius,
				unit->get_y() + GroupRadius
			).for_each([&](Unit* target) {
				if (!target_match(target) || !unit->canAttackTarget(target, true)) {
					return;
				}

				UnitExt* target_ext = globals.get_unit_ext(target);
				if (!target_ext->ai_timer_avoid_targeting[unit->player] && unit->canAttackTarget(target, true) && target->is_visible_to(unit->player)) {
					s32 cost = get_target_cost(unit, target);
					if (cost < best_cost) {
						best_target = target;
						best_cost = cost;
					}
				}
			});

			if (best_target) {
				unit_ext->timers[TimerAI_KitingCooldown] = 2;
				KITE_DEBUG("%s: attack", unit->getName());

				u32 attack_order_id = OrderId::AttackUnit;
				if (unit->id == UnitId::ProtossSolarion) {
					attack_order_id = OrderId::CarrierFight;
				} else if (unit->id == UnitId::ProtossAccantor) {	
					attack_order_id = OrderId::ReaverFight;
				} else {
					unit_ext->values[ValueAI_PreventTargeting] = 1;
				}
				unit_ext->timers[TimerAI_KitingCombat] = 4 * 24;
				if (unit->main_order_id != attack_order_id || unit->order_target.unit != best_target) {
					unit->order_to(attack_order_id, best_target);
					unit->issue_queued_order_target(OrderId::ComputerAI, 0, false);
				}
			} else {
				unit_ext->timers[TimerAI_KitingCooldown] = 4;
				unit_ext->timers[TimerAI_KitingLeash] = 0;
				unit_ext->values[ValueAI_KitingSignal] = 0;
				unit_ext->values[ValueAI_PreventTargeting] = 0;
			}
		} else if (unit_can_kite(unit)) {
			scbw::UnitFinder local_group(
				unit->get_x() - LocalGroupRadius,
				unit->get_y() - LocalGroupRadius,
				unit->get_x() + LocalGroupRadius,
				unit->get_y() + LocalGroupRadius
			);
			local_group.filter(cosmonarchy_finder_match(unit, Flags::IncludingSelf | Flags::Own | Flags::CanAttackOrCast, {unit->id}));
			local_group.filter([&](Unit* local_unit){ return unit_could_kite(local_unit); });

			bool sync = false;
			bool update_sync = false;
			if (unit->id == UnitId::ProtossSolarion || unit->id == UnitId::ProtossAccantor) {
				sync = true;
			} else if (unit_ext->timers[TimerAI_KitingSync]) {
				sync = true;
			} else {
				u32 signal_sum = 0;
				local_group.for_each([&](Unit* local_unit) {
					UnitExt* local_unit_ext = globals.get_unit_ext(local_unit);
					if (local_unit_ext->values[ValueAI_KitingSignal] >= 256) {
						signal_sum += 1;
					}
				});
				if (signal_sum >= local_group.unitCount * 0.4) {
					sync = true;
					update_sync = true;
				}
			}

			if (sync) {
				KitePositions result = get_kite_positions(unit);

				if (!result.targets_found) {			
					local_group.for_each([&](Unit* local_unit) {
						UnitExt* local_unit_ext = globals.get_unit_ext(local_unit);
						local_unit_ext->timers[TimerAI_KitingCooldown] = 4;
						local_unit_ext->timers[TimerAI_KitingLeash] = 0;
						local_unit_ext->values[ValueAI_KitingSignal] = 0;
						local_unit_ext->values[ValueAI_PreventTargeting] = 0;
					});
					return;
				}

				if (update_sync) {
					local_group.for_each([&](Unit* local_unit) {
						UnitExt* local_unit_ext = globals.get_unit_ext(local_unit);
						local_unit_ext->timers[TimerAI_KitingSync] = 24;
					});
				}
				unit_ext->timers[TimerAI_KitingSync] = 0;

				if (unit->id == UnitId::ProtossSolarion || unit->id == UnitId::ProtossAccantor) {
					unit_ext->timers[TimerAI_KitingCooldown] = 5 * 24;
				} else if (unit->movement_flags & MovementFlags::Accelerating || unit->flingy_turn_speed < 35) {
					unit_ext->timers[TimerAI_KitingCooldown] = 2;
				} else {
					unit_ext->timers[TimerAI_KitingCooldown] = 6;
				}
				unit_ext->timers[TimerAI_KitingLeash] = 0;
				unit_ext->values[ValueAI_KitingSignal] = 0;
				unit_ext->values[ValueAI_PreventTargeting] = 0;
				unit_ext->timers[TimerAI_KitingCombat] = 4 * 24;

				bool should_kite_back = result.backward_found;
				bool prefer_kiting_forward = !unit_has_min_range(unit) && result.forward_found;
				f32 hostile_advantage = result.hostile_strength / (f32)result.ally_strength;
				u32 distance = result.distance;
				u32 max_range = get_max_range(unit->id);
				switch (get_kiter_type(unit->id)) {
					case KiterType_Melee: {
						if (!prefer_kiting_forward && unit->hitpoints < unit->get_total_hp_plus_shields() * 0.2) {
							unit_ext->timers[TimerAI_KitingCooldown] = 24;
							unit->order_to(OrderId::Move, result.backward_pos.x, result.backward_pos.y);
							KITE_DEBUG("%s: fall back", unit->getName());
						} else if (prefer_kiting_forward || distance >= max_range * 1.5) {
							unit_ext->timers[TimerAI_KitingCooldown] = 24;
							unit->order_to(OrderId::Move, result.forward_pos.x, result.forward_pos.y);
							KITE_DEBUG("%s: forward", unit->getName());
						} else if (should_kite_back) {
							unit->order_to(OrderId::Move, result.backward_pos.x, result.backward_pos.y);
							KITE_DEBUG("%s: backward", unit->getName());
						}
						unit->issue_queued_order_target(OrderId::ComputerAI, 0, false);
					} break;
					case KiterType_Ranged: {
						if (!prefer_kiting_forward
							&& (unit->hitpoints < unit->get_total_hp_plus_shields() * 0.2
							|| hostile_advantage >= 2.0
							|| distance <= max_range * 0.5)) {
							unit->order_to(OrderId::Move, result.backward_pos.x, result.backward_pos.y);
							KITE_DEBUG("%s: fall back", unit->getName());
						} else if (prefer_kiting_forward || distance > max_range * 2
							|| (distance > max_range * 0.75 && hostile_advantage <= 0.5)) {
							unit->order_to(OrderId::Move, result.forward_pos.x, result.forward_pos.y);
							KITE_DEBUG("%s: forward", unit->getName());
						} else if (should_kite_back || hostile_advantage >= 1.0) {
							unit->order_to(OrderId::Move, result.backward_pos.x, result.backward_pos.y);
							KITE_DEBUG("%s: backward", unit->getName());
						}
						unit->issue_queued_order_target(OrderId::ComputerAI, 0, false);
					} break;
					case KiterType_Artillery: {
						if (distance > max_range * 2) {
							unit->order_to(OrderId::Move, result.forward_pos.x, result.forward_pos.y);
							KITE_DEBUG("%s: forward", unit->getName());
						} else if (should_kite_back || distance < max_range * 1.25) {
							unit->order_to(OrderId::Move, result.backward_pos.x, result.backward_pos.y);
							KITE_DEBUG("%s: backward", unit->getName());
						}
						unit->issue_queued_order_target(OrderId::ComputerAI, 0, false);
					} break;
					case KiterType_Flagship: {
						if (!prefer_kiting_forward
							&& (unit->hitpoints < unit->get_total_hp_plus_shields() * 0.2 || hostile_advantage >= 2.0)) {
							unit->order_to(OrderId::Move, result.backward_pos.x, result.backward_pos.y);
							KITE_DEBUG("%s: fall back", unit->getName());
						} else if (prefer_kiting_forward || (distance > max_range * 0.75 && hostile_advantage <= 0.5)) {
							unit->order_to(OrderId::Move, result.forward_pos.x, result.forward_pos.y);
							KITE_DEBUG("%s: forward", unit->getName());
						} else if (should_kite_back) {
							unit->order_to(OrderId::Move, result.backward_pos.x, result.backward_pos.y);
							KITE_DEBUG("%s: backward", unit->getName());
						}
						unit->issue_queued_order_target(OrderId::ComputerAI, 0, false);
					} break;
				}
			}
		}
	}
}

static f32 get_blissful_embrace_target_cost(Unit* source, Unit* target) {
	f32 cost = 0;

	// Value attacking units with higher dps
	u32 weapon_id = 0;
	if (source->status & UnitStatus::InAir) {
		weapon_id = target->get_air_weapon();
	} else {
		weapon_id = target->get_ground_weapon();
	}
	f32 multiplier = weapons_dat::InnerSplashRadius[weapon_id] ? 1.5 : 1;
	cost -= hooks::getCurrentDPS(target, weapon_id) * multiplier;

	// Value attacking units with smaller pathing cost more
	s32 dist = (s32)scbw::get_distance_fast(source->position, target->position) - get_max_range(source->id);
	if (dist > 0) {
		cost += dist / 32.0;
	}

	// Value attacking strategically important targets
	if (units_dat::GroupFlags[target->id].is_factory) {
		cost -= 50;
	}
	if (units_dat::BaseProperty[target->id] & UnitProperty::ResourceDepot) {
		cost -= 25;
	}
	if (scbw::unit_id_is_tech(target->id)) {
		cost -= 25;
	}
	if (target->id == UnitId::ProtossPylon) {
		cost -= 25;
	}
	if (target->id == UnitId::TerranBunker && target->has_loaded_unit()) {
		cost -= 15;
	}

	return cost;
}

void tactical_ai(Unit* unit, UnitExt* unitExt) {
	if (unit->id == UnitId::TerranBunker && !unitExt->timers[TimerAI_GenericAbilityCooldown]) {
		const u32 bunker_capacity = units_dat::SpaceProvided[unit->id];

		EntryArray allies = unit_search_square(unit->position, unit->get_sight_range(false) * 32);
		unit_search_sort_by_distance(&allies, unit->position);
		EntryArray enemies = allies;
		unit_search_filter(&allies, unit, FilterAlly);
		unit_search_filter(&enemies, unit, unit->player == 11 ? 0 : FilterEnemy);

		if (enemies.size) {
			u32 counter = 0;
			for (u32 i = 0; i < bunker_capacity; ++i) {
				Unit* target = unit->get_loaded_unit(i);
				if (target) {
					counter += units_dat::SpaceRequired[target->id];
				}
			}
			for (u32 i = 0; i < allies.size && counter <= bunker_capacity; ++i) {
				Unit* target = allies.units[i];

				if (target->unit_ai && helper::canEnterTransport(unit, target)
					&& !is_melee_attacker(target->id) && !(units_dat::BaseProperty[unit->id] & UnitProperty::Worker)) {
					target->order_to(OrderId::EnterTransport, unit);
					counter += units_dat::SpaceRequired[target->id];
				}
			}
			unitExt->timers[TimerAI_GenericAbilityCooldown] = 2 * 24;
			unitExt->values[ValueAI_BunkerCustom] = 1;
		} else if (!unitExt->values[ValueIsNeutralBunker] && unitExt->values[ValueAI_BunkerCustom]) {
			for (u32 i = 0; i < bunker_capacity; ++i) {
				Unit* target = unit->get_loaded_unit(i);
				if (target && target->unit_ai) {
					unit->order_to(OrderId::Unload);
				}
			}
			unitExt->timers[TimerAI_GenericAbilityCooldown] = 1 * 24;
			unitExt->values[ValueAI_BunkerCustom] = 0;
		}
	}

	if (unit->unit_ai && !helper::is_unit_loaded(unit) &&
		playerTable[unit->player].type != PlayerType::Human &&
		playerTable[unit->player].type != PlayerType::HumanDefeated) {
		kiting_ai(unit, unitExt);

		if (unit->id == UnitId::TerranApostle) {
			if (unit->hasEnergy(techdata_dat::EnergyCost[TechId::ApostleLazarusAgent] * 256) && unitExt->timers[TimerAI_Lazarus] == 0)			
			{
				s32 count = 0;
				cosmonarchyFinderLoop(0, unit->position, 128, Flags::NoBuilding | Flags::Ally, {}, [&](Unit* u)
				{
					UnitExt* uExt = globals.get_unit_ext(u);
					if (uExt->values[ValueLazarusAgent] != 1)
					{
						count++;
					}
				});
				if (count >= 5)
				{
					scbw::set_generic_rust_value(unit, RustId::CastLazarusAgent, 1);
					unitExt->timers[TimerAI_Lazarus] = ApostleAILastBreathTimer;
				}
			}
		} else if (unit->id == UnitId::TerranEidolon) {
			// mines
		} else if (unit->id == UnitId::TerranSavant) {
			Unit* target = unit->order_target.unit;
			if (target && target->energy > 5 * 256 && unit->energy < 190 * 256) {
				scbw::set_generic_rust_value(unit, RustId::ToggleSavantOnhit, 1);
			} else {
				scbw::set_generic_rust_value(unit, RustId::ToggleSavantOnhit, 0);
			}
		} else if (unit->id == UnitId::TerranMadrigal) {
			f32 furiaDamagePotential = hooks::getCurrentDPS(unit, WeaponId::MadrigalFuriaMissiles);
			furiaDamagePotential *= cosmonarchyUnitCount(unit, unit->position, 5 * 32, Flags::Enemy, {});
			if (unit->order_target.unit) {
				UnitExt* target_ext = globals.get_unit_ext(unit->order_target.unit);
				furiaDamagePotential += (f32)target_ext->values[ValueDirgeStacks] * MadrigalDirgeDamage;
			}
			f32 dirgeDamagePotential = (weapons_dat::DamageAmount[WeaponId::MadrigalImperiosoMissiles] + MadrigalDirgeDamage) / hooks::getWeaponCooldownFloat(unit, WeaponId::MadrigalImperiosoMissiles);

			if (furiaDamagePotential > dirgeDamagePotential) {
				scbw::set_generic_rust_value(unit, RustId::ToggleTempoChange, 0);
			} else {
				scbw::set_generic_rust_value(unit, RustId::ToggleTempoChange, 1);
			}
		} else if (unit->id == UnitId::ZergMatraleth) {
			if (unit->main_order_id != OrderId::CastParasite && unit->energy >= techdata_dat::EnergyCost[TechId::Parasite] * 256 && unitExt->timers[TimerAI_Parasite] == 0) {
				cosmonarchyFinderLoop(unit, unit->sprite->position, weapons_dat::MaxRange[WeaponId::Parasite], Flags::NotSelf, {}, [&](Unit* u) {
					std::vector<std::pair<Unit*, u32>> group;
					s32 supply = 0;
					cosmonarchyFinderLoop(u, u->position, 128, 0, {}, [&](Unit* u2)
					{
						u32 priority = units_dat::SupplyRequired[u->id] * 100;
						if (u->status & UnitStatus::InAir)
						{
							priority /= 4;
						}
						auto weapon = u->get_ground_weapon();
						if (weapon != WeaponId::None)
						{
							if (weapons_dat::MaxRange[weapon] >= 64)
							{
								priority /= 2;
							}
						}
						group.push_back(std::make_pair(u, priority));
						supply += units_dat::SupplyRequired[u->id];
					});
					std::sort(group.begin(), group.end(), [](auto& left, auto& right)
					{
						return left.second > right.second;
					});
					supply /= 2;
					if (supply >= 5)
					{
						unit->order_to(OrderId::CastParasite, group[0].first);
						unitExt->timers[TimerAI_Parasite] = MatralethAIParasiteTimer;
						return;
					}
				});
			}

			if (unit->hasEnergy(techdata_dat::EnergyCost[TechId::MatralethEnsnaringBrood] * 256) && unitExt->timers[TimerAI_EnsnaringBrood] == 0) {
				s32 enemy_count = cosmonarchyUnitCount(0, unit->position, 256, Flags::NoBuilding | Flags::Enemy | Flags::CanAttackOrCast, {});
				if (enemy_count >= 2) {
					s32 count = 0;
					cosmonarchyFinderLoop(0, unit->position, 128, Flags::NoBuilding | Flags::Ally | Flags::CanAttackOrCast, {}, [&](Unit* u) {
						UnitExt* uExt = globals.get_unit_ext(u);
						if (!uExt->values[TimerEnsnaringBrood]) {
							count++;
						}
					});
					if (count >= 5) {
						scbw::set_generic_rust_value(unit, RustId::CastEnsnaringBrood, 1);
						unitExt->timers[TimerAI_EnsnaringBrood] = MatralethAIEnsnaringBroodTimer;
					}
				}
			}
		} else if (unit->id == UnitId::ProtossLanifect) {
			u32 web_state = scbw::get_generic_rust_value(unit, RustId::ToggleDisruptionField);
			u32 lanifects = 0;
			u32 webs = 0;
			cosmonarchyFinderLoop(unit, unit->position, AILanifectSeekRange, Flags::Own, {}, [&](Unit* target) {
				if (target->id == UnitId::ProtossLanifect) {
					lanifects += 1;
					if (scbw::get_generic_rust_value(target, RustId::ToggleDisruptionField)) {
						webs += 1;
					}
				}
			});

			u32 allies = cosmonarchyUnitCount(unit, unit->position, AILanifectSeekRange, Flags::IncludingSelf | Flags::Ally, {});
			u32 enemies = cosmonarchyUnitCount(unit, unit->position, AILanifectSeekRange, Flags::Enemy, {});
			u32 threshold = std::min(std::min(ceilf(enemies / 4), ceilf(lanifects / 4)), ceilf(allies / 2));
			if (web_state || webs < threshold) {
				Unit* best_target = 0;
				s32 max_score = INT_MIN;
				cosmonarchyFinderLoop(unit, unit->position, AILanifectSeekRange, Flags::Enemy, {}, [&](Unit* target) {
					bool valid = true;
					cosmonarchyFinderLoop(unit, target->position, AILanifectCheckRange / 2, Flags::Own, {}, [&](Unit* dweb) {
						if (scbw::get_generic_rust_value(dweb, RustId::ToggleDisruptionField)) {
							valid = false;
						}
					});
					if (valid) {
						s32 score = cosmonarchyUnitCount(unit, target->position, AILanifectCheckRange, Flags::Enemy, {});
						score -= cosmonarchyUnitCount(unit, target->position, AILanifectCheckRange, Flags::Ally, {});
						score -= scbw::get_distance_fast(unit->position, target->position) / 128;
						if (score > max_score) {
							max_score = score;
							best_target = target;
						}
					}
				});
				if (best_target) {
					unit->order_to(OrderId::Move, best_target->position.x, best_target->position.y);
					scbw::set_generic_rust_value(unit, RustId::ToggleDisruptionField, 1);
					unitExt->timers[TimerAI_DisruptionField] = AIWebTimer;
				}
			}

			if (web_state && webs > threshold) {
				scbw::set_generic_rust_value(unit, RustId::ToggleDisruptionField, 0);
				if (unit->main_order_id == OrderId::Move)
				{
					unit->order_to_idle();
				}
			}
		} else if (unit->id == UnitId::ProtossStarSovereign && unitExt->timers[TimerAI_GriefOfAllGods] == 0) {
			auto enemyUnitsHealth = 0;
			auto alliedUnitsHealth = 0;
			bool alliedTownHalls = false;
			cosmonarchyFinderLoop(unit, unit->sprite->position, AIStarsovereignAbilSeekRange, Flags::NotSelf | Flags::NoAir, {}, [&](Unit* u) {
				if (!scbw::is_allied_to(unit->player, u->player))
				{
					enemyUnitsHealth += (u->hitpoints + u->shields) / 256;
				}
				if (scbw::is_allied_to(unit->player, u->player))
				{
					if (units_dat::BaseProperty[u->id] & UnitProperty::ResourceDepot)
					{
						alliedTownHalls = true;
						return;
					}
					alliedUnitsHealth += (u->hitpoints + u->shields) / 256;
				}
			});
			if (!alliedTownHalls && enemyUnitsHealth >= 2000 && alliedUnitsHealth < 2000
				&& (unit->hitpoints + unit->shields) * 2 >=	(units_dat::MaxHitPoints[unit->id] + units_dat::MaxShieldPoints[unit->id])) {
				unitExt->timers[TimerAI_GriefOfAllGods] = 612;
				unitExt->timers[TimerAI_KitingCooldown] = 5 * 24;
				unit->order_computer(OrderId::KhaydarinEclipse);
				unit->order_target.pt = unit->sprite->position;
				unit->main_order_state = 0;
			}
		} else if (unit->id == UnitId::ProtossDemiurge && unitExt->timers[TimerAI_GenericAbilityCooldown] == 0) {
			if (unit->energy >= 50 * 256) {
				auto& vec = unitExt->unit_vectors[UnitVectorWorshipersDemiurge];

				u32 totalCount = vec.size();
				u32 inCombatCount = 0;
				for (Unit* worshiper : vec) {
					UnitExt* worshiperExt = globals.get_unit_ext(worshiper);
					if (worshiperExt->damage_dealt_timer < 4 * 24 || worshiperExt->damage_receive_timer < 4 * 24)
					{
						inCombatCount++;
					}
				}

				if (totalCount > 3 && inCombatCount > totalCount - inCombatCount) {
					scbw::set_generic_rust_value(unit, RustId::CastGodsMadeFlesh, 1);
					unitExt->timers[TimerAI_GenericAbilityCooldown] = 10 * 24;
				} else {
					unitExt->timers[TimerAI_GenericAbilityCooldown] = 1 * 24;
				}
			} else {
				unitExt->timers[TimerAI_GenericAbilityCooldown] = 1 * 24;
			}
		} else if (unit->id == UnitId::ZergAkistrokor && unit->main_order_id != OrderId::BlissfulEmbrace) {
			EntryArray targets = unit_search_radial(unit->position, 512 * 32);
			unit_search_filter(&targets, unit, FilterEnemy | FilterBuilding);

			f32 min_cost = FLT_MAX;
			Unit* best_target = 0;
			for (u32 i = 0; i < targets.size; ++i) {
				Unit* target = targets.units[i];
				if (target->hitpoints <= target->get_max_hp_raw() / 2 && target->status & UnitStatus::Completed
					&& units_dat::BaseProperty[target->id] & UnitProperty::Building && !(units_dat::BaseProperty[target->id] & UnitProperty::Addon)) {
					f32 cost = get_blissful_embrace_target_cost(unit, target);
					if (cost < min_cost) {
						min_cost = cost;
						best_target = target;
					}
				}
			}

			if (best_target) {
				unit->order_to(OrderId::BlissfulEmbrace, best_target);
			}
		}

		/*	
		else if (unit->id == UnitId::TerranPaladin) {
			if (unit->main_order_id != OrderId::Skyfall && scbw::get_generic_timer(unit,RustId::AI_PaladinTimer)) {
				bool skyfallCondition = false;
				cosmonarchyFinderLoop(unit, unit->sprite->position, weapons_dat::MaxRange[WeaponId::Skyfall],
					CFlags::Enemy, {}, [&](Unit* u) {

					// scbw::print("Loop");
					Unit* tryTarget = u;
					s32 innerCount = 0;
					cosmonarchyFinderLoop(u, u->position, weapons_dat::MaxRange[WeaponId::Skyfall] , CFlags::Ally, {},
						[&](Unit* internalUnit) {
						innerCount++;
					});
					// scbw::printf("Count is %d", innerCount);
					if (innerCount >= 6) {
						Point16 source = unit->position;
						Point16 target = u->position;

						auto distance = scbw::get_distance_fast(source.x, source.y, target.x, target.y);
						if (distance >= 64) {
							f64 fraction = (distance - 64) / distance;
							u16 resultX, resultY;
							resultX = MathUtils::lerp(fraction, source.x, target.x, false);
							resultY = MathUtils::lerp(fraction, source.y, target.y, false);
							unit->order_to(OrderId::Skyfall, u->position.x, u->position.y);
							scbw::set_generic_timer(unit, RustId::AI_PaladinTimer, 50);
						}
					}
				});
			}
		}
		*/
	}
}
