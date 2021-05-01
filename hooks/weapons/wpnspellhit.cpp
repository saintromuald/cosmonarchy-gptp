//The source file for the Weapon/Spell Hit hook module.
#include "wpnspellhit.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include "logger.h"
#include "globals.h"
#include <algorithm>

namespace hooks {
	// no bullet because this function can be called in another context without bullet
	void IrradiateHit(CUnit* attacker, CUnit* target, uint8 attackingPlayerId) {
		// no longer handled here
	}

	// bullet is not normally part of the function, so if the function is called
	// in an unexpected way, it may not contain the correct value.
	void OpticalFlareHit(CUnit* target, uint32 attackingPlayerId, CBullet* bullet) {
		// can take the value 100 (0x64), thus not bool32
		static uint32* const u32_0051CE98 = (uint32*)0x0051CE98;
		if (target->status & UnitStatus::IsHallucination)
			target->remove();
		else {
			uint32 overlayImageId;
			CUnit* overlayTargetUnit;

			//blindness is applied for a specific player
			//just like parasite, but due to how it's used,
			//a blind unit is still blind for all players
			target->isBlind |= (uint8)(1 << attackingPlayerId);

			//playing the impact sound
			scbw::playSound(SoundId::Terran_Medic_Miopia1_wav, target);

			//create impact overlay

			if (units_dat::BaseProperty[target->id] & UnitProperty::MediumOverlay)
				overlayImageId = ImageId::OpticalFlareHit_Medium;
			else
				if (units_dat::BaseProperty[target->id] & UnitProperty::LargeOverlay)
					overlayImageId = ImageId::OpticalFlareHit_Large;
				else
					overlayImageId = ImageId::OpticalFlareHit_Small;

			if (target->subunit != nullptr)
				overlayTargetUnit = target->subunit;
			else
				overlayTargetUnit = target;

			(overlayTargetUnit->sprite)->createTopOverlay(overlayImageId, 0, 0, 0);

			//was hardcoded in original code
			scbw::refreshConsole();

			//cause some functions to be skipped in Game Loop
			//possibly delay fog of war reappearance following
			//loss of sight, or cause it?
			*u32_0051CE98 = 1;
		}
	}

	//bullet is not normally part of the function, so if the function is called
	//in an unexpected way, it may not contain the correct value.
	void RestoreHit(CUnit* target, CBullet* bullet) {

		if (target->status & UnitStatus::IsHallucination)
			target->remove();
		else {
			//create impact overlay
			scbw::restoreFunc(target);
		}

	}

	//bullet is not normally part of the function, so if the function is called
	//in an unexpected way, it may not contain the correct value.
	//...
	//the timer is reset before this call 2 times out of 3 (with
	//previousLockdownTimer holding the pre-reset value), and
	//the 3rd case, previousLockdownTimer is set to 131 (0x83)
	void LockdownHit(CUnit* target, uint8 previousLockdownTimer, CBullet* bullet) {

		//add lockdown overlay if no ongoing lockdown already
		if (target->lockdownTimer == 0) {
			uint32 overlayImageId = ImageId::LockdownField_Small + scbw::getOverlayOffset(target->id);
			CUnit* overlayTargetUnit;
			if (target->subunit != nullptr) {
				overlayTargetUnit = target->subunit;
			}
			else {
				overlayTargetUnit = target;
			}
			(overlayTargetUnit->sprite)->createTopOverlay(overlayImageId, 0, 0, 0);
		}

		//this added to the fact that target->lockdownTimer is set to 0 before
		//this function probably prevent lockdown to reset the duration of a 
		//previous lock
		if (previousLockdownTimer > target->lockdownTimer)
			target->lockdownTimer = previousLockdownTimer;

		//should stop movements/orders/scripts of the unit, give it the status 
		//DoodadStatesThing, may have an effect on selection, and perform
		//scbw::refreshConsole() code at the end
		function_492CC0(target);

	}

