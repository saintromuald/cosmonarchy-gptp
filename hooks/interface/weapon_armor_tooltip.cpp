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

// Text colors	TODO: Move to other file
#define BWTC_RESET		"\002"
#define BWTC_TEAL		"\017"
#define BWTC_GREEN		"\007"
#define BWTC_YELLOW		"\003"
#define BWTC_ORANGE		"\021"
#define BWTC_RED		"\006"
#define BWTC_WHITE		"\004"

char buffer[256];

// Returns the special damage multiplier factor for units that don't use the
// "Damage Factor" property in weapons.dat.
u8 getDamageFactorForTooltip(u8 weapon_id, Unit* unit)
{
	if (weapon_id == WeaponId::CohortGaussPistols
	||  weapon_id == WeaponId::HarakanFlameThrower
	||  weapon_id == WeaponId::PazuzuJupiterDriver
	||  weapon_id == WeaponId::ZealotPsiBlades
	||  weapon_id == WeaponId::LegionnairePsiSabers
	||  weapon_id == WeaponId::AmaranthPsiCleaver
	||  weapon_id == WeaponId::GolemBloodlessAssault
	||  weapon_id == WeaponId::PariahIconoclastBlades
	||  weapon_id == WeaponId::StarSovereignCelestialCannon
	||  weapon_id == WeaponId::LiiraliskHollowedSpores
	||  weapon_id == WeaponId::BactaliskCorrosiveAcid
	||  weapon_id == WeaponId::AlmaksaliskPiercingSpines
	||  weapon_id == WeaponId::KeskathalorScreamingVenom)
	{
		return 2;
	}
	return weapons_dat::DamageFactor[weapon_id];
}

// Returns the C-string for the tooltip text of the unit's weapon icon.
// This function is used for weapon icons and special icons.
// Precondition: @p entryStrIndex is a stat_txt.tbl string index.
#include "SCBW/stat_txt_spec.h"
#include <iomanip>

static std::string formatStringVector(std::vector<std::string> strings)
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

