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
	uint16 getUnitStrength(CUnit* unit, bool useGroundStrength);

	/// Definition of damage factors (explosive, concussive, etc.)
	struct {
		int32 damageType;
		int32 unitSizeFactor[4];	 //Order: {independent, small, medium, large}
	} const damageFactor[5] = {
		{0, 0, 0,	0,	 0},			//Independent
		{1, 0, 128, 192, 256},			//Explosive
		{2, 0, 256, 192, 128},			//Concussive
		{3, 0, 256, 256, 256},			//Normal
		{4, 0, 256, 256, 256}			//IgnoreArmor
	};

	CUnit* createHallucination(CUnit* target, uint32 playerId);
	bool placeHallucination(CUnit* unit);
	void unitDestructor(CUnit* unit);

	// For updateCorrosiveSpores
	uint32 getCorrosiveSporesImage(CUnit* unit); // 004F41F0

}

//copied from wpnspellhit and renamed
const uint32 Func_displayLastNetErrForPlayer = 0x0049E530;
void displayneterr_49E530(uint32 playerId) {
	__asm {
		PUSHAD
		PUSH playerId
		CALL Func_displayLastNetErrForPlayer
		POPAD
	}
}

const uint32 Func_UpdateUnitStrength = 0x0049FA40;
void updateStrength_49FA40(CUnit* unit) {
	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_UpdateUnitStrength
		POPAD
	}
}

const uint32 Func_Sub4A01F0 = 0x004A01F0;
void function_004A01F0_copy(CUnit* unit) {
	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub4A01F0
		POPAD
	}
}

const uint32 Func_497ED0 = 0x00497ED0;
void makeSpriteVisible(CSprite* sprite) {
	__asm {
		PUSHAD
		MOV EAX, sprite
		CALL Func_497ED0
		POPAD
	}
}

const uint32 Func_AI_TrainingUnit = 0x004A2830;
void AI_Train_4A2830(CUnit* unit_creator, CUnit* created_unit) {
	__asm {
		PUSHAD
		MOV EAX, created_unit
		MOV ECX, unit_creator
		CALL Func_AI_TrainingUnit
		POPAD
	}
}

