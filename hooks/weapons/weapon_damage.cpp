#include "weapon_damage.h"
#include "../../SCBW/api.h"
#include <algorithm>
#include <cmath>
#include "MathUtils.h"
#include "Utils/captivating_claws.h"
#include "hooks/hydraFinder.h"
#include "globals.h"
#include "logger.h"
namespace {
	//Helper functions
	void createShieldOverlay(CUnit* unit, u32 attackDirection);
	u16 getUnitStrength(CUnit* unit, bool useGroundStrength);

	/// Definition of damage factors (explosive, concussive, etc.)
	struct {
		s32 damageType;
		s32 unitSizeFactor[4];	 //Order: {independent, small, medium, large}
	} const damageFactor[5] = {
		{0, 0, 0,	0,	 0},			//Independent
		{1, 0, 128, 192, 256},			//Explosive
		{2, 0, 256, 192, 128},			//Concussive
		{3, 0, 256, 256, 256},			//Normal
		{4, 0, 256, 256, 256}			//IgnoreArmor
	};

	CUnit* createHallucination(CUnit* target, u32 playerId);
	bool placeHallucination(CUnit* unit);
	void unitDestructor(CUnit * unit);

} //unnamed namespace

//copied from wpnspellhit and renamed
const u32 Func_displayLastNetErrForPlayer = 0x0049E530;
void displayneterr_49E530(u32 playerId) {
	__asm {
		PUSHAD
		PUSH playerId
		CALL Func_displayLastNetErrForPlayer
		POPAD
	}
}

const u32 Func_UpdateUnitStrength = 0x0049FA40;
void updateStrength_49FA40(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_UpdateUnitStrength
		POPAD
	}

}

const u32 Func_Sub4A01F0 = 0x004A01F0;
void function_004A01F0_copy(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub4A01F0
		POPAD
	}

}

const u32 Func_497ED0 = 0x00497ED0;
void makeSpriteVisible(CSprite* sprite) {

	__asm {
		PUSHAD
		MOV EAX, sprite
		CALL Func_497ED0
		POPAD
	}

}

const u32 Func_AI_TrainingUnit = 0x004A2830;
void AI_Train_4A2830(CUnit* unit_creator, CUnit* created_unit) {
	__asm {
		PUSHAD
		MOV EAX, created_unit
		MOV ECX, unit_creator
		CALL Func_AI_TrainingUnit
		POPAD
	}
}

const u32 Func_CreateUnit = 0x004A09D0;
CUnit* CreateUnit_09D0(u32 unitId, int x, int y, u32 playerId) {
	static CUnit* unit_created;

	__asm {
		PUSHAD
		PUSH playerId
		PUSH y
		MOV ECX, unitId
		MOV EAX, x
		CALL Func_CreateUnit
		MOV unit_created, EAX
		POPAD
	}

	return unit_created;

}

//copied from recall_spell, replace later
const u32 Func_CreateThingy = 0x00488210;
CThingy* function_00488210(u32 spriteId, s16 x, s16 y, u32 playerId) {

	static CThingy* thingy;
	s32 x_ = x;
	__asm {
		PUSHAD
		PUSH playerId
		MOVSX EDI, y
		PUSH x_
		PUSH spriteId
		CALL Func_CreateThingy
		MOV thingy, EAX
		POPAD
	}
	return thingy;
}
;
const u32 Func_SetThingyVisibilityFlags = 0x004878F0;
bool function_004878f0(CThingy* thingy) {

	static Bool32 bPreResult;

	__asm {
		PUSHAD
		MOV ESI, thingy
		CALL Func_SetThingyVisibilityFlags
		MOV bPreResult, EAX
		POPAD
	}

	return (bPreResult != 0);

}

const u32 Func_Sub498D70 = 0x00498D70;
void imgul(CSprite* sprite, u32 imageId, u32 unk1, u32 unk2, u32 unk3) {

	__asm {
		PUSHAD
		MOV EAX, sprite
		MOV ESI, imageId
		PUSH unk1
		PUSH unk2
		PUSH unk3
		CALL Func_Sub498D70
		POPAD
	}

}

//
#include "SCBW\UnitFinder.h"
class ascendancyProc : public scbw::UnitFinderCallbackProcInterface {

private:
public:
	void proc(CUnit* unit) {
		if (unit->id != UnitId::ProtossTemplar) {
			return;
		}
		if (
			unit->status & UnitStatus::DoodadStatesThing ||
			unit->lockdownTimer != 0 ||
			unit->stasisTimer != 0 ||
			unit->maelstromTimer != 0
			) {
			return;
		}
		bool show = true;
		if (unit->energy == unit->getMaxEnergy()) {
			show = false;
		}
		unit->addUnitEnergy(5 * 256);
		if (show) {
			imgul(unit->sprite, 425, 0, 0, 0);
		}
	}
};

class astralaegisProc : public scbw::UnitFinderCallbackProcInterface {

private:
public:
	void proc(CUnit* unit) {
		if (!units_dat::ShieldsEnabled[unit->id]) {
			return;
		}
		u32 value = 50 * 256;
		if (unit->shields + value >= (u32)units_dat::MaxShieldPoints[unit->id] * 256)
			unit->shields = units_dat::MaxShieldPoints[unit->id] * 256;
		else
			unit->shields += value;
	}
};

