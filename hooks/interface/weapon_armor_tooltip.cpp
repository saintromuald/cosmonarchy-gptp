#include "weapon_armor_tooltip.h"
#include <SCBW/api.h>
#include <SCBW/enumerations/WeaponId.h>
#include <cstdio>
#include "../unit_speed.h"
#include "../weapons/weapon_cooldown.h"
#include "../config.h"
#include <Windows.h>


char buffer[256];

//Returns the special damage multiplier factor for units that don't use the
//"Damage Factor" property in weapons.dat.
u8 getDamageFactorForTooltip(u8 weaponId, CUnit* unit) {

	//Default StarCraft behavior
	if (unit->id == UnitId::TerranHarakan
	||	unit->id == UnitId::ProtossZealot
	||	unit->id == UnitId::ProtossLegionnaire
	||	unit->id == UnitId::ProtossAmaranth
	||	unit->id == UnitId::ProtossGolem
	||	unit->id == UnitId::ProtossPariah
	||	unit->id == UnitId::ZergBactalisk
	||	unit->id == UnitId::ZergAlmaksalisk
	||	weaponId == WeaponId::CelestialCannon
		)
		return 2;

	if (unit->id == UnitId::TerranValkyrie)
		return 1;

	return weapons_dat::DamageFactor[weaponId];
}

//Returns the C-string for the tooltip text of the unit's weapon icon.
//This function is used for weapon icons and special icons.
//Precondition: @p entryStrIndex is a stat_txt.tbl string index.
#include "SCBW/stat_txt_spec.h"
#include <iomanip>

static std::string formatStringVector(std::vector<std::string> strings) {
	std::string result = "";
	if (strings.size() == 0)
		strings.push_back("None");
	int i = 0;
	for (const std::string& str : strings) {
		result += str;
		if (++i != strings.size()) result += ", ";
	}
	return result;
}

