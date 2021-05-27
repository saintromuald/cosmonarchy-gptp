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

char buffer[256];

// Returns the special damage multiplier factor for units that don't use the
// "Damage Factor" property in weapons.dat.
u8 getDamageFactorForTooltip(u8 weaponId, Unit* unit)
{
	if (weaponId == WeaponId::FlameThrower
		|| weaponId == WeaponId::CohortGaussPistols
		|| weaponId == WeaponId::PsiBlades
		|| weaponId == WeaponId::LegionnairePsiSabers
		|| weaponId == WeaponId::AmaranthPsiCleaver
		|| weaponId == WeaponId::GolemBloodlessAssault
		|| weaponId == WeaponId::PariahIconoclastBlades
		|| weaponId == WeaponId::StarSovereignCelestialCannon
		|| weaponId == WeaponId::LiiraliskHollowedSpores
		|| weaponId == WeaponId::BactaliskCorrosiveAcid
		|| weaponId == WeaponId::AlmaksaliskPiercingSpines
		|| weaponId == WeaponId::KeskathalorScreamingVenom
		)
	{
		return 2;
	}
	return weapons_dat::DamageFactor[weaponId];
}

//Returns the C-string for the tooltip text of the unit's weapon icon.
//This function is used for weapon icons and special icons.
//Precondition: @p entryStrIndex is a stat_txt.tbl string index.
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