class lastordersProc2 : public scbw::UnitFinderCallbackMatchInterface {
	CUnit* martyr;

public:
	//Check if @p unit is a suitable target for the @p spiderMine.
	bool match(CUnit* target) {

		//Don't attack friendly / allied units
		if (target->playerId == martyr->playerId) {
			return false;
		}
		if (scbw::isAlliedTo(target->playerId, martyr->playerId)) {
			return false;
		}
		if (target->status & UnitStatus::Invincible) {
			return false;
		}
		if (target->status & UnitStatus::InAir) {
			return false;
		}
		if (scbw::getDistanceFast(martyr->position.x, martyr->position.y, target->position.x, target->position.y) > 288) {
			return false;
		}
		return true;

	}

	//Constructor
	lastordersProc2(CUnit* martyr) : martyr(martyr) {}
};

class augurDeathProc : public scbw::UnitFinderCallbackMatchInterface {
	CUnit* augur;

public:
	//Check if @p unit is a suitable target for the @p spiderMine.
	bool match(CUnit* target) {

		//Don't attack friendly / allied units
		if (target->sprite == NULL) {
			return false;
		}
		if (target->playerId == augur->playerId) {
			return false;
		}
		if (scbw::isAlliedTo(target->playerId, augur->playerId)) {
			return false;
		}
		if (target->status & UnitStatus::Invincible) {
			return false;
		}
		return true;

	}

	//Constructor
	augurDeathProc(CUnit* augur) : augur(augur) {}
};


bool irradiateTarget(CUnit* unit) {

/*	if (scbw::isZergEggNew(unit))
		return false;*/

	return true;
}

class ecclesiastProc : public scbw::UnitFinderCallbackProcInterface {
private:
	int _stacks = 0;
	int _playerID = 0;

public:
	ecclesiastProc(int stacks, int playerID) {
		_stacks = stacks;
		_playerID = playerID;
	}

	void proc(CUnit* unit) {
		if (_playerID != unit->playerId && !scbw::isAlliedTo(_playerID, unit->playerId)) {
			return; // Ally Check
		}
		if (units_dat::ShieldsEnabled[unit->id] && unit->shields < units_dat::MaxShieldPoints[unit->id] * 256) { // Shield Check for default behavior
			imgul(unit->sprite, 425, 0, 0, 0);
		}
		else { // Return if both checks failed
			return;
		}
		int healAmount = (5 * (_stacks + 1)) * 256;
		if (units_dat::ShieldsEnabled[unit->id]) { // Shield Heal
			unit->shields = std::min(unit->shields + healAmount, units_dat::MaxShieldPoints[unit->id] * 256);
		}
	}
};

class nathrokorProc : public scbw::UnitFinderCallbackProcInterface {
private:
	int _playerId = 0;

public:
	nathrokorProc(int playerId) {
		_playerId = playerId;
	}

	void proc(CUnit* unit) {
		if (unit == NULL
		|| (_playerId != unit->playerId && !scbw::isAlliedTo(_playerId, unit->playerId))
		|| !(units_dat::BaseProperty[unit->id] & UnitProperty::Organic)) {
			return;
		}
		scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericTimer, ValueId::NathrokorTimer, (4 * 24) + 1, 0);
		unit->updateSpeed();
	}
};

class bloodTitheProc : public scbw::UnitFinderCallbackProcInterface {
	CUnit* attacker;
	CUnit* target;

private:
public:
	void proc(CUnit* unit) {
		if (!(scbw::isAlliedTo(attacker->playerId, unit->playerId))	// Return if recipient is not allied, or if recipient is the target
		||	unit == target) {
			return;
		}
		if (!(units_dat::BaseProperty[target->id] & UnitProperty::Organic)	// Return if recipient or target are not organic
		||	!(units_dat::BaseProperty[unit->id] & UnitProperty::Organic)) {
			return;
		}
		u32 value = 0;
		if (units_dat::ShieldsEnabled[target->id]) {
			value = (units_dat::MaxHitPoints[target->id] + (units_dat::MaxShieldPoints[target->id] * 256)) * 0.10;
		}
		else {
			value = (units_dat::MaxHitPoints[target->id] * 0.10);
		}
		unit->setHp(unit->hitPoints + value, true);
	}

	//Constructor
	bloodTitheProc(CUnit* attacker, CUnit* target) : attacker(attacker), target(target) {}
};

void augurSacrifice(CUnit* target) {
	if (scbw::get_generic_value(target, ValueId::AugurSafetyCheck) == 0) {
		scbw::set_generic_value(target, ValueId::AugurSafetyCheck, 1);
		augurDeathProc Orders = augurDeathProc(target);
		scbw::UnitFinder unitFinder(target->getX() - 384, target->getY() - 384,
			target->getX() + 384, target->getY() + 384);
		CUnit* nearest = unitFinder.getNearestTarget(target, Orders);
		if (nearest != NULL) {
			target->orderTarget.unit = nearest;
			scbw::createBullet(WeaponId::FinalHour, target, target->position.x, target->position.y, target->playerId,
				scbw::getAngle(target->position.x, target->position.y, nearest->getX(), nearest->getY()));

			scbw::hideAndDisableUnit(target);
			target->status |= UnitStatus::Invincible;
			scbw::createThingy(642, target->getX(), target->getY(), target->playerId);
			scbw::playSound(scbw::randBetween(1235, 1238), target);
			target->removeTimer = 5 * 32;
			target->userActionFlags |= 0x4;
			return;
		}
	}
}

