#include "weapon_armor_tooltip.h"
#include <SCBW/api.h>
#include <SCBW/enumerations/WeaponId.h>
#include <cstdio>


char buffer[256];

//Returns the special damage multiplier factor for units that don't use the
//"Damage Factor" property in weapons.dat.
u8 getDamageFactorForTooltip(u8 weaponId, CUnit* unit) {

	//Default StarCraft behavior
	if (unit->id == UnitId::TerranFirebat
	||	unit->id == UnitId::ProtossZealot
	||	unit->id == UnitId::ProtossLegionnaire
	||	unit->id == UnitId::ProtossAmaranth
	||	unit->id == UnitId::ProtossGolem
	||	unit->id == UnitId::ProtossPariah
	||	weaponId == WeaponId::RetractorBatteries
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
	//Default StarCraft behavior

	//get strings
	const char* entryName = statTxtTblNew->getString(entryStrIndex);
	const char* damageStr = statTxtTblNew->getString(777);					//"Damage:"
	const char* perRocketStr = statTxtTblNew->getString(1301);				//"per rocket"
	const char* type = "Type:";
	const char* typeName = "";
	const char* rangeText = "Range:";
	const char* meleeText = "Melee";

	//grab weapon details from weapon dat
	u8 damageFactor = getDamageFactorForTooltip(weaponId, unit);
	u8 upgradeLevel = scbw::getUpgradeLevel(unit->playerId, weapons_dat::DamageUpgrade[weaponId]);
	u16 baseDamage = weapons_dat::DamageAmount[weaponId] * damageFactor;
	u16 bonusDamage = weapons_dat::DamageBonus[weaponId] * damageFactor * upgradeLevel;
	u8 damageType = weapons_dat::DamageType[weaponId];
	//calculate max range for weapon
	u32 range = weapons_dat::MaxRange[weaponId] / 32;
	u32 bonusRange = 0;

	//find range bonus for weapon
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

	//grab the string for the damage type of the weapon
	switch (damageType) {
	case DamageType::Normal:
		//Normal type damage uses green text
		typeName = "\017Normal\002";
		break;
	case DamageType::Explosive:
		//Explosive type damage uses orange text
		typeName = "\021Explosive\002";
		break;
	case DamageType::Concussive:
		//Concussive type damage uses yellow text
		typeName = "\003Concussive\002";
		break;
	case DamageType::Independent:
		//Independent type damage uses white text
		typeName = "\004Independent\002";
		break;
	case DamageType::IgnoreArmor:
		//Ignore Armor type damage uses white text
		typeName = "\004Ignores Armor\002";
		break;
	default:
		typeName = "None";
		break;
	};

	int minRange = weapons_dat::MinRange[weaponId] / 32;

	std::string attackCdText = "Cooldown:";
	double attackCd = (double) weapons_dat::Cooldown[weaponId] / 24;

	std::string targetText = "Targets:";
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

	std::string splashRadText = "Splash Radius:";
	int splashInner = weapons_dat::InnerSplashRadius[weaponId];
	int splashMedium = weapons_dat::MediumSplashRadius[weaponId];
	int splashOuter = weapons_dat::OuterSplashRadius[weaponId];
	bool hasSplash = splashInner || splashMedium || splashOuter;
	std::string splashTypeText = "Splash Type:";
	int splashType = weapons_dat::ExplosionType[weaponId];
	std::string splashTypeValue = "";

	switch (splashType) {
	case WeaponEffect::SplashRadial:
		// Orange
		splashTypeValue = "\021Unsafe\002";
		break;
	case WeaponEffect::SplashEnemy:
		// Green
		splashTypeValue = "\017Safe\002";
		break;
	case WeaponEffect::Unknown_Crash:
		// Orange
		splashTypeValue = "\017Ally Safe\002";
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

	// Name
	std::string text = "\004" + std::string(entryName) + "\002\n";

	// Damage
	text += std::string(damageStr) + " " + std::to_string(baseDamage);
	if (bonusDamage > 0) text += "+" + std::to_string(bonusDamage);
	if (weaponId == WeaponId::HaloRockets) text += " " + std::string(perRocketStr);

	// Cooldown
	std::stringstream ss;
	ss << std::setprecision(4) << attackCd;
	text += "\n" + std::string(attackCdText) + " " + ss.str() + "s";

	// Range
	text += "\n" + std::string(rangeText) + " ";
	if (range > 0) {
		if (minRange > 0) {
			text += std::to_string(minRange) + "-";
		}
		text += std::to_string(range);
		if (bonusRange > 0) text += "+" + std::to_string(bonusRange);
	}
	else text += std::string(meleeText);

	// Target
	text += "\n" + targetText + " " + targetFlagsText;

	// Type
	text += "\n" + std::string(type) + " " + std::string(typeName);

	// Splash
	if (hasSplash) {
		text += "\n" + splashRadText + " " + std::to_string(splashInner) + "/" + std::to_string(splashMedium) + "/" + std::to_string(splashOuter);
		text += "\n" + splashTypeText + " " + splashTypeValue;
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
					else text += aisetimerToString(timer);
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

	//Returns the C-string for the tooltip text of the unit's armor icon.
	const char* getArmorTooltipString(CUnit* unit) {
		//Default StarCraft behavior

		const u16 labelId = upgrades_dat::Label[units_dat::ArmorUpgrade[unit->id]];
		const char* armorUpgradeName = statTxtTblNew->getString(labelId);
		const char* armorStr = statTxtTblNew->getString(778);						//"Armor:"

		const u8 baseArmor = units_dat::ArmorAmount[unit->id];
		const u8 bonusArmor = unit->getArmorBonus();
		const u8 unitSize = units_dat::SizeType[unit->id];
		const char* sizeLabel = "Size:";
		const char* sizeText = "";

		const int sightRange = units_dat::SightRange[unit->id];

		switch (unitSize) {
		case 1:
			sizeText = "\017Small\002";
			break;
		case 2:
			sizeText = "\003Medium\002";
			break;
		case 3:
			sizeText = "\021Large\002";
			break;
		default:
			sizeText = "\004Unknown\002";
			break;
		}


		std::string plane = unit->status & UnitStatus::InAir ? "Air" : "Ground";

		std::vector<std::string> statuses;
		if (unit->status & UnitStatus::Burrowed)
			statuses.push_back("Burrowed");
		else if (unit->status & UnitStatus::Cloaked)
			statuses.push_back("Cloaked");
		addStatus(statuses, "Defensive Matrix", unit->defensiveMatrixHp / 256, unit->defensiveMatrixTimer);
		addStatus(statuses, "Malediction", -1, scbw::get_generic_timer(unit, ValueId::MaledictionSlow), TimerType::Aise);
		addStatus(statuses, "Ensnaring Brood", -1, scbw::get_generic_timer(unit, ValueId::EnsnaringBrood), TimerType::Aise);
		addStatus(statuses, "Knockout Drivers", scbw::get_generic_value(unit, ValueId::KnockoutDriversValue), scbw::get_generic_timer(unit, ValueId::KnockoutDriversTimer), TimerType::Aise);
		addStatus(statuses, "Lobotomy Slow", -1, scbw::get_generic_timer(unit, ValueId::LobotomySlow), TimerType::Aise);
		addStatus(statuses, "Observance", -1, scbw::get_generic_timer(unit, ValueId::ObservanceTimerDebuff), TimerType::Aise);
		addStatus(statuses, "Energy Decay", scbw::get_generic_value(unit, ValueId::EnergyDecay), -1);
		addStatus(statuses, "Armor Reduction", scbw::get_generic_value(unit, ValueId::ArmorReduce), -1);
		addStatus(statuses, "Irradiated", -1, unit->irradiateTimer);
		addStatus(statuses, "Ensnared", -1, unit->ensnareTimer);
		addStatus(statuses, "Lobotomized", -1, unit->lockdownTimer);
		addStatus(statuses, "Maelstromed", -1, unit->maelstromTimer);
		addStatus(statuses, "Corrosive Spores", unit->acidSporeCount);
		addStatus(statuses, "Plagued", -1, unit->plagueTimer);
		addStatus(statuses, "Stasis", -1, unit->stasisTimer);
		addStatus(statuses, "Timed Life", -1, unit->removeTimer, TimerType::RemoveTimer);
		if (scbw::isUnderDarkSwarm(unit)) {
			CUnit* darkSwarm = scbw::getDarkSwarm(unit);
			addStatus(statuses, "Under Dark Swarm", darkSwarm->hitPoints / 256, darkSwarm->removeTimer, TimerType::RemoveTimer);
		}
		addStatus(statuses, "Parasited", unit->parasiteFlags != 0);
		// TODO: add optical flare, cowardice, malice

		std::string status = formatStringVector(statuses);

		std::string text = "\004" + std::string(armorUpgradeName) + "\002";
		text += "\n" + std::string(armorStr) + " " + std::to_string(baseArmor);
		if (bonusArmor > 0) text += "+" + std::to_string(bonusArmor);
		text += "\nSize: " + std::string(sizeText);
		text += "\nSight Range: " + std::to_string(sightRange);
		text += "\nPlane: " + plane;
		text += "\nStatus: " + status;

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

		if (iconUnitId == UnitId::ProtossInterceptor)
			return getDamageTooltipString(WeaponId::PulseCannon, unit, 791);		//"Interceptors"

		if (iconUnitId == UnitId::ProtossScarab)
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