	// TODO: Cleanup
	//was called "Shot" instead of "Hit", changed due to the context
	/*
	CUnit* swarmsearch(CUnit* hitunit){
		scbw::print("Loop...");
		for (CUnit *unit = *firstVisibleUnit; unit; unit = unit->link.next) {
			if(unit->id==UnitId::Spell_DarkSwarm){
				scbw::print("dark swarm detected in the loop!");
				char buf[64];
				sprintf(buf,"Compare coords, unit: %d %d, swarm %d %d", hitunit->position.x,hitunit->position.y,
																	  unit->position.x,unit->position.y);
				scbw::print(buf);
				if(hitunit->position.x >= unit->getLeft() && hitunit->position.x <= unit->getRight()){
					if(hitunit->position.y >= unit->getTop() && hitunit->position.y <= unit->getBottom()){
						scbw::print("Found!");
						return unit;
					}
				}
			}
		}
		return nullptr;
	}*/

	void WeaponBulletHit(CBullet* bullet, CUnit* target, uint32 damageDivisor) {
		if (bullet->hitFlags & 2) {
			//bullet doing no damage (shot from hallucination)
			bool ephemeralBlades = false;
			if (bullet->sourceUnit != nullptr) {
				ephemeralBlades = true;
			}
			if (ephemeralBlades) {
				uint32 base_damage = GetUnitBulletDamage(bullet, target);
				target->damageWith(
					bullet->weaponType,
					base_damage,
					bullet->sourceUnit,
					bullet->srcPlayer,
					bullet->direction1,
					damageDivisor
				);
			} else {
				attackOverlayAndNotify(bullet->sourceUnit, target, bullet->weaponType, bullet->direction1); // no damage, but still ask for retaliation
			}
		} else {
			uint32 base_damage = GetUnitBulletDamage(bullet, target);
			target->damageWith(
				bullet->weaponType,
				base_damage,
				bullet->sourceUnit,
				bullet->srcPlayer,
				bullet->direction1,
				damageDivisor
			);
		}

	}

	uint16 GetUnitWidth(CUnit* unit) {
		return unit->getRight() - unit->getLeft();
	}
	uint16 GetUnitHeight(CUnit* unit) {
		return unit->getBottom() - unit->getTop();
	}

	bool IsInRange(CUnit* source, CUnit* target, uint32 range) {
		uint16 sourceX = source->position.x;
		uint16 sourceY = source->position.y;
		uint16 targetX = target->position.x;
		uint16 targetY = target->position.y;

		uint16 horizontalDifference = abs(sourceX - targetX);
		uint16 verticalDifference = abs(sourceY - targetY);
		uint16 horizontalRange = range + GetUnitWidth(source) / 2 + GetUnitWidth(target) / 2 + 1;
		uint16 verticalRange = range + GetUnitHeight(source) / 2 + GetUnitHeight(target) / 2 + 1;

		if (horizontalDifference > horizontalRange
			|| verticalDifference > verticalRange) {
			return false;
		} else {
			return true;
		}
	}

	//original name was ISCRIPT_AttackMelee @ 00479B40
	//Changed due to the context
	void MeleeAttackHit(CUnit* attacker) {
		CUnit* target = attacker->orderTarget.unit;
		if (target != nullptr) {
			uint8 weaponId;

			// Lakizilisks can only attack while burrowed even if
			// given a melee attack according to this code
			if (attacker->id != UnitId::ZergLakizilisk
				|| (attacker->status & UnitStatus::Burrowed
					|| attacker->status & UnitStatus::InBuilding)
				) { // Attempt to allow Lakizilisks to attack from within Bunkers
				weaponId = units_dat::GroundWeapon[attacker->id];
			} else {
				weaponId = WeaponId::None;
			}

			if (attacker->status & UnitStatus::IsHallucination) {
				bool ephemeralBlades = false;
				if (attacker != nullptr) {
					ephemeralBlades = true;
				}
				if (ephemeralBlades) {
					uint32 damage = getUnitDamageBonus(attacker, weaponId);
					damage = ((uint16)damage + weapons_dat::DamageAmount[weaponId]) * 256;
					target->damageWith(
						weaponId,
						damage,
						attacker,
						attacker->playerId,
						attacker->currentDirection1,
						1
					);
				} else {
					attackOverlayAndNotify(attacker, target, weaponId, attacker->currentDirection1); //no damage, but still ask for retaliation
				}
			} else {
				uint32 damage = getUnitDamageBonus(attacker, weaponId);
				damage = ((uint16)damage + weapons_dat::DamageAmount[weaponId]) * 256;
				target->damageWith(
					weaponId,
					damage,
					attacker,
					attacker->playerId,
					attacker->currentDirection1,
					1
				);
			}
		}
	}

