// All functions in this file are heavily involved with memory management.

#include "unit_destructor_special.h"
#include "../SCBW/api.h"
#include "../hook_tools.h"
#include "hooks/weapons/weapon_spellhit.h"
#include "logger.h"
#include <SCBW/UnitFinder.h>
#include "globals.h"

void augurProc(Unit* augur, s32 radius) {
	Unit* target = scbw::UnitFinder::getNearestTarget(
		augur->getX() - radius,
		augur->getY() - radius,
		augur->getX() + radius,
		augur->getY() + radius,
		augur, [&](Unit* unit)
		{
			// Don't attack friendly / allied units
			if (unit->sprite == nullptr)
			{
				return false;
			}
			if (unit->player == augur->player)
			{
				return false;
			}
			if (scbw::isAlliedTo(unit->player, augur->player))
			{
				return false;
			}
			if (unit->status & UnitStatus::Invincible)
			{
				return false;
			}
			return true;
		});
	Thingy* thingy = helper::createThingy(SpriteId::AugurDeathExplosion, augur->getX(), augur->getY(), augur->player);
	helper::playSoundFromSource(scbw::randBetween(SoundId::AugurDeath01, SoundId::AugurDeath04), (Unit*)thingy, 1, 0);
	if (target)
	{
		augur->orderTarget.unit = target;
		helper::createBullet(WeaponId::AugurFinalHour, augur, augur->getX(), augur->getY(), augur->player, scbw::getAngle(augur->getX(), augur->getY(), target->getX(), target->getY()));
	}
}

void augurSacrifice(Unit* augur, s32 radius)
{
	UnitExt* targetExt = globals.get_unit_ext(augur);
	if (targetExt->values[ValueAugurSafetyCheck] == 0)
	{
		targetExt->values[ValueAugurSafetyCheck] = 1;
		augurProc(augur, radius);
		augur->remove();
	}
}

// Identical to 00466170 kill_hangar_units
// Not hooked but used by hooked unit_destructor_special_cases
void kill_hangar_units(Unit* unit)
{
	u32 random_value;
	while (unit->carrier.inHangarCount != 0)
	{
		Unit* childInside = unit->carrier.inHangarChild;
		unit->carrier.inHangarChild = childInside->interceptor.hangar_link.prev;
		childInside->interceptor.parent = nullptr;
		childInside->remove();
		unit->carrier.inHangarCount--;
	}
	while (unit->carrier.outHangarCount != 0)
	{
		Unit* childOutside = unit->carrier.outHangarChild;
		childOutside->interceptor.parent = nullptr;
		// Kill Stewards only (Plasma Shells will defuse anyway)
		if (childOutside->id != UnitId::ProtossPlasmaShell)
		{
			u16 randomDeathTimer;
			// TODO: Remove RNG, make farthest Stewards detonate first
			// hardcoded RNG management
			if (*IS_IN_GAME_LOOP)
			{
				static u32* randomnessCounter_0051C6AC = (u32*)0x0051C6AC;
				static u32* randomnessCounter_0051CA18 = (u32*)0x0051CA18;
				*randomnessCounter_0051C6AC = *randomnessCounter_0051C6AC + 1;
				*randomnessCounter_0051CA18 = *randomnessCounter_0051CA18 + 1;
				*lastRandomNumber = (*lastRandomNumber * 0x015A4E35) + 1;
				// number between 0 and 32766
				random_value = (*lastRandomNumber / 65536) & 0x7FFF;
			}
			else
			{
				random_value = 0;
			}
			// should produce a number between 15 and 45 (or maybe 46)
			randomDeathTimer = ((random_value * 31) / 32768) + 15;
			if (childOutside->removeTimer == 0
				|| randomDeathTimer < childOutside->removeTimer)
			{
				childOutside->removeTimer = randomDeathTimer;
			}
		}
		// 00466239
		unit->carrier.outHangarChild = childOutside->interceptor.hangar_link.prev;
		childOutside->interceptor.hangar_link.prev = nullptr;
		childOutside->interceptor.hangar_link.next = nullptr;
		unit->carrier.outHangarCount--;
	}
	unit->carrier.outHangarChild = nullptr;
}

