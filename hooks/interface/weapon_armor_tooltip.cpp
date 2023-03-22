#include "weapon_armor_tooltip.h"
#include <SCBW/api.h>
#include <SCBW/enumerations/WeaponId.h>
#include <cstdio>
#include "../unit_speed.h"
#include "../weapons/weapon_cooldown.h"
#include "../unit_stats/weapon_range.h"
#include "../config.h"
#define NOMINMAX
#include <Windows.h>
#include <Utils/captivating_claws.h>
#include <algorithm>
#include "player_color.h"

char buffer[256];

// Returns the C-string for the tooltip text of the unit's weapon grp_frame.
// This function is used for weapon icons and special icons.
// Precondition: @p entryStrIndex is a stat_txt.tbl string index.
#include "SCBW/stat_txt_spec.h"
#include <iomanip>

static std::string format_string_vector(std::vector<std::string> strings)
{
	std::string result = "";
	if (strings.size() == 0)
	{
		strings.push_back("None");
	}
	s32 i = 0;
	for (const std::string& str : strings)
	{
		result += str;
		if (++i != strings.size())
		{
			result += ", ";
		}
	}
	return result;
}

void append_tooltip_value(std::string& text, f32 value, f32 bonus, bool extended, bool invert_bonus = false, std::string suffix = "") {
	std::stringstream builder = {}; // TODO: Replace with non std
	builder << std::setprecision(4);

	f32 sign = invert_bonus ? -bonus : bonus;
	std::string color = sign < 0 ? BWTC_RED : sign > 0 ? BWTC_GREEN : BWTC_RESET;
	text += color;

	f32 total_value = value + bonus;
	builder.str("");
	builder << total_value << suffix;
	text += builder.str();

	if (extended && bonus != 0) {
		text += " (";

		builder.str("");
		builder << value << suffix;
		text += builder.str();

		if (bonus > 0) text += " + ";
		else text += " - ";

		builder.str("");
		builder << abs(bonus) << suffix;
		text += builder.str();

		text += ")";
	}
	text += BWTC_RESET;
}