const char* getDamageTooltipString(u8 weaponId, CUnit* unit, u16 entryStrIndex) {
	const bool extended = GetKeyState(VK_SHIFT) & 0x8000 || config.always_show_extended_tooltips;

	// Get game strings
	const char* entryName = statTxtTblNew->getString(entryStrIndex);
	const char* damageStr = statTxtTblNew->getString(777);					//"Damage:"
	const char* perRocketStr = statTxtTblNew->getString(1301);				//"per rocket"
	const char* typeName = "";

	u8 damageFactor = getDamageFactorForTooltip(weaponId, unit);
	u8 upgradeLevel = scbw::getUpgradeLevel(unit->playerId, weapons_dat::DamageUpgrade[weaponId]);
	u16 baseDamage = weapons_dat::DamageAmount[weaponId];
	u16 bonusDamage = weapons_dat::DamageBonus[weaponId] * upgradeLevel;
	u8 damageType = weapons_dat::DamageType[weaponId];

	u32 minRange = weapons_dat::MinRange[weaponId] / 32;
	u32 range = weapons_dat::MaxRange[weaponId] / 32;
	u32 bonusRange = 0;

	std::string damageModifiers = "";
	double attackCd = (double)weapons_dat::Cooldown[weaponId] / 24;
	double attackCdModifier = ((double)hooks::getModifiedWeaponCooldownHook(unit, weaponId) / 24) - attackCd;

	int splashInner = weapons_dat::InnerSplashRadius[weaponId];
	int splashMedium = weapons_dat::MediumSplashRadius[weaponId];
	int splashOuter = weapons_dat::OuterSplashRadius[weaponId];
	bool hasSplash = splashInner || splashMedium || splashOuter;
	int splashType = weapons_dat::ExplosionType[weaponId];
	std::string splashTypeValue = "";

	bool isSuicideUnit = unit->id == UnitId::ZergInfestedTerran ||
						 unit->id == UnitId::ZergSkryling;

	if (scbw::get_generic_value(unit, ValueId::RoboticSapience)) {
		u32 stacks = scbw::get_generic_value(unit, ValueId::RoboticSapience);
		if (stacks >= 10) {
			bonusRange += 5;
		}
		else {
			bonusRange += stacks * 0.5;
		}
	}
	switch (weaponId) {

	case UnitId::ZergBactalisk:	{
		ActiveTile tile = scbw::getActiveTileAt(unit->position.x, unit->position.y);
		if (tile.hasCreep || tile.creepReceeding) {
			bonusRange += 2;
		}
		break;
	}
	default:
		bonusRange += 0;
		break;
	}

	switch (damageType) {
	case DamageType::Normal:
		// Yellow
		typeName = "\003Normal\002";
		break;
	case DamageType::Explosive:
		// Orange
		typeName = "\021Explosive\002";
		damageModifiers = "\01750\002/\00375\002/\021100\002";
		break;
	case DamageType::Concussive:
		// Teal
		typeName = "\017Concussive\002";
		damageModifiers = "\017100\002/\00375\002/\02150\002";
		break;
	case DamageType::Independent:
		// White
		typeName = "\004Independent\002";
		break;
	case DamageType::IgnoreArmor:
		// White
		typeName = "\004Ignores Armor\002";
		break;
	default:
		typeName = "None";
		break;
	};

	switch (splashType) {
	case WeaponEffect::SplashRadial:
		// Red
		splashTypeValue = "\006Unsafe\002";
		break;
	case WeaponEffect::SplashEnemy:
		// Green
		splashTypeValue = "\007Safe\002";
		break;
	case WeaponEffect::Unknown_Crash:
		// Green
		splashTypeValue = "\007Ally Safe\002";
		break;
	case WeaponEffect::SplashAir:
		// Yellow
		splashTypeValue = "\003Air Only\002";
		break;
	default:
		// White
		splashTypeValue = "\004Normal Hit\002";
		break;
	};

	std::vector<std::string> targetFlags;
	if (weapons_dat::TargetFlags[weaponId].ground)
		targetFlags.push_back("Ground");
	if (weapons_dat::TargetFlags[weaponId].air)
		targetFlags.push_back("Air");
	if (weapons_dat::TargetFlags[weaponId].mechanical || weapons_dat::TargetFlags[weaponId].orgOrMech)
		targetFlags.push_back("Mechanical");
	if (weapons_dat::TargetFlags[weaponId].organic || weapons_dat::TargetFlags[weaponId].orgOrMech)
		targetFlags.push_back("Organic");
	if (weapons_dat::TargetFlags[weaponId].nonBuilding)
		targetFlags.push_back("Not Building");
	if (weapons_dat::TargetFlags[weaponId].nonRobotic)
		targetFlags.push_back("Not Robotic");
	if (weapons_dat::TargetFlags[weaponId].terrain)
		targetFlags.push_back("Terrain");

	std::string targetFlagsText = formatStringVector(targetFlags);

	// Name
	std::string text = "\004" + std::string(entryName) + "\002\n";

	// Damage
	if (extended) {
		text += std::string(damageStr) + " " + std::to_string(damageFactor) + "x(" + std::to_string(baseDamage);
		if (bonusDamage > 0) text += "+" + std::to_string(bonusDamage);
		text += ")";
	}
	else {
		text += std::string(damageStr) + " " + std::to_string(damageFactor * baseDamage);
		if (bonusDamage > 0) text += "+" + std::to_string(damageFactor * bonusDamage);
	}
	
	if (!extended && weaponId == WeaponId::HaloRockets) text += " " + std::string(perRocketStr);

	// Cooldown
	std::string modifierColor = attackCdModifier > 0 ? "\006" : attackCdModifier < 0 ? "\005" : "\002";
	std::stringstream ss;
	ss << std::setprecision(4);
	if (!isSuicideUnit) {
		if (extended) ss << attackCd;
		else ss << (attackCd + attackCdModifier);
		text += "\nCooldown: ";
		text += modifierColor;
		text += ss.str() + "s";
		if (extended && attackCdModifier != 0) {
			ss.str("");
			ss << abs(attackCdModifier);

			if (attackCdModifier > 0) text += " + ";
			else text += " - ";
			text += modifierColor + ss.str() + "s";
		}
		text += "\002";
	}

	// DPS
	if (!isSuicideUnit && extended) {
		ss.str("");
		double cd = (attackCd + attackCdModifier);
		ss << ((double)damageFactor * (baseDamage + bonusDamage) / cd);
		text += "\nDPS: " + modifierColor + ss.str() + "\002";
	}

	// Range
	text += "\nRange: ";
	if (range > 0) {
		if (minRange > 0) {
			text += std::to_string(minRange) + "-";
		}
		text += std::to_string(range);
		if (bonusRange > 0) text += "+" + std::to_string(bonusRange);
	}
	else text += "Melee";

	// Target
	text += "\nTargets: " + targetFlagsText;

	// Type
	text += "\nType: " + std::string(typeName);
	if (extended && damageModifiers != "") {
		text += " (" + damageModifiers + ")";
	}

	// Splash
	if (hasSplash) {
		text += "\nSplash Radius: " + std::to_string(splashInner) + "/" + std::to_string(splashMedium) + "/" + std::to_string(splashOuter);
		text += "\nSplash Type: " + splashTypeValue;
	}

	if (!extended) {
		text += "\n-- Hold \003Shift\002 for more details --";
	}

	strncpy(buffer, text.c_str(), sizeof(buffer));
	buffer[sizeof(buffer) - 1] = 0;
	return buffer;
}

