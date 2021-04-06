#include "weapon_effect.h"

#include <SCBW/UnitFinder.h>
#include <SCBW/enumerations.h>
#include <SCBW/api.h>
#include "hooks/weapons/wpnspellhit.h"
#include "globals.h"
#include "limits/ccmu.h"
namespace {	// helper declarations
	bool isUnitDistanceWithin(CUnit* target, CBullet* bullet, uint32 radius);
	void weaponBulletShot(CUnit* target, CBullet* bullet, uint32 dmgDivisor);
	void doWeaponHit(CUnit* target, CUnit* attacker, uint8 dmgDivisor);
	void Ai_UnitWasHit(uint32 main_target_reaction, uint32 dont_call_help, CUnit* attacker, CUnit* target);

	// for airSplash
	CUnit* findBestUnit(Box16* radius, int32 bullet, int32(__fastcall* func)(int32, int32));					// 004E8830
	int16 randomizeShort(int32 counts);																		// 004DC4A0
	int32 splashFunc(CBullet* bullet, CUnit* unit);														// 0048B570

	// SC internal data for Lurker spines
	struct SpineHitTable {
		struct {
			CUnit* source;
			CUnit* target;
		} hits[32][16];
	};
	SpineHitTable* const spineHitTable = (SpineHitTable*)(0x0064DEC8);
	uint32* const spineHitRow = (uint32*)(0x64EEC8);
	uint32* const spineHitCol = (uint32*)(0x64DEA8);

}//unnamed namespace

namespace hooks {



	void darkSwarmDamage(uint16 x, uint16 y, uint16 r, CBullet* bullet, uint32 weaponId) {
		CUnit* dsw = scbw::getDarkSwarm(x, y, r);
		if (dsw != nullptr) {
			//scbw::printText("Dark swarm detected");
			uint32 damage = (weapons_dat::DamageAmount[weaponId] * 256) +
				(weapons_dat::DamageBonus[weaponId] * 256 * scbw::getUpgradeLevel(bullet->srcPlayer, weapons_dat::DamageUpgrade[weaponId]));
			if (bullet->hitFlags & 2) {
				//hallucination
				damage /= 2;
			}
			if (damage >= dsw->hitPoints) {
				dsw->remove();
				//scbw::printText("Destroy dark swarm");
			} else {
				uint32 old = dsw->hitPoints;
				dsw->hitPoints -= damage;
				//char buf[64];
				//sprintf(buf,"Dark swarm hit: %d --> %d",old,dsw->hitPoints);
				//scbw::printText(buf);
			}
		}
	}
	bool nathrokorCondition(CBullet* bullet, CUnit* target) {
		if (bullet->sourceUnit == nullptr || target == nullptr) {
			return true;
		}
		if (bullet->weaponType == WeaponId::NathrokorPlasmidBreath) {
			CUnit* nathrokor = bullet->sourceUnit;
			uint16 angle = scbw::getAngle(nathrokor->position.x, nathrokor->position.y,
				target->position.x, target->position.y);
			uint16 unit_angle = nathrokor->velocityDirection1;
			int arc = (double)15 / 1.40625;
			//30 degrees
			int delta = unit_angle - angle;
			delta = abs((delta + 128) % 256 - 128);
			if (delta > arc) {
				return false;
			} else {
			}
		}
		return true;
	}
	void friendlySplash(CBullet* bullet, uint32 weaponId, uint8 splash_type, CUnit* guaranteed_hit) {
		int splash = splash_type;
		if (bullet->sourceUnit != nullptr && bullet->sourceUnit->sprite != nullptr) {
			if (bullet->sourceUnit->isBlind) {
				splash = SplashType::Radial;
			}

			//must be moved to aise
			if (bullet->sourceUnit->id == UnitId::TerranSalamander) {
				auto hit = bullet->attackTarget.pt;
				if (bullet->attackTarget.unit != nullptr) {
					hit = bullet->attackTarget.unit->position;
				}
				Globals.createSalamanderFire(hit.x, hit.y, 11, 5, true);
			}
			/*			if (bullet->sourceUnit->id == WeaponId::IonCannon) {
							auto hit = bullet->attackTarget.pt;
							if (bullet->attackTarget.unit != nullptr) {
								hit = bullet->attackTarget.unit->position;
							}
							Globals.createIonField(hit.x, hit.y, 11, 5, true);
						}*/
		}

		int16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::OuterSplashRadius[weaponId];

		// Probing Incisors - Gorgoleth passive
		if (isChargedGorgolethBullet(bullet, weaponId)) {
			r = 45;
		}
		// Nathrokor weapon
		if (weaponId == WeaponId::NathrokorPlasmidBreath) {
			r = 32;
		}

		// Get units within the outer splash radius
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);
		//
		darkSwarmDamage(x, y, r, bullet, weaponId);
		//

