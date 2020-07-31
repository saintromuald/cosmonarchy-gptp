#include "weapon_damage.h"
#include "../../SCBW/api.h"
#include <algorithm>
#include "CUnitExtended.h"

namespace {
	//Helper functions
	void createShieldOverlay(CUnit* unit, u32 attackDirection);
	u16 getUnitStrength(CUnit* unit, bool useGroundStrength);

	/// Definition of damage factors (explosive, concussive, etc.)
	struct {
		s32 damageType;
		s32 unitSizeFactor[4];	 //Order: {independent, small, medium, large}
	} const damageFactor[5] = {
		{0, 0, 0, 0, 0},				//Independent
		{1, 0, 128, 192, 256},			//Explosive
		{2, 0, 256, 128, 64},			//Concussive
		{3, 0, 256, 256, 256},			//Normal
		{4, 0, 256, 256, 256}			//IgnoreArmor
	};

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
;
const u32 Func_UpdateUnitStrength = 0x0049FA40;
void updateStrength_49FA40(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_UpdateUnitStrength
		POPAD
	}

}

;

const u32 Func_Sub4A01F0 = 0x004A01F0;
void function_004A01F0_copy(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub4A01F0
		POPAD
	}

}
;


const u32 Func_497ED0 = 0x00497ED0;
void makeSpriteVisible(CSprite* sprite) {

	__asm {
		PUSHAD
		MOV EAX, sprite
		CALL Func_497ED0
		POPAD
	}

}
;


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

;

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

;



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

;

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
;