	//bullet is not normally part of the function, so if the function is called
	//in an unexpected way, it may not contain the correct value.
	///...
	//In the original code, the parameters are gotten from CBullet as:
	//attacker from bullet->sourceUnit
	//x and y from bullet->sprite->position
	void EMPShockwaveHit(CUnit* attacker, int x, int y, CBullet* bullet) {
		CThingy* emp_sprite = scbw::createThingy(371, x, y, 0);
		if (emp_sprite != nullptr) {
			emp_sprite->sprite->elevationLevel = 19; //0x13
			setThingyVisibilityFlags(emp_sprite);
		}
		static uint16* const maxBoxRightValue = (uint16*)0x00628450;	//should usually be mapTileSize->width * 32
		static uint16* const maxBoxBottomValue = (uint16*)0x006284B4;	//should usually be mapTileSize->height * 32
		Box16 area_of_effect;
		CUnit** unitsInAreaOfEffect;
		CUnit* current_unit;

		//define the base area of effect
		area_of_effect.left = (uint16)x - weapons_dat::InnerSplashRadius[WeaponId::EMP_Shockwave];
		area_of_effect.right = (uint16)x + weapons_dat::InnerSplashRadius[WeaponId::EMP_Shockwave];
		area_of_effect.top = (uint16)y - weapons_dat::InnerSplashRadius[WeaponId::EMP_Shockwave];
		area_of_effect.bottom = (uint16)y + weapons_dat::InnerSplashRadius[WeaponId::EMP_Shockwave];

		//check and fix effect if beyond width of map
		if (area_of_effect.left < 0) {
			area_of_effect.left = 0;
		}
		else
			if (area_of_effect.right > *maxBoxRightValue) {
				area_of_effect.right = *maxBoxRightValue;
			}
		//check and fix effect if beyond height of map
		if (area_of_effect.top < 0) {
			area_of_effect.top = 0;
		}
		else
			if (area_of_effect.bottom > *maxBoxBottomValue) {
				area_of_effect.bottom = *maxBoxBottomValue;
			}

		//find all units in area of effect and pick the first
		unitsInAreaOfEffect = getAllUnitsInBounds(&area_of_effect);
		current_unit = *unitsInAreaOfEffect;
		while (current_unit != nullptr) {
			if (current_unit != attacker &&				//EMP doesn't affect the attacker
				(attacker == nullptr ||	current_unit != attacker->subunit)
				) {
				if (current_unit->status & UnitStatus::IsHallucination) {
					current_unit->remove();
				}
				else
					if (current_unit->stasisTimer == 0) { // don't affect units in stasis
						current_unit->energy -= 100;
						current_unit->shields -= 100;
						if (current_unit->sprite->flags & CSprite_Flags::Selected &&
							current_unit->sprite != nullptr
							) { // Follows original code, though this order of tests doesn't make sense
							setAllImageGroupFlagsPal11(current_unit->sprite); //redraw the unit
						}
					}
			}
			unitsInAreaOfEffect++;					//go on next unit of the list (or null)
			current_unit = *unitsInAreaOfEffect;
		}
		//reload the previous temporary unit list from before the call to getAllUnitsInBounds
		*tempUnitsListArraysCountsListLastIndex = *tempUnitsListArraysCountsListLastIndex - 1;
		*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsList[*tempUnitsListArraysCountsListLastIndex];
	}