const char* get_damage_tootlip_string(u8 weapon_id, Unit* unit, u16 entryStrIndex)
{
	const bool extended = *g_is_holding_shift || config.always_show_extended_tooltips;

	// Get game strings
	const char* entry_name = g_stat_txt_tbl_new->get_string(entryStrIndex);
	const char* damage_string = g_stat_txt_tbl_new->get_string(777);					//"Damage:"

	u8 damage_factor = scbw::get_damage_factor_for_tooltip(weapon_id, unit);
	u8 upgrade_level = scbw::get_upgrade_level(unit->player, weapons_dat::damage_upgrade[weapon_id]);
	u8 damage_type = weapons_dat::damage_type[weapon_id];
	u32 base_damage = weapons_dat::damage_amount[weapon_id];
	u32 armor_pen = weapons_dat::armor_pen[weapon_id];
	s32 bonus_armor_pen = hooks::get_modified_armor_pen(unit, weapon_id);
	if (bonus_armor_pen < 0) {
		bonus_armor_pen = 0;
	}
	bonus_armor_pen -= armor_pen;

	f32 min_range = weapons_dat::min_range[weapon_id] / 32.0;
	f32 range = weapons_dat::max_range[weapon_id] / 32.0;
	f32 bonus_range = hooks::get_max_weapon_range(unit, weapon_id, unit->order_target.unit) / 32.0 - range;
	bonus_range -= scbw::get_cliff_advantage(unit, unit->order_target.unit, weapon_id);

	std::string damage_modifiers = "";
	f64 attack_cooldown = (f64)weapons_dat::cooldown[weapon_id] / 24;
	f64 attack_cooldown_modifier = ((f64)hooks::get_modified_weapon_cooldown(unit, weapon_id) / 24) - attack_cooldown;

	f64 attack_speed = 1.0 / attack_cooldown;
	f64 attack_speed_modifier = (1.0 / ((f64)hooks::get_modified_weapon_cooldown(unit, weapon_id) / 24)) - attack_speed;

	s32 splash_inner = weapons_dat::inner_splash_radius[weapon_id];
	s32 splash_medium = weapons_dat::medium_splash_radius[weapon_id];
	s32 splash_outer = weapons_dat::outer_splash_radius[weapon_id];
	bool has_splash = splash_inner || splash_medium || splash_outer;
	s32 splash_type = weapons_dat::explosion_type[weapon_id];
	std::string splash_type_value = "";

	/*
	const char* type_name = "";
	switch (damage_type) {
	case DamageType::Concussive: {
	type_name = BWTC_TEAL "Concussive" BWTC_RESET;
	damage_modifiers = BWTC_TEAL "100" BWTC_RESET "/" BWTC_YELLOW "75" BWTC_RESET "/" BWTC_ORANGE "50" BWTC_RESET "/" BWTC_RED "25" BWTC_RESET;
	} break;
	case DamageType::Impact: {
	type_name = BWTC_YELLOW "Impact" BWTC_RESET;
	damage_modifiers = BWTC_TEAL "100" BWTC_RESET "/" BWTC_YELLOW "100" BWTC_RESET "/" BWTC_ORANGE "75" BWTC_RESET "/" BWTC_RED "50" BWTC_RESET;
	} break;
	case DamageType::Explosive: {
	type_name = BWTC_ORANGE "Explosive" BWTC_RESET;
	damage_modifiers = BWTC_TEAL "50" BWTC_RESET "/" BWTC_YELLOW "75" BWTC_RESET "/" BWTC_ORANGE "100" BWTC_RESET "/" BWTC_RED "75" BWTC_RESET;
	} break;
	case DamageType::Chronal: {
	type_name = BWTC_WHITE "Chronal" BWTC_RESET;
	damage_modifiers = BWTC_TEAL "100" BWTC_RESET "/" BWTC_YELLOW "100" BWTC_RESET "/" BWTC_ORANGE "100" BWTC_RESET "/" BWTC_RED "100" BWTC_RESET;
	} break;
	case DamageType::Independent: {
	type_name = BWTC_WHITE "Independent" BWTC_RESET;
	} break;
	case DamageType::IgnoreArmor: {
	type_name = BWTC_WHITE "Ignores Armor" BWTC_RESET;
	} break;
	default: {
	type_name = "None";
	} break;
	};
	*/

	switch (splash_type) {
		case WeaponEffect::SplashRadial: {
			splash_type_value = BWTC_RED "Unsafe" BWTC_RESET;
		} break;
		case WeaponEffect::SplashEnemy: {
			splash_type_value = BWTC_GREEN "Safe" BWTC_RESET;
		} break;
		case WeaponEffect::Unknown_Crash: {
			splash_type_value = BWTC_GREEN "Ally Safe" BWTC_RESET; 
		} break;
		case WeaponEffect::SplashAir: {	
			splash_type_value = BWTC_YELLOW "Air Only" BWTC_RESET;
		} break;
		default: {
			if (scbw::weapon_id_is_melee_aoe(weapon_id)) {
				splash_type_value = BWTC_GREEN "Ally Safe" BWTC_RESET;
			}
			else {
				splash_type_value = BWTC_WHITE "Normal Hit" BWTC_RESET;
			}
		} break;
	};

	std::vector<std::string> target_flags;
	if (weapons_dat::target_flags[weapon_id].ground)
	{
		target_flags.push_back("Ground");
	}
	if (weapons_dat::target_flags[weapon_id].air)
	{
		target_flags.push_back("Air");
	}
	if (weapons_dat::target_flags[weapon_id].mechanical || weapons_dat::target_flags[weapon_id].orgOrMech)
	{
		target_flags.push_back("Mechanical");
	}
	if (weapons_dat::target_flags[weapon_id].organic || weapons_dat::target_flags[weapon_id].orgOrMech)
	{
		target_flags.push_back("Organic");
	}
	if (weapons_dat::target_flags[weapon_id].nonBuilding)
	{
		target_flags.push_back("Not Building");
	}
	if (weapons_dat::target_flags[weapon_id].nonRobotic)
	{
		target_flags.push_back("Not Robotic");
	}
	if (weapons_dat::target_flags[weapon_id].terrain)
	{
		target_flags.push_back("Terrain");
	}
	std::string target_flags_text = format_string_vector(target_flags);

	// Name
	std::string text = BWTC_WHITE + std::string(entry_name) + BWTC_RESET"\n";

	// Damage
	if (extended) {
		text += std::string(damage_string) + " " + std::to_string(damage_factor) + "x(" + std::to_string(base_damage) + ")";
	} else {
		text += std::string(damage_string) + " " + std::to_string(damage_factor * base_damage);
	}

	// Armor Pen
	text += "\nArmor Pen: ";
	append_tooltip_value(text, armor_pen, bonus_armor_pen, extended);

	// Attack Speed
	text += "\nAttack Speed: ";
	append_tooltip_value(text, attack_speed, attack_speed_modifier, extended);

	// Cooldown
	text += "\nCooldown: ";
	append_tooltip_value(text, attack_cooldown, attack_cooldown_modifier, extended, true, "s");

	// DPS
	std::stringstream builder;
	builder << std::setprecision(4);
	if (extended) {
		std::string dps_color = attack_speed_modifier < 0 ? BWTC_RED : attack_speed_modifier > 0 ? BWTC_GREEN : BWTC_RESET;
		f64 total_cooldown = (attack_cooldown + attack_cooldown_modifier);
		builder.str("");
		builder << ((f64)damage_factor * base_damage / total_cooldown);
		text += "\nDPS: " + dps_color + builder.str() + BWTC_RESET;
	}

	// Range
	text += "\nRange: ";
	if (range <= 1) {
		text += "Melee";
	} else {
		if (min_range > 0) {
			builder.str("");
			builder << min_range << "-";
			text += builder.str();
		}
		append_tooltip_value(text, range, bonus_range, extended);
	}

	// Target
	text += "\nTargets: " + target_flags_text;

	// Type
	/*
	text += "\nType: " + std::string(type_name);
	if (extended && damage_modifiers != "")
	{
	text += " (" + damage_modifiers + ")";
	}
	*/

	// Splash
	if (has_splash)
	{
		text += "\nSplash Radius: " + std::to_string(splash_inner) + "/" + std::to_string(splash_medium) + "/" + std::to_string(splash_outer);
		text += "\nSplash Type: " + splash_type_value;
	}

	if (!extended)
	{
		text += "\n-- Hold \003Shift\002 for more details --";
	}

	strncpy(buffer, text.c_str(), sizeof(buffer));
	buffer[sizeof(buffer) - 1] = 0;
	return buffer;
}