namespace hooks {
	/// Hooks into the CUnit::damageWith() function.
	void weaponDamageHook(s32		damage,
		CUnit* target,
		u8		weaponId,
		CUnit* attacker,
		u8		attackingPlayerId,
		s8		direction,
		u8		dmgDivisor
	) {
		//the unit must neither be already dead nor invincible

		if (target->hitPoints != 0 && !(target->status & UnitStatus::Invincible)) {
			u8 damageType;
			bool canDamage = true; // debug only!
			if (scbw::isCheatEnabled(CheatFlags::PowerOverwhelming) &&			//If Power Overwhelming is enabled
				playerTable[target->playerId].type == PlayerType::Human)		//and the target player is a human player
			{
				damage = 0;
			}
			if (target->pAI != NULL) {
				WorkerAi* ai = (WorkerAi*)target->pAI;
				if (ai->type == 2 || ai->type == 3) {
					if (ai->town == nullptr) {
						scbw::printFormattedText("Ai_ReactToHit crash! Attacker id %d, target id %d", attacker->id, target->id);
						canDamage = false;
					}
				}
			}
			// Fertile Spines - Konvilisk behavior
			if (attacker != nullptr
			&&	scbw::get_generic_value(attacker, ValueId::FertileSpines) > 0
			&&	!(target->parasiteFlags)) {
				scbw::set_generic_timer(target, ValueId::ParasiteArmTimer, (24 * 2) + 1); // replace with unitconstant later
			}
			// Deconstruction - Pazuzu behavior - apply stack
			if (attacker!=nullptr
			&&	attacker->id == UnitId::TerranPazuzu
			&&	!(scbw::unitIsPacifist(target))
			&&	scbw::get_generic_value(target, ValueId::Deconstruction) == 0) {
				scbw::set_generic_value(target, ValueId::Deconstruction, 1);
				scbw::set_generic_value(target, ValueId::DeconstructionPlayer, attacker->playerId);
				s32 overlayOffset = scbw::getOverlayOffset(target->id);
				if (target->subunit != nullptr) {
					scbw::set_generic_value(target->subunit, ValueId::Deconstruction, 1);
					scbw::set_generic_value(target->subunit, ValueId::DeconstructionPlayer, attacker->playerId);
					target->subunit->sprite->createOverlay(1332 + overlayOffset);
				}
				else {
					target->sprite->createOverlay(1332 + overlayOffset);
				}
			}
			// Savant Entropy Gauntlets behavior - Apply timer and overlay
			if (attacker != NULL && weaponId == WeaponId::DiscoidRailgun) {
				if (units_dat::BaseProperty[target->id] & UnitProperty::Spellcaster) {
					scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericTimer, ValueId::EntropyGauntlets, (5 * 24) + 1, 0);
					scbw::sendGPTP_aise_cmd(target, attacker, GptpId::SetGenericUnit, ValueId::DecaySavant, 0, 0);
					u32 overlayImageId;
					if (units_dat::BaseProperty[target->id] & UnitProperty::MediumOverlay)
						overlayImageId = 1108;
					else
						if (units_dat::BaseProperty[target->id] & UnitProperty::LargeOverlay)
							overlayImageId = 1109;
						else
							overlayImageId = 1107;
					if (target->subunit != NULL) {
						target->subunit->sprite->removeOverlay(overlayImageId);
						target->subunit->sprite->createTopOverlay(overlayImageId, 0, 0, 0);
					}
					else {
						target->sprite->removeOverlay(overlayImageId);
						target->sprite->createTopOverlay(overlayImageId, 0, 0, 0);
					}
				}
			}
			// Adrenal Glands - Zergling behavior
			if (attacker != NULL && weaponId == WeaponId::Claws) {
				if (scbw::get_aise_value(attacker, NULL, AiseId::GenericValue, ValueId::AdrenalStacks, 0) < 4) {
					scbw::sendGPTP_aise_cmd(attacker, NULL, GptpId::AddGenericValue, ValueId::AdrenalStacks, 1, 0);
				}
				scbw::sendGPTP_aise_cmd(attacker, NULL, GptpId::SetGenericTimer, ValueId::AdrenalTimer, 49, 0); //49 is 2 seconds (48) + 1
			}
			// Dirge - Madrigal behavior
			if (weaponId == 22 && attacker != NULL) {
				scbw::sendGPTP_aise_cmd(target, NULL, GptpId::DirgeImpale, 24, 0, 0);
				scbw::sendGPTP_aise_cmd(target, attacker, GptpId::SetGenericUnit, ValueId::DirgeAttacker, 0, 0);
			}
			// Knockout Drivers - Southpaw behavior
			if (attacker != NULL && weaponId == WeaponId::UppercutLaser) {
				if (scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::KnockoutDriversStacks, 0) < 20) {
					scbw::sendGPTP_aise_cmd(target, NULL, GptpId::AddGenericValue, ValueId::KnockoutDriversStacks, 1, 0);
				}
				scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericTimer, ValueId::KnockoutDriversTimer, 49, 0); //49 is 2 seconds (48) + 1
			}
			// Captivating Claws - Vorvaling behavior
			if (attacker != NULL && weaponId == WeaponId::VorvalClaws) {
				captivating_claws::applyCaptivatingClaws(attacker, target);
			}
			// Astral Blessing - Ecclesiast behavior
			if (attacker != NULL && weaponId == WeaponId::AstralBlaster) {
				if (scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::EcclesiastStack, 0) < 10) {
					scbw::sendGPTP_aise_cmd(target, NULL, GptpId::AddGenericValue, ValueId::EcclesiastStack, 1, 0);
				}
				scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericTimer, ValueId::EcclesiastTimer, 49, 0); //49 is 2 seconds (48) + 1
				scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericValue, ValueId::EcclesiastPlayer, attacker->playerId, 0);
			}
			// Eternal Service - Amaranth behavior
			if (attacker != NULL) {
				if (attacker->id == UnitId::ProtossAmaranth) {
					s32 hp = attacker->hitPoints / 256;
					s32 maxHp = attacker->getMaxHpInGame();
					double shieldStealRate = MathUtils::map(hp, maxHp / 4, maxHp, 0.75, 0.0, true);
					s32 healAmount = damage * shieldStealRate;
					s32 maxShields = units_dat::MaxShieldPoints[attacker->id] * 256;
					attacker->shields = std::min(attacker->shields + healAmount, maxShields);
				}
			}
			// Reactive Payload - Architect behavior
			if (weaponId == WeaponId::DispersiveSalvo) {
				double div = 1.0;
				if (dmgDivisor != 0) {
					div = (double)dmgDivisor;
				}
				int dmg = ((double)damage / div);
				damage = dmg + (target->acidSporeCount * 256);
			}
			// Searing Acid - Guardian behavior
			else if (weaponId == WeaponId::AcidSpore) {
				damage = damage + (target->acidSporeCount * 256) + (dmgDivisor * 10 * 256);
			}
			// Lithe Organelle - Mutalisk bounce damage
			else if (weaponId == WeaponId::GlaveWurm) {
				double mul = 1.0;
				switch (dmgDivisor) {
				case 0:
					mul = 1.0;
					break;
				case 1:
					mul = 1.0 / 3.0; // glave wurm 1st bounce
					break;
				case 2:
					mul = 1.0 / 6.0;
					break;
				default:
					mul = 1.0 / 9.0; // glave wurm 4th bounce
					break;
				};
				int dmg = ((double)damage * mul);
				damage = dmg + (target->acidSporeCount * 256);
			}

			// Kalkaleth behavior
			else if (weaponId == WeaponId::ViscousAcid) {
				int decrement = dmgDivisor * 256;
				if (decrement < damage) {
					damage = damage - decrement + (target->acidSporeCount * 256);
				}
				else {
					damage = damage + (target->acidSporeCount * 256);
				}
			}
			else {
				u32 divisor = dmgDivisor;
				if (divisor == 0) {
					divisor = 1;
					scbw::printFormattedText("Incorrect weapon hit %d", weaponId);
				}
				damage = damage / divisor + (target->acidSporeCount * 256);
			}

			// Ensnaring Brood - Queen enchantment ability
			if  (attacker != NULL
			&&   scbw::get_generic_timer(attacker, ValueId::EnsnaringBroodTimer) > 0
			&& !(units_dat::BaseProperty[target->id] & UnitProperty::Building)) {
				applyEnsnare(target);
			}

			// Quazilisk - Melting Points
			if (weaponId == WeaponId::AcridBarbs) {
				float missingFactor = 1 - ((float)target->hitPoints / target->getMaxHpRaw());
				float damageIncrease = floor(missingFactor / QUAZILISK_PASSIVE_FACTOR);
				//scbw::print("Damage Increase: " + std::to_string(damageIncrease));
				damage += damageIncrease * HEALTH_MULTIPLIER;
			}

			// Hallucinations deal half damage
			if (attacker != NULL) {
				if (attacker->status & UnitStatus::IsHallucination) {
					damage /= 2;
				}
			}

			// Minimum amount of damage that can be dealt
			if (damage < 128)
				damage = 128;

			damageType = weapons_dat::DamageType[weaponId];

			// Dark Swarm - Defiler Ability
			if (scbw::isUnderDarkSwarm(target) && weapons_dat::MaxRange[weaponId] > 32 && (dmgDivisor == 1 || weaponId == WeaponId::AcidSpore) && weaponId != WeaponId::SubterraneanSpines) {
				damageType = DamageType::Independent;
			}

			//Reduce Defensive Matrix
			if (target->defensiveMatrixHp != 0) {
				s32 d_matrix_reduceAmount;
				if (target->defensiveMatrixHp > damage)
					d_matrix_reduceAmount = damage;
				else
					d_matrix_reduceAmount = target->defensiveMatrixHp;
				damage -= d_matrix_reduceAmount;
				target->reduceDefensiveMatrixHp(d_matrix_reduceAmount);
			}

			// Almaksalisk - Flanking Spines
			if (weaponId == WeaponId::PiercingSpines && scbw::isDebuffed(target)) {
				damageType = DamageType::IgnoreArmor;
			}

			// Cliff Advantage - Armor
			if (damageType != DamageType::IgnoreArmor) {
				damage -= std::min(scbw::getCliffAdvantage(attacker), 0) * -256; // not passing weapon allows for melee, which is desirable with armor
			}

			//Reduce Plasma Shields...but not just yet
			s32 shieldReduceAmount = 0;
			if (units_dat::ShieldsEnabled[target->id] && target->shields >= 256) {
				if (damageType != DamageType::IgnoreArmor) {
					s32 plasmaShieldUpg = scbw::getUpgradeLevel(target->playerId, UpgradeId::ProtossPlasmaShields) * 256;
					if (damage > plasmaShieldUpg) { //Weird logic, Blizzard dev must have been sleepy
						damage -= plasmaShieldUpg;
					}
					else {
						damage = 128;
					}
				}
				shieldReduceAmount = std::min(damage, target->shields);
				if (attacker != NULL && target != NULL) {
					if (attacker->id == UnitId::ZergUltrakor) {
						u32 lifesteal = shieldReduceAmount / 2;
						attacker->setHp(attacker->hitPoints + lifesteal, true);
					}
					// Wendigo Automedicus passive (shield component)
					if (attacker->id == UnitId::TerranWendigo && units_dat::BaseProperty[target->id] & UnitProperty::Mechanical) {
						u32 lifesteal = shieldReduceAmount / 4;
						attacker->setHp(attacker->hitPoints + lifesteal, true);
					}
				}
				damage -= shieldReduceAmount;
			}

			//Apply armor
			bool shield_overlay = false;
			if (damageType != DamageType::IgnoreArmor) {
				s32 armorTotal = target->getArmor() * 256;
				if (attacker != NULL && target != NULL) {
					armorTotal += scbw::get_aise_value(target, NULL, AiseId::GetShepherds, 0, 0) * 256;
					if (target->id == UnitId::TerranBattlecruiser) {
						u16 angle = scbw::getAngle(target->position.x, target->position.y,
							attacker->position.x, attacker->position.y);
						u16 unit_angle = target->velocityDirection1;
						int arc = (double)45 / 1.40625;	//90 degrees
						int delta = unit_angle - angle;
						delta = abs((delta + 128) % 256 - 128);
						if (delta <= arc) {
							armorTotal += 512;
							shield_overlay = true;
						}
					}
				}
				damage -= std::min(damage, armorTotal);
				auto rend = 256 * scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::ObservanceRend, 0);
				damage += rend;
			}

			//Apply damage type/unit size factor (not checked by UndeadStar, I will hope it work)
			s32 damage_factor = damageFactor[damageType].unitSizeFactor[units_dat::SizeType[target->id]];
			if (attacker != NULL) {
				if (attacker->id == UnitId::firebat) {
					if (units_dat::BaseProperty[target->id] & UnitProperty::Building) {
						damage_factor = 256;
					}
				}
			}

			damage = (damage * damage_factor) / 256;
			if (attacker != NULL && target != NULL) {
				// Ultrakor Saber Siphon passive
				if (attacker->id == UnitId::ZergUltrakor) {
					u32 lifesteal = damage / 2;
					attacker->setHp(attacker->hitPoints + lifesteal, true);
				}

				// Wendigo Automedicus passive (HP component)
				if (attacker->id == UnitId::TerranWendigo && units_dat::BaseProperty[target->id] & UnitProperty::Mechanical) {
					u32 lifesteal = damage / 4;
					attacker->setHp(attacker->hitPoints + lifesteal, true);
				}
			}

			if (shieldReduceAmount == 0 && damage < 128) {
				damage = 128;
			}

			if (attacker != NULL && target != NULL) {
				if ((units_dat::BaseProperty[attacker->id] & UnitProperty::PermanentCloak
					||	attacker->status & UnitStatus::CloakingForFree)
				&&	attacker->playerId
				&&	attacker->id != UnitId::ZergLakizilisk) {
					bool silent_kill = false;
					if (damage >= target->hitPoints
					&&	attacker->id == UnitId::ProtossCabalist) {
						silent_kill = true;
					}
					if (!silent_kill && !(attacker->visibilityStatus & 1 << target->playerId)) {
						attacker->visibilityStatus |= (1 << target->playerId);
						makeSpriteVisible(attacker->sprite);
						scbw::sendGPTP_aise_cmd(attacker, NULL, GptpId::SetGenericTimer, ValueId::PermanentCloakTimer, 48 + 1, 0);
					}
				}

				if (weaponId == WeaponId::NuclearMissile && target->hitPoints > damage) {
					if (attacker != NULL && irradiateTarget(target)) {
						hooks::IrradiateHit(attacker, target, attacker->playerId);
					}
				}
				if ((weaponId == WeaponId::PlasmidSpores || weaponId==WeaponId::ParasiticSpores) && damage >= target->hitPoints) {
					scbw::set_generic_value(target, ValueId::AlkajeliskParasite, 1);
					scbw::set_generic_value(target, ValueId::AlkajeliskParasitePlayer, attacker->playerId);
				}
				if (damage >= target->hitPoints) {

					// Last Orders - Aurora behavior
					if (target->id == UnitId::ProtossAurora) {
						if (scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::LastOrdersInit, 0) == 0) {
							scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericValue, ValueId::LastOrdersInit, 1, 0);
							lastordersProc2 Orders = lastordersProc2(target);
							scbw::UnitFinder unitFinder(target->getX() - 288, target->getY() - 288, target->getX() + 288, target->getY() + 288);
							CUnit* nearest = unitFinder.getNearestTarget(target, Orders);
							if (nearest != NULL) {
								damage = 0;
								target->setHp(units_dat::MaxHitPoints[target->id] / 2);
								target->orderTo(OrderId::Move, nearest->position.x, nearest->position.y);
								target->status |= UnitStatus::CanNotReceiveOrders;
							}
						}
					}

					// Blind Judge relic
					/*if (units_dat::BaseProperty[attacker->id] & UnitProperty::Hero &&
						scbw::get_aise_value(attacker, NULL, AiseId::HasRelic, UpgradeId::BlindJudge, 0) == 1
						&& (units_dat::BaseProperty[target->id] & UnitProperty::Organic)
						&& !(units_dat::BaseProperty[target->id] & UnitProperty::Building)) {
							scbw::sendGPTP_aise_cmd(attacker, target, GptpId::BlindJudgeTimedEffect, target->energy, 0, 0);
						
						u32 energy = target->energy;
						Point16 pos = target->position;
						CUnit* resurrected_unit;
						//fixTargetLocation(&pos, target->id);
						resurrected_unit = CreateUnit_09D0(target->id, pos.x, pos.y, attacker->playerId);
						if (resurrected_unit == NULL) {
							displayneterr_49E530(attacker->playerId);
						}
						else {
							function_004A01F0_copy(resurrected_unit);
							updateStrength_49FA40(resurrected_unit);
							resurrected_unit->removeTimer = 360;
							resurrected_unit->energy = energy;
							resurrected_unit->setHp(units_dat::MaxHitPoints[resurrected_unit->id]);
							if (attacker->pAI != NULL)
								AI_Train_4A2830(attacker, resurrected_unit);
						}
					}*/	
				}
			}

			//Deal damage to target HP, killing it if possible
			bool Halluc = false;
			if (attacker != NULL) {
				if (attacker->status & UnitStatus::IsHallucination) {
					Halluc = true;
				}
			}

			if (damage >= target->hitPoints) {
				if (target->id == UnitId::ProtossArchon) {
					astralaegisProc Aegis = astralaegisProc();
					scbw::UnitFinder unitFinder(target->getX() - 96,
						target->getY() - 96,
						target->getX() + 96,
						target->getY() + 96);
					unitFinder.forEach(Aegis);
				}
				if (weaponId == WeaponId::PsiStorm || Halluc) {
					//psi mastery effect
					scbw::UnitFinder unitFinder(target->getX() - 288,
						target->getY() - 288,
						target->getX() + 288,
						target->getY() + 288);
					unitFinder.forEach(ascendancyProc());
				}
				if (scbw::get_generic_value(target, ValueId::LazarusAgent) == 1) {
					Globals.initLazarusAgent(target->position.x, target->position.y,
						target->playerId, target->id, target->energy,
						(CUnit*)scbw::get_generic_value(target, ValueId::LazarusAgentApostleSrc));
				}
			}

			// Observance - Science Vessel ability
			if (attacker != NULL) {
				if (weaponId == WeaponId::Observance
				&&	(!scbw::isAlliedTo(attacker->playerId, target->playerId))) {
					scbw::set_generic_timer(attacker, ValueId::ObservanceOverlayTimer, 18);
					scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericTimer, ValueId::ObservanceTimerDebuff, 1 + (5 * 24), 0);
					int rend = scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::ObservanceRend, 0);
					if (rend < 5) {
						if (rend == 0) {
							int overlay = 1233;
							if (units_dat::BaseProperty[target->id] & UnitProperty::LargeOverlay) {
								overlay -= 2;
							}
							else if (units_dat::BaseProperty[target->id] & UnitProperty::MediumOverlay) {
								overlay -= 1;
							}
							target->sprite->createOverlay(overlay);
						}
						scbw::sendGPTP_aise_cmd(target, NULL, GptpId::AddGenericValue, ValueId::ObservanceRend, 1, 0);
					}
				}
			}

			// Tendril Amitosis - Sunken Colony upgrade
			/*if (weaponId == WeaponId::SubterraneanTentacle && target != NULL) {
				if (scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::Amitosis, 0) == 1) {
					damage /= 2;
					scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericValue, ValueId::Amitosis, 0, 0);
				}
			}*/

			// Essence Drain - Gorgoleth Energy Steal
			if (attacker != NULL) {
				if (attacker->id == UnitId::ZergGorgrokor
				&& units_dat::BaseProperty[target->id] & UnitProperty::Spellcaster) {
					u16 energy = std::min(target->energy, (u16)(damage + shieldReduceAmount));
					target->energy -= energy;
					attacker->energy += energy;
				}
			}

			// Stellar Enforcement - Magister behavior
			if (attacker != NULL) {
				if (attacker->id == UnitId::ProtossMagister
				&&	!(scbw::get_generic_value(target, ValueId::StellarEnforcement))) {
					scbw::sendGPTP_aise_cmd(target, NULL, GptpId::AddGenericValue, ValueId::StellarEnforcement, 1, 0);
				}
			}

			// Blood Tithe - Apostle behavior
			if (attacker != NULL) {
				if (attacker->id == UnitId::TerranApostle
//				&& scbw::getDistanceFast(attacker->position.x, attacker->position.y, target->position.x, target->position.y) >= 160
				) {
					u32 range = 3 * 32;
					scbw::UnitFinder unitFinder(target->getX() - range, target->getY() - range, target->getX() + range, target->getY() + range);
					unitFinder.forEach(bloodTitheProc(attacker, target));
				}
			}

			// Phase Rush - Manifold behavior
			if (attacker != NULL) {
				if (units_dat::BaseProperty[attacker->id] & UnitProperty::RoboticUnit) {
					scbw::set_generic_timer(target, ValueId::PhaseRushTimerArrayStart+attacker->playerId, 5 * 24);
				}
			}

			// Petrification - Gorgon behavior
			if (GorgonCondition(attacker, target)) {
				scbw::refreshDebuffs(target);
			}

			// Autovitality - Simulacrum behavior
			if (target != NULL && attacker != NULL
			&&	target->id == UnitId::ProtossSimulacrum
			&& !(scbw::isAlliedTo(target->playerId, attacker->playerId))
			&&	(scbw::get_generic_timer(target, ValueId::SimulacrumCloneTimer) == 0)) {
				scbw::set_generic_timer(target, ValueId::SimulacrumCloneTimer, 5 * 24); // 5 seconds may be too long, needs testing
			}

			if (attacker != NULL) { // looks like target will always be alive if this function was called?
				if (scbw::get_aise_value(target, 0, AiseId::GenericTimer, ValueId::HallucinatingTimer, 0) >= 1) {
					CUnit* haluAttacker = attacker;
					if ((haluAttacker->id == UnitId::ProtossInterceptor
						|| haluAttacker->id == UnitId::ProtossScarab)
						&& attacker->interceptor.parent != NULL) {
						haluAttacker = attacker->interceptor.parent;
					}
					if (scbw::get_generic_unit(haluAttacker, ValueId::HallucinationPointer) == NULL
					&& !(haluAttacker->status & UnitStatus::IsHallucination)) {
						u32 haluPlayer = scbw::get_aise_value(target, 0, AiseId::GenericValue, ValueId::HallucinatingSourcePlayer, 0);
						if (haluPlayer != haluAttacker->playerId && !scbw::isAlliedTo(haluPlayer, haluAttacker->playerId)) return;

						CUnit* hal_unit = createHallucination(haluAttacker, haluPlayer);
						if (hal_unit != NULL) {
							hal_unit->sprite->createOverlay(ImageId::HallucinationDeath3);
							scbw::sendGPTP_aise_cmd(haluAttacker, hal_unit, GptpId::SetGenericUnit, ValueId::HallucinationPointer, 0, 0);

							if (!placeHallucination(hal_unit)) {
								unitDestructor(hal_unit);
							}
						}
					}
				}

				if (damage >= target->hitPoints) { // <- keep this as hitPoints, because the damage is already calculated to include armor, shields and damage factor calculations

					// Radiation Shockwave on death
					if (target->irradiateTimer > 0) {
						scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericValue, ValueId::RadiationShockwaveCount, 1, 0);
					}

					// Kaiser Rampage - Ultralisk behavior
					if (attacker->id == UnitId::ZergUltrakor) {
						scbw::sendGPTP_aise_cmd(attacker, NULL, GptpId::SetGenericTimer, ValueId::KaiserRampage, 1 + (2 * 24), 0);
						attacker->updateSpeed();
					}

					// Adrenal Frenzy - Nathrokor behavior
					if (attacker->id == UnitId::ZergNathrokor) {
						int player = attacker->playerId;
						const int range = 48;
						scbw::UnitFinder(attacker->getX() - range, attacker->getY() - range, attacker->getX() + range, attacker->getY() + range).forEach(nathrokorProc(player));
					}

					// Incubators - Broodling behavior
					if (attacker->id == UnitId::ZergBroodling ||
						attacker->id == UnitId::ZergBroodlisk) {
						int type = attacker->id;
						int supplyCount = scbw::canSpawnBroodling(target) ? 2 : 0;// scbw::getBroodlingCountFromSupply(target);
						auto sourceQueen = scbw::getSourceQueens(attacker)[0];
						if (sourceQueen) {
							scbw::createUnitFromAbility(type, supplyCount, attacker->playerId,
								target->position.x, target->position.y, attacker->removeTimer,
								scbw::getSourceQueens(attacker)[0]);
						}
					}

					// Ecclesiast - Astral Blessing proc
					if (scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::EcclesiastStack, 0)) {
						int stacks = scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::EcclesiastStack, 0);
						int player = scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::EcclesiastPlayer, 0);
						const int range = 64;
						scbw::UnitFinder(target->getX() - range, target->getY() - range, target->getX() + range, target->getY() + range).forEach(ecclesiastProc(stacks, player));
					}

					// Cyclops - Flak Cannon passive
					if (weaponId == WeaponId::FlakCannons) {
						auto newAngle = scbw::getAngle(attacker->getX(), attacker->getY(), target->getX(), target->getY());
						scbw::createBullet(WeaponId::FragmentaryShells, attacker, target->getX(), target->getY(), attacker->playerId, newAngle);
					}

					// Augur - Death behavior
					if (target->id == UnitId::ProtossAugur) {
						augurSacrifice(target);
					}
				}
			}

			// Deal Damage, except for Independent damage, which always deals 0
			if (damageType != DamageType::Independent && target->defensiveMatrixHp == 0) {
				if (canDamage) {
					target->damageHp(damage, attacker, attackingPlayerId, weaponId != WeaponId::Irradiate);
					// Prevent Science Vessels from being continuously revealed to the irradiated target
				}
			}

			// Reduce shields (finally)
			if (shieldReduceAmount != 0
			&&	canDamage) {
				if (scbw::get_aise_value(target, NULL, AiseId::HasClarionLink, 0, 0)) {
					scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SpreadClarionDamage, shieldReduceAmount, damageType, direction);
				}
				else {
					target->shields -= shieldReduceAmount;
					if (damageType != DamageType::Independent && target->shields != 0) {
						shield_overlay = true;
					}
				}
			}
			if (shield_overlay) {
				createShieldOverlay(target, direction);
			}

			//Update unit strength data (?)
			target->airStrength = getUnitStrength(target, false);
			target->groundStrength = getUnitStrength(target, true);
		}
	}

	void applyEnsnare(CUnit* a1) { // 004F45E0
		s32 baseProperty; // eax
		u8 overlayPadding; // al
		CUnit* victim; // ecx

		if (!(a1->ensnareTimer)
		&&  !(a1->status & UnitStatus::Burrowed)) {
			// Calculate overlay ID offset based on units_dat flags
			overlayPadding = 0;
			baseProperty = units_dat::BaseProperty[a1->id];
			if (baseProperty & UnitProperty::MediumOverlay) {
				overlayPadding = 1;
			}
			else if (baseProperty & UnitProperty::LargeOverlay) {
				overlayPadding = 2;
			}
			victim = a1;
			if (a1->subunit != NULL) {
				victim = a1->subunit;
			}
			// Create overlay
			victim->sprite->createOverlay(ImageId::EnsnareOverlay_Small + overlayPadding);
		}
		// Apply effect
		a1->ensnareTimer = 37;
		a1->updateSpeed();
	}

	bool GorgonCondition(CUnit* attacker, CUnit* target) {
		if (attacker != NULL && target != NULL) {
			if (attacker->id == UnitId::TerranGorgon) {
				if (scbw::isStunned(target) || scbw::isSlowed(target)) {
					return true;
				}
			}
		}
		return false;
	}
} //hooks
namespace {