		bool hasGuaranteedHitUnit = guaranteed_hit != nullptr;
		if (hasGuaranteedHitUnit && nathrokorCondition(bullet, bullet->attackTarget.unit)) {
			weaponBulletShot(guaranteed_hit, bullet, 1);
		}
		int divisor = 1;
		if (weaponId == WeaponId::ArchitectDispersiveSalvo) {
			divisor = Globals.get_divider(bullet);
		}
		for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
			CUnit* target = unitsInSplash.getUnit(i);
			if (hasGuaranteedHitUnit && target == guaranteed_hit) { // skips target check if hasGuaranteedHitUnit is unset, just for little bit of performance
				continue; // guaranteed hit unit was already damaged before splash!
			}

			// Units can't splash themselves -- except psi storm
			if (target == bullet->sourceUnit && bullet->weaponType != WeaponId::PsiStorm) continue;

			// Don't hit allied units -- normally just checks target->playerId == bullet->srcPlayer (also checks if type is 'splash enemy' or 'nuclear missile')
	//      if(scbw::isUnitEnemy(bullet->srcPlayer, target) == false && target != bullet->attackTarget.unit) continue;

			if (splash == SplashType::Enemy) {
				if (bullet->srcPlayer == target->playerId && target != bullet->attackTarget.unit) continue;
			}

			else if (splash == SplashType::FriendlyNew) {
				if (scbw::isUnitEnemy(bullet->srcPlayer, target) == false && target != bullet->attackTarget.unit) continue;
			}

			// Can the unit be targeted?
			if (!scbw::canWeaponTargetUnit(bullet->weaponType, target, target)) continue;

			// We found units in a square around the target, but is the target within the splash radius?
			int16 outer = weapons_dat::OuterSplashRadius[bullet->weaponType];

			// Probing Incisors - Gorgoleth passive
			if (isChargedGorgolethBullet(bullet, weaponId)) {
				outer = 45;
			}

			// Nathrokor weapon
			if (weaponId == WeaponId::NathrokorPlasmidBreath) {
				outer = 32;
			}

			if (weaponId == WeaponId::HeraclesJavelinCannons) {
				const double splashBase = weapons_dat::OuterSplashRadius[WeaponId::HeraclesJavelinCannons];
				const uint32 stacks = scbw::get_generic_value(bullet->sourceUnit, ValueId::NemeanReactorsStack);
				outer = (int16)(splashBase * (1 + 0.1 * stacks));
			}

			if (!isUnitDistanceWithin(target, bullet, outer)) continue;

			// Don't storm a unit that's already being stormed
			if (bullet->weaponType == WeaponId::PsiStorm) {
				if (target->isUnderStorm) continue;
				target->isUnderStorm = 1;
			}

			// Inner radius 
			int16 inner = weapons_dat::InnerSplashRadius[bullet->weaponType];

			// Probing Incisors - Gorgoleth passive
			if (isChargedGorgolethBullet(bullet, weaponId)) {
				inner = 30;
			}

			if (weaponId == WeaponId::HeraclesJavelinCannons) {
				const double splashBase = weapons_dat::InnerSplashRadius[WeaponId::HeraclesJavelinCannons];
				const uint32 stacks = scbw::get_generic_value(bullet->sourceUnit, ValueId::NemeanReactorsStack);
				inner = (int16)(splashBase * (1 + 0.1 * stacks));
			}

			if (isUnitDistanceWithin(target, bullet, inner) && nathrokorCondition(bullet, target)) {
				weaponBulletShot(target, bullet, divisor); // 100% damage
				continue;
			}