namespace hooks
{
	// Returns the C-string for the tooltip text of the unit's weapon grp_frame.
	const char* get_weapon_tooltip_string(u8 weapon_id, Unit* unit) {
		return get_damage_tootlip_string(weapon_id, unit, weapons_dat::label[weapon_id]);
	}

	static std::string vanilla_timer_to_string(s32 timer) {
		std::stringstream builder;
		builder << std::setprecision(4);
		builder << ceil(timer * 0.042 * 8.0) << "s";
		return builder.str();
	}

	static std::string gptp_timer_to_string(s32 timer) {
		std::stringstream builder;
		builder << std::setprecision(4);
		builder << ceil((timer - 1) / 24.0) << "s";
		return builder.str();
	}

	static std::string remove_timer_to_string(s32 timer) {
		std::stringstream builder;
		builder << std::setprecision(4);
		builder << ceil(timer / 24.0) << "s";
		return builder.str();
	}

	enum struct TimerType {
		Vanilla = 0,
		Gptp,
		RemoveTimer
	};

	// max_value					value format
	// 0  - unlimited stacks		x
	// 1  - binary true / false
	// >1 - limited stacks			x/max
	static void add_status(std::vector<std::string>& vec, std::string name, s32 value = -1, s32 max_value = 0, s32 timer = -1, TimerType timer_type = TimerType::Vanilla) {
		if (timer > 0 || value > 0) {
			std::string text = name;
			bool show_value = false;
			if (max_value <= 0) {
				show_value = value > 1;
			} else if (max_value > 1)  {
				show_value = value > 0;
			}
			if (timer > 0 || show_value) {
				text += " (";
				if (timer > 0) {
					if (timer_type == TimerType::Vanilla) {
						text += vanilla_timer_to_string(timer);
					} else if (timer_type == TimerType::Gptp) {
						text += gptp_timer_to_string(timer);
					} else {
						text += remove_timer_to_string(timer);
					}
					if (show_value) {
						text += ", ";
					}
				}
				if (show_value) {
					text += std::to_string(value);
					if (max_value > 1) {
						text += "/" + std::to_string(max_value);
					}
				}
				text += ")";
			}
			vec.emplace_back(text);
		}
	}