//
#include "SCBW\UnitFinder.h"
class ascendancyProc : public scbw::UnitFinderCallbackProcInterface
{

private:
public:
	void proc(CUnit* unit)
	{
		if (unit->id != UnitId::high_templar) {
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
		if (!(scbw::getUpgradeLevel(unit->playerId,
			UpgradeId::Ascendancy))) {
			return;
		}
		bool show = true;
if (unit->energy == unit->getMaxEnergy()) {
	show = false;
}
if (unit->energy + 5 * 256 >= unit->getMaxEnergy()) {
	unit->energy = unit->getMaxEnergy();
} else {
	unit->energy += 5 * 256;
}
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

	CUnit* scout;

public:
	//Check if @p unit is a suitable target for the @p spiderMine.
	bool match(CUnit* target) {

		//Don't attack friendly / allied units
		if (target->playerId == scout->playerId) {
			return false;
		}
		if (scbw::isAlliedTo(target->playerId, scout->playerId)) {
			return false;
		}
		if (target->status & UnitStatus::Invincible) {
			return false;
		}
		if (target->status & UnitStatus::InAir) {
			return false;
		}
		return true;

	}

	//Constructor
	lastordersProc2(CUnit* scout) : scout(scout) {}
};


bool irratiateTarget(CUnit* unit) {

	if (unit->id == UnitId::larva || unit->id == UnitId::egg || unit->id == UnitId::lurker_egg)
		return false;

	return true;
}


class ecclesiastProc : public scbw::UnitFinderCallbackProcInterface {
private:
	int _healValue = 0;
	int _playerID = 0;

public:
	ecclesiastProc(int stacks, int playerID) {
		_healValue = (5 * stacks) * 256;
		_playerID = playerID;
		char buf[32];
		sprintf(buf, "heal value: %i", _healValue);
		scbw::printText(buf);
	}

	void proc(CUnit* unit) {
		if (!units_dat::ShieldsEnabled[unit->id]) { scbw::printText("shields not enabled"); return; }
		if (_playerID != unit->playerId && !scbw::isAlliedTo(_playerID, unit->playerId)) { scbw::printText("not allied or own unit"); return; }
		unit->shields = std::min(unit->shields + _healValue, units_dat::MaxShieldPoints[unit->id] * 256);
		scbw::printText("heal shields");
	}
};


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

			//if (attacker != nullptr) {
			//	if (attacker->id == UnitId::ProtossDarkTemplar) {
			//		CUnitExtended* flags = CUnitExtended::get(attacker);
			//		flags->timerSpecial1 = 42;
			//	}
			//}
		

			u8 damageType;

			if (scbw::isCheatEnabled(CheatFlags::PowerOverwhelming) &&			//If Power Overwhelming is enabled
				playerTable[attackingPlayerId].type != PlayerType::Human)		//and the attacker is not a human player
				damage = 0;
			/*
					if (target->status & UnitStatus::IsHallucination)
						damage *= 2;*/

			if (weaponId == WeaponId::EcclesiastWeapon && scbw::getUpgradeLevel(attacker->playerId, UpgradeId::EcclesiastStacks)) {
				target->_unknown_0x08C = target->_unknown_0x08C | std::min((((target->_unknown_0x08C & 0x11100) >> 2) + 1) << 2, 10);
				scbw::printText("increment stack");
			}
			else if (weaponId == WeaponId::GlaveWurm || weaponId == WeaponId::GlaveWurm_Kukulza_Mutalisk) {
				double mul = 1.0;
				switch (dmgDivisor) {
				case 0:
					mul = 1.0;
					break;
				case 1:
					mul = 1.0/3.0;//glave wurm 1st bounce
					break;
				case 2:
					mul = 1.0/6.0;
					break;
				default:
					mul = 1.0/9.0;//glave wurm 2nd bounce
					break;
				};
				if (mul != 1 && attacker!=NULL && scbw::getUpgradeLevel(attacker->playerId,UpgradeId::AblativeWurm)) {
					mul *= 1.5;
				}
				int dmg = ((double)damage*mul);
				/*
				char buf[64];
				sprintf(buf, "Mutalisk: damage %d divisor %d mul %lf result %d", damage, dmgDivisor, mul, dmg);
				scbw::printText(buf);*/
				damage = dmg + (target->acidSporeCount * 256);
			}
			else {
				damage = damage / dmgDivisor + (target->acidSporeCount * 256);
			}
			

			if (attacker != NULL) {
				if (attacker->status & UnitStatus::IsHallucination) {
					damage /= 2;
				}
			}
			if (damage < 128)
				damage = 128;
			if (attacker != NULL) {
				if (attacker->id == UnitId::broodling && scbw::getUpgradeLevel(attacker->playerId,84) 
					&& !(units_dat::BaseProperty[target->id] & UnitProperty::Building)) {
					u32 overlayImageId;
					if (units_dat::BaseProperty[target->id] & UnitProperty::MediumOverlay)
						overlayImageId = ImageId::EnsnareOverlay_Medium;
					else
						if (units_dat::BaseProperty[target->id] & UnitProperty::LargeOverlay)
							overlayImageId = ImageId::EnsnareOverlay_Large;
						else
							overlayImageId = ImageId::EnsnareOverlay_Small;
					if (!target->getOverlay(overlayImageId)) {
						target->sprite->createTopOverlay(overlayImageId,
							0, 0, 0);
					}
					target->ensnareTimer = 30;
					target->updateSpeed();
				}
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

			damageType = weapons_dat::DamageType[weaponId];

			//Reduce Plasma Shields...but not just yet
			s32 shieldReduceAmount = 0;
			if (units_dat::ShieldsEnabled[target->id] && target->shields >= 256) {

				if (damageType != DamageType::IgnoreArmor) {

					s32 plasmaShieldUpg = scbw::getUpgradeLevel(target->playerId,
						UpgradeId::ProtossPlasmaShields) * 256;
					if (target->id == UnitId::dragoon || target->id == UnitId::tassadar_zeratul) {
						if (scbw::getUpgradeLevel(target->playerId, UpgradeId::TaldarinsGrace)) {//taldarin's grace
							plasmaShieldUpg += 256;
						}
					}
					if (target->id == UnitId::photon_cannon) {
						if (scbw::getUpgradeLevel(target->playerId, UpgradeId::PrismaticShield)) {//taldarin's grace
							plasmaShieldUpg += 512;
						}
					}
					if (damage > plasmaShieldUpg) //Weird logic, Blizzard dev must have been sleepy
						damage -= plasmaShieldUpg;
					else
						damage = 128;

				}
				shieldReduceAmount = std::min(damage, target->shields);
				if (attacker != NULL && target != NULL) {
					if (attacker->id == UnitId::ultralisk && attacker->parasiteFlags==0 && scbw::getUpgradeLevel(attacker->playerId, UpgradeId::SaberSiphon)) {
						u32 lifesteal = shieldReduceAmount / 2;
						attacker->setHp(attacker->hitPoints + lifesteal);
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
					if (scbw::getUpgradeLevel(target->playerId, UpgradeId::AdrenalinePacks) > 0) {
						if (scbw::get_aise_value(target, NULL, AiseId::GenericTimer, ValueId::Adrenaline, 0) > 0) {
							armorTotal += 512;
						}
					}
					if (target->id == UnitId::battlecruiser && scbw::getUpgradeLevel(target->playerId, UpgradeId::ForeCastle)) {
						u16 angle = scbw::getAngle(target->position.x, target->position.y,
							attacker->position.x, attacker->position.y);
						u16 unit_angle = target->velocityDirection1;
						int arc = (double)45 / 1.40625;
						//90 degrees
						int delta = unit_angle - angle;
						delta = abs((delta + 128) % 256 - 128);
						if (delta <= arc) {
							armorTotal += 512;
							shield_overlay = true;
							//scbw::printText("Fore Castle works");
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
				if (attacker->id == UnitId::firebat && scbw::getUpgradeLevel(attacker->playerId, 50)) {
					if (units_dat::BaseProperty[target->id] & UnitProperty::Building) {
						damage_factor = 256;
					}
				}
			}
			if (weaponId == WeaponId::YamatoGun) {
				//scbw::printText("YamatoGunHit!");
				if (scbw::getUpgradeLevel(attackingPlayerId, UpgradeId::UmojanBatteries)) {
					//scbw::printText("Umojan Batteries!");
					damage += 256 * 50;
				}
			}
			if (weaponId == WeaponId::NuclearMissile) {
				if (scbw::getUpgradeLevel(attackingPlayerId, UpgradeId::AugustgradsRevenge)) {
					damage += 256 * 250;
				}
			}

			damage = (damage * damage_factor) / 256;
			if (attacker != NULL && target != NULL) {
				if (attacker->id == UnitId::ultralisk && attacker->parasiteFlags==0 && scbw::getUpgradeLevel(attacker->playerId, UpgradeId::SaberSiphon)) {
					u32 lifesteal = damage / 2;
					attacker->setHp(attacker->hitPoints + lifesteal);
				}
			}
			if (shieldReduceAmount == 0 && damage < 128)
				damage = 128;
			if (weaponId == WeaponId::Missiles_Unused) {
				if (scbw::getUpgradeLevel(attackingPlayerId, 48)) {
					//scbw::printText("Set overcharge timer");
					target->unusedTimer = 6;
					target->updateSpeed();
				}
			}
			
			if (attacker != NULL && target != NULL) {
				if (attacker->id == UnitId::dark_templar && attacker->playerId) {
					bool silent_kill = false;
					if (damage >= target->hitPoints && scbw::getUpgradeLevel(attacker->playerId, UpgradeId::SilentStrikes)) {
						silent_kill = true;
					}
					if (!silent_kill && !(attacker->visibilityStatus & 1<<target->playerId)) {
						attacker->visibilityStatus |= (1 << target->playerId);
						makeSpriteVisible(attacker->sprite);
						scbw::sendGPTP_aise_cmd(attacker, NULL, GptpId::SetGenericTimer,ValueId::DarkTemplarTimer, 24+1, 0);
					}
				}
				
				if (weaponId == WeaponId::NuclearMissile && target->hitPoints > damage) {
					if (attacker != NULL && scbw::getUpgradeLevel(attacker->playerId, UpgradeId::GroundZero) && irratiateTarget(target)) {
						hooks::IrradiateHit(attacker, target, attacker->playerId);
					}
				}
				if (damage > target->hitPoints) {
					//relic death effect
					//scbw::printText("Check relic");

					if (damage > target->hitPoints) {//death
						if (target->id == UnitId::scout && scbw::getUpgradeLevel(target->playerId, UpgradeId::LastOrders)) {
							if (!scbw::get_aise_value(target, NULL, AiseId::IsSuicidingScout, 0, 0)) {
								scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetLastOrders, 0, 0, 0);
								lastordersProc2 Orders = lastordersProc2(target);
								scbw::UnitFinder unitFinder(target->getX() - 288,
									target->getY() - 288,
									target->getX() + 288,
									target->getY() + 288);
								CUnit* nearest = unitFinder.getNearestTarget(target, Orders);
								if (nearest != NULL) {
									//								scbw::printText("Target found");
									damage = 0;
									target->setHp(50 * 256);
									target->orderTo(OrderId::Move, nearest->position.x, nearest->position.y);
									target->status |= UnitStatus::CanNotReceiveOrders;

								}
							}
						}

					}

					
					if (units_dat::BaseProperty[attacker->id] & UnitProperty::Hero && 
						scbw::get_aise_value(attacker, NULL, AiseId::HasRelic, UpgradeId::BlindJudge, 0) == 1
						&& (units_dat::BaseProperty[target->id] & UnitProperty::Organic)
						&& !(units_dat::BaseProperty[target->id] & UnitProperty::Building)) {

						scbw::sendGPTP_aise_cmd(attacker, target, GptpId::BlindJudgeTimedEffect, target->energy, 0, 0);
						/*
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
	*/
					}
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
				if (target->id == UnitId::archon) {
					if (scbw::getUpgradeLevel(target->playerId, UpgradeId::AstralAegis)) {
						astralaegisProc Aegis = astralaegisProc();
						scbw::UnitFinder unitFinder(target->getX() - 96,
							target->getY() - 96,
							target->getX() + 96,
							target->getY() + 96);
						unitFinder.forEach(Aegis);
					}
				}
				if (weaponId == WeaponId::PsiStorm || Halluc) {
					//psi mastery effect
					ascendancyProc Ascendancy = ascendancyProc();
					scbw::UnitFinder unitFinder(target->getX() - 288,
						target->getY() - 288,
						target->getX() + 288,
						target->getY() + 288);
					unitFinder.forEach(Ascendancy);

				}
			}
			if (attacker != NULL && target != NULL) {
				if (weaponId == WeaponId::ObservanceWeapon) {
					scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericTimer, ValueId::ObservanceTimerDebuff, 1 + (5 * 24), 0);
					int rend = scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::ObservanceRend, 0);
					if (rend < 5) {
						scbw::sendGPTP_aise_cmd(target, NULL, GptpId::AddGenericValue, ValueId::ObservanceRend, 1, 0);
					}
				}
				if (attacker->id == UnitId::ultralisk && scbw::getUpgradeLevel(attacker->playerId, UpgradeId::KaiserRampage) &&
					damage >= target->hitPoints) {
					//ultralisk kills target
					scbw::sendGPTP_aise_cmd(attacker, NULL, GptpId::SetGenericTimer, ValueId::KaiserRampage, 1 + (2 * 24), 0);
				}
			}
			if (weaponId == WeaponId::SubterraneanTentacle && target != NULL) {
				if (scbw::get_aise_value(target, NULL, AiseId::GenericValue, ValueId::Amitosis, 0) == 1) {
					damage /= 2;
					scbw::sendGPTP_aise_cmd(target, NULL, GptpId::SetGenericValue, ValueId::Amitosis, 0, 0);
				}
			}

			target->damageHp(damage, attacker, attackingPlayerId,
				weaponId != WeaponId::Irradiate);		//Prevent Science Vessels from being continuously revealed to the irradiated target

//Reduce shields (finally)
			if (shieldReduceAmount != 0) {

				target->shields -= shieldReduceAmount;

				if (damageType != DamageType::Independent && target->shields != 0)
				{
					shield_overlay = true;
				}

			}
			if (shield_overlay) {
				createShieldOverlay(target, direction);
			}

			//Update unit strength data (?)
			target->airStrength = getUnitStrength(target, false);
			target->groundStrength = getUnitStrength(target, true);

		}


		// procs for killed units
		if (target->hitPoints == 0) {

			// ecclesiastProc
			int stacks = (target->_unknown_0x08C & 0x11100) >> 2;
			const int range = 32;
			char buf[32];
			sprintf(buf, "stack count: %i", stacks);
			scbw::printText(buf);
			sprintf(buf, "unknown value: %i", target->_unknown_0x08C);
			scbw::printText(buf);
			//scbw::printText("unit finder");
			scbw::UnitFinder unitFinder(target->getX() - range,
				target->getY() - range,
				target->getX() + range,
				target->getY() + range);
			unitFinder.forEach(ecclesiastProc(stacks, attacker->playerId));
		}

	}

} //hooks
namespace {

	/**** Definitions of helper functions. Do NOT modify anything below! ****/

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

} //unnamed namespace