			// Burrowed units can only be hit by inner radius
			if (target->status & UnitStatus::Burrowed) continue;

			// Medium radius
			int16 medium = weapons_dat::MediumSplashRadius[bullet->weaponType];

			// Probing Incisors - Gorgoleth passive
			if (isChargedGorgolethBullet(bullet, weaponId)) {
				medium = 37;
			}

			if (weaponId == WeaponId::HeraclesJavelinCannons) {
				const double splashBase = weapons_dat::MediumSplashRadius[WeaponId::HeraclesJavelinCannons];
				const uint32 stacks = scbw::get_generic_value(bullet->sourceUnit, ValueId::NemeanReactorsStack);
				medium = (int16)(splashBase * (1 + 0.1 * stacks));
			}

			if (weaponId == WeaponId::NathrokorPlasmidBreath) {
				r = 32;
			}

			if (isUnitDistanceWithin(target, bullet, medium) && nathrokorCondition(bullet, target)) {
				weaponBulletShot(target, bullet, divisor * 2); // 50% damage
				continue;
			}

			// Outer radius
			if (nathrokorCondition(bullet, target)) {
				weaponBulletShot(target, bullet, divisor * 4); // 25% damage
			}
		}
	}

	void friendlyLurkerSplash(CBullet* bullet, uint32 weaponId, uint8 splash) {
		int16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::OuterSplashRadius[weaponId];

		// Get units within the outer splash radius
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);

		for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
			CUnit* target = unitsInSplash.getUnit(i);

			// Units can't splash themselves
			if (target == bullet->sourceUnit) continue;

			// Don't hit allied units -- normally just checks target->playerId == bullet->srcPlayer
		//     if (scbw::isUnitEnemy(bullet->srcPlayer, target) == false && target != bullet->attackTarget.unit) continue;
			if (splash == SplashType::Enemy) {
				if (bullet->srcPlayer == target->playerId && target != bullet->attackTarget.unit) continue;
			} else if (splash == SplashType::FriendlyNew) {
				bool lurkerOverride = false;
				if (bullet->sourceUnit && (CUnit*)scbw::get_generic_value(bullet->sourceUnit, ValueId::LurkerPickTarget) == target) {
					lurkerOverride = true;
				}
				if (scbw::isUnitEnemy(bullet->srcPlayer, target) == false && !lurkerOverride && target != bullet->attackTarget.unit) {
					continue;
				}
			}

			// Can the unit be targetted?
			if (!scbw::canWeaponTargetUnit(bullet->weaponType, target, target)) continue;

			// We found units in a square around the target, but is the target within the splash radius?
			if (!isUnitDistanceWithin(target, bullet, weapons_dat::InnerSplashRadius[bullet->weaponType])) continue;

			if (bullet->sourceUnit != nullptr) {
				/*
				bool cont = false; // supposed to be a return within the loops, but since we can't it's a bool
				for (int r = 0; r < 32 && !cont; r++) {
					for (int c = 0; c < 16; c++) {
					if (spineHitTable->hits[r][c].source == bullet->sourceUnit && spineHitTable->hits[r][c].target == target) {
						cont = true; // unit has already been hit
						break;
					}
					}
				}
				if (cont) continue;

				if (*spineHitCol != 16) {
					spineHitTable->hits[*spineHitRow][*spineHitCol].source = bullet->sourceUnit;
					spineHitTable->hits[*spineHitRow][*spineHitCol].target = target;
					(*spineHitCol)++;
				}*/
				bool cont = false;
				if (Globals.spine_pair_exists(bullet->sourceUnit, target)) {
					cont = true;
					break;
				}
				if (cont)
					continue;
				Globals.add_lurker_spine_pair(bullet->sourceUnit, target);
			}
			weaponBulletShot(target, bullet, 1);
		}
	}

	bool isChargedGorgolethBullet(CBullet* bullet, uint32 weaponId) {
		if (weaponId == WeaponId::GorgrokorSerratedCleavers) {
			if (bullet != nullptr && bullet->sourceUnit != nullptr && bullet->sourceUnit->id == UnitId::ZergGorgrokor) {
				//				if (bullet->sourceUnit->energy >= 5 * 256) {
				return true;
				//				}
			}
		}
		return false;
	}

	bool isUnsafeCyprianBullet(CBullet* bullet, uint32 weaponId) {
		if (weaponId == 26) {
			if (bullet != nullptr) {
				if (bullet->sourceUnit != nullptr) {
					if (scbw::get_aise_value(bullet->sourceUnit, nullptr, AiseId::SendCyprianValue, 0, 0) == 1) {
						return true;
					}
				}
			}
		}
		return false;
	}

	const uint32 Func_turnUnit = 0x00495F20;
	void turnUnit(CUnit* a1, char a2) {
		__asm {
			PUSHAD
			MOV EAX, a1
			MOVZX BL, a2
			CALL Func_turnUnit
			POPAD
		}
	}

	// clone of 'Normal' damage for reference
	void normalDamage(CBullet* bullet, uint32 weaponId) {
		// Create Lobotomy Mines
		if (bullet->sourceUnit != nullptr && weaponId == WeaponId::Lockdown) {
			CUnit* mine = scbw::createUnitIngame(UnitId::TerranLobotomyMine, bullet->srcPlayer, bullet->position.x, bullet->position.y);
			if (mine != nullptr) {
				turnUnit(mine, bullet->sourceUnit->currentDirection1);
			}
		}

		// 
		if (isChargedGorgolethBullet(bullet, weaponId)) {
			//			bullet->sourceUnit->spendUnitEnergy(5 * 256);
			friendlySplash(bullet, weaponId, SplashType::FriendlyNew, bullet->attackTarget.unit);
			return;
		}

		// 
		CUnit* target = bullet->attackTarget.unit;
		if (target == nullptr) {
			return;
		}

		// 
		darkSwarmDamage(bullet->attackTarget.unit->position.x, bullet->attackTarget.unit->position.y, 1, bullet, weaponId);

		// 
		uint8 divisor = 1;
		if (weaponId == WeaponId::ArchitectDispersiveSalvo) {
			divisor = Globals.get_divider(bullet);
		}

		// 
		else if (weaponId == WeaponId::KalkaliskViscousAcid) {
			//			divisor = Globals.get_multihit_count(bullet);
			divisor = 1;
			//			scbw::printFormattedText("Get divisor %d, bullet: %X",divisor,bullet);
		}

		// 
		else if (weaponId == WeaponId::AcidSpore
			&& bullet->sourceUnit
			&& bullet->sourceUnit->sprite
			&& target->sprite) {
			auto X = scbw::get_generic_value(bullet->sourceUnit, ValueId::GuardianMissileLaunchPosX);
			auto Y = scbw::get_generic_value(bullet->sourceUnit, ValueId::GuardianMissileLaunchPosY);
			if (X != 0 && Y != 0) {
				divisor = scbw::getDistanceFast(X, Y, target->getX(), target->getY()) / (32 * 3);
			}
		}

		// 
		if (bullet->hitFlags & 1) { // Miss ?
			if (target->pAI == nullptr) return;
			if (bullet->sourceUnit == nullptr) return;
			doWeaponHit(target, bullet->sourceUnit, divisor);
			return;
		}

		// 
		else {
			if (weapons_dat::Behavior[weaponId] == WeaponBehavior::Bounce) {
				bool mutalisk = true;
				bool bounces = true;
				if (bullet != nullptr && bullet->sourceUnit != nullptr) {
					if (bullet->sourceUnit->id == 129) {
						if (bullet->remainingBounces <= 1) {
							mutalisk = false;
						} else {
							bounces = false;
						}
					}
				}
				if (bounces) {
					if (mutalisk) {
						int bounceCount = 2;
						divisor = bounceCount - bullet->remainingBounces;
						/*
						for (int i = 2 - bullet->remainingBounces; i != 0; i--) {
							divisor *= 3;
						}*/
						//1-3-9  2-1-0 - old
						//1-2-3  2-1-0 - new
					} else {
						divisor *= 3;
					}
				}
			}
			weaponBulletShot(target, bullet, divisor);
		}
	}

	//  case WPNEffect_Splash_Air:
	void airSplash(CBullet* bullet, uint32 weaponId) { // 0048B790
		CUnit* target = bullet->attackTarget.unit;
		if (target == nullptr) {
			return;
		}
		int16 radius = weapons_dat::OuterSplashRadius[weaponId];
		CSprite* bulletSprite = bullet->sprite;
		int16 posX = bulletSprite->position.x;
		int16 posY = bulletSprite->position.y;
		int32 list;
		Box16* searchRadius;

		*airSplashListCount = 0;
		searchRadius->left = posX - radius;
		searchRadius->top = posY - radius;
		searchRadius->right = radius + posX;
		searchRadius->bottom = radius + posY;

		int16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::OuterSplashRadius[weaponId];
		// Get units within the outer splash radius
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);
		//
		darkSwarmDamage(x, y, r, bullet, weaponId);
		//
		int32 result;
		for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
			// airSplashProc
			int8 weaponIdLoop; // bl
			int32 listCount; // eax
			CUnit* target = unitsInSplash.getUnit(i);

			weaponIdLoop = bullet->weaponType;
			if (target != bullet->sourceUnit && (target->playerId != bullet->srcPlayer || target == bullet->attackTarget.unit)) {
				if (scbw::canWeaponTargetUnit(weaponIdLoop, target, target)) {
					if (isUnitDistanceWithin(target, bullet, weapons_dat::OuterSplashRadius[weaponIdLoop])) {
						if (isUnitDistanceWithin(target, bullet, weapons_dat::InnerSplashRadius[weaponIdLoop])
							&& target == bullet->attackTarget.unit) {
							if (Globals.extended_unit_table) {
								new_unit_table.air_splash_results[0] = target;
							} else {
								airSplashList[0] = target;
							}
							result = 1;
							*airSplashListCount = 1;
							break;
						}
						listCount = *airSplashListCount;
						if (Globals.extended_unit_table) {
							new_unit_table.air_splash_results[*airSplashListCount] = target;
						} else {
							airSplashList[*airSplashListCount] = target;
						}
						*airSplashListCount = listCount + 1;
					}
				}
			}
		}
		if (airSplashListCount) {
			if (*airSplashListCount == 1) {
				list = 0;
			} else {
				list = randomizeShort(58) % *airSplashListCount;
			}
			if (Globals.extended_unit_table) {
				weaponBulletShot(target, bullet, (int32)new_unit_table.air_splash_results[list]);
			} else {
				weaponBulletShot(target, bullet, (int32)airSplashList[list]);
			}

		}
		// airSplashProc2
		friendlySplash(bullet, weaponId, SplashType::FriendlyNew);
	}

	// clone of 'Yamato' damage for reference
	void yamatoDamage(CBullet* bullet, uint32 weaponId) {
		if (bullet->attackTarget.unit == nullptr) return;
		darkSwarmDamage(bullet->attackTarget.unit->position.x, bullet->attackTarget.unit->position.y, 1, bullet, weaponId);
		weaponBulletShot(bullet->attackTarget.unit, bullet, 1);
	}

	void parasite(CBullet* bullet, uint32 weaponId) {
		if (bullet->sourceUnit == nullptr) return;
		constexpr uint32 radius = 96;
		scbw::UnitFinder(
			bullet->position.x - radius / 2,
			bullet->position.y - radius / 2,
			bullet->position.x + radius / 2,
			bullet->position.y + radius / 2
		).forEach([&](CUnit* unit) {
			scbw::applyParasite(unit, bullet->sourceUnit->playerId, bullet->sourceUnit);
		});
	}

	void broodlings(CBullet* bullet, uint32 weaponId) {
		if (bullet->sourceUnit == nullptr) return;
		CUnit* target = bullet->attackTarget.unit;
		// if (target != nullptr) {
		//	  if (!target->isDead()) {
	//			  Ai_UnitWasHit(1, 0, bullet->sourceUnit, target);
				 // BroodlingHit(bullet->sourceUnit, target,bullet);
		hooks::BroodlingHit(bullet->sourceUnit, target, bullet);
		//	  }
		//  }
	}

	void lockdown(CBullet* bullet, uint32 weaponId) {
		/*	CUnit* target = bullet->attackTarget.unit;
			//original code
			if (target) {
				if (!target->isDead()) {
					hooks::LockdownHit(target,0x83,bullet);
				}
			}*/
		auto targPos = bullet->targetPosition;
		int16 x, y, r;
		x = bullet->sprite->position.x;
		y = bullet->sprite->position.y;
		r = weapons_dat::OuterSplashRadius[WeaponId::LobotomyMineExplosion];
		scbw::UnitFinder unitsInSplash(x - r, y - r, x + r, y + r);
		for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
			CUnit* target = unitsInSplash.getUnit(i);
			if (!target->isDead()
				&& !units_dat::GroupFlags[target->id].isBuilding
				&& !(units_dat::BaseProperty[target->id] & UnitProperty::ResourceContainer)
				&& target->id != UnitId::TerranSpiderMine
				&& target->id != UnitId::TerranLobotomyMine) {
				if (units_dat::BaseProperty[target->id] & UnitProperty::Organic) {
					scbw::set_generic_timer(target, ValueId::LobotomySlow, 6 * 24);
				} else {
					hooks::LockdownHit(target, 18, bullet);
				}
			}
		}
	}
}//hooks