	//Creates the Plasma Shield flickering effect.
	//Identical to function @ 0x004E6140
	void createShieldOverlay(CUnit* unit, u32 attackDirection) {

		//		const LO_Header* shield_lo = shieldOverlays[unit->sprite->mainGraphic->id];
		const LO_Header* shield_lo = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlayShieldPtr,
			unit->sprite->mainGraphic->id));
		u32 frameAngle = (attackDirection - 124) / 8 % 32;

		Point8 offset = shield_lo->getOffset(unit->sprite->mainGraphic->direction, frameAngle);

		unit->sprite->createOverlay(ImageId::ShieldOverlay, offset.x, offset.y, frameAngle);

	}

	//Somehow related to AI stuff; details unknown.
	const u32 Helper_GetUnitStrength = 0x00431800;
	u16 getUnitStrength(CUnit* unit, bool useGroundStrength) {
		static u16 strength;
		static u32 useGroundStrength_;
		useGroundStrength_ = useGroundStrength ? 1 : 0;
		__asm {
			PUSHAD
			PUSH useGroundStrength_
			MOV EAX, unit
			CALL Helper_GetUnitStrength
			MOV strength, AX
			POPAD
		}
		return strength;
	}

	// Hallucination-related
	// Moved here instead of including an entire other file...
	const u32 Func_createHallucination = 0x004F6B90;
	CUnit* createHallucination(CUnit* target, u32 playerId) {
		static CUnit* return_value;
		__asm {
			PUSHAD
			PUSH playerId
			MOV ECX, target
			CALL Func_createHallucination
			MOV return_value, EAX
			POPAD
		}
		return return_value;
	}

	const u32 Func_placeHallucination = 0x004F66D0;
	bool placeHallucination(CUnit* unit) {
		static Bool32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_placeHallucination
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	const u32 Func_unitDestructor = 0x004A0990;
	void unitDestructor(CUnit* unit) {
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_unitDestructor
			POPAD
		}
	}

} //unnamed namespace