	s32 get_bonus_armor(Unit* unit, u8 base_armor) {
		UnitExt* unit_ext = get_unit_ext(unit);
		s32 bonus_armor = base_armor + unit->get_armor_bonus();
		bonus_armor -= unit_ext->values[ValueControlledOpposition];
		if (bonus_armor < 0) {
			bonus_armor = 0;
		}
		bonus_armor -= unit_ext->values[ValueObservanceRend];
		bonus_armor -= base_armor;
		return bonus_armor;
	}

	const char* get_armor_tooltip_string(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);

		const bool extended = *g_is_holding_shift || config.always_show_extended_tooltips;

		const u16 label_id = upgrades_dat::label[units_dat::armor_upgrade[unit->id]];
		const char* armor_upgrade_name = g_stat_txt_tbl_new->get_string(label_id);
		const char* armor_text = g_stat_txt_tbl_new->get_string(778); // "Armor:"

		const u8 unit_size = units_dat::size_type[unit->id];
		const char* size_label = "Size:";
		const char* size_text = "";

		const u8 base_armor = units_dat::armor_amount[unit->id];
		s32 bonus_armor = get_bonus_armor(unit, base_armor);

		const u8 transport_space = units_dat::space_required[unit->id];
		const s32 sight_range = units_dat::sight_range[unit->id];

		/*
		switch (unit_size) {
			case UnitSize::Light: {
				size_text = BWTC_TEAL "Light" BWTC_RESET;
			} break;
			case UnitSize::Medium: {
				size_text = BWTC_YELLOW "Medium" BWTC_RESET;
			} break;
			case UnitSize::Heavy: {
				size_text = BWTC_ORANGE "Heavy" BWTC_RESET;
			} break;
			case UnitSize::Titanic: {
				size_text = BWTC_RED "Titanic" BWTC_RESET;
			} break;
			default: {
				size_text = BWTC_WHITE "Unknown" BWTC_RESET;
			} break;
		}
		*/

		std::stringstream builder;
		builder << std::setprecision(4);

		f32 speed_calc = (unit->flingy_top_speed / 256.0) / 24.0 / 32.0;
		if (unit->movement_flags & MovementFlags::Moving && !(unit->movement_flags & MovementFlags::Accelerating)) {
			speed_calc = (unit->current_speed_2 / 256.0) / 24.0 / 32.0;
		}
		else if (unit->stim_timer) {
			speed_calc /= 2;
		}
		builder << speed_calc << " tiles/sec";
		std::string movement_speed = builder.str();
		builder.str("");

		s32 accel = hooks::get_modified_unit_acceleration(unit);
		builder << accel;
		std::string acceleration = builder.str();
		builder.str("");

		builder << hooks::get_modified_unit_turn_speed(unit);
		std::string turn_rate = builder.str();
		builder.str("");

		std::string plane = unit->status & UnitStatus::InAir ? "Air" : "Ground";

		std::vector<std::string> statuses;
		if (unit->status & UnitStatus::Burrowed)
		{
			statuses.push_back("Burrowed");
		}
		else if (unit->status & UnitStatus::Cloaked)
		{
			statuses.push_back("Cloaked");
		}

		// Gptp Timer + Value
		add_status(statuses, "Adrenal Glands", unit_ext->values[ValueAdrenalStacks], 4,
			unit_ext->timers[TimerAdrenal], TimerType::Gptp);
		add_status(statuses, "Captivating Claws", captivating_claws_get_stacks(unit), 4,
			captivating_claws_get_timer(unit), TimerType::Gptp);
		add_status(statuses, "Observance", unit_ext->values[ValueObservanceRend], 0,
			unit_ext->timers[TimerObservanceDebuff], TimerType::Gptp);
		add_status(statuses, "Corrosive Spores", unit_ext->values[ValueCorrosiveTouchStacks], 0,
			unit_ext->timers[TimerCorrosiveTouch], TimerType::Gptp);
		add_status(statuses, "Dirge Fuse", unit_ext->values[ValueDirgeStacks] * MadrigalDirgeDamage, 0,
			unit_ext->timers[TimerDirgeFuse], TimerType::Gptp);
		add_status(statuses, "Petrification", unit_ext->values[ValuePetrificationStacks], 0,
			unit_ext->timers[TimerPetrification], TimerType::Gptp);
		add_status(statuses, "Deconstruction", unit_ext->values[ValueDeconstruction], 0,
			unit_ext->timers[TimerDeconstruction], TimerType::Gptp);