namespace {
	/*** helper functions that I couldn't find in gptp ***/
	const uint32 Func_0043F320 = 0x0043F320;
	void Ai_UnitWasHit(uint32 main_target_reaction, uint32 dont_call_help, CUnit* attacker, CUnit* target) {
		__asm {
			PUSHAD
			PUSH main_target_reaction
			PUSH dont_call_help
			MOV ECX, target
			MOV EDX, attacker
			CALL Func_0043F320
			POPAD
		}
	}

	// checks if the distance between target and bullet's positions is < radius
	const uint32 Helper_IsUnitDistanceWithin = 0x0048ADB0;
	bool isUnitDistanceWithin(CUnit* target, CBullet* bullet, uint32 radius) {

		uint32 result;
		__asm {
			PUSHAD
			PUSH radius;
			PUSH bullet;
			MOV ECX, target
				CALL Helper_IsUnitDistanceWithin
				MOV result, EAX
				POPAD
		}
		return result;
	}

	// creates attack overlay & deals damage from the bullet (just calls overlayAndNotify if the weapon deals no damage, or calls target->damageWith if it does
	const uint32 Helper_WeaponBulletShot = 0x00479AE0;
	void weaponBulletShot(CUnit* target, CBullet* bullet, uint32 dmgDivisor) {

		if (bullet != nullptr && target != nullptr && bullet->sourceUnit && bullet->weaponType == WeaponId::PariahIconoclastBlades) {
			if (!Globals.was_hit_by_pariah(bullet->sourceUnit, target)) {
				Globals.register_pariah_hit(bullet->sourceUnit, target);
			} else {
				return;
			}
		}
		__asm {
			PUSHAD
			PUSH dmgDivisor
			MOV EAX, target
			MOV EDX, bullet
			CALL Helper_WeaponBulletShot
			POPAD
		}
	}

