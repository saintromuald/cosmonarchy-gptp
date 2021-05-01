//All functions in this file are heavily involved with memory management.
//Do NOT modify anything unless you really know what you are doing.

#include "unit_destructor_special.h"
#include "../SCBW/api.h"
#include "../hook_tools.h"
#include "hooks/weapons/wpnspellhit.h"
#include "logger.h"
//Identical to 00466170  sub_466170
//Not hooked but used by hooked unitDestructorSpecialHook
#include <SCBW/UnitFinder.h>
#include "globals.h"
#include "hooks\weapons\wpnspellhit.h"

void killAllHangarUnits(CUnit* unit) {
	uint32 random_value;
	while (unit->carrier.inHangarCount != 0) {
		CUnit* childInside = unit->carrier.inHangarChild;
		unit->carrier.inHangarChild = childInside->interceptor.hangar_link.prev;
		childInside->interceptor.parent = nullptr;
		childInside->remove();
		unit->carrier.inHangarCount--;
	}
	while (unit->carrier.outHangarCount != 0) {
		CUnit* childOutside = unit->carrier.outHangarChild;
		childOutside->interceptor.parent = nullptr;
		// Kill Stewards only (Plasma Shells will defuse anyway)
		if (childOutside->id != UnitId::ProtossPlasmaShell) {
			uint16 randomDeathTimer;
			// TODO: Remove RNG, make farthest Stewards detonate first
			// hardcoded RNG management
			if (*IS_IN_GAME_LOOP) {
				static uint32* randomnessCounter_0051C6AC = (uint32*)0x0051C6AC;
				static uint32* randomnessCounter_0051CA18 = (uint32*)0x0051CA18;
				*randomnessCounter_0051C6AC = *randomnessCounter_0051C6AC + 1;
				*randomnessCounter_0051CA18 = *randomnessCounter_0051CA18 + 1;
				*lastRandomNumber = (*lastRandomNumber * 0x015A4E35) + 1;
				// number between 0 and 32766
				random_value = (*lastRandomNumber / 65536) & 0x7FFF;
			} else {
				random_value = 0;
			}
			// should produce a number between 15 and 45 (or maybe 46)
			randomDeathTimer = ((random_value * 31) / 32768) + 15;
			if (childOutside->removeTimer == 0 ||
				randomDeathTimer < childOutside->removeTimer)
			{
				childOutside->removeTimer = randomDeathTimer;
			}
		}
		//66239
		unit->carrier.outHangarChild = childOutside->interceptor.hangar_link.prev;
		childOutside->interceptor.hangar_link.prev = nullptr;
		childOutside->interceptor.hangar_link.next = nullptr;
		unit->carrier.outHangarCount--;
	}
	unit->carrier.outHangarChild = nullptr;
}

// Identical to 00468770 sub_468770
// Not hooked but used by hooked unitDestructorSpecialHook
void freeResourceContainer(CUnit* resource) {
	resource->building.resource.gatherQueueCount = 0;
	CUnit* worker = resource->building.resource.nextGatherer;
	CUnit* nextWorker;
	while (worker != nullptr) {
		if (worker->worker.harvest_link.prev != nullptr) { //untie the previous worker from current worker (tie it to next)
			(worker->worker.harvest_link.prev)->worker.harvest_link.next = worker->worker.harvest_link.next;
		} else {
			resource->building.resource.nextGatherer = worker->worker.harvest_link.next;
		}
		if (worker->worker.harvest_link.next != nullptr) { //untie the next worker from current worker (tie it to previous)
			(worker->worker.harvest_link.next)->worker.harvest_link.prev = worker->worker.harvest_link.prev;
		}
		nextWorker = worker->worker.harvest_link.next;
		worker->worker.harvestTarget = nullptr;
		worker->worker.harvest_link.prev = nullptr;
		worker->worker.harvest_link.next = nullptr;
		worker = nextWorker;
	}
}