// Identical to 00468770 remove_harvesters
// Not hooked but used by hooked unit_destructor_special_cases
void free_resource_container(Unit* resource)
{
	resource->building.resource.gatherQueueCount = 0;
	Unit* worker = resource->building.resource.nextGatherer;
	Unit* nextWorker;
	while (worker != nullptr)
	{
		if (worker->worker.harvest_link.prev != nullptr) // untie the previous worker from current worker (tie it to next)
		{
			(worker->worker.harvest_link.prev)->worker.harvest_link.next = worker->worker.harvest_link.next;
		}
		else
		{
			resource->building.resource.nextGatherer = worker->worker.harvest_link.next;
		}
		if (worker->worker.harvest_link.next != nullptr) // untie the next worker from current worker (tie it to previous)
		{
			(worker->worker.harvest_link.next)->worker.harvest_link.prev = worker->worker.harvest_link.prev;
		}
		nextWorker = worker->worker.harvest_link.next;
		worker->worker.harvest_target = nullptr;
		worker->worker.harvest_link.prev = nullptr;
		worker->worker.harvest_link.next = nullptr;
		worker = nextWorker;
	}
}

void unit_destructor_special_cases(Unit* unit)
{
	UnitExt* unitExt = globals.get_unit_ext(unit);
	unit->resetSupplyProvided();

	if (!(unit->userActionFlags & 4) && // for silent remove
		  unit->status & UnitStatus::Completed && 
	     (unit->id == UnitId::ZergHatchery || unit->id == UnitId::ZergLair ||
		  unit->id == UnitId::ZergHive     || unit->id == UnitId::ZergSire ||
		  units_dat::BaseProperty[unit->id] & UnitProperty::CreepBuilding) ||
		  unitExt->values[ValueBlissfulEmbraceOriginalPlayer])
	{
		scbw::clear_creep(scbw::unit_area(unit));
	}

	// These do not require the unit to be completed
	switch (unit->id)
	{
		case UnitId::TerranEidolon:
		case UnitId::TerranSeraph:
		{
			if (unit->building.ghostNukeMissile != nullptr)
			{
				(unit->building.ghostNukeMissile)->sprite->play_iscript_animation(IscriptAnimation::Death, true);
				unit->building.ghostNukeMissile = nullptr;
			}
		} break;
		case UnitId::TerranNuclearMissile:
		{
			if (unit->connectedUnit != nullptr && unit->connectedUnit->id == UnitId::TerranNuclearSilo)
			{
				(unit->connectedUnit)->building.silo.nuke = nullptr;
				(unit->connectedUnit)->building.silo.isReady = false;
			}
		} break;
		case UnitId::TerranNuclearSilo:
		{
			if (unit->building.silo.nuke != nullptr)
			{
				(unit->building.silo.nuke)->remove();
				unit->building.silo.nuke = nullptr;
			}
		} break;
		case UnitId::ProtossPylon:
		{
			// code from hooks::removePsiField(Unit* unit)
			// copied because the content is hardcoded here
			if (unit->building.pylonAura != nullptr)
			{
				unit->building.pylonAura->free();	// should be equivalent to SpriteDestructor @ 00497B40
				unit->building.pylonAura = nullptr;
			}

			// begin hardcoding removeFromPsiProviderList @ 00493100 to avoid use of a helper
			if (unit->psi_link.prev != nullptr)  // untie the previous pylon from current pylon (tie it to next)
			{
				(unit->psi_link.prev)->psi_link.next = unit->psi_link.next;
			}
			if (unit->psi_link.next != nullptr)  // untie the next pylon from current pylon (tie it to previous)
			{
				(unit->psi_link.next)->psi_link.prev = unit->psi_link.prev;
			}

			if (unit == *firstPsiFieldProvider)
			{
				*firstPsiFieldProvider = unit->psi_link.next;
			}
			unit->psi_link.prev = nullptr;
			unit->psi_link.next = nullptr;
			// end hardcoding removeFromPsiProviderList @ 00493100 to avoid use of a helper
			*canUpdatePoweredStatus = true;
		} break;
		case UnitId::ZergNydusCanal:
		{
			Unit* nydusExit = unit->building.nydusExit;
			if (nydusExit != nullptr)
			{
				unit->building.nydusExit = nullptr;
				nydusExit->building.nydusExit = nullptr;
				nydusExit->remove();
			}
			break;
		}
		default:
		{
		}
	}

	// These require the unit to be completed
	if (unit->status & UnitStatus::Completed)
	{
		switch (unit->id)
		{
			case UnitId::SharedAspirant:
			{
				globals.createEffect(EffectId::Criticality, 10 * 24, { unit->getX(), unit->getY() }, 2 * 32, SpriteId::Criticality, 5, unit->player, unit);
			} break;
			case UnitId::ProtossClarion:
			{
				// Phase Link
				scbw::clearClarionLinks(unit);
			} break;
			case UnitId::ProtossVassal:
			{
				// Terminal Surge
				u32 range = 5 * 32;
				scbw::UnitFinder(
					unit->getX() - range,
					unit->getY() - range,
					unit->getX() + range,
					unit->getY() + range
				).forEach([&](Unit* target)
					{
						UnitExt* targetExt = globals.get_unit_ext(target);
						if (!scbw::isAlliedTo(unit->player, target->player))
						{
							return;
						}
						targetExt->values[ValueTerminalSurge]++;
						target->updateSpeed();
						
						if (target->subunit != nullptr)
						{
							UnitExt* subunitExt = globals.get_unit_ext(target->subunit);
							subunitExt->values[ValueTerminalSurge]++;
						}
					});
			} break;
			case UnitId::ProtossAccantor:
			case UnitId::ProtossSolarion:
			{
				kill_hangar_units(unit);				
			} break;
			case UnitId::ProtossPlasmaShell:
			case UnitId::ProtossSteward:
			{
				if (unit->interceptor.parent != nullptr)
				{
					if (unit->interceptor.hangar_link.next != nullptr) // untie the next interceptor/scarab from current interceptor/scarab (tie it to previous)
					{
						(unit->interceptor.hangar_link.next)->interceptor.hangar_link.prev = unit->interceptor.hangar_link.prev;
					}
					else
					{
						if (unit->interceptor.isOutsideHangar)
						{
							unit->interceptor.parent->carrier.outHangarChild = unit->interceptor.hangar_link.prev;
						}
						else
						{
							unit->interceptor.parent->carrier.inHangarChild = unit->interceptor.hangar_link.prev;
						}
					}
					if (unit->interceptor.isOutsideHangar)
					{
						unit->interceptor.parent->carrier.outHangarCount--;
					}
					else
					{
						unit->interceptor.parent->carrier.inHangarCount--;
					}

					if (unit->interceptor.hangar_link.prev != nullptr)  // untie the previous interceptor/scarab from current interceptor/scarab (tie it to next)
					{
						(unit->interceptor.hangar_link.prev)->interceptor.hangar_link.next = unit->interceptor.hangar_link.next;
					}
				}
				else
				{
					unit->interceptor.hangar_link.prev = nullptr;
					unit->interceptor.hangar_link.next = nullptr;
				}
			} break;
			case UnitId::ResourceMineralField:
			case UnitId::ResourceMineralFieldType2:
			case UnitId::ResourceMineralFieldType3:
			case UnitId::ResourceVespeneGeyser:
			case UnitId::TerranReservoir:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExcisant:
			{
				free_resource_container(unit);
			} break;
			case UnitId::ProtossAugur:
			{
				augurSacrifice(unit);
			} break;
		}
	}

	{ // Spellcaster - restore energy on ally kill
		UnitArray unit_array = UnitSearchRadial(unit->position, 11 * 32);
		UnitArrayFilter(&unit_array, unit, FilterEnemy | FilterSpellcaster);

		for (u32 i = 0; i < unit_array.size; ++i)
		{
			Unit* caster = unit_array.units[i];
			if (scbw::getDistanceFast(unit->position, caster->position) <= caster->getSightRange() * 32)
			{
				caster->healEnergy(5 * 256);
			}
		}
	}

	// Nathrokor - Adrenal Frenzy
	for (u32 i = 0; i < 8; i++)
	{
		u32 stacks = unitExt->nathrokorTargetStacks[i];
		if (stacks)
		{
			const u32 radius = 3 * 32;
			scbw::UnitFinder(
				unit->getX() - radius,
				unit->getY() - radius,
				unit->getX() + radius,
				unit->getY() + radius
			).forEach([&](Unit* target)
			{
				if (scbw::isAlliedTo(i, target->player))
				{
					UnitExt* targetExt = globals.get_unit_ext(target);
					targetExt->timers[TimerNathrokorBuff] = NathrokorAdrenalFrenzyTimer;
					targetExt->values[ValueNathrokorBuffStacks] += stacks;
					target->updateSpeed();
				}
			});
		}
	}

	// Alkajelisk Parasitic Spores behavior
	if (unitExt->values[ValueAlkajeliskParasite] == 1)
	{
		unit->orderTarget.pt.x = unit->position.x + 1;
		unit->orderTarget.pt.y = unit->position.y;
		auto bullet = helper::createBullet(WeaponId::AlkajeliskParasiticSpores, unit, unit->position.x, unit->position.y, unitExt->values[ValueAlkajeliskParasitePlayer], 0);
		if (bullet != nullptr)
		{
			bullet->time_remaining = 255;
		}
	}

	// Madrigal - Dirge
	if (unitExt->timers[TimerDirgeFuse])
	{
		scbw::dirgeFuse(unit, unitExt);
	}

	// Apostle Lazarus Agent - imply revive
	if (unitExt->values[ValueLazarusAgent] || unitExt->timers[TimerAnticthonRevive])
	{
		scbw::createAndInitThingy(SpriteId::ReconstitutionConsumption, unit->sprite->position.x, unit->sprite->position.y - 12);
	}

	// Nuclear Silo - death
	if (unit->id == UnitId::TerranNuclearSilo && unit->building.silo.isReady && !unitExt->timers[ControlledDemolitionTimer])
	{
		scbw::createAndInitThingy(SpriteId::Nuke_Hit, unit->getX(), unit->getY(), 11, 12);
		helper::createBullet(WeaponId::NuclearMissile, unit, unit->getX(), unit->getY(), unit->player, 0);
	}

	// Apostle Last Breath behavior
	{
		u32 range = 4 * 32;
		scbw::UnitFinder(
			unit->getX() - range,
			unit->getY() - range,
			unit->getX() + range,
			unit->getY() + range
		).forEach([&](Unit* target)
		{
			if (target->id != UnitId::TerranApostle)
			{
				return;
			}
			if (target == unit)
			{
				return;
			}
			if (!scbw::isAlliedTo(unit->player, target->player))
			{
				return;
			}
			target->healEnergy(ApostleLastBreathEnergyGain);
			target->sprite->createUnderlay(ImageId::SmallExplosion_Unused);
		});
	}

	// Parasite - spawn broodlings on death
	if (unit->parasiteFlags != 0)
	{
		u32 supplyCount = scbw::getBroodlingCountFromSupply(unit);
		u16 type = UnitId::ZergRilirokor;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Flyer)
		{
			type = UnitId::ZergRililisk;
		}
		for (u32 player = 0; player < PLAYER_COUNT; player++)
		{
			if (unit->parasiteFlags & (1 << player) && unitExt->timers[TimerParasiteArm] <= 1)
			{
				scbw::createUnitFromAbility(type, supplyCount, player, unit->position.x, unit->position.y, 720);
			}
		}
	}

	// Metastasis - Plague spreads when a unit dies
	if (unit->plagueTimer)
	{
		helper::createBullet(WeaponId::Plague, unit, unit->position.x, unit->position.y, unit->player, unit->currentDirection1);
		u32 range = 48;
		Box16 coords = {};
		coords.left   = unit->position.x - range;
		coords.top    = unit->position.y - range;
		coords.right  = unit->position.x + range;
		coords.bottom = unit->position.y + range;
		helper::IterateUnitsAtLocationTargetProc_Plague(unit, &coords);
	}

	// Update Custom Creep Disappearance
	scbw::update_custom_creep_disappearance(unit);

	// Globals memory management
	globals.remove_unit_from_pariah_table(unit, false);
	globals.unitDeathHook(unit);
}