	const uint32 Helper_DoWeaponHit = 0x004795D0;
	void doWeaponHit(CUnit* target, CUnit* attacker, uint8 dmgDivisor) {

		uint32 divisor = dmgDivisor;
		__asm {
			PUSHAD
			PUSH divisor
			PUSH attacker
			MOV EBX, target
			CALL Helper_DoWeaponHit
			POPAD
		}
	}

	// for airSplash

	const uint32 Func_findBestUnit = 0x004E8830;
	CUnit* findBestUnit(Box16* radius, int32 bullet, int32(__fastcall* func)(int32, int32)) {
		static CUnit* result;
		__asm {
			PUSHAD
			MOV EAX, radius
			MOV EBX, bullet
			MOV EDI, func
			CALL Func_findBestUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_randomizeShort = 0x004DC4A0;
	int16 randomizeShort(int32 counts) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, counts
			CALL Func_randomizeShort
			MOV result, EAX
			POPAD
		}
		return (int16)result;
	}

	const uint32 Func_splashFunc = 0x0048B570;
	int32 splashFunc(CBullet* bullet, CUnit* unit) {
		static int32 result;

		__asm {
			PUSHAD
			MOV EDI, bullet
			MOV ESI, unit
			CALL Func_splashFunc
			MOV result, EAX
			POPAD
		}
		return result;
	}

}//unnamed namespace