const char* getDamageTooltipString(u8 weaponId, Unit* unit, u16 entryStrIndex)
{
	const bool extended = GetKeyState(VK_SHIFT) & 0x8000 || config.always_show_extended_tooltips;

	// Get game strings
	const char* entryName = statTxtTblNew->getString(entryStrIndex);
	const char* damageStr = statTxtTblNew->getString(777);					//"Damage:"
//	const char* perRocketStr = statTxtTblNew->getString(1301);				//"per rocket"
	const char* typeName = "";

	u8 damageFactor = getDamageFactorForTooltip(weaponId, unit);
	u8 upgradeLevel = scbw::getUpgradeLevel(unit->playerId, weapons_dat::DamageUpgrade[weaponId]);
	u8 damageType = weapons_dat::DamageType[weaponId];
	u32 baseDamage = weapons_dat::DamageAmount[weaponId];
	u32 bonusDamage = weapons_dat::DamageBonus[weaponId] * upgradeLevel;

	f32 minRange = weapons_dat::MinRange[weaponId] / 32.0;
	f32 range = weapons_dat::MaxRange[weaponId] / 32.0;
	f32 bonusRange = hooks::getMaxWeaponRangeHook(unit, weaponId) / 32.0 - range;

	std::string damageModifiers = "";
	f64 attackCd = (f64)weapons_dat::Cooldown[weaponId] / 24;
	f64 attackCdModifier = ((f64)hooks::getModifiedWeaponCooldownHook(unit, weaponId) / 24) - attackCd;

	s32 splashInner = weapons_dat::InnerSplashRadius[weaponId];
	s32 splashMedium = weapons_dat::MediumSplashRadius[weaponId];
	s32 splashOuter = weapons_dat::OuterSplashRadius[weaponId];
	bool hasSplash = splashInner || splashMedium || splashOuter;
	s32 splashType = weapons_dat::ExplosionType[weaponId];
	std::string splashTypeValue = "";

	bool isSuicideUnit = unit->id == UnitId::ZergSkryling;

	switch (damageType)
	{
		case DamageType::Concussive:
		{
			// Teal
			typeName = "\017Concussive\002";
			damageModifiers = "\017100\002/\00375\002/\02150\002/\00625\002";
		} break;
		case DamageType::Impact:
		{
			// Yellow
			typeName = "\003Impact\002";
			damageModifiers = "\017100\002/\003100\002/\02175\002/\00650\002";
		} break;
		case DamageType::Explosive:
		{
			// Orange
			typeName = "\021Explosive\002";
			damageModifiers = "\01750\002/\00375\002/\021100\002/\00675\002";
		} break;
		case DamageType::Chronal:
		{
			// White
			typeName = "\004Chronal\002";
			damageModifiers = "\017100\002/\003100\002/\021100\002/\006100\002";
		} break;
		case DamageType::Independent:
		{
			// White
			typeName = "\004Independent\002";
		} break;
		case DamageType::IgnoreArmor:
		{
			// White
			typeName = "\004Ignores Armor\002";
		} break;
		default:
		{
			typeName = "None";
		} break;
	};

	switch (splashType)
	{
		case WeaponEffect::SplashRadial:
		{
			// Red
			splashTypeValue = "\006Unsafe\002";
		} break;
		case WeaponEffect::SplashEnemy:
		{
			// Green
			splashTypeValue = "\007Safe\002";
		} break;
		case WeaponEffect::Unknown_Crash:
		{
			// Green
			splashTypeValue = "\007Ally Safe\002"; 
		} break;
		case WeaponEffect::SplashAir:
		{	// Yellow
			splashTypeValue = "\003Air Only\002";
		} break;
		default:
		{
			// White
			splashTypeValue = "\004Normal Hit\002";
		} break;
	};

	std::vector<std::string> targetFlags;
	if (weapons_dat::TargetFlags[weaponId].ground)
	{
		targetFlags.push_back("Ground");
	}
	if (weapons_dat::TargetFlags[weaponId].air)
	{
		targetFlags.push_back("Air");
	}
	if (weapons_dat::TargetFlags[weaponId].mechanical || weapons_dat::TargetFlags[weaponId].orgOrMech)
	{
		targetFlags.push_back("Mechanical");
	}
	if (weapons_dat::TargetFlags[weaponId].organic || weapons_dat::TargetFlags[weaponId].orgOrMech)
	{
		targetFlags.push_back("Organic");
	}
	if (weapons_dat::TargetFlags[weaponId].nonBuilding)
	{
		targetFlags.push_back("Not Building");
	}
	if (weapons_dat::TargetFlags[weaponId].nonRobotic)
	{
		targetFlags.push_back("Not Robotic");
	}
	if (weapons_dat::TargetFlags[weaponId].terrain)
	{
		targetFlags.push_back("Terrain");
	}

	std::string targetFlagsText = formatStringVector(targetFlags);

	// Name
	std::string text = "\004" + std::string(entryName) + "\002\n";

	// Damage
	if (extended)
	{
		text += std::string(damageStr) + " " + std::to_string(damageFactor) + "x(" + std::to_string(baseDamage);
		if (bonusDamage > 0) text += "+" + std::to_string(bonusDamage);
		text += ")";
	}
	else
	{
		text += std::string(damageStr) + " " + std::to_string(damageFactor * baseDamage);
		if (bonusDamage > 0) text += "+" + std::to_string(damageFactor * bonusDamage);
	}

	/*
	if (!extended && weaponId == WeaponId::HaloRockets)
	{
		text += " " + std::string(perRocketStr);
	}
	*/

	// Cooldown
	std::string modifierColor = attackCdModifier > 0 ? "\006" : attackCdModifier < 0 ? "\007" : "\002";
	std::stringstream ss;
	ss << std::setprecision(4);
	if (!isSuicideUnit)
	{
		if (extended) ss << attackCd;
		else ss << (attackCd + attackCdModifier);
		text += "\nCooldown: ";
		text += modifierColor;
		text += ss.str() + "s";
		if (extended && attackCdModifier != 0)
		{
			ss.str("");
			ss << abs(attackCdModifier);

			if (attackCdModifier > 0) text += " + ";
			else text += " - ";
			text += modifierColor + ss.str() + "s";
		}
		text += "\002";
	}

	// DPS
	if (!isSuicideUnit && extended)
	{
		ss.str("");
		f64 cd = (attackCd + attackCdModifier);
		ss << ((f64)damageFactor * (baseDamage + bonusDamage) / cd);
		text += "\nDPS: " + modifierColor + ss.str() + "\002";
	}

	// Range
	text += "\nRange: ";
	if (range > 0)
	{
		ss.str("");
		if (minRange > 0)
		{
			ss << minRange << "-";
		}
		ss << range;
		if (bonusRange > 0)
		{
			ss << "+" << bonusRange;
		}
		text += ss.str();
	}
	else
	{
		text += "Melee";
	}

	// Target
	text += "\nTargets: " + targetFlagsText;

	// Type
	text += "\nType: " + std::string(typeName);
	if (extended && damageModifiers != "")
	{
		text += " (" + damageModifiers + ")";
	}

	// Splash
	if (hasSplash)
	{
		text += "\nSplash Radius: " + std::to_string(splashInner) + "/" + std::to_string(splashMedium) + "/" + std::to_string(splashOuter);
		text += "\nSplash Type: " + splashTypeValue;
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
	const char* getWeaponTooltipString(u8 weaponId, Unit* unit)
	{
		return getDamageTooltipString(weaponId, unit, weapons_dat::Label[weaponId]);
	}

	static std::string timerToString(s32 timer)
	{
		std::stringstream ss;
		ss << round(timer * 0.042 * 8.0) << "s";
		return ss.str();
	}

	static std::string aiseTimerToString(s32 timer)
	{
		std::stringstream ss;
		ss << round((timer - 1) / 24.0) << "s";
		return ss.str();
	}

	static std::string removeTimerToString(s32 timer)
	{
		std::stringstream ss;
		ss << round(timer / 24.0) << "s";
		return ss.str();
	}

	enum struct TimerType
	{
		Vanilla = 0,
		Aise,
		RemoveTimer
	};

	// maxValue						value format
	// 0  - unlimited stacks		x
	// 1  - binary true / false
	// >1 - limited stacks			x/max
	static void addStatus(std::vector<std::string>& vec, std::string name, s32 value = -1, s32 maxValue = 0, s32 timer = -1, TimerType timerType = TimerType::Vanilla)
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
					else if (timerType == TimerType::Aise)
					{
						text += aiseTimerToString(timer);
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

	const char* getArmorTooltipString(Unit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		const bool extended = GetKeyState(VK_SHIFT) & 0x8000 || config.always_show_extended_tooltips;

		const u16 labelId = upgrades_dat::Label[units_dat::ArmorUpgrade[unit->id]];
		const char* armorUpgradeName = statTxtTblNew->getString(labelId);
		const char* armorStr = statTxtTblNew->getString(778); // "Armor:"

		const u8 baseArmor = units_dat::ArmorAmount[unit->id];
		const u8 unitSize = units_dat::SizeType[unit->id];
		const char* sizeLabel = "Size:";
		const char* sizeText = "";
		f32 bonusArmor = unit->getArmorBonus();

		bonusArmor = std::min(baseArmor - (s32)unitExt->values[ValueControlledOpposition], 0);
		bonusArmor -= unitExt->values[ValueArmorReduce];
		for (Unit* unit : unitExt->unitVectors[UnitVectorAttachedSource])
		{
			if (unit->id == UnitId::ZergSovroleth)
			{
				bonusArmor += 0.5;
			}
		}

		const u8 transportSpace = units_dat::SpaceRequired[unit->id];
		const s32 sightRange = units_dat::SightRange[unit->id];

		switch (unitSize)
		{
			case 1:
			{
				// Teal
				sizeText = "\017Light\002";
			} break;
			case 2:
			{
				// Yellow
				sizeText = "\003Medium\002";
			} break;
			case 3:
			{
				// Orange
				sizeText = "\021Heavy\002";
			} break;
			case 4:
			{
				// Red
				sizeText = "\006Titanic\002";
			} break;
			default:
			{
				// White
				sizeText = "\004Unknown\002";
			} break;
		}

		std::stringstream ss;
		ss << std::setprecision(4);

		ss << unit->flingyTopSpeed / 32 / 15 * 24;
		std::string movementSpeed = ss.str();
		ss.str("");

		s32 accel = hooks::getModifiedUnitAccelerationHook(unit);
		ss << accel;
		std::string acceleration = ss.str();
		ss.str("");

		ss << hooks::getModifiedUnitTurnSpeedHook(unit);
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

		// Aise Timer + Value
		addStatus(statuses, "Knockout Drivers", unitExt->values[ValueKnockoutDriversStacks], 20,
			unitExt->timers[TimerKnockoutDrivers], TimerType::Aise);
		addStatus(statuses, "Adrenal Glands", unitExt->values[ValueAdrenalStacks], 4,
			unitExt->timers[TimerAdrenal], TimerType::Aise);
		addStatus(statuses, "Astral Blessing", unitExt->values[ValueEcclesiastStack], 0,
			unitExt->timers[TimerEcclesiast], TimerType::Aise);
		addStatus(statuses, "Captivating Claws", captivatingClawsGetStacks(unit), 4,
			captivatingClawsGetTimer(unit), TimerType::Aise);
		addStatus(statuses, "Swarming Omen Aura", unitExt->values[ValueSwarmingOmenAbility], 1,
			unitExt->timers[TimerSwarmingOmen], TimerType::Aise);
		addStatus(statuses, "Nemean Reactors", unitExt->values[ValueNemeanReactorsStack], 5,
			unitExt->timers[TimerNemeanReactors], TimerType::Aise);
		addStatus(statuses, "Observance", unitExt->values[ValueObservanceRend], 0,
			unitExt->timers[TimerObservanceDebuff], TimerType::Aise);
		addStatus(statuses, "Corrosive Spores", unitExt->values[ValueCorrosiveTouchStacks], 0,
			unitExt->timers[TimerCorrosiveTouch], TimerType::Aise);
		addStatus(statuses, "Dirge Fuse", unitExt->values[ValueDirgeStacks] * MadrigalDirgeDamage, 0,
			unitExt->timers[TimerDirgeFuse], TimerType::Aise);
		addStatus(statuses, "Adrenal Frenzy", unitExt->values[ValueNathrokorBuffStacks], 0,
			unitExt->timers[TimerNathrokorBuff], TimerType::Aise);

		if (unitExt->values[ValueSignifyStacks] < 5)
		{
			addStatus(statuses, "Cowardice", unitExt->values[ValueSignifyStacks], 5,
				unitExt->timers[TimerSignify], TimerType::Aise);
		}
		else
		{
			addStatus(statuses, "Malice", unitExt->values[ValueSignifyStacks], 1,
				unitExt->timers[TimerSignify], TimerType::Aise);
		}

		// Aise Timer
		addStatus(statuses, "Malediction", -1, 1,
			unitExt->timers[TimerMaledictionSlow], TimerType::Aise);
		addStatus(statuses, "Ensnaring Brood", -1, 1,
			unitExt->timers[TimerEnsnaringBrood], TimerType::Aise);
		addStatus(statuses, "Lobotomy Slow", -1, 1,
			unitExt->timers[TimerLobotomySlow], TimerType::Aise);
		addStatus(statuses, "Kaiser Rampage", -1, 1,
			unitExt->timers[TimerKaiserRampage], TimerType::Aise);
		addStatus(statuses, "Hallucinating", -1, 1,
			unitExt->timers[TimerHallucinating], TimerType::Aise);
		//addStatus(statuses, "Ectomorphic Accelerant", 0, -1,
		//	unitExt->EggBoostTimer, TimerType::Aise)); // currently can't read tooltips from  eggs
		//addStatus(statuses, "Endomorphic Accelerant", 0, -1,
		//	unitExt->MineralSpecialRegenTimer, TimerType::Aise)); // nor mineral fields
		addStatus(statuses, "Controlled Demolition", -1, 1,
			unitExt->timers[TimerControlledDemolition], TimerType::Aise);
		addStatus(statuses, "Emergency Detonation", -1, 1,
			unitExt->timers[TimerEmergencyDetonation], TimerType::Aise);
		addStatus(statuses, "Binding Grip", -1, 1,
			unitExt->timers[TimerBindingGrip], TimerType::Aise);
		addStatus(statuses, "Autovitality", -1, 1,
			unitExt->timers[TimerSimulacrumClone], TimerType::Aise);
		addStatus(statuses, "Caustic Discharge", -1, 1,
			unitExt->timers[TimerCausticDischargeSpeed], TimerType::Aise);
		addStatus(statuses, "Ion Field", -1, 1,
			unitExt->timers[TimerIonFieldSpeed], TimerType::Aise);

		// Aise Value
		addStatus(statuses, "Armor Reduction", unitExt->values[ValueArmorReduce], 0);
		addStatus(statuses, "Cabeirus Feeders", ceil((f64)unitExt->values[ValueMadcapStacks] / MadcapPassiveDecayTime), 10);
		addStatus(statuses, "Sublime Shepherd", unitExt->values[ValueSublimeShepherd], 0);
		addStatus(statuses, "Lazarus Agent", unitExt->values[ValueLazarusAgent], 0);
		addStatus(statuses, "Expanded Storage", unitExt->values[ValueTreasuryStacks], 0); // currently only works for Ovileth and its morphs
		addStatus(statuses, "Reconstitution", unitExt->values[ValueReconstitution], 1);
		addStatus(statuses, "Repentent Chorus", unitExt->values[ValueRepententChorus], 0);
		addStatus(statuses, "Stellar Enforcement", unitExt->values[ValueStellarEnforcement], 1);
		addStatus(statuses, "Terminal Surge", unitExt->values[ValueTerminalSurge], 0);
		addStatus(statuses, "Tertiary Eyes [Caster]", unitExt->unitVectors[UnitVectorTertiaryEyes].size(), 0);
		addStatus(statuses, "Tertiary Eye [Linked]", unitExt->unitVectors[UnitVectorTertiaryEyesSirens].size(), 0);
		addStatus(statuses, "Controlled Opposition", unitExt->values[ValueControlledOpposition], 0);
		addStatus(statuses, "Anodyne Annex", unitExt->values[ValueEmpressPassiveStacks], 0);

		// Aise Value - Gimmicked
		addStatus(statuses, "Tempo Up", unit->id == UnitId::TerranMadrigal && scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange) == 0, 1);
		addStatus(statuses, "Tempo Down", scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange) == 1, 1);
		addStatus(statuses, "Observing", unitExt->timers[TimerObservanceOverlay] > 0, 1);
		addStatus(statuses, "Swarming Omen", unitExt->values[ValueSwarmingOmenStatus] > 0, 1);
		addStatus(statuses, "Protective Instincts", unitExt->values[ValueProtectiveInstinctsStatus] > 0, 1);
		addStatus(statuses, "Deconstructing", unitExt->values[ValueDeconstruction] > 0, 1);
		addStatus(statuses, "Worshiping", unitExt->values[ValueWorshipStack] > 0, 1);
		addStatus(statuses, "Making Gods from Flesh", unitExt->values[ValueGodsMadeFleshActive] > 0, 1);
		addStatus(statuses, "Adaptive Amitosis", unitExt->values[ValueAdaptiveAmitosis] > 0, 1);
		addStatus(statuses, "Cutaneous Barrier", unitExt->values[ValueCutaneousBarrier] > 0, 1);
		addStatus(statuses, "Infested", unitExt->values[ValueInfestedByAkistrokor] > 0, 1);
		addStatus(statuses, "Enthralling Assault", unitExt->values[ValueEnthrallingTalons] > 0, 1);
		addStatus(statuses, "Kelvin Munitions", unitExt->values[ValueKelvinMunitionsSlow] > 0, 1);
		addStatus(statuses, "Dermal Accelerant", unitExt->timers[TimerAlaszilethRegenBoost] > 0, 1);

		addStatus(statuses, "Phase Link", scbw::hasClarionLinks(unit), 1);
		addStatus(statuses, "Mind Controlling", scbw::get_aise_value(unit, nullptr, AiseId::SumMindControlCost, 0, 0) > 0, 1); // for caster
		addStatus(statuses, "Energy Decay", unitExt->timers[TimerEntropyGauntlets] > 0, 1);
		addStatus(statuses, "Godlike", unitExt->timers[TimerGodsMadeFlesh] > 0, 1);

		// Vanilla Timer + Value
		addStatus(statuses, "Defensive Matrix", unit->defensiveMatrixHp / 256, 200, unit->defensiveMatrixTimer);

		// Vanilla Timers
		addStatus(statuses, "Stimpack", -1, 1, unit->stimTimer);
		addStatus(statuses, "Irradiated", -1, 1, unit->irradiateTimer);
		addStatus(statuses, "Ensnared", -1, 1, unit->ensnareTimer);
		addStatus(statuses, "Lobotomized", -1, 1, unit->lockdownTimer);
		addStatus(statuses, "Maelstromed", -1, 1, unit->maelstromTimer);
		addStatus(statuses, "Plagued", -1, 1, unit->plagueTimer);
		addStatus(statuses, "Stasis", -1, 1, unit->stasisTimer);
		addStatus(statuses, "Timed Life", -1, 1, unit->removeTimer, TimerType::RemoveTimer);

		// Vanilla Value
		addStatus(statuses, "Parasited", unit->parasiteFlags != 0, 1);
		addStatus(statuses, "Blinded", unit->isBlind != 0, 1);
		addStatus(statuses, "Lifted Off", unit->status & UnitStatus::InAir && units_dat::BaseProperty[unit->id] & UnitProperty::Building, 1);
		addStatus(statuses, "Cannot Attack", unitExt->timers[TimerUnderDisruptionWeb] > 0, 1);

		// Case Specific
		addStatus(statuses, "Sieged", unit->id == UnitId::TerranPhalanxSiegeMode, 1);
		if (scbw::isUnderDarkSwarm(unit))
		{
			Unit* darkSwarm = scbw::getDarkSwarm(unit);
			addStatus(statuses, "Under Dark Swarm", darkSwarm->hitPoints / 256, 1, darkSwarm->removeTimer, TimerType::RemoveTimer);
		}
		u32 nathrokorTargetStacks = 0;
		for (int i = 0; i < 8; i++)
		{
			nathrokorTargetStacks += unitExt->nathrokorTargetStacks[i];
		}
		addStatus(statuses, "Adrenal Mark", nathrokorTargetStacks, 0);

		std::string status = formatStringVector(statuses);

		// TODO Statuses
		//	make sublime shepard show on targets not only caster
		//	mind control on target, last orders
		//	color code source player if possible

		std::vector<std::string> flags;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Building)
		{
			flags.push_back("Structure");
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::ProducesUnits)
		{
			flags.push_back("Factory");
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Addon)
		{
			flags.push_back("Addon");
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
		{
			flags.push_back("Worker");
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Hero)
		{
			flags.push_back("Hero");
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Invincible)
		{
			flags.push_back("Invincible");
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Organic)
		{
			flags.push_back("Organic");
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical)
		{
			flags.push_back("Mechanical");
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::RoboticUnit)
		{
			flags.push_back("Robotic");
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Detector)
		{
			flags.push_back("Detector");
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::CreepBuilding
			|| (unit->id == UnitId::ZergBactalisk && unit->status & UnitStatus::Burrowed))
		{
			flags.push_back("Spreads Creep");
		}

		std::string flag = formatStringVector(flags);

		std::string text = "\004" + std::string(armorUpgradeName) + "\002";
		text += "\n" + std::string(armorStr) + " " + std::to_string(baseArmor);
		ss.str("");
		ss << bonusArmor;
		if (bonusArmor > 0) text += "+" + ss.str();
		else if (bonusArmor < 0) text += ss.str();

		text += "\nSize: " + std::string(sizeText);
		text += "\nPlane: " + plane;
		text += "\nMove Speed: " + movementSpeed;
		text += "\nStatus: " + status;

		if (extended)
		{
			text += "\nFlags: " + flag;
		}
		if (extended && accel != 1)
		{
			text += "\nAcceleration: " + acceleration;
		}
		if (extended)
		{
			text += "\nTurn Rate: " + turnRate;
		}
		if (extended)
		{
			text += "\nSight Range: " + std::to_string(sightRange);
		}
		if (extended && transportSpace != 255)
		{
			text += "\nTransport Space: " + std::to_string(transportSpace);
		}

		if (!extended)
		{
			text += "\n-- Hold \003Shift\002 for more details --";
		}

		strncpy(buffer, text.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = 0;
		return buffer;
	}

	// Returns the C-string for the tooltip text of the plasma shield icon.
	const char* getShieldTooltipString(Unit* unit)
	{
		const u16 labelId = upgrades_dat::Label[UpgradeId::ProtossPlasmaShields];
		const char* shieldUpgradeName = statTxtTblNew->getString(labelId);
		const char* shieldStr = statTxtTblNew->getString(779); // "Shields:"

		u8 shieldUpgradeLevel = scbw::getUpgradeLevel(unit->playerId, UpgradeId::ProtossPlasmaShields);

		std::string text = "\004" + std::string(shieldUpgradeName) + "\002";
		text += "\n" + std::string(shieldStr) + " 0";
		if (shieldUpgradeLevel > 0) text += "+" + std::to_string(shieldUpgradeLevel);

		strncpy(buffer, text.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = 0;
		return buffer;
	}

	// Returns the C-string for the tooltip text of the Interceptor icon (Carriers),
	// Scarab icon (Reavers), Nuclear Missile icon (Nuclear Silos), and Spider Mine
	// icon (Vultures).
	const char* getSpecialTooltipString(u16 iconUnitId, Unit* unit)
	{
		if (iconUnitId == UnitId::ProtossSteward)
		{
			return getDamageTooltipString(WeaponId::PulseCannon, unit, 791);				// "Interceptors"
		}
		if (iconUnitId == UnitId::ProtossPlasmaShell)
		{
			return getDamageTooltipString(WeaponId::Scarab, unit, 792);						// "Scarabs"
		}
		if (iconUnitId == UnitId::TerranNuclearMissile)
		{
			return statTxtTblNew->getString(793);											// "Nukes"
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