	//bullet is not normally part of the function, so if the function is called
	//in an unexpected way, it may not contain the correct value.

	void CorrosiveAcidHit(uint32 attackingPlayerId, int x, int y, CBullet* bullet) {
		Box16 coords;

		coords.left = x - 64;
		coords.bottom = y + 64;
		coords.top = y - 64;
		coords.right = x + 64;

		// IterateUnitsAtLocationTargetProc_CorrosiveAcid(attackingPlayerId,&coords);
		// spreads originally
		if (bullet->attackTarget.unit != nullptr) {
			auto vec = scbw::getClarionLinks(bullet->attackTarget.unit);
			if (vec.size() != 0) {
				for (auto unit : vec) {
					ApplySpores(unit);
				}
			} else {
				ApplySpores(bullet->attackTarget.unit);
			}
		}
	}

	//no bullet because the code normally calling this function lose the 
	//easily available information (and getting it the hard way seems overkill)
	void ConsumeHit(CUnit* attacker, CUnit* target) {
		//scbw::print("Try consume");
		if (attacker == nullptr
			|| target == nullptr) {
			return;
		} else {
			if (getSpellStatString(target, TechId::Consume, attacker->playerId) != 0) {
				//char buf[32];
				//ebx/bl  = player id, arg 1 = 0x10, eax = edi (target)
				//sprintf(buf,"Spell error: %d",getSpellStatString(target,TechId::Consume,attacker->playerId) );
				//scbw::print(buf);
			}
		}

		if (target != nullptr &&
			!(target->status & UnitStatus::Invincible) &&
			getSpellStatString(target, TechId::Consume, attacker->playerId) == 0 //returning a value != 0 means there was an error
			) {
			incrementUnitDeathScores(target, attacker->playerId);
			uint32 power = target->hitPoints;
			target->remove();

			if (!(target->status & UnitStatus::IsHallucination)) {
				uint32 energyMax = attacker->getMaxEnergy();
				uint32 diff = energyMax - attacker->energy;
				if (power > diff) {
					attacker->energy = energyMax;
					power -= diff;
					attacker->setHp(attacker->hitPoints + power);
				} else {
					attacker->energy += power;
				}
			}
		}
	}

	//bullet is not normally part of the function, so if the function is called
	//in an unexpected way, it may not contain the correct value.
	void EnsnareHit(CUnit* attacker, int x, int y, CBullet* bullet) {
		Box16 coords;
		CThingy* ensnare_effect_sprite;
		ensnare_effect_sprite = createThingy(SpriteId::Ensnare, x, y, 0);
		if (ensnare_effect_sprite != nullptr) {
			ensnare_effect_sprite->sprite->elevationLevel = 19; //0x13
			setThingyVisibilityFlags(ensnare_effect_sprite);
		}
		coords.top = y - 64;
		coords.right = x + 64;
		coords.left = x - 64;
		coords.bottom = y + 64;
		IterateUnitsAtLocationTargetProc_Ensnare(attacker, &coords);
	}

	int ensnareProc(CUnit* target, CUnit* attacker) {
		if (target != attacker
			&& !(units_dat::BaseProperty[target->id] & UnitProperty::Building)
			&& target->id != UnitId::ZergInfestedTerran) {
			if (!(target->status & UnitStatus::Invincible) && !(target->status & UnitStatus::Burrowed))
				createEnsnareOverlay(target);
		}
		return 0;
	}