		if (unit_ext->values[ValueSignifyStacks] < 5)
		{
			add_status(statuses, "Cowardice", unit_ext->values[ValueSignifyStacks], 5,
				unit_ext->timers[TimerSignify], TimerType::Gptp);
		}
		else
		{
			add_status(statuses, "Malice", unit_ext->values[ValueSignifyStacks], 1,
				unit_ext->timers[TimerSignify], TimerType::Gptp);
		}

		// Gptp Timer
		add_status(statuses, "Kaiser Rampage", -1, 1, unit_ext->timers[TimerKaiserRampage], TimerType::Gptp);
		add_status(statuses, "Hallucinating", -1, 1, unit_ext->timers[TimerHallucinating], TimerType::Gptp);
		add_status(statuses, "Controlled Demolition", -1, 1, unit_ext->timers[TimerControlledDemolition], TimerType::Gptp);
		add_status(statuses, "Emergency Detonation", -1, 1, unit_ext->timers[TimerEmergencyDetonation], TimerType::Gptp);
		add_status(statuses, "Binding Grip", -1, 1, unit_ext->timers[TimerBindingGrip], TimerType::Gptp);
		add_status(statuses, "Autovitality", -1, 1, unit_ext->timers[TimerSimulacrumClone], TimerType::Gptp);
		add_status(statuses, "Caustic Discharge", -1, 1, unit_ext->timers[TimerCausticDischargeSpeed], TimerType::Gptp);
		add_status(statuses, "Ion Field", -1, 1, unit_ext->timers[TimerIonFieldAttackSpeed], TimerType::Gptp);
		add_status(statuses, "Triumphant Rise", -1, 1, unit_ext->timers[TimerTriumphantRiseDebuff], TimerType::Gptp);
		add_status(statuses, "Terminal Surge", -1, 1, unit_ext->timers[TimerTerminalSurge], TimerType::Gptp);
		add_status(statuses, "Stellar Enforcement", -1, 1, unit_ext->timers[TimerStellarEnforcement], TimerType::Gptp);
		add_status(statuses, "Kelvin Munitions", -1, 1, unit_ext->timers[TimerKelvinMunitions], TimerType::Gptp);
		add_status(statuses, "Incubating", -1, 1, unit_ext->timers[TimerIncubationArm], TimerType::Gptp);

		// Gptp Value
		add_status(statuses, "Ensnaring Brood", unit_ext->timers[TimerEnsnaringBrood], 1);
		add_status(statuses, "Cabeirus Feeders", ceil((f64)unit_ext->values[ValueMadcapStacks] / MadcapPassiveDecayTime), 10);
		add_status(statuses, "Nemean Reactors", unit_ext->values[ValueNemeanReactorsStack], 0);
		add_status(statuses, "Sublime Shepherd", unit_ext->values[ValueSublimeShepherd], 0);
		add_status(statuses, "Stonewall Protocol", unit_ext->values[ValueCuirassIsSelfReviving], 0);
		add_status(statuses, "Reconstitution", unit_ext->values[ValueReconstitution], 1);
		add_status(statuses, "Repentent Chorus", unit_ext->values[ValueRepententChorus], 0);
		add_status(statuses, "Controlled Opposition", unit_ext->values[ValueControlledOpposition], 0);
		add_status(statuses, "Anodyne Annex", unit_ext->values[ValueEmpressPassiveStacks], 0);
		add_status(statuses, "Fragmentary Shells", unit_ext->timer_vectors[TimerVectorCyclopsFuse].size(), 0);
		add_status(statuses, "Energy Decay", unit_ext->values[ValueEntropyGauntlets], 0);
		add_status(statuses, "Khaydarin Charge", unit_ext->unit_vectors[UnitVectorCrucibles].size(), 0);
		add_status(statuses, "Decaying", unit_ext->values[ValueNoCorpse], 1);