const uint32 Func_CreateUnit = 0x004A09D0;
CUnit* CreateUnit_09D0(uint32 unitId, int x, int y, uint32 playerId) {
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
const uint32 Func_CreateThingy = 0x00488210;
CThingy* function_00488210(uint32 spriteId, int16 x, int16 y, uint32 playerId) {
	static CThingy* thingy;
	int32 x_ = x;
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

const uint32 Func_SetThingyVisibilityFlags = 0x004878F0;
bool function_004878f0(CThingy* thingy) {
	static bool32 bPreResult;
	__asm {
		PUSHAD
		MOV ESI, thingy
		CALL Func_SetThingyVisibilityFlags
		MOV bPreResult, EAX
		POPAD
	}
	return (bPreResult != 0);
}

const uint32 Func_Sub498D70 = 0x00498D70;
void imgul(CSprite* sprite, uint32 imageId, uint32 unk1, uint32 unk2, uint32 unk3) {
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

#include "SCBW\UnitFinder.h"

bool irradiateTarget(CUnit* unit) {
	return true; // Originally prohibited targeting eggs
}

void augurSacrifice(CUnit* target) {
	if (scbw::get_generic_value(target, ValueId::AugurSafetyCheck) == 0) {
		scbw::set_generic_value(target, ValueId::AugurSafetyCheck, 1);
		const int radius = 6 * 32;
		CUnit* nearest = scbw::UnitFinder::getNearestTarget(
			target->getX() - radius,
			target->getY() - radius,
			target->getX() + radius,
			target->getY() + radius,
			target, [&](CUnit* unit) {
			//Don't attack friendly / allied units
			if (unit->sprite == nullptr) {
				return false;
			}
			if (unit->playerId == target->playerId) {
				return false;
			}
			if (scbw::isAlliedTo(unit->playerId, target->playerId)) {
				return false;
			}
			if (unit->status & UnitStatus::Invincible) {
				return false;
			}
			return true;
		});
		if (nearest != nullptr) {
			target->orderTarget.unit = nearest;
			scbw::createBullet(WeaponId::AugurFinalHour, target, target->position.x, target->position.y, target->playerId,
				scbw::getAngle(target->position.x, target->position.y, nearest->getX(), nearest->getY()));
			scbw::hideAndDisableUnit(target);
			target->status |= UnitStatus::Invincible;
			scbw::createThingy(SpriteId::AugurDeathExplosion, target->getX(), target->getY(), target->playerId);
			scbw::playSound(scbw::randBetween(SoundId::AugurDeath01, SoundId::AugurDeath04), target);
			target->removeTimer = 5 * 32;
			target->userActionFlags |= 0x4;
			return;
		} else {
			target->remove();
		}
	}
}

namespace hooks {
	static void weaponDamage_damageModifiers(WeaponDamageData& data);
	static void weaponDamage_lifesteal(WeaponDamageData& data);
	static void weaponDamage_behaviors(WeaponDamageData& data);
	static void weaponDamage_fatal(WeaponDamageData& data);

	/// Hooks into the CUnit::damageWith() function.
	void weaponDamageHook(
		int32 damage,
		CUnit* target,
		uint8 weaponId,
		CUnit* attacker,
		uint8 attackingPlayerId,
		int8 direction,
		uint8 damageDivisor
	) {
		WeaponDamageData data = {
			damage,
			damage,
			0,
			weapons_dat::DamageType[weaponId],
			damageDivisor,
			weaponId,
			direction,
			target,
			attacker,
			attackingPlayerId,
			false,
			false
		};
		weaponDamage_root(data);
	}

	void weaponDamage_root(WeaponDamageData& data) {
		// The unit must neither be already dead nor invincible
		if (data.target == nullptr || data.target->hitPoints == 0 || data.target->status & UnitStatus::Invincible) {
			return;
		}

		// Demiurge - Gods Made Flesh - target is impervious to damage
		if (scbw::get_generic_timer(data.target, ValueId::GodsMadeFlesh) > 0) { // Power Overwhelming used to be here :wave_me_dad:
			data.finalDamage = 0;
		}

		bool canDamage = true; // debug only!
		// Ai_ReactToHit crash - null town
		if (data.target->pAI != nullptr) {
			// Ai_ReactToHit crash - null town
			WorkerAi* ai = (WorkerAi*)data.target->pAI;
			if (ai->type == 2 || ai->type == 3) {
				if (ai->town == nullptr) {
					scbw::printf("Ai_ReactToHit crash - null town! Attacker id %d, Attacker player %d, Target id %d, Target player %d",
						data.attacker->id, data.attacker->playerId, data.target->id, data.target->playerId);
					canDamage = false;
				}
			}
		}

		if (!data.dontProcPhaseLink) { // damage from phase link was already calculated
			weaponDamage_damageModifiers(data);
		}

		// Hallucinations deal half damage
		if (data.attacker != nullptr) {
			if (data.attacker->status & UnitStatus::IsHallucination) {
				data.finalDamage /= 2;
			}
		}

		// Minimum amount of damage that can be dealt
		if (data.finalDamage < 128) {
			data.finalDamage = 128;
		}

		// Reduce Defensive Matrix
		if (data.target->defensiveMatrixHp != 0) {
			int32 d_matrix_reduceAmount;
			if (data.target->defensiveMatrixHp > data.finalDamage)
				d_matrix_reduceAmount = data.finalDamage;
			else
				d_matrix_reduceAmount = data.target->defensiveMatrixHp;
			data.finalDamage -= d_matrix_reduceAmount;
			data.target->reduceDefensiveMatrixHp(d_matrix_reduceAmount);
		}

		// Cliff Advantage - Armor
		if (data.damageType != DamageType::IgnoreArmor && !data.ignoreFlatArmor) {
			data.finalDamage -= std::min(scbw::getCliffAdvantage(data.attacker), 0) * -256; // not passing weapon allows for melee, which is desirable with armor
		}

		// Reduce Plasma Shields
		if (units_dat::ShieldsEnabled[data.target->id] && data.target->shields >= 256) {
			if (data.damageType != DamageType::IgnoreArmor && !data.ignoreFlatArmor) {
				int32 plasmaShieldUpg = scbw::getUpgradeLevel(data.target->playerId, UpgradeId::ProtossPlasmaShields) * 256;
				if (data.finalDamage > plasmaShieldUpg) {
					data.finalDamage -= plasmaShieldUpg;
				} else {
					data.finalDamage = 128;
				}
			}
			data.shieldReduceAmount = std::min(data.finalDamage, data.target->shields);
			data.finalDamage -= data.shieldReduceAmount;
		}

		// Apply armor
		bool shield_overlay = false;
		if (data.damageType != DamageType::IgnoreArmor && !data.ignoreFlatArmor) {
			int32 armorTotal = data.target->getArmor() * 256;
			if (data.attacker != nullptr) {
				armorTotal += scbw::get_aise_value(data.target, nullptr, AiseId::GetShepherds, 0, 0) * 256;
				if (data.target->id == UnitId::TerranMinotaur) {
					uint16 angle = scbw::getAngle(data.target->position.x, data.target->position.y,
						data.attacker->position.x, data.attacker->position.y);
					uint16 unit_angle = data.target->velocityDirection1;
					int arc = (double)45 / 1.40625;	//90 degrees
					int delta = unit_angle - angle;
					delta = abs((delta + 128) % 256 - 128);
					if (delta <= arc) {
						armorTotal += 512;
						shield_overlay = true;
					}
				}
			}
			data.finalDamage -= std::min(data.finalDamage, armorTotal);
			auto rend = 256 * scbw::get_generic_value(data.target, ValueId::ObservanceRend);
			data.finalDamage += rend;
		}

		// Apply damage type/unit size factor
		int32 damage_factor = damageFactor[data.damageType].unitSizeFactor[units_dat::SizeType[data.target->id]];
		if (data.attacker != nullptr) {
			switch (data.attacker->id)
			case UnitId::TerranHarakan:
			{
				if (units_dat::BaseProperty[data.target->id] & UnitProperty::Building) {
					damage_factor = 256;
				}
				break;
			}
		}

		data.finalDamage = (data.finalDamage * damage_factor) / 256;
		if (data.shieldReduceAmount == 0 && data.finalDamage < 128) {
			data.finalDamage = 128;
		}

		// Never bleed damage from phase link to hp
		if (data.dontProcPhaseLink) {
			data.finalDamage = 0;
		}

		weaponDamage_behaviors(data);
		if (data.attacker != nullptr && !data.dontProcPhaseLink) { // lifesteal 
			weaponDamage_lifesteal(data);
		}
		if (data.finalDamage >= data.target->hitPoints) { // <- keep this as hitPoints, because the damage is already calculated to include armor, shields and damage factor calculations
			weaponDamage_fatal(data);
		}

		// Deal damage, except for Independent damage, which always deals 0
		if (data.damageType != DamageType::Independent && data.target->defensiveMatrixHp == 0 && canDamage) {
			data.target->damageHp(data.finalDamage, data.attacker, data.attackingPlayerId, data.weaponId != WeaponId::Irradiate);
			// Prevent Seraphs from being continuously revealed to the irradiated target
		}

		// Reduce shields
		if (data.shieldReduceAmount != 0 && canDamage) {
			if (scbw::hasClarionLinks(data.target) && !data.dontProcPhaseLink) {
				scbw::spreadClarionDamage(data);
			} else {
				data.target->shields -= data.shieldReduceAmount;
				if (data.damageType != DamageType::Independent && data.target->shields != 0) {
					shield_overlay = true;
				}
			}
		}

		if (shield_overlay) {
			scbw::createShieldOverlay(data.target, data.direction);
		}

		//Update unit strength data
		data.target->airStrength = getUnitStrength(data.target, false);
		data.target->groundStrength = getUnitStrength(data.target, true);
	}

	static void weaponDamage_damageModifiers(WeaponDamageData& data) {
		// Dark Swarm - Defiler Ability
		if (scbw::isUnderDarkSwarm(data.target) && weapons_dat::MaxRange[data.weaponId] > 32 && (data.damageDivisor == 1 || data.weaponId == WeaponId::AcidSpore) && data.weaponId != WeaponId::SubterraneanSpines) {
			data.damageType = DamageType::Independent;
		}

		switch (data.weaponId) {
			case WeaponId::ArchitectDispersiveSalvo:
			{
				// Reactive Payload - Architect behavior
				double div = 1.0;
				if (data.damageDivisor != 0) {
					div = (double)data.damageDivisor;
				}
				int dmg = ((double)data.finalDamage / div);
				data.finalDamage = dmg + scbw::get_generic_value(data.target, ValueId::CorrosiveTouchStacks * VithriliskCorrosiveTouchArmorReduction);
				break;
			}
			case WeaponId::AcidSpore:
			{
				// Searing Acid - Guardian behavior
				data.finalDamage += (scbw::get_generic_value(data.target, ValueId::CorrosiveTouchStacks) * VithriliskCorrosiveTouchArmorReduction) + (data.damageDivisor * 10 * 256);
				break;
			}
			case WeaponId::GlaveWurm:
			{
				// Lithe Organelle - Mutalisk bounce damage
				double mul = 1.0;
				switch (data.damageDivisor) {
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
				data.finalDamage *= mul;
				data.finalDamage += (int)scbw::get_generic_value(data.target, ValueId::CorrosiveTouchStacks) * VithriliskCorrosiveTouchArmorReduction;
				break;
			}
			case WeaponId::KalkaliskViscousAcid:
			{
				// Kalkalisk behavior
				int decrement = data.damageDivisor * 256;
				if (decrement < data.damage) {
					data.finalDamage = data.finalDamage - decrement + (scbw::get_generic_value(data.target, ValueId::CorrosiveTouchStacks) * VithriliskCorrosiveTouchArmorReduction);
				} else {
					data.finalDamage = data.finalDamage + (scbw::get_generic_value(data.target, ValueId::CorrosiveTouchStacks) * VithriliskCorrosiveTouchArmorReduction);
				}
				break;
			}
			case WeaponId::DemiurgeGalacticWeaver:
			{
				// Demiurge - Worship
				if (data.target->getCurrentHpInGame() <= (data.target->getMaxHpInGame() / 2)) {
					// TODO: implement unit transfer
					scbw::set_generic_value(data.target, ValueId::WorshipStack, 1);
					scbw::set_generic_unit(data.target, ValueId::WorshipDemiurge, data.attacker);
					scbw::set_generic_value(data.target, ValueId::WorshipOriginalPlayer, data.target->playerId);
				}
			}
			default:
			{
				uint32 divisor = data.damageDivisor;
				if (divisor == 0) {
					divisor = 1;
					scbw::printf("Incorrect weapon hit %d", data.weaponId);
				}
				data.finalDamage = data.finalDamage / divisor + (scbw::get_generic_value(data.target, ValueId::CorrosiveTouchStacks) * VithriliskCorrosiveTouchArmorReduction);
			}
		}

		switch (data.weaponId) {
			case WeaponId::QuaziliskAcridBarbs:
			{
				// Quazilisk - Melting Points
				float missingFactor = 1 - ((float)data.target->hitPoints / data.target->getMaxHpRaw());
				float damageIncrease = floor(missingFactor / QuazaliskPassiveFactor);
				//scbw::printString("Damage Increase: " + std::to_string(damageIncrease));
				data.finalDamage += damageIncrease * HealthMultiplier;
				break;
			}
			case WeaponId::AlmaksaliskPiercingSpines:
			{
				// Almaksalisk - Flanking Spines
				if (scbw::isDebuffed(data.target)) {
					data.damageType = DamageType::IgnoreArmor;
				}
				break;
			}
		}
	}

	static void weaponDamage_behaviors(WeaponDamageData& data) {
		if (data.attacker != nullptr) {
			// Vithrilisk - Critical Mass (procced by any attack now)
			if (scbw::get_generic_value(data.target, ValueId::CorrosiveTouchStacks)) {
				int range = 160;
				scbw::UnitFinder(
					data.target->getX() - 96,
					data.target->getY() - 96,
					data.target->getX() + 96,
					data.target->getY() + 96
				).forEach([&](CUnit* target) {
					if (target->status & UnitStatus::Burrowed) {
						return;
					}
					if (target->status & UnitStatus::Invincible) {
						return;
					}
					scbw::ApplySpores(target);
					CThingy* thingy = scbw::createThingy(SpriteId::Corrosive_Acid_Hit, target->getX(), target->getY(), 11);
					if (thingy != nullptr) {
						thingy->sprite->elevationLevel = target->sprite->elevationLevel + 1;
						scbw::setThingyVisibilityFlags(thingy);
					}
					//target->damageWith(30 * 256, WeaponId::PsiStorm, unit, unit->playerId);
				});
			}

			// Cabalist - Decloak, Silent Kill
			if (data.attacker->id == UnitId::ProtossCabalist && data.finalDamage < data.target->hitPoints) {
				scbw::decloakProc(data.attacker);
			}

			// Cantavis - Hallucinating
			if (scbw::get_generic_timer(data.target, ValueId::HallucinatingTimer) >= 1) {
				CUnit* haluAttacker = data.attacker;
				if ((haluAttacker->id == UnitId::ProtossSteward
					|| haluAttacker->id == UnitId::ProtossPlasmaShell)
					&& data.attacker->interceptor.parent != nullptr) {
					haluAttacker = data.attacker->interceptor.parent;
				}
				if (scbw::get_generic_unit(haluAttacker, ValueId::HallucinationPointer) == nullptr
					&& !(haluAttacker->status & UnitStatus::IsHallucination)) {
					uint32 haluPlayer = scbw::get_generic_value(data.target, ValueId::HallucinatingSourcePlayer);
					if (haluPlayer != haluAttacker->playerId && !scbw::isAlliedTo(haluPlayer, haluAttacker->playerId)) return;

					CUnit* hal_unit = createHallucination(haluAttacker, haluPlayer);
					if (hal_unit != nullptr) {
						hal_unit->sprite->createOverlay(ImageId::HallucinationDeath3);
						scbw::set_generic_unit(haluAttacker, ValueId::HallucinationPointer, hal_unit);

						if (!placeHallucination(hal_unit)) {
							unitDestructor(hal_unit);
						}
					}
				}
			}

			// Petrification - Gorgon behavior
			if (GorgonCondition(data.attacker, data.target)) {
				scbw::refreshDebuffs(data.target);
			}

			// Ensnaring Brood - Queen enchantment ability
			if (scbw::get_generic_timer(data.attacker, ValueId::EnsnaringBroodTimer) > 0
				&& !(units_dat::BaseProperty[data.target->id] & UnitProperty::Building)) {
				applyEnsnare(data.target);
			}

			// Phase Rush - Manifold behavior
			if (units_dat::BaseProperty[data.attacker->id] & UnitProperty::RoboticUnit) {
				scbw::set_generic_timer(data.target, ValueId::PhaseRushTimerArrayStart + data.attacker->playerId, 5 * 24);
			}

			switch (data.target->id) {
				case UnitId::ProtossSimulacrum:
				{
					// Autovitality - Simulacrum behavior
					if (!scbw::isAlliedTo(data.target->playerId, data.attacker->playerId)
						&& scbw::get_generic_timer(data.target, ValueId::SimulacrumCloneTimer) == 0) {
						scbw::set_generic_timer(data.target, ValueId::SimulacrumCloneTimer, 5 * 24);
					}
					break;
				}
			}

			switch (data.weaponId) {
				case WeaponId::Irradiate:
				{
					// Seraph - Irradiate Spread
					if (data.attacker->irradiateTimer) {
						ApplyIrradiate(data.target, data.attackingPlayerId, data.attacker->irradiateTimer);
					}
					break;
				}
				case WeaponId::KonviliskSowingSpines:
				{
					// Fertile Spines - Konvilisk behavior
					/*if (data.target->getCurrentHpInGame() < (data.target->getMaxHpInGame() / 2))*/
					scbw::applyParasite(data.target, data.attacker->playerId, data.attacker);
					break;
				}
				case WeaponId::PazuzuJupiterDriver:
				{
					// Deconstruction - Pazuzu behavior - apply stack
					if (!(scbw::unitIsPacifist(data.target))
						&& scbw::get_generic_value(data.target, ValueId::Deconstruction) == 0) {

						scbw::set_generic_value(data.target, ValueId::Deconstruction, 1);
						scbw::set_generic_value(data.target, ValueId::DeconstructionPlayer, data.attacker->playerId);
						int32 overlayOffset = scbw::getOverlayOffset(data.target->id);
						if (data.target->subunit != nullptr) {
							scbw::set_generic_value(data.target->subunit, ValueId::Deconstruction, 1);
							scbw::set_generic_value(data.target->subunit, ValueId::DeconstructionPlayer, data.attacker->playerId);
							data.target->subunit->sprite->createOverlay(1332 + overlayOffset);
						} else {
							data.target->sprite->createOverlay(1332 + overlayOffset);
						}
					}
					break;
				}
				case WeaponId::SavantDiscoidRailgun:
				{
					// Savant Entropy Gauntlets behavior - Apply timer and overlay
					if (units_dat::BaseProperty[data.target->id] & UnitProperty::Spellcaster) {
						scbw::set_generic_timer(data.target, ValueId::EntropyGauntlets, (5 * 24) + 1);
						scbw::set_generic_unit(data.target, ValueId::DecaySavant, data.attacker);
						uint32 overlayImageId;
						if (units_dat::BaseProperty[data.target->id] & UnitProperty::LargeOverlay)
							overlayImageId = 1109;
						else if (units_dat::BaseProperty[data.target->id] & UnitProperty::MediumOverlay)
							overlayImageId = 1108;
						else
							overlayImageId = 1107;
						if (data.target->subunit != nullptr) {
							data.target->subunit->sprite->removeOverlay(overlayImageId);
							data.target->subunit->sprite->createTopOverlay(overlayImageId, 0, 0, 0);
						} else {
							data.target->sprite->removeOverlay(overlayImageId);
							data.target->sprite->createTopOverlay(overlayImageId, 0, 0, 0);
						}
					}
					break;
				}
				case WeaponId::Claws:
				{
					// Adrenal Glands - Zergling behavior
					if (scbw::get_generic_value(data.attacker, ValueId::AdrenalStacks) < 4) {
						scbw::add_generic_value(data.attacker, ValueId::AdrenalStacks, 1);
					}
					scbw::set_generic_timer(data.attacker, ValueId::AdrenalTimer, 49); //49 is 2 seconds (48) + 1
					break;
				}
				case WeaponId::SouthpawUppercutLaser:
				{
					// Knockout Drivers - Southpaw behavior
					if (scbw::get_generic_value(data.target, ValueId::KnockoutDriversStacks) < 20) {
						scbw::add_generic_value(data.target, ValueId::KnockoutDriversStacks, 1);
					}
					scbw::set_generic_timer(data.target, ValueId::KnockoutDriversTimer, 49); //49 is 2 seconds (48) + 1
					data.target->updateSpeed();
					break;
				}
				case WeaponId::MadrigalImperiosoMissiles:
				{
					// Dirge - Madrigal behavior
					scbw::sendGPTP_aise_cmd(data.target, nullptr, GptpId::DirgeImpale, 24, 0, 0);
					scbw::set_generic_rust_unit(data.target, ValueId::DirgeAttacker, data.attacker);
					break;
				}
				case WeaponId::VorvalingClaws:
				{
					// Captivating Claws - Vorvaling behavior
					captivatingClawsApply(data.attacker, data.target);
					break;
				}
				case WeaponId::EcclesiastAstralBlaster:
				{
					// Astral Blessing - Ecclesiast behavior
					scbw::add_generic_value(data.target, ValueId::EcclesiastStack, 1);
					scbw::set_generic_timer(data.target, ValueId::EcclesiastTimer, (24 * 5) + 1);
					scbw::set_generic_value(data.target, ValueId::EcclesiastPlayer, data.attacker->playerId);
					break;
				}
				case WeaponId::SubterraneanTentacle:
				case WeaponId::SeekerSpores:
				{
					if (scbw::get_generic_value(data.attacker, ValueId::Amitosis) == 1) {
						// data.damage /= 2; // TODO: Fix compiler error
						scbw::set_generic_value(data.attacker, ValueId::Amitosis, 0);
					}
					break;
				}
				case WeaponId::NuclearMissile:
				{
					// Nuclear Missile - Ground Zero
					if (data.target->hitPoints > data.finalDamage && irradiateTarget(data.target)) {
						hooks::IrradiateHit(data.attacker, data.target, data.attacker->playerId);
						// Create radiation fields here - thinking a set of 3 or 4, offset in some sort of pattern
					}
					break;
				}
				case WeaponId::SeraphObservance:
				{
					// Observance - Science Vessel ability
					if (data.weaponId == WeaponId::SeraphObservance
						&& (!scbw::isAlliedTo(data.attacker->playerId, data.target->playerId))) {
						scbw::set_generic_timer(data.attacker, ValueId::ObservanceOverlayTimer, 2);
						scbw::set_generic_timer(data.target, ValueId::ObservanceTimerDebuff, 1 + (5 * 24));
						scbw::set_generic_timer(data.target, ValueId::ObservanceTimerPeriod, 1 + (5 * 24));
						int rend = scbw::get_generic_value(data.target, ValueId::ObservanceRend);
						if (rend == 0) {
							int overlay = ImageId::ObservanceOverlaySmall - scbw::getOverlayOffset(data.target->id);
							data.target->sprite->createOverlay(overlay);
						}
						scbw::add_generic_value(data.target, ValueId::ObservanceRend, 1);
					}
					break;
				}
				case WeaponId::GorgrokorSerratedCleavers:
				{
					// Essence Drain - Gorgoleth Energy Steal
					if (units_dat::BaseProperty[data.target->id] & UnitProperty::Spellcaster) {
						uint16 energy = std::min(data.target->energy, (uint16)(data.finalDamage + data.shieldReduceAmount));
						data.target->energy -= energy;
						data.attacker->energy += energy;
					}
					break;
				}
				case WeaponId::MagisterNeutronCannon:
				{
					// Stellar Enforcement - Magister behavior
					if (!(scbw::get_generic_value(data.target, ValueId::StellarEnforcement))) {
						scbw::add_generic_value(data.target, ValueId::StellarEnforcement, 1);
					}
					break;
				}
				case WeaponId::ApostleCharonRifle:
				{
					// Blood Tithe - Apostle behavior
					uint32 range = 3 * 32;
					scbw::UnitFinder unitFinder(data.target->getX() - range, data.target->getY() - range, data.target->getX() + range, data.target->getY() + range);
					unitFinder.forEach([&data](CUnit* foundUnit) {
						if (!(scbw::isAlliedTo(data.attacker->playerId, foundUnit->playerId)) // Return if recipient is not allied, or if recipient is the target
							|| foundUnit == data.target) {
							return;
						}
						uint32 value = 0;
						value = ApostleBloodTitheHealing;
						data.target->heal(value, data.target);
					});
					break;
				}
			}
		}
	}

	static void weaponDamage_lifesteal(WeaponDamageData& data) {
		uint32 lifesteal = data.finalDamage + data.shieldReduceAmount;
		switch (data.weaponId) {
			case WeaponId::KaiserBlades:
			{
				// Ultrakor Saber Siphon passive
				lifesteal *= 0.5;
				data.attacker->heal(lifesteal, data.attacker);
				break;
			}
			case WeaponId::WendigoFusionSaw:
			{
				// Wendigo Automedicus passive (HP component)
				if (!(units_dat::BaseProperty[data.target->id] & UnitProperty::Mechanical))
					return;
				lifesteal *= 0.25;
				data.attacker->heal(lifesteal, data.attacker);
				break;
			}
			case WeaponId::AmaranthPsiCleaver:
			{
				// Eternal Service - Amaranth behavior
				const int32 hp = data.attacker->hitPoints / 256;
				const int32 maxHp = data.attacker->getMaxHpInGame();
				lifesteal *= MathUtils::map(hp, maxHp / 4, maxHp, 0.5, 0.2, true);
				data.attacker->healShields(lifesteal, data.attacker);
				break;
			}
		}
	}

	static void weaponDamage_fatal(WeaponDamageData& data) {
		{
			bool Halluc = false;
			if (data.attacker != nullptr) {
				if (data.attacker->status & UnitStatus::IsHallucination) {
					Halluc = true;
				}
			}

			// Cantavis - Ascendancy
			if (data.weaponId == WeaponId::PsiStorm || Halluc) {
				//psi mastery effect
				scbw::UnitFinder(
					data.target->getX() - 288,
					data.target->getY() - 288,
					data.target->getX() + 288,
					data.target->getY() + 288
				).forEach([&](CUnit* unit) {
					if (unit->id != UnitId::ProtossCantavis) {
						return;
					}
					if (unit->status & UnitStatus::DoodadStatesThing ||
						unit->lockdownTimer != 0 ||
						unit->stasisTimer != 0 ||
						unit->maelstromTimer != 0) {
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
				});
			}
		}

		// Apostle - Lazarus Agent
		if (scbw::get_generic_value(data.target, ValueId::LazarusAgent) == 1) {
			globals.initLazarusAgent(data.target->position.x, data.target->position.y,
				data.target->playerId, data.target->id, data.target->energy,
				(CUnit*)scbw::get_generic_value(data.target, ValueId::LazarusAgentApostleSrc));
		}

		// Ecclesiast - Astral Blessing proc
		if (scbw::get_generic_value(data.target, ValueId::EcclesiastStack)) {
			int stacks = scbw::get_generic_value(data.target, ValueId::EcclesiastStack);
			int player = scbw::get_generic_value(data.target, ValueId::EcclesiastPlayer);
			const int range = 64;
			scbw::UnitFinder(
				data.target->getX() - range,
				data.target->getY() - range,
				data.target->getX() + range,
				data.target->getY() + range
			).forEach([&](CUnit* unit) {
				if (player != unit->playerId && !scbw::isAlliedTo(player, unit->playerId)) {
					return; // Ally Check
				}
				if (units_dat::ShieldsEnabled[unit->id] && unit->shields < units_dat::MaxShieldPoints[unit->id] * 256) { // Shield Check for default behavior
					imgul(unit->sprite, 425, 0, 0, 0);
				} else { // Return if both checks failed
					return;
				}
				int healAmount = (stacks + 1) * 256;
				if (units_dat::ShieldsEnabled[unit->id]) { // Shield Heal
					unit->healShields(healAmount, data.target);
				}
			});
		}

		switch (data.target->id) { // TARGET SWITCH! MUST BE UNIT
			case UnitId::ProtossArchon:
			{
				// Archon - Astral Aegis
				scbw::UnitFinder(
					data.target->getX() - 96,
					data.target->getY() - 96,
					data.target->getX() + 96,
					data.target->getY() + 96
				).forEach([&](CUnit* unit) {
					if (!units_dat::ShieldsEnabled[unit->id]) {
						return;
					}
					uint32 value = 50 * 256;
					unit->healShields(value, data.target);
				});
				break;
			}
			case UnitId::ProtossAurora:
			{
				// Last Orders - Aurora behavior
				if (scbw::get_generic_value(data.target, ValueId::LastOrdersInit) == 0) {
					scbw::set_generic_value(data.target, ValueId::LastOrdersInit, 1);

					CUnit* nearest = scbw::UnitFinder(
						data.target->getX() - 288,
						data.target->getY() - 288,
						data.target->getX() + 288,
						data.target->getY() + 288
					).getNearestTarget(data.target,
						[&](CUnit* unit) {
						//Don't attack friendly / allied units
						if (unit->playerId == data.target->playerId) {
							return false;
						}
						if (scbw::isAlliedTo(data.target->playerId, unit->playerId)) {
							return false;
						}
						if (unit->status & UnitStatus::Invincible) {
							return false;
						}
						if (unit->status & UnitStatus::InAir) {
							return false;
						}
						// TODO: iquare consultation
						// Is this needed? Seems like it's already guaranteed
						if (scbw::getDistanceFast(data.target->position.x, data.target->position.y, unit->position.x, unit->position.y) > 288) {
							return false;
						}
						return true;
					});
					if (nearest != nullptr) {
						data.finalDamage = 0;
						data.target->setHp(units_dat::MaxHitPoints[data.target->id] / 2);
						data.target->orderTo(OrderId::Move, nearest->position.x, nearest->position.y);
						data.target->status |= UnitStatus::CanNotReceiveOrders;
					}
				}
				break;
			}
			case UnitId::ProtossAugur:
			{
				// Augur - Death behavior
				augurSacrifice(data.target);
			}
		}

		if (data.attacker != nullptr) {
			switch (data.weaponId) {
				case WeaponId::AlkajeliskPlasmidSpores:
				case WeaponId::AlkajeliskParasiticSpores:
				{
					// Alkajelisk - Parasite
					scbw::set_generic_value(data.target, ValueId::AlkajeliskParasite, 1);
					scbw::set_generic_value(data.target, ValueId::AlkajeliskParasitePlayer, data.attacker->playerId);
					break;
				}
				case WeaponId::KaiserBlades:
				{
					// Kaiser Rampage - Ultralisk behavior
					scbw::set_generic_timer(data.attacker, ValueId::KaiserRampage, 1 + (2 * 24));
					data.attacker->updateSpeed();
					break;
				}
				case WeaponId::NathrokorPlasmidBreath:
				{
					// Adrenal Frenzy - Nathrokor behavior
					int player = data.attacker->playerId;
					const int range = 48;
					scbw::UnitFinder(
						data.attacker->getX() - range,
						data.attacker->getY() - range,
						data.attacker->getX() + range,
						data.attacker->getY() + range
					).forEach([&](CUnit* unit) {
						if (unit == nullptr
							|| (player != unit->playerId && !scbw::isAlliedTo(player, unit->playerId))) {
							return;
						}
						scbw::set_generic_timer(unit, ValueId::NathrokorTimer, (4 * 24) + 1);
						unit->updateSpeed();
					});
					break;
				}
				case WeaponId::ToxicSpores:
				case WeaponId::BroodliskSwarmingSpores:
				{
					// Incubators - Broodling/Broodlisk behavior
					int type = data.attacker->id;
					int supplyCount = scbw::canSpawnBroodling(data.target) ? 2 : 0;// scbw::getBroodlingCountFromSupply(data.target);
					scbw::createUnitFromAbility(type, supplyCount, data.attacker->playerId,
						data.target->position.x, data.target->position.y, data.attacker->removeTimer,
						scbw::getSourceQueens(data.attacker)[0]);
					break;
				}
				case WeaponId::CyclopsFlakCannons:
				{
					// Cyclops - Flak Cannon passive
					auto newAngle = scbw::getAngle(data.attacker->getX(), data.attacker->getY(), data.target->getX(), data.target->getY());
					scbw::createBullet(WeaponId::CyclopsFragmentaryShells, data.attacker, data.target->getX(), data.target->getY(), data.attacker->playerId, newAngle);
					break;
				}
			}
		}
	}

	void applyEnsnare(CUnit* a1) { // 004F45E0
		int32 baseProperty; // eax
		uint8 overlayPadding; // al
		CUnit* victim; // ecx

		if (!(a1->ensnareTimer)
			&& !(a1->status & UnitStatus::Burrowed)) {
			// Calculate overlay ID offset based on units_dat flags
			overlayPadding = 0;
			baseProperty = units_dat::BaseProperty[a1->id];
			if (baseProperty & UnitProperty::MediumOverlay) {
				overlayPadding = 1;
			} else if (baseProperty & UnitProperty::LargeOverlay) {
				overlayPadding = 2;
			}
			victim = a1;
			if (a1->subunit != nullptr) {
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
		if (attacker != nullptr && target != nullptr) {
			if (attacker->id == UnitId::TerranGorgon) {
				if (scbw::isStunned(target) || scbw::isSlowed(target)) {
					return true;
				}
			}
		}
		return false;
	}

	uint16 GetSporesOverlayId(CUnit* target) {
		auto stacks = scbw::get_generic_value(target, ValueId::CorrosiveTouchStacks);
		auto overlayMultiplier = 0;
		if (stacks > 18) overlayMultiplier++;
		if (stacks > 35) overlayMultiplier++;
		if (stacks > 50) overlayMultiplier++;
		return overlayMultiplier + 4 * scbw::getOverlayOffset(target->id) + ImageId::AcidSpores_1_Overlay_Small;
	}

	// TODO: Refactor
	// Hooked to change Corrosive Spores timer - Pr0nogo
	int32 ApplySpores_hook(CUnit* target) { // 004F4480
		uint8 sporeCount; // al
		uint32 overlayMultiplier; // eax
		uint8 overlayOffset; // cl
		int32 overlay; // eax
		int32 v6; // eax
		CUnit* targetFinal; // ecx
		uint32 result_sub_1; // esi
		uint32 acidTimeMax; // edx
		uint32 result; // eax
		uint32 v11_acidSporeTime; // ecx
		CImage* image;
		CSprite* spriteFinal;

		scbw::add_generic_value(target, ValueId::CorrosiveTouchStacks, 1);
		image = target->getOverlay(GetSporesOverlayId(target));
		if (image) {
			overlay = image->id;
		}
		if (!overlay) {
			target->removeOverlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
			v6 = getCorrosiveSporesImage(target);
			targetFinal = target->subunit;
			if (!targetFinal)
				targetFinal = target;
			spriteFinal = targetFinal->sprite;
			spriteFinal->createOverlay(v6, spriteFinal->position.x, spriteFinal->position.y, 0);
		}

		result_sub_1 = 0;
		acidTimeMax = 99999;
		result = 0;

		scbw::set_generic_timer(target, ValueId::CorrosiveTouchTimer, VithriliskCorrosiveTouchTimer);
		return result;
	}
} //hooks

namespace {
	//Somehow related to AI stuff; details unknown.
	const uint32 Helper_GetUnitStrength = 0x00431800;
	uint16 getUnitStrength(CUnit* unit, bool useGroundStrength) {
		static uint16 strength;
		static uint32 useGroundStrength_;
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
	const uint32 Func_createHallucination = 0x004F6B90;
	CUnit* createHallucination(CUnit* target, uint32 playerId) {
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

	const uint32 Func_placeHallucination = 0x004F66D0;
	bool placeHallucination(CUnit* unit) {
		static bool32 bPreResult;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_placeHallucination
			MOV bPreResult, EAX
			POPAD
		}
		return (bPreResult != 0);
	}

	const uint32 Func_unitDestructor = 0x004A0990;
	void unitDestructor(CUnit* unit) {
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_unitDestructor
			POPAD
		}
	}

	// For updateCorrosiveSpores
	const uint32 Func_getCorrosiveSporesImage = 0x004F41F0;
	uint32 getCorrosiveSporesImage(CUnit* unit) {
		static uint32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_getCorrosiveSporesImage
			MOV result, EAX
			POPAD
		}
		return result;
	}

} //unnamed namespace