	//bullet is not normally part of the function, so if the function is called
	//in an unexpected way, it may not contain the correct value.
	void DarkSwarmHit(int x, int y, uint32 attackingPlayerId, CBullet* bullet) {
		Point16 coords;
		CUnit* dark_swarm_unit;
		coords.x = x;
		coords.y = y;

		//adjust location where Dark Swarm will be created
		//if necessary (by modifying coords)
		fixTargetLocation(&coords, UnitId::Spell_DarkSwarm);

		//the dark swarm is created for player 11 (neutral), not for the owner
		dark_swarm_unit = createUnit(UnitId::Spell_DarkSwarm, coords.x, coords.y, 11);

		if (dark_swarm_unit == nullptr) {
			displayLastNetErrorForPlayer(attackingPlayerId);
		}
		else {
			dark_swarm_unit->status |= UnitStatus::NoCollide;
			dark_swarm_unit->sprite->elevationLevel = 11;

			//update various stuff (set hp, set shield...) not finished on createUnit
			updateUnitStatsFinishBuilding(dark_swarm_unit);
			updateUnitStrength(dark_swarm_unit);
			if (dark_swarm_unit->removeTimer == 0
				|| dark_swarm_unit->removeTimer > 900) {
				dark_swarm_unit->removeTimer = 900;	//0x0384
			}
		}
	}

	//bullet is not normally part of the function, so if the function is called
	//in an unexpected way, it may not contain the correct value.
	void BroodlingHit(CUnit* attacker, CUnit* target, CBullet* bullet) {
		uint32 broodlings_to_spawn = 3;
		ActiveTile tile = scbw::getActiveTileAt(bullet->sprite->position.x, bullet->sprite->position.y);
		if (tile.hasCreep || tile.creepReceeding) {
			broodlings_to_spawn = 5;
		}
		int current_x = bullet->sprite->position.x - 2;
		int current_y = bullet->sprite->position.y - 2;
		while (broodlings_to_spawn > 0) {
			bool jump_to_end_of_loop = false; //F4A82
			//check various conditions before attempting to create a broodling
			if (function_0042D810(current_x, current_y, UnitId::ZergBroodling)) {
				Point16 inPos, outPos;
				Box16 moveArea;

				inPos.y = current_y;
				moveArea.top = current_y - 32;
				moveArea.bottom = current_y + 32;
				moveArea.left = current_x - 32;
				inPos.x = current_x;
				moveArea.right = current_x + 32;

				//if impossible to avoid a collision even within moveArea,
				//skip the broodling creation code for the current position
				jump_to_end_of_loop = !scbw::checkUnitCollisionPos(attacker, &inPos, &outPos, &moveArea, false, 0);

				if (!jump_to_end_of_loop) {
					current_y = outPos.y;
					current_x = outPos.x;
				}
			}

			jump_to_end_of_loop = false; // <- allows to target air units, and presumably terrain, if only the function would actually be called
			if (!jump_to_end_of_loop) { //F49E2
				Point16 pos;
				CUnit* created_broodling;
				pos.x = current_x;
				pos.y = current_y;

				//adjust the position to welcome the unit...I think
				fixTargetLocation(&pos, UnitId::ZergBroodling);

				//create the broodling
				created_broodling = createUnit(UnitId::ZergBroodling, pos.x, pos.y, attacker->playerId);
				scbw::set_generic_unit(created_broodling, ValueId::BroodlingMother, attacker);

				if (created_broodling == nullptr) {
					displayLastNetErrorForPlayer(attacker->playerId);
					jump_to_end_of_loop = true;
				} else { //F4A26
					scbw::playSound(SoundId::Zerg_BROODLING_ZBrRdy00_WAV, created_broodling);

					uint32 timerBeforeDisappearance;

					//update various stuff (set hp, set shield...) not finished on createUnit
					updateUnitStatsFinishBuilding(created_broodling);
					updateUnitStrength(created_broodling);
					timerBeforeDisappearance = 720;	//0x708
					if (created_broodling->removeTimer == 0
						|| created_broodling->removeTimer > timerBeforeDisappearance
						) {
						created_broodling->removeTimer = timerBeforeDisappearance;
					}

					if (attacker->pAI != nullptr) {
						AI_TrainingUnit(attacker, created_broodling);
					}
				}
			}
				//F4A82 (end of loop)
			current_y += 4;
			current_x += 4;
			broodlings_to_spawn--;
		}
	}