		// Gptp Value - Gimmicked
		add_status(statuses, "Power Siphon", scbw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit) == 1, 1);
		add_status(statuses, "Entropy Gauntlets", unit->id == UnitId::TerranSavant && scbw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit) == 0, 1);
		add_status(statuses, "Tempo Up", unit->id == UnitId::TerranMadrigal && unit_ext->values[ValueMadrigalTempoDown] == 0, 1);
		add_status(statuses, "Tempo Down", unit_ext->values[ValueMadrigalTempoDown] == 1, 1);
		add_status(statuses, "Phase Link", scbw::has_clarion_links(unit), 1);
		add_status(statuses, "Astral Rule", unit_ext->timers[TimerPatriarchAstralRule] > 0, 1);
	//	add_status(statuses, "Making Gods from Flesh", unit_ext->values[ValueGodsMadeFleshActive] > 0, 1);
		add_status(statuses, "Swarming Omen", unit_ext->values[ValueSwarmingOmenPlayer] > 0, 1);
		add_status(statuses, "Blissful Embrace", unit_ext->values[ValueBlissfulEmbraceOriginalPlayer] > 0, 1);

		if (unit_ext->units[UnitMindController]) {
			switch (unit_ext->units[UnitMindController]->id) {
				case UnitId::ProtossMindTyrant: {
					add_status(statuses, "Tyranny", 1, 1);
				} break;
				case UnitId::ProtossDemiurge: {
					add_status(statuses, "Worshiping", 1, 1);
				} break;
			}
		}

		// Gptp Timers - Gimmicked
		add_status(statuses, "Observing", unit_ext->timers[TimerObservanceOverlay] > 0, 1);
		add_status(statuses, "Dermal Accelerant", unit_ext->timers[TimerAlaszilethRegenBoost] > 0, 1);
	//	add_status(statuses, "Godlike", unit_ext->timers[TimerGodsMadeFlesh] > 0, 1);

		// Vanilla Timer + Value
		add_status(statuses, "Defensive Matrix", unit->defensive_matrix_hp / 256, 200, unit->defensive_matrix_timer);

		// Vanilla Timers
		add_status(statuses, "Stimpack", -1, 1, unit->stim_timer);
		add_status(statuses, "Irradiated", -1, 1, unit->irradiate_timer);
		add_status(statuses, "Ensnared", -1, 1, unit->ensnare_timer);
		add_status(statuses, "Lobotomized", -1, 1, unit->lockdown_timer);
		add_status(statuses, "Maelstromed", -1, 1, unit->maelstrom_timer);
		add_status(statuses, "Plagued", -1, 1, unit->plague_timer);
		add_status(statuses, "Stasis", -1, 1, unit->stasis_timer);
		add_status(statuses, "Timed Life", -1, 1, unit->remove_timer, TimerType::RemoveTimer);

		// Vanilla Value
		add_status(statuses, "Blinded", unit->is_blind != 0, 1);
		add_status(statuses, "Lifted Off", unit->status & UnitStatus::InAir && units_dat::base_property[unit->id] & UnitProperty::Building, 1);
		add_status(statuses, "Cannot Attack", unit_ext->timers[TimerUnderDisruptionField] > 0, 1);

		// Case Specific
		add_status(statuses, "Sieged", unit->id == UnitId::TerranPhalanxSiegeMode, 1);
		if (scbw::is_under_dark_swarm(unit))
		{
			Unit* darkSwarm = scbw::get_dark_swarm_unit(unit);
			add_status(statuses, "Under Dark Swarm", darkSwarm->hitpoints / 256, 1, darkSwarm->remove_timer, TimerType::RemoveTimer);
		}

		/* // Can't see armor icons on eggs yet
		if ((unit->id == UnitId::ZergEgg && !scbw::is_larval_egg(unit)) || unit->id == UnitId::ZergCocoon) {
			if (scbw::get_active_tile_at(unit->position.x, unit->position.y).hasCreep) {
				add_status(statuses, "Biogenesis");
			}
		}
		*/

		std::string status = format_string_vector(statuses);

		// TODO Statuses
		// mind control on target, last orders
		// color code source player if possible

		std::vector<std::string> flags; // TODO: replace with something not gay
		if (units_dat::base_property[unit->id] & UnitProperty::Building)
		{
			flags.push_back("Structure");
		}
		if (units_dat::base_property[unit->id] & UnitProperty::ProducesUnits)
		{
			flags.push_back("Factory");
		}
		if (units_dat::base_property[unit->id] & UnitProperty::Addon)
		{
			flags.push_back("Addon");
		}
		if (units_dat::base_property[unit->id] & UnitProperty::Worker)
		{
			flags.push_back("Worker");
		}
		if (units_dat::base_property[unit->id] & UnitProperty::Hero)
		{
			flags.push_back("Hero");
		}
		if (units_dat::base_property[unit->id] & UnitProperty::Invincible)
		{
			flags.push_back("Invincible");
		}
		/* // Disabled since they mean nothing now. -Pr0nogo
		if (units_dat::base_property[unit->id] & UnitProperty::Organic)
		{
		flags.push_back("Organic");
		}
		if (units_dat::base_property[unit->id] & UnitProperty::Mechanical)
		{
		flags.push_back("Mechanical");
		}
		if (units_dat::base_property[unit->id] & UnitProperty::RoboticUnit)
		{
		flags.push_back("Robotic");
		}
		*/
		if (units_dat::base_property[unit->id] & UnitProperty::Detector)
		{
			if (!(units_dat::base_property[unit->id] & UnitProperty::Building)
			||   (units_dat::base_property[unit->id] & UnitProperty::Building && !(unit->status & UnitStatus::InAir)))
			{
				flags.push_back("Detector");
			}
			else {
				flags.push_back("Detector (disabled)");
			}
		}
		if (units_dat::base_property[unit->id] & UnitProperty::CreepBuilding)
		{
			flags.push_back("Spreads creep");
		}

		std::string flag = format_string_vector(flags);

		std::string text = "\004" + std::string(armor_upgrade_name) + "\002";
		text += "\n" + std::string(armor_text) + " ";
		append_tooltip_value(text, base_armor, bonus_armor, extended);

	//	text += "\nSize: " + std::string(size_text);
		text += "\nPlane: " + plane;
	//	text += "\nMove Speed: " + movement_speed;
		text += "\nStatus: " + status;

		if (extended) {
			if (unit->player < PlayerLimit) {
				text += "\nColor: " + std::string(scbw::get_player_color_code(unit->player).data) + std::string(player_color_names[player_data.player_color_list[unit->player]]) + "\002"; // TODO: move to rank text
			}
			text += "\nFlags: " + flag;
			if (accel != 1) {
				text += "\nAcceleration: " + acceleration;
			}
			text += "\nTurn Rate: " + turn_rate;
			text += "\nSight Range: " + std::to_string(sight_range);
			if (transport_space != 255)
			{
				text += "\nTransport Space: " + std::to_string(transport_space);
			}
		} else {
			text += "\n-- Hold \003Shift\002 for more details --";
		}

		strncpy(buffer, text.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = 0;
		return buffer;
	}

	// Returns the C-string for the tooltip text of the plasma shield grp_frame.
	const char* get_shield_tooltip_string(Unit* unit) {
		const bool extended = *g_is_holding_shift || config.always_show_extended_tooltips;
		const u16 label_id = upgrades_dat::label[UpgradeId::ProtossPlasmaShields];
		const char* shield_upgrade_name = g_stat_txt_tbl_new->get_string(label_id);
		//const char* shields_text = g_stat_txt_tbl_new->get_string(779); // "Shields:"
		const char* armor_text = g_stat_txt_tbl_new->get_string(778); // "Armor:"

		const u8 base_armor = units_dat::armor_amount[unit->id];
		s32 bonus_armor = get_bonus_armor(unit, base_armor);

		std::string text = "\004" + std::string(shield_upgrade_name) + "\002";
		text += "\n" + std::string(armor_text) + " ";
		append_tooltip_value(text, base_armor, bonus_armor, extended);

		strncpy(buffer, text.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = 0;
		return buffer;
	}

	// Returns the C-string for the tooltip text of the Interceptor grp_frame (Carriers), Scarab grp_frame (Reavers), Nuclear Missile grp_frame (Nuclear Silos), and Spider Mine grp_frame (Vultures).
	const char* get_special_tooltip_string(u16 icon_unit_id, Unit* unit)
	{
		if (icon_unit_id == UnitId::ProtossSteward)
		{
			return get_damage_tootlip_string(WeaponId::StewardPulseCannon, unit, 791);				// "Stewards"
		}
		if (icon_unit_id == UnitId::ProtossPlasmaShell)
		{
			return get_damage_tootlip_string(WeaponId::Scarab, unit, 792);						// "Plasma Shells"
		}
		if (icon_unit_id == UnitId::TerranNuclearMissile)
		{
			return g_stat_txt_tbl_new->get_string(793);											// "Missiles"
		}
		if (icon_unit_id == UnitId::TerranSpiderMine && unit->id == UnitId::TerranVulture)
		{
			return get_damage_tootlip_string(WeaponId::SpiderMines, unit, 794);				// "Spider Mines"
		}
		// Should never reach here
		return "";
	}
}