namespace hooks {

	//Returns the C-string for the tooltip text of the unit's weapon icon.
	const char* getWeaponTooltipString(u8 weaponId, CUnit* unit) {
		return getDamageTooltipString(weaponId, unit, weapons_dat::Label[weaponId]);
	}

	static std::string timerToString(int timer) {
		std::stringstream ss;
		ss << floor((timer * 0.042) * 8) << "s";
		return ss.str();
	}

	static std::string aisetimerToString(int timer) {
		std::stringstream ss;
		ss << ceil((timer - 1) / 24) << "s";
		return ss.str();
	}

	static std::string removetimerToString(int timer) {
		std::stringstream ss;
		ss << ceil(timer / 24) << "s";
		return ss.str();
	}

	enum class TimerType {
		Vanilla = 0,
		Aise,
		RemoveTimer
	};

	static void addStatus(std::vector<std::string>& vec, std::string name, int value = -1, int timer = -1, TimerType timerType = TimerType::Vanilla) {
		if (timer > 0 || value > 0) {
			std::string text = name;
			if (timer > 0 || value > 1) {
				text += " (";
				if (timer > 0) {
					if (timerType == TimerType::Vanilla) text += timerToString(timer);
					else if (timerType == TimerType::Aise) text += aisetimerToString(timer);
					else text += removetimerToString(timer);
					if (value > 1) {
						text += ", ";
					}
				}
				if (value > 1) {
					text += std::to_string(value);
				}
				text += ")";
			}
			vec.push_back(text);
		}
	}