void clear_tile_flags_from_area(u32 flags, u32 tile_width, u32 tile_height, s32 tile_x, s32 tile_y) {
	for (s32 y = tile_height; y >= 0; --y) {
		for (s32 x = tile_width; x >= 0; --x) {
			s32 fx = tile_x + x;
			s32 fy = tile_y + y;
			if (fx >= 0 && fx < mapTileSize->width && fy >= 0 && fy < mapTileSize->height) {
				(*activeTileHandle)[fx + fy * mapTileSize->width] &= ~flags;
			}
		}
	}
}

//-------- Actual hooking --------//

void __declspec(naked) unit_destructor_special_cases_wrapper()
{

	static Unit* unit;

	__asm {
		MOV unit, EAX
		PUSHAD
	}

	unit_destructor_special_cases(unit);

	__asm {
		POPAD
		RETN
	}
}

void __declspec(naked) clear_tile_flags_from_area_wrapper() {
	static u32 flags;
	static u32 tile_width;
	static u32 tile_height;
	static s32 tile_x;
	static s32 tile_y;
	__asm {
		PUSH EBP
		MOV EBP, ESP
		PUSH EAX
		MOV EAX, [EBP + 0x8]
		MOV tile_x, EAX
		MOV EAX, [EBP + 0xC]
		MOV tile_y, EAX
		POP EAX
		MOV flags, EAX
		MOV tile_width, EDI
		MOV tile_height, ECX
		PUSHAD
	}
	clear_tile_flags_from_area(flags, tile_width, tile_height, tile_x, tile_y);
	__asm {
		POPAD
		MOV ESP, EBP
		POP EBP
		RETN 0x8
	}
}

namespace hooks
{
	void injectUnitDestructorSpecial()
	{
		jump_patch(unit_destructor_special_cases_wrapper,	0x0049F4A0, 2);
		jump_patch(clear_tile_flags_from_area_wrapper,		0x00472E50, 2);
	}
}