const char* getDamageTooltipString(u8 weapon_id, Unit* unit, u16 entryStrIndex)
{
	const bool extended = GetKeyState(VK_SHIFT) & 0x8000 || config.always_show_extended_tooltips;

	// Get game strings
	const char* entryName = statTxtTblNew->get_string(entryStrIndex);
	const char* damageStr = statTxtTblNew->get_string(777);					//"Damage:"
//	const char* perRocketStr = statTxtTblNew->get_string(1301);				//"per rocket"
	const char* type_name = "";

	u8 damage_factor = getDamageFactorForTooltip(weapon_id, unit);
	u8 upgrade_level = scbw::get_upgrade_level(unit->player, weapons_dat::DamageUpgrade[weapon_id]);
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
	f64 attack_cooldown = (f64)weapons_dat::Cooldown[weapon_id] / 24;
	f64 attack_cooldown_modifier = ((f64)hooks::get_modified_weapon_cooldown(unit, weapon_id) / 24) - attack_cooldown;

	s32 splashInner = weapons_dat::InnerSplashRadius[weapon_id];
	s32 splashMedium = weapons_dat::MediumSplashRadius[weapon_id];
	s32 splashOuter = weapons_dat::OuterSplashRadius[weapon_id];
	bool hasSplash = splashInner || splashMedium || splashOuter;
	s32 splash_type = weapons_dat::ExplosionType[weapon_id];
	std::string splash_type_value = "";

	bool isSuicideUnit = unit->id == UnitId::ZergSkryling;

	/*
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
			splash_type_value = BWTC_WHITE "Normal Hit" BWTC_RESET;
		} break;
	};

	std::vector<std::string> targetFlags;
	if (weapons_dat::TargetFlags[weapon_id].ground)
	{
		targetFlags.push_back("Ground");
	}
	if (weapons_dat::TargetFlags[weapon_id].air)
	{
		targetFlags.push_back("Air");
	}
	if (weapons_dat::TargetFlags[weapon_id].mechanical || weapons_dat::TargetFlags[weapon_id].orgOrMech)
	{
		targetFlags.push_back("Mechanical");
	}
	if (weapons_dat::TargetFlags[weapon_id].organic || weapons_dat::TargetFlags[weapon_id].orgOrMech)
	{
		targetFlags.push_back("Organic");
	}
	if (weapons_dat::TargetFlags[weapon_id].nonBuilding)
	{
		targetFlags.push_back("Not Building");
	}
	if (weapons_dat::TargetFlags[weapon_id].nonRobotic)
	{
		targetFlags.push_back("Not Robotic");
	}
	if (weapons_dat::TargetFlags[weapon_id].terrain)
	{
		targetFlags.push_back("Terrain");
	}

	std::string targetFlagsText = formatStringVector(targetFlags);

	// Name
	std::string text = "\004" + std::string(entryName) + "\002\n";

	// Damage
	if (extended)
	{
		text += std::string(damageStr) + " " + std::to_string(damage_factor) + "x(" + std::to_string(base_damage);
		text += ")";
	}
	else
	{
		text += std::string(damageStr) + " " + std::to_string(damage_factor * base_damage);

		/*
		if (weapon_id == WeaponId::ValkyrieHaloRockets)
		{
			text += " " + std::string(perRocketStr);
		}
		*/
	}

	// Armor Pen
	if (armor_pen > 0) {
		text += "\nArmor Pen: ";

		std::stringstream ss;
		ss << armor_pen;
		if (bonus_armor_pen > 0)      text += BWTC_GREEN "(+" + ss.str() + BWTC_RESET ")";
		else if (bonus_armor_pen < 0) text += BWTC_RED   "("  + ss.str() + BWTC_RESET ")";
		text += ss.str();
	}

	// Cooldown
	std::string modifier_color = attack_cooldown_modifier > 0 ? "\006" : attack_cooldown_modifier < 0 ? "\007" : "\002";
	std::stringstream string_builder; // TODO: replace with something not gay
	string_builder << std::setprecision(4);
	if (!isSuicideUnit)
	{
		if (extended) string_builder << attack_cooldown;
		else string_builder << (attack_cooldown + attack_cooldown_modifier);
		text += "\nCooldown: ";
		text += modifier_color;
		text += string_builder.str() + "s";
		if (extended && attack_cooldown_modifier != 0)
		{
			string_builder.str("");
			string_builder << abs(attack_cooldown_modifier);

			if (attack_cooldown_modifier > 0) text += " + ";
			else text += " - ";
			text += modifier_color + string_builder.str() + "s";
		}
		text += "\002";
	}

	// DPS
	if (!isSuicideUnit && extended)
	{
		string_builder.str("");
		f64 cd = (attack_cooldown + attack_cooldown_modifier);
		string_builder << ((f64)damage_factor * base_damage / cd);
		text += "\nDPS: " + modifier_color + string_builder.str() + "\002";
	}

	// Range
	text += "\nRange: ";
	if (range > 1)
	{
		string_builder.str("");
		if (min_range > 0)
		{
			string_builder << min_range << "-";
		}
		string_builder << range;
		if (bonus_range > 0)
		{
			string_builder << "+" << bonus_range;
		}
		text += string_builder.str();
	}
	else
	{
		text += "Melee";
	}

	// Target
	text += "\nTargets: " + targetFlagsText;

	// Type
	/*
	text += "\nType: " + std::string(type_name);
	if (extended && damage_modifiers != "")
	{
		text += " (" + damage_modifiers + ")";
	}
	*/

	// Splash
	if (hasSplash)
	{
		text += "\nSplash Radius: " + std::to_string(splashInner) + "/" + std::to_string(splashMedium) + "/" + std::to_string(splashOuter);
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
	// Returns the C-string for the tooltip text of the unit's weapon icon.
	const char* get_weapon_tooltip_string(u8 weapon_id, Unit* unit)
	{
		return getDamageTooltipString(weapon_id, unit, weapons_dat::label[weapon_id]);
	}

	static std::string timerToString(s32 timer)
	{
		std::stringstream ss;
		ss << ceil(timer * 0.042 * 8.0) << "s";
		return ss.str();
	}

	static std::string gptpTimerToString(s32 timer)
	{
		std::stringstream ss;
		ss << ceil((timer - 1) / 24.0) << "s";
		return ss.str();
	}

	static std::string removeTimerToString(s32 timer)
	{
		std::stringstream ss;
		ss << ceil(timer / 24.0) << "s";
		return ss.str();
	}

	enum struct TimerType
	{
		Vanilla = 0,
		Gptp,
		RemoveTimer
	};

	// maxValue						value format
	// 0  - unlimited stacks		x
	// 1  - binary true / false
	// >1 - limited stacks			x/max
	static void add_status(std::vector<std::string>& vec, std::string name, s32 value = -1, s32 maxValue = 0, s32 timer = -1, TimerType timerType = TimerType::Vanilla)
	{
		if (timer > 0 || value > 0)
		{
			std::string text = name;
			if (timer > 0 || value > 1 || (value > 0 && maxValue != 1))
			{
				text += " (";
				if (timer > 0)
				{
					if (timerType == TimerType::Vanilla)
					{
						text += timerToString(timer);
					}
					else if (timerType == TimerType::Gptp)
					{
						text += gptpTimerToString(timer);
					}
					else
					{
						text += removeTimerToString(timer);
					}
					if (value > 0 && maxValue != 1)
					{
						text += ", ";
					}
				}
				if (value > 0 && maxValue != 1)
				{
					text += std::to_string(value);
					if (maxValue > 1)
					{
						text += "/" + std::to_string(maxValue);
					}
				}
				text += ")";
			}
			vec.push_back(text);
		}
	}

	const char* get_armor_tooltip_string(Unit* unit)
	{
		UnitExt* unit_ext = globals.get_unit_ext(unit);

		const bool extended = GetKeyState(VK_SHIFT) & 0x8000 || config.always_show_extended_tooltips;

		const u16 label_id = upgrades_dat::label[units_dat::ArmorUpgrade[unit->id]];
		const char* armor_upgrade_name = statTxtTblNew->get_string(label_id);
		const char* armorStr = statTxtTblNew->get_string(778); // "Armor:"

		const u8 base_armor = units_dat::ArmorAmount[unit->id];
		const u8 unit_size = units_dat::SizeType[unit->id];
		const char* size_label = "Size:";
		const char* size_text = "";

		f32 bonus_armor = base_armor + unit->get_armor_bonus();
		bonus_armor -= unit_ext->values[ValueControlledOpposition];
		bonus_armor -= unit_ext->values[ValueObservanceRend];
		if (bonus_armor < 0) {
			bonus_armor = 0;
		}
		bonus_armor -= base_armor;

		const u8 transport_space = units_dat::SpaceRequired[unit->id];
		const s32 sight_range = units_dat::SightRange[unit->id];
		
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

		std::stringstream ss;
		ss << std::setprecision(4);

		ss << unit->flingy_top_speed / 32 / 15 * 24;
		std::string movementSpeed = ss.str();
		ss.str("");

		s32 accel = hooks::get_modified_unit_acceleration(unit);
		ss << accel;
		std::string acceleration = ss.str();
		ss.str("");

		ss << hooks::get_modified_unit_turn_speed(unit);
		std::string turnRate = ss.str();
		ss.str("");

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
		add_status(statuses, "Captivating ZethrokorClaws", captivating_claws_get_stacks(unit), 4,
			captivating_claws_get_timer(unit), TimerType::Gptp);
		add_status(statuses, "Swarming Omen Aura", unit_ext->values[ValueSwarmingOmenAbility], 1,
			unit_ext->timers[TimerSwarmingOmen], TimerType::Gptp);
		add_status(statuses, "Observance", unit_ext->values[ValueObservanceRend], 0,
			unit_ext->timers[TimerObservanceDebuff], TimerType::Gptp);
		add_status(statuses, "Corrosive Spores", unit_ext->values[ValueCorrosiveTouchStacks], 0,
			unit_ext->timers[TimerCorrosiveTouch], TimerType::Gptp);
		add_status(statuses, "Dirge Fuse", unit_ext->values[ValueDirgeStacks] * MadrigalDirgeDamage, 0,
			unit_ext->timers[TimerDirgeFuse], TimerType::Gptp);
		add_status(statuses, "Petrification", unit_ext->values[ValuePetrificationStacks], 0,
			unit_ext->timers[TimerPetrification], TimerType::Gptp);

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
		// TODO: Add mineral and egg boost timers eventually (currently can't read tooltips from eggs nor mineral fields)
		add_status(statuses, "Ensnaring Brood", -1, 1,
			unit_ext->timers[TimerEnsnaringBrood], TimerType::Gptp);
		add_status(statuses, "Kaiser Rampage", -1, 1,
			unit_ext->timers[TimerKaiserRampage], TimerType::Gptp);
		add_status(statuses, "Hallucinating", -1, 1,
			unit_ext->timers[TimerHallucinating], TimerType::Gptp);
		add_status(statuses, "Controlled Demolition", -1, 1,
			unit_ext->timers[TimerControlledDemolition], TimerType::Gptp);
		add_status(statuses, "Emergency Detonation", -1, 1,
			unit_ext->timers[TimerEmergencyDetonation], TimerType::Gptp);
		add_status(statuses, "Binding Grip", -1, 1,
			unit_ext->timers[TimerBindingGrip], TimerType::Gptp);
		add_status(statuses, "Autovitality", -1, 1,
			unit_ext->timers[TimerSimulacrumClone], TimerType::Gptp);
		add_status(statuses, "Caustic Discharge", -1, 1,
			unit_ext->timers[TimerCausticDischargeSpeed], TimerType::Gptp);
		add_status(statuses, "Ion Field", -1, 1,
			unit_ext->timers[TimerIonFieldAttackSpeed], TimerType::Gptp);

		// Gptp Value
		add_status(statuses, "Cabeirus Feeders", ceil((f64)unit_ext->values[ValueMadcapStacks] / MadcapPassiveDecayTime), 10);
		add_status(statuses, "Nemean Reactors", unit_ext->values[ValueNemeanReactorsStack], 0);
		add_status(statuses, "Sublime Shepherd", unit_ext->values[ValueSublimeShepherd], 0);
		add_status(statuses, "Lazarus Agent", unit_ext->values[ValueLazarusAgent], 0);
		add_status(statuses, "Stonewall Protocol", unit_ext->values[ValueIsSelfReviving], 0);
		add_status(statuses, "Expanded Storage", unit_ext->values[ValueTreasuryStacks], 0); // currently only works for Ovileth and its morphs
		add_status(statuses, "Reconstitution", unit_ext->values[ValueReconstitution], 1);
		add_status(statuses, "Repentent Chorus", unit_ext->values[ValueRepententChorus], 0);
		add_status(statuses, "Stellar Enforcement", unit_ext->values[ValueStellarEnforcement], 1);
		add_status(statuses, "Terminal Surge", unit_ext->values[ValueTerminalSurge], 0);
		add_status(statuses, "Controlled Opposition", unit_ext->values[ValueControlledOpposition], 0);
		add_status(statuses, "Anodyne Annex", unit_ext->values[ValueEmpressPassiveStacks], 0);
		add_status(statuses, "Counter-Proposal", unit_ext->values[ValueAnticthonStacks], 0);
		add_status(statuses, "Fragmentary Shells", unit_ext->timer_vectors[TimerVectorCyclopsFuse].size(), 0);
		add_status(statuses, "Energy Decay", unit_ext->values[ValueEntropyGauntlets], 0);
		add_status(statuses, "Deconstructing", unit_ext->values[ValueDeconstruction], 0);
		add_status(statuses, "Khaydarin Charge", unit_ext->unit_vectors[UnitVectorCrucibles].size(), 0);

		// Gptp Value - Gimmicked
		add_status(statuses, "Tempo Up", unit->id == UnitId::TerranMadrigal && scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange) == 0, 1);
		add_status(statuses, "Tempo Down", scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange) == 1, 1);
		add_status(statuses, "Entropy Gauntlets", unit->id == UnitId::TerranSavant && scbw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit) == 0, 1);
		add_status(statuses, "Power Siphon", scbw::get_generic_rust_value(unit, RustId::ToggleSavantOnhit) == 1, 1);
		add_status(statuses, "Swarming Omen", unit_ext->values[ValueSwarmingOmenStatus] > 0, 1);
		add_status(statuses, "Protective Instincts", unit_ext->values[ValueProtectiveInstinctsStatus] > 0, 1);
		add_status(statuses, "Making Gods from Flesh", unit_ext->values[ValueGodsMadeFleshActive] > 0, 1);
		add_status(statuses, "Blissful Embrace", unit_ext->values[ValueBlissfulEmbraceOriginalPlayer] > 0, 1);
		add_status(statuses, "Kelvin Munitions", unit_ext->values[ValueKelvinMunitionsSlow] > 0, 1);
		add_status(statuses, "Phase Link", scbw::has_clarion_links(unit), 1);

		if (unit_ext->units[UnitMindController])
		{
			switch (unit_ext->units[UnitMindController]->id)
			{
				case UnitId::ProtossMindTyrant:
				{
					add_status(statuses, "Tyranny", 1, 1);
				} break;
				case UnitId::ProtossDemiurge:
				{
					add_status(statuses, "Worshiping", 1, 1);
				} break;
			}
		}

		// Gptp Timers - Gimmicked
		add_status(statuses, "Observing", unit_ext->timers[TimerObservanceOverlay] > 0, 1);
		add_status(statuses, "Dermal Accelerant", unit_ext->timers[TimerAlaszilethRegenBoost] > 0, 1);
		add_status(statuses, "Counter-Proposal Active", unit_ext->timers[TimerAnticthonRevive] > 0, 1);
		add_status(statuses, "Godlike", unit_ext->timers[TimerGodsMadeFlesh] > 0, 1);

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

		u32 parasite_spawn_count = 0;
		for (u32 i = 0; i < PlayerLimit; ++i) {
			parasite_spawn_count += unit_ext->parasite_spawn_count[i];
		}

		// Vanilla Value
		add_status(statuses, "Parasited", parasite_spawn_count, 0);
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

		std::string status = formatStringVector(statuses);

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
		if (units_dat::base_property[unit->id] & UnitProperty::Detector)
		{
			flags.push_back("Detector");
		}
		if (units_dat::base_property[unit->id] & UnitProperty::CreepBuilding
			|| (unit->id == UnitId::ZergBactalisk && unit->status & UnitStatus::Burrowed))
		{
			flags.push_back("Spreads Creep");
		}

		std::string flag = formatStringVector(flags);

		std::string text = "\004" + std::string(armor_upgrade_name) + "\002";
		text += "\n" + std::string(armorStr) + " " + std::to_string(base_armor);

		ss.str("");
		ss << bonus_armor;
		if (bonus_armor > 0) text += BWTC_GREEN "(+" + ss.str() + BWTC_RESET ")";
		else if (bonus_armor < 0) text += BWTC_RED "(" + ss.str() + BWTC_RESET ")";

		// text += "\nSize: " + std::string(size_text);
		text += "\nPlane: " + plane;
		text += "\nMove Speed: " + movementSpeed;
		text += "\nStatus: " + status;

		if (extended)
		{
			text += "\nColor: " + std::string(playerColorNames[player_color_list[unit->player]]); // TODO: move to rank text
			text += "\nFlags: " + flag;
			if (accel != 1)
			{
				text += "\nAcceleration: " + acceleration;
			}
			text += "\nTurn Rate: " + turnRate;
			text += "\nSight Range: " + std::to_string(sight_range);
			if (transport_space != 255)
			{
				text += "\nTransport Space: " + std::to_string(transport_space);
			}
		}
		else
		{
			text += "\n-- Hold \003Shift\002 for more details --";
		}

		strncpy(buffer, text.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = 0;
		return buffer;
	}

	// Returns the C-string for the tooltip text of the plasma shield icon.
	const char* get_shield_tooltip_string(Unit* unit)
	{
		const u16 labelId = upgrades_dat::label[UpgradeId::ProtossPlasmaShields];
		const char* shieldUpgradeName = statTxtTblNew->get_string(labelId);
		const char* shieldStr = statTxtTblNew->get_string(779); // "Shields:"

		u8 shieldUpgradeLevel = scbw::get_upgrade_level(unit->player, UpgradeId::ProtossPlasmaShields);

		std::string text = "\004" + std::string(shieldUpgradeName) + "\002";
		text += "\n" + std::string(shieldStr) + " 0";
		if (shieldUpgradeLevel > 0) text += "+" + std::to_string(shieldUpgradeLevel);

		strncpy(buffer, text.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = 0;
		return buffer;
	}

	// Returns the C-string for the tooltip text of the Interceptor icon (Carriers), Scarab icon (Reavers), Nuclear Missile icon (Nuclear Silos), and Spider Mine icon (Vultures).
	const char* get_special_tooltip_string(u16 iconUnitId, Unit* unit)
	{
		if (iconUnitId == UnitId::ProtossSteward)
		{
			return getDamageTooltipString(WeaponId::StewardPulseCannon, unit, 791);				// "Interceptors"
		}
		if (iconUnitId == UnitId::ProtossPlasmaShell)
		{
			return getDamageTooltipString(WeaponId::Scarab, unit, 792);						// "Scarabs"
		}
		if (iconUnitId == UnitId::TerranNuclearMissile)
		{
			return statTxtTblNew->get_string(793);											// "Nukes"
		}
		if (iconUnitId == UnitId::TerranSpiderMine && unit->id == UnitId::TerranVulture)
		{
			return getDamageTooltipString(WeaponId::SpiderMines, unit, 794);				// "Spider Mines"
		}
		if (iconUnitId == UnitId::TerranSpiderMine && unit->id == UnitId::TerranEidolon)
		{
			return getDamageTooltipString(WeaponId::LobotomyMineExplosion, unit, 794);		// "Lobotomy Mines"
		}

		// Should never reach here
		return "";
	}
}