	//bullet is not normally part of the function, so if the function is called
	//in an unexpected way, it may not contain the correct value.
	void PlagueHit(CUnit* attacker, int x, int y, CBullet* bullet) {
		Box16 coords;
		coords.left = x - 64;
		coords.bottom = y + 64;
		coords.top = y - 64;
		coords.right = x + 64;

		IterateUnitsAtLocationTargetProc_Plague(attacker, &coords);
		if (attacker != nullptr) {
			uint32 range = 64;
			scbw::UnitFinder(
				x - range,
				y - range,
				x + range,
				y + range
			).forEach([&](CUnit* unit) {
				if (unit->plagueTimer > 0) {
					unit->newFlags1 |= NewFlags1::Metastasis;
				}
				if (attacker != nullptr
					&& scbw::get_generic_timer(attacker, ValueId::EnsnaringBroodTimer) > 0
					&& !(units_dat::BaseProperty[unit->id] & UnitProperty::Building)) {
					scbw::applyEnsnare(unit);
				}
			});
		}
	}

	//bullet is not normally part of the function, so if the function is called
	//in an unexpected way, it may not contain the correct value.
	void MaelstromHit(CUnit* attacker, int x, int y, CBullet* bullet) {
		Box16 coords;
		CThingy* maelstrom_effect_sprite = createThingy(SpriteId::Maelstrom_Hit, x, y, 0);
		if (maelstrom_effect_sprite != nullptr) {
			maelstrom_effect_sprite->sprite->elevationLevel = 17; //0x11
			setThingyVisibilityFlags(maelstrom_effect_sprite);
		}
		scbw::playSound(SoundId::Protoss_Darchon_Parahit_wav, attacker);
		coords.right = x + 48;
		coords.left = x - 48;
		coords.top = y - 48;
		coords.bottom = y + 48;
		IterateUnitsAtLocationTargetProc_Maelstrom(attacker, &coords);
	}

	//no bullet because this function is called in another context without bullet
	void MindControlHit(CUnit* attacker, CUnit* target) {
		if (target->status & UnitStatus::IsHallucination)
			target->remove();
		else {
			target->giveTo(attacker->playerId);
		}
	}

	//bullet is not normally part of the function, so if the function is called
	//in an unexpected way, it may not contain the correct value.
	void StasisFieldHit(CUnit* attacker, int x, int y, CBullet* bullet) {
		Box16 coords;
		CThingy* stasisfield_effect_sprite = createThingy(SpriteId::Stasis_Field_Hit, x, y, 0);
		if (stasisfield_effect_sprite != nullptr) {
			stasisfield_effect_sprite->sprite->elevationLevel = 17; //0x11
			setThingyVisibilityFlags(stasisfield_effect_sprite);
		}
		coords.top = y - 48;
		coords.right = x + 48;
		coords.left = x - 48;
		coords.bottom = y + 48;
		IterateUnitsAtLocationTargetProc_StasisField(attacker, &coords);
	}

	//bullet is not normally part of the function, so if the function is called
	//in an unexpected way, it may not contain the correct value.
	CUnit* createDisruptionWeb(uint32 attackingPlayerId, int x, int y) {
		Point16 coords;
		CUnit* disruption_web_unit;
		coords.x = x;
		coords.y = y;

		//adjust location where Disruption Web will be created
		//if necessary (by modifying coords)
		fixTargetLocation(&coords, UnitId::Spell_DisruptionWeb);
		disruption_web_unit = createUnit(UnitId::Spell_DisruptionWeb, coords.x, coords.y, 11);
		if (disruption_web_unit == nullptr) {
			displayLastNetErrorForPlayer(attackingPlayerId);
		}
		else {
			disruption_web_unit->status |= UnitStatus::NoCollide;
			disruption_web_unit->sprite->elevationLevel = 11;
			//update various stuff (set hp, set shield...) not finished on createUnit
			updateUnitStatsFinishBuilding(disruption_web_unit);
			updateUnitStrength(disruption_web_unit);
			if (disruption_web_unit->removeTimer == 0 ||
				disruption_web_unit->removeTimer > 360
				) {
				disruption_web_unit->removeTimer = 360; //0x0168
			}
		}
		return disruption_web_unit;
	}