//Hooked down there instead of a specific inject.cpp
void unitDestructorSpecialHook(CUnit* unit) {
	// Aspirant Criticality behavior
	if (unit->id == UnitId::TerranAspirant && unit->status & UnitStatus::Completed) {
		globals.createGenericPersistImpact(ImpactId::Criticality, 5 * 24, {unit->getX(), unit->getY()}, 1.25 * 32, SpriteId::Criticality, 5, unit->playerId);
	}

	// Remove Disruption Fields
	if (unit->id == UnitId::ProtossLanifect) {
		auto web = scbw::get_generic_unit(unit, ValueId::AttachedDisruptionWeb);
		if (web != nullptr) {
			web->removeTimer = 5 * 24;
		}
	}

	// Alkajelisk Parasitic Spores behavior
	if (scbw::get_generic_value(unit, ValueId::AlkajeliskParasite) == 1) {
		unit->orderTarget.pt.x = unit->position.x + 1;
		unit->orderTarget.pt.y = unit->position.y;
		auto bullet = helper::createBulletRet(WeaponId::AlkajeliskParasiticSpores, unit, unit->position.x, unit->position.y,
			scbw::get_generic_value(unit, ValueId::AlkajeliskParasitePlayer), 0);
		if (bullet != nullptr) {
			bullet->time_remaining = 255;
		}
	}

	// Vassal Terminal Surge behavior
	if (unit->id == UnitId::ProtossVassal) {
		uint32 range = 5 * 32;
		scbw::UnitFinder(
			unit->getX() - range,
			unit->getY() - range,
			unit->getX() + range,
			unit->getY() + range
		).forEach([&](CUnit* target) {
			if (!scbw::isAlliedTo(unit->playerId, target->playerId)) {
				return;
			}
			scbw::add_generic_value(target, ValueId::TerminalSurge, 1);
			target->updateSpeed();
		});
	}

	// Apostle Last Breath behavior
	{
		uint32 range = 4 * 32;
		scbw::UnitFinder(
			unit->getX() - range,
			unit->getY() - range,
			unit->getX() + range,
			unit->getY() + range
		).forEach([&](CUnit* target) {
			if (target->id != UnitId::TerranApostle) {
				return;
			}
			if (target == unit) {
				return;
			}
			if (!scbw::isAlliedTo(unit->playerId, target->playerId)) {
				return;
			}
			target->energy += ApostleLastBreathEnergyGain;
			target->sprite->createUnderlay(425);
		});
	}

	// Clarion Phase Link behavior
	if (unit->id == UnitId::ProtossClarion) {
		scbw::clearClarionLinks(unit);
	}

	// Parasite - spawn broodlings on death
	if (unit->parasiteFlags != 0) {
		uint32 supplyCount = scbw::getBroodlingCountFromSupply(unit);
		uint16 type = UnitId::ZergBroodling;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Flyer) {
			type = UnitId::ZergBroodlisk;
		}
		for (uint32 player = 0; player < PLAYER_COUNT; player++) {
			if (unit->parasiteFlags & (1 << player) && scbw::get_generic_timer(unit, ValueId::ParasiteArmTimer) <= 1) {
				scbw::createUnitFromAbility(type, supplyCount, player, unit->position.x, unit->position.y, 720);
			}
		}
	}

	// Metastasis - Plague spreads when a unit dies
	if ((unit->newFlags1 & NewFlags1::Metastasis) && unit->plagueTimer > 0) {
		helper::createBullet(WeaponId::Plague, unit, unit->position.x, unit->position.y, unit->playerId,
			unit->currentDirection1);
		Box16 coords;
		coords.left = unit->position.x - 48;
		coords.bottom = unit->position.y + 48;
		coords.top = unit->position.y - 48;
		coords.right = unit->position.x + 48;
		helper::IterateUnitsAtLocationTargetProc_Plague(unit, &coords);
		uint32 range = 48;
		scbw::UnitFinder(
			unit->getX() - range,
			unit->getY() - range,
			unit->getX() + range,
			unit->getY() + range
		).forEach([&](CUnit* target) {
			if (target->plagueTimer > 0) {
				target->newFlags1 |= NewFlags1::Metastasis;
			}
		});
		//add finder later to trace plague flag
	}

	// Destroy Stewards and Plasma Shells
	if (unit->id == UnitId::ProtossSolarion
		|| unit->id == UnitId::ProtossAccantor) {
		killAllHangarUnits(unit);
	}

	// Destroy nuclear missiles mid-launch
	else
		if (unit->id == UnitId::TerranEidolon
			|| unit->id == UnitId::TerranSeraph) {
			if (unit->building.ghostNukeMissile != nullptr) {
				(unit->building.ghostNukeMissile)->sprite->playIscriptAnim(IscriptAnimation::Death, true);
				unit->building.ghostNukeMissile = nullptr;
			}

		} else

			// Is a harvestable mineral patch or gas building
			if (
				(unit->id >= UnitId::ResourceMineralField && unit->id <= UnitId::ResourceMineralFieldType3) ||
				((unit->id == UnitId::TerranReservoir || unit->id == UnitId::ProtossAquifer || unit->id == UnitId::ZergExtractor)
					&& unit->status & UnitStatus::Completed
					)	/*this is equivalent to function unit_isGeyserUnit @ 004688B0*/
				) {
				freeResourceContainer(unit);
			} else

				if (unit->id >= UnitId::TerranNuclearMissile && unit->id <= UnitId::ProtossPylon) {

					//In original code, this part is handled by an internal id_based array and a
					//switch-like system to execute the code corresponding to the unit
					//If the id were within the previous test, the resource units would go through
					//those tests

					//Is a scarab or interceptor
					if (unit->id == UnitId::ProtossPlasmaShell || unit->id == UnitId::ProtossSteward) {

						if (unit->status & UnitStatus::Completed) {

							//following code is equivalent to a jump to function 004652A0
							//with unit as parameter in EAX

							if (unit->interceptor.parent != nullptr) {

								if (unit->interceptor.hangar_link.next != nullptr) //untie the next interceptor/scarab from current interceptor/scarab (tie it to previous)
									(unit->interceptor.hangar_link.next)->interceptor.hangar_link.prev = unit->interceptor.hangar_link.prev;
								else {
									if (unit->interceptor.isOutsideHangar)
										unit->interceptor.parent->carrier.outHangarChild = unit->interceptor.hangar_link.prev;
									else
										unit->interceptor.parent->carrier.inHangarChild = unit->interceptor.hangar_link.prev;
								}

								if (unit->interceptor.isOutsideHangar)
									unit->interceptor.parent->carrier.outHangarCount--;
								else
									unit->interceptor.parent->carrier.inHangarCount--;

								if (unit->interceptor.hangar_link.prev != nullptr)  //untie the previous interceptor/scarab from current interceptor/scarab (tie it to next)
									(unit->interceptor.hangar_link.prev)->interceptor.hangar_link.next = unit->interceptor.hangar_link.next;

							} else {
								unit->interceptor.hangar_link.prev = nullptr;
								unit->interceptor.hangar_link.next = nullptr;
							}

						}

					} else

						//Is a Nuclear Silo
						if (unit->id == UnitId::TerranNuclearSilo) {

							if (unit->building.silo.nuke != nullptr) {
								(unit->building.silo.nuke)->remove();
								unit->building.silo.nuke = nullptr;
							}

						} else



							//Is a Nuclear Missile
							if (unit->id == UnitId::TerranNuclearMissile) {

								if (unit->connectedUnit != nullptr && unit->connectedUnit->id == UnitId::TerranNuclearSilo) {
									(unit->connectedUnit)->building.silo.nuke = nullptr;
									(unit->connectedUnit)->building.silo.isReady = false;
								}

							} else

								//Is a Protoss Pylon
								if (unit->id == UnitId::ProtossPylon) {

									//code from hooks::removePsiField(CUnit* unit)
									//copied because the content is hardcoded here

									if (unit->building.pylonAura != nullptr) {
										unit->building.pylonAura->free();	//should be equivalent to SpriteDestructor @ 00497B40
										unit->building.pylonAura = nullptr;
									}

									//begin hardcoding removeFromPsiProviderList @ 00493100 to avoid use of a helper

									if (unit->psi_link.prev != nullptr)  //untie the previous pylon from current pylon (tie it to next)
										(unit->psi_link.prev)->psi_link.next = unit->psi_link.next;
									if (unit->psi_link.next != nullptr)  //untie the next pylon from current pylon (tie it to previous)
										(unit->psi_link.next)->psi_link.prev = unit->psi_link.prev;

									if (unit == *firstPsiFieldProvider)
										*firstPsiFieldProvider = unit->psi_link.next;

									unit->psi_link.prev = nullptr;
									unit->psi_link.next = nullptr;

									//end hardcoding removeFromPsiProviderList @ 00493100 to avoid use of a helper

									*canUpdatePoweredStatus = true;

								} else
									//Is a Nydus Canal
									if (unit->id == UnitId::ZergNydusCanal) {
										CUnit* nydusExit = unit->building.nydusExit;
										if (nydusExit != nullptr) {
											unit->building.nydusExit = nullptr;
											nydusExit->building.nydusExit = nullptr;
											nydusExit->remove();
										}
									}

								globals.remove_unit_from_pariah_table(unit, false);
								globals.clear_ptr_in_apostle_table(unit);
								globals.unitDeathHook(unit);
				}
}

//-------- Actual hooking --------//

void __declspec(naked) unitDestructorSpecialWrapper() {

	static CUnit* unit;

	__asm {
		MOV unit, EAX
		PUSHAD
	}

	unitDestructorSpecialHook(unit);

	__asm {
		POPAD
		RETN
	}
}

namespace hooks {

	void injectUnitDestructorSpecial() {
		jmpPatch(unitDestructorSpecialWrapper, 0x0049F4A0, 2);
	}

} //hooks