	const char* getArmorTooltipString(CUnit* unit) {
		const bool extended = GetKeyState(VK_SHIFT) & 0x8000 || config.always_show_extended_tooltips;

		const u16 labelId = upgrades_dat::Label[units_dat::ArmorUpgrade[unit->id]];
		const char* armorUpgradeName = statTxtTblNew->getString(labelId);
		const char* armorStr = statTxtTblNew->getString(778);						//"Armor:"

		const u8 baseArmor = units_dat::ArmorAmount[unit->id];
		const u8 bonusArmor = unit->getArmorBonus();
		const u8 unitSize = units_dat::SizeType[unit->id];
		const char* sizeLabel = "Size:";
		const char* sizeText = "";

		const u8 transportSpace = units_dat::SpaceRequired[unit->id];

		const int sightRange = units_dat::SightRange[unit->id];

		switch (unitSize) {
		case 1:
			// Teal
			sizeText = "\017Small\002";
			break;
		case 2:
			// Yellow
			sizeText = "\003Medium\002";
			break;
		case 3:
			// Orange
			sizeText = "\021Large\002";
			break;
		default:
			// White
			sizeText = "\004Unknown\002";
			break;
		}

		std::stringstream ss;
		ss << std::setprecision(4);

		ss << unit->flingyTopSpeed / 32 / 15 * 24;
		std::string movementSpeed = ss.str();
		ss.str("");

		int accel = hooks::getModifiedUnitAccelerationHook(unit);
		ss << accel;
		std::string acceleration = ss.str();
		ss.str("");

		ss << hooks::getModifiedUnitTurnSpeedHook(unit);
		std::string turnRate = ss.str();
		ss.str("");

		std::string plane = unit->status & UnitStatus::InAir ? "Air" : "Ground";

		std::vector<std::string> statuses;
		if (unit->status & UnitStatus::Burrowed)
			statuses.push_back("Burrowed");
		else if (unit->status & UnitStatus::Cloaked)
			statuses.push_back("Cloaked");

		// Aise Timer + Value
		addStatus(statuses, "Knockout Drivers", scbw::get_generic_value(unit, ValueId::KnockoutDriversStacks), scbw::get_generic_timer(unit, ValueId::KnockoutDriversTimer), TimerType::Aise);
		addStatus(statuses, "Adrenal Glands", scbw::get_generic_value(unit, ValueId::AdrenalStacks), scbw::get_generic_timer(unit, ValueId::AdrenalTimer), TimerType::Aise);
		addStatus(statuses, "Astral Blessing", scbw::get_generic_value(unit, ValueId::EcclesiastStack), scbw::get_generic_timer(unit, ValueId::EcclesiastTimer), TimerType::Aise);
		addStatus(statuses, "Captivating Claws", scbw::get_generic_value(unit, ValueId::CaptivatingClawsStacks), scbw::get_generic_timer(unit, ValueId::CaptivatingClawsTimer), TimerType::Aise);
		addStatus(statuses, "Swarming Omen Aura", scbw::get_generic_value(unit, ValueId::SwarmingOmenAbility), scbw::get_generic_timer(unit, ValueId::SwarmingOmenTimer), TimerType::Aise);
		addStatus(statuses, "Nemean Reactors", scbw::get_generic_value(unit, ValueId::NemeanReactorsStack), scbw::get_generic_timer(unit, ValueId::NemeanReactorsTimer), TimerType::Aise);

		// Aise Timer
		addStatus(statuses, "Malediction", -1, scbw::get_generic_timer(unit, ValueId::MaledictionSlow), TimerType::Aise);
		addStatus(statuses, "Ensnaring Brood", -1, scbw::get_generic_timer(unit, ValueId::EnsnaringBroodTimer), TimerType::Aise);
		addStatus(statuses, "Lobotomy Slow", -1, scbw::get_generic_timer(unit, ValueId::LobotomySlow), TimerType::Aise);
		addStatus(statuses, "Observance", -1, scbw::get_generic_timer(unit, ValueId::ObservanceTimerDebuff), TimerType::Aise);
		addStatus(statuses, "Kaiser Rampage", -1, scbw::get_generic_timer(unit, ValueId::KaiserRampage), TimerType::Aise);
		addStatus(statuses, "Hallucinating", -1, scbw::get_generic_timer(unit, ValueId::HallucinatingTimer), TimerType::Aise);
//		addStatus(statuses, "Ectomorphic Accelerant", -1, scbw::get_generic_timer(unit, ValueId::EggBoostTimer), TimerType::Aise); // currently can't read tooltips from eggs
//		addStatus(statuses, "Endomorphic Accelerant", -1, scbw::get_generic_timer(unit, ValueId::MineralSpecialRegenTimer), TimerType::Aise); // nor mineral fields
		addStatus(statuses, "Adrenaline Packs", -1, scbw::get_generic_timer(unit, ValueId::Adrenaline), TimerType::Aise);
		addStatus(statuses, "Adrenal Frenzy", -1, scbw::get_generic_timer(unit, ValueId::NathrokorTimer), TimerType::Aise);
		addStatus(statuses, "Controlled Demolition", -1, scbw::get_generic_timer(unit, ValueId::ControlledDemolitionTimer), TimerType::Aise);
		addStatus(statuses, "Emergency Detonation", -1, scbw::get_generic_timer(unit, ValueId::EmergencyDetonationTimer), TimerType::Aise);
		addStatus(statuses, "Binding Grip", -1, scbw::get_generic_timer(unit, ValueId::BindingGripTimer), TimerType::Aise);

		// Aise Value
		addStatus(statuses, "Energy Decay", scbw::get_generic_value(unit, ValueId::EntropyGauntlets));
		addStatus(statuses, "Armor Reduction", scbw::get_generic_value(unit, ValueId::ArmorReduce));
		addStatus(statuses, "Cabeirus Feeders", ceil((double)scbw::get_generic_value(unit, ValueId::MadcapStacks) / MadcapPassiveDecayTime));
		addStatus(statuses, "Reverse Thrust", scbw::get_generic_value(unit, ValueId::ReverseThrust));
		addStatus(statuses, "Sublime Shepherd", scbw::get_generic_value(unit, ValueId::SublimeShepherd));
		addStatus(statuses, "Lazarus Agent", scbw::get_generic_value(unit, ValueId::LazarusAgent));
		addStatus(statuses, "Expanded Storage", scbw::get_generic_value(unit, ValueId::TreasuryStacks)); // currently only works for Ovileth and its morphs
		addStatus(statuses, "Khaydarian Eclipse", scbw::get_generic_value(unit, ValueId::ExemplarSwitch));
		addStatus(statuses, "Robotic Sapience", scbw::get_generic_value(unit, ValueId::RoboticSapience));
		addStatus(statuses, "Reconstitution", scbw::get_generic_value(unit, ValueId::Reconstitution));
		addStatus(statuses, "Furia Salvo", unit->id == UnitId::TerranMadrigal && scbw::get_generic_value(unit, ValueId::TempoChange) == 0);
		addStatus(statuses, "Dirge", scbw::get_generic_value(unit, ValueId::TempoChange) == 1);
		addStatus(statuses, "Swarming Omen", scbw::get_generic_timer(unit, ValueId::SwarmingOmenStatus) > 0);
		addStatus(statuses, "Protective Instincts", scbw::get_generic_timer(unit, ValueId::ProtectiveInstinctsStatus) > 0);
		addStatus(statuses, "Deconstructing", scbw::get_generic_value(unit, ValueId::Deconstruction) > 0);
		addStatus(statuses, "Worshiping", scbw::get_generic_value(unit, ValueId::WorshipStack) > 0);
		addStatus(statuses, "Godlike", scbw::get_generic_value(unit, ValueId::GodsMadeFlesh) > 0);
		addStatus(statuses, "Adaptive Amitosis", scbw::get_generic_value(unit, ValueId::AdaptiveAmitosis) > 0);
		addStatus(statuses, "Cutaneous Barrier", scbw::get_generic_value(unit, ValueId::CutaneousBarrier) > 0);
		addStatus(statuses, "Infested", scbw::get_generic_value(unit, ValueId::InfestedByAkistrokor) > 0);
		addStatus(statuses, "Enthralling Assault", scbw::get_generic_value(unit, ValueId::EnthrallingTalons) > 0);

		// Aise Value - Gimmicked
		addStatus(statuses, "Phase Link", scbw::get_aise_value(unit, NULL, AiseId::HasClarionLink, 0, 0));
		addStatus(statuses, "Mind Controlling", scbw::get_aise_value(unit, NULL, AiseId::SumMindControlCost, 0, 0) > 0); // for caster
		bool malice = scbw::get_aise_value(unit, NULL, AiseId::HasMalice, 0, 0);
		addStatus(statuses, "Cowardice", scbw::get_generic_value(unit, ValueId::HierophantOverlay) || malice);
		addStatus(statuses, "Malice", malice);

		// Vanilla Timer + Value
		addStatus(statuses, "Defensive Matrix", unit->defensiveMatrixHp / 256, unit->defensiveMatrixTimer);

		// Vanilla Timers
		addStatus(statuses, "Stimpack", -1, unit->stimTimer);
		addStatus(statuses, "Irradiated", -1, unit->irradiateTimer);
		addStatus(statuses, "Ensnared", -1, unit->ensnareTimer);
		addStatus(statuses, "Lobotomized", -1, unit->lockdownTimer);
		addStatus(statuses, "Maelstromed", -1, unit->maelstromTimer);
		addStatus(statuses, "Corrosive Spores", unit->acidSporeCount);
		addStatus(statuses, "Plagued", -1, unit->plagueTimer);
		addStatus(statuses, "Stasis", -1, unit->stasisTimer);
		addStatus(statuses, "Timed Life", -1, unit->removeTimer, TimerType::RemoveTimer);

		// Vanilla Value
		addStatus(statuses, "Parasited", unit->parasiteFlags != 0);
		addStatus(statuses, "Blinded", (bool) unit->isBlind);

		// Case Specific
		addStatus(statuses, "Sieged", unit->id == UnitId::TerranPhalanxSiegeMode);
		if (scbw::isUnderDarkSwarm(unit)) {
			CUnit* darkSwarm = scbw::getDarkSwarm(unit);
			addStatus(statuses, "Under Dark Swarm", darkSwarm->hitPoints / 256, darkSwarm->removeTimer, TimerType::RemoveTimer);
		}

		std::string status = formatStringVector(statuses);

		// TODO Statuses
		//	make sublime shepard show on targets not only caster
		//	add optical flare timer, mind control on target, last orders?, infested (when it's implemented)
		//	properly track cowardice, malice timers
		//	color code source player if possible

		std::vector<std::string> flags;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Building)
			flags.push_back("Structure");
		if (units_dat::BaseProperty[unit->id] & UnitProperty::ProducesUnits)
			flags.push_back("Factory");
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Addon)
			flags.push_back("Addon");
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
			flags.push_back("Worker");
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Hero)
			flags.push_back("Hero");
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Invincible)
			flags.push_back("Invincible");
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Organic)
			flags.push_back("Organic");
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical)
			flags.push_back("Mechanical");
		if (units_dat::BaseProperty[unit->id] & UnitProperty::RoboticUnit)
			flags.push_back("Robotic");
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Detector)
			flags.push_back("Detector");
		if (units_dat::BaseProperty[unit->id] & UnitProperty::CreepBuilding
			|| (unit->id == UnitId::ZergBactalisk && unit->status & UnitStatus::Burrowed))
			flags.push_back("Spreads Creep");

		std::string flag = formatStringVector(flags);

		std::string text = "\004" + std::string(armorUpgradeName) + "\002";
		text += "\n" + std::string(armorStr) + " " + std::to_string(baseArmor);
		if (bonusArmor > 0) text += "+" + std::to_string(bonusArmor);
		text += "\nSize: " + std::string(sizeText);
		text += "\nPlane: " + plane;
		text += "\nMove Speed: " + movementSpeed;
		text += "\nStatus: " + status;
		if (extended) text += "\nFlags: " + flag;
		if (extended && accel != 1) text += "\nAcceleration: " + acceleration;
		if (extended) text += "\nTurn Rate: " + turnRate;
		if (extended) text += "\nSight Range: " + std::to_string(sightRange);
		if (extended && transportSpace != 255) text += "\nTransport Space: " + std::to_string(transportSpace);

		if (!extended) {
			text += "\n-- Hold \003Shift\002 for more details --";
		}

		strncpy(buffer, text.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = 0;
		return buffer;
	}


	//Returns the C-string for the tooltip text of the plasma shield icon.
	const char* getShieldTooltipString(CUnit* unit) {
		//Default StarCraft behavior

		const u16 labelId = upgrades_dat::Label[UpgradeId::ProtossPlasmaShields];
		const char* shieldUpgradeName = statTxtTblNew->getString(labelId);
		const char* shieldStr = statTxtTblNew->getString(779);					 //"Shields:"

		u8 shieldUpgradeLevel = scbw::getUpgradeLevel(unit->playerId, UpgradeId::ProtossPlasmaShields);

		std::string text = "\004" + std::string(shieldUpgradeName) + "\002";
		text += "\n" + std::string(shieldStr) + " 0";
		if (shieldUpgradeLevel > 0) text += "+" + std::to_string(shieldUpgradeLevel);

		strncpy(buffer, text.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = 0;
		return buffer;
	}

	//Returns the C-string for the tooltip text of the Interceptor icon (Carriers),
	//Scarab icon (Reavers), Nuclear Missile icon (Nuclear Silos), and Spider Mine
	//icon (Vultures).
	const char* getSpecialTooltipString(u16 iconUnitId, CUnit* unit) {
		//Default StarCraft behavior

		if (iconUnitId == UnitId::ProtossSteward)
			return getDamageTooltipString(WeaponId::PulseCannon, unit, 791);		//"Interceptors"

		if (iconUnitId == UnitId::ProtossPlasmaShell)
			return getDamageTooltipString(WeaponId::Scarab, unit, 792);				//"Scarabs"

		if (iconUnitId == UnitId::TerranNuclearMissile)
			return statTxtTblNew->getString(793);										//"Nukes"

		if (iconUnitId == UnitId::TerranSpiderMine)
			return getDamageTooltipString(WeaponId::SpiderMines, unit, 794);		//"Spider Mines"
		
		if (iconUnitId == UnitId::TerranLobotomyMine){
			return getDamageTooltipString(WeaponId::LobotomyMine, unit, 794);
		}
		


		//Should never reach here
		return "";

	}

} //hooks