	void DisruptionWebHit(uint32 attackingPlayerId, int x, int y, CBullet* bullet) {
		auto unit = createDisruptionWeb(attackingPlayerId, x, y);
	}

	uint8 stasisFieldOverlay(CUnit* a1, uint8 a2) {
		uint32 v2;
		unsigned __int8 v3;
		CUnit* v4;
		unsigned __int8 v5;

		if (!a1->stasisTimer) {
			v2 = units_dat::BaseProperty[a1->id];
			if (v2 & UnitProperty::MediumOverlay)
				v3 = 1;
			else if (v2 & UnitProperty::LargeOverlay)
				v3 = 2;
			v4 = a1->subunit;
			if (!v4)
				v4 = a1;
			a1->sprite->createOverlay(ImageId::StasisField_Small + v3, 0, 0, 0);
			a1->status |= UnitStatus::Invincible; // remove
		}
		v5 = a1->stasisTimer;
		if (a2 > v5)
			v5 = a2;
		a1->stasisTimer = v5;
		return function_492CC0_ret(a1);
	}

	void createScanner(CUnit* a1) { // 00464E40
		CUnit* v1; // esi
		uint16 v2; // ax
		bool v3; // zf
		GroupFlag v4; // al
		uint8 v5; // al
		int16 v6; // cx
		int16 v7; // ax
		const char* v8; // eax
		int16 v9; // cx
		CUnit* v10; // eax
		CUnit* v11; // ebx
		int16 v12; // ax
		Point16 a2; // [esp+4h] [ebp-4h]

		v1 = a1;
		if (!(*CHEAT_STATE & 0x800)) {
			v2 = techdata_dat::EnergyCost[4] * 256;
			v3 = v1->energy >= v2;
			//			a2 = (Point32)(v1->energy < v2);		// check this out
			if (!v3) {
				v4 = units_dat::GroupFlags[(uint16)v1->id];
				if (v4.isZerg) {
					v5 = 0;
				} else if (v4.isTerran) {
					v5 = 1;
				} else {
					v5 = (v4.isProtoss) != 0 ? 2 : 4;
				}
				v6 = v5 + 864;
				v7 = v5 + 863;
				if (!v6)
					goto LABEL_15;
				if (v7 < *statTxtTbl) {
					v8 = (char*)statTxtTbl + statTxtTbl[v7 + 1];
					if (!v8) {
					LABEL_15:
						if (v1->orderQueueHead) {
							v1->userActionFlags |= 1u;
							orders_Nothing2(v1);
						} else if (v1->pAI) {
							orderComputerClear(v1, 0x9Cu);
						} else {
							orderComputerClear(v1, units_dat::ReturnToIdleOrder[(uint16)v1->id]);
						}
						return;
					}
				} else {
					v8 = "";
				}
				if ((uint8)v1->playerId == *LOCAL_NATION_ID && *ACTIVE_PLAYER_ID == *LOCAL_HUMAN_ID)
					setUnitStatTxtErrorMsg((int32)v8);
				goto LABEL_15;
			}
		}
		v9 = v1->orderTarget.pt.x;
		a2.y = v1->orderTarget.pt.y;
		a2.x = v9;
		fixTargetLocation(&a2, UnitId::Spell_ScannerSweep);
		v10 = createUnit(0x21u, a2.x, a2.y, (uint8)v1->playerId);
		v11 = v10;
		if (v10) {
			updateUnitStatsFinishBuilding(v10);
			updateUnitStrength(v11);
			playSoundFromSource(388, v1, 1, 0);
			if (!(*CHEAT_STATE & 0x800)) {
				v1->spendUnitEnergy(techdata_dat::EnergyCost[4] * 256);
			}
			toIdle(v1);
		} else {
			displayLastNetErrorForPlayer(v1->playerId);
			toIdle(v1);
		}
	}

}