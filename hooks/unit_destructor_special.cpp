// All functions in this file are heavily involved with memory management.

#include "unit_destructor_special.h"
#include "../SCBW/api.h"
#include "../hook_tools.h"
#include "hooks/weapons/weapon_spellhit.h"
#include "logger.h"
#include <SCBW/UnitFinder.h>
#include "globals.h"

void augur_proc(Unit* augur, s32 radius) {
	Unit* target = scbw::UnitFinder::get_nearest_target(
		augur->get_x() - radius,
		augur->get_y() - radius,
		augur->get_x() + radius,
		augur->get_y() + radius,
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
			if (scbw::is_allied_to(unit->player, augur->player))
			{
				return false;
			}
			if (unit->status & UnitStatus::Invincible)
			{
				return false;
			}
			return true;
		});
	Thingy* thingy = helper::create_thingy(SpriteId::AugurDeathExplosion, augur->get_x(), augur->get_y(), augur->player);
	helper::play_sound_from_unit(scbw::rand_between(SoundId::AugurDeath01, SoundId::AugurDeath04), (Unit*)thingy, 1, 0);
	if (target)
	{
		augur->order_target.unit = target;
		helper::create_bullet(WeaponId::AugurFinalHour, augur, augur->get_x(), augur->get_y(), augur->player, scbw::get_angle(augur->get_x(), augur->get_y(), target->get_x(), target->get_y()));
	}
}

void augur_sacrifice(Unit* augur, s32 radius)
{
	UnitExt* targetExt = globals.get_unit_ext(augur);
	if (targetExt->values[ValueAugurSafetyCheck] == 0)
	{
		targetExt->values[ValueAugurSafetyCheck] = 1;
		augur_proc(augur, radius);
		augur->remove();
	}
}

// Identical to 00466170 kill_hangar_units
// Not hooked but used by hooked unit_destructor_special_cases
void kill_hangar_units(Unit* unit)
{
	u32 random_value;
	while (unit->unit_with_hangar.in_hangar_count != 0)
	{
		Unit* childInside = unit->unit_with_hangar.in_hangar_child;
		unit->unit_with_hangar.in_hangar_child = childInside->hangar_unit.hangar_link.prev;
		childInside->hangar_unit.parent = nullptr;
		childInside->remove();
		unit->unit_with_hangar.in_hangar_count--;
	}
	while (unit->unit_with_hangar.out_hangar_count != 0)
	{
		Unit* childOutside = unit->unit_with_hangar.out_hangar_child;
		childOutside->hangar_unit.parent = nullptr;
		// Kill Stewards only (Plasma Shells will defuse anyway)
		if (childOutside->id != UnitId::ProtossPlasmaShell)
		{
			u16 randomDeathTimer;
			// TODO: Remove RNG, make farthest Stewards detonate first
			// hardcoded RNG management
			if (*is_in_game_loop)
			{
				static u32* randomnessCounter_0051C6AC = (u32*)0x0051C6AC;
				static u32* randomnessCounter_0051CA18 = (u32*)0x0051CA18;
				*randomnessCounter_0051C6AC = *randomnessCounter_0051C6AC + 1;
				*randomnessCounter_0051CA18 = *randomnessCounter_0051CA18 + 1;
				*last_random_number = (*last_random_number * 0x015A4E35) + 1;
				// number between 0 and 32766
				random_value = (*last_random_number / 65536) & 0x7FFF;
			}
			else
			{
				random_value = 0;
			}
			// should produce a number between 15 and 45 (or maybe 46)
			randomDeathTimer = ((random_value * 31) / 32768) + 15;
			if (childOutside->remove_timer == 0
			||  randomDeathTimer < childOutside->remove_timer)
			{
				childOutside->remove_timer = randomDeathTimer;
			}
		}
		// 00466239
		unit->unit_with_hangar.out_hangar_child = childOutside->hangar_unit.hangar_link.prev;
		childOutside->hangar_unit.hangar_link.prev = nullptr;
		childOutside->hangar_unit.hangar_link.next = nullptr;
		unit->unit_with_hangar.out_hangar_count--;
	}
	unit->unit_with_hangar.out_hangar_child = nullptr;
}

// Identical to 00468770 remove_harvesters
// Not hooked but used by hooked unit_destructor_special_cases
void free_resource_container(Unit* resource)
{
	resource->building.resource.gather_queue_count = 0;
	Unit* worker = resource->building.resource.next_gatherer;
	Unit* nextWorker;
	while (worker != nullptr)
	{
		if (worker->worker.harvest_link.prev != nullptr) // untie the previous worker from current worker (tie it to next)
		{
			(worker->worker.harvest_link.prev)->worker.harvest_link.next = worker->worker.harvest_link.next;
		}
		else
		{
			resource->building.resource.next_gatherer = worker->worker.harvest_link.next;
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
	unit->reset_supply_provided();

	/*
	if (!(unit->user_action_flags & 4) && // for silent remove
		  unit->status & UnitStatus::Completed && 
	     (unit->id == UnitId::ZergHatchery || unit->id == UnitId::ZergLair ||
		  unit->id == UnitId::ZergHive     || unit->id == UnitId::ZergSire ||
		  units_dat::base_property[unit->id] & UnitProperty::CreepBuilding) ||
		  unitExt->values[ValueBlissfulEmbraceOriginalPlayer])
	{
		scbw::clear_creep(scbw::unit_area(unit));
	}
	*/

	// These do not require the unit to be completed
	switch (unit->id)
	{
		case UnitId::TerranEidolon:
		case UnitId::TerranSeraph:
		{
			if (unit->building.nuke_missile != nullptr)
			{
				(unit->building.nuke_missile)->sprite->play_iscript_animation(IscriptAnimation::Death, true);
				unit->building.nuke_missile = nullptr;
			}
		} break;
		case UnitId::TerranNuclearMissile:
		{
			if (unit->connected_unit != nullptr && unit->connected_unit->id == UnitId::TerranNuclearSilo)
			{
				(unit->connected_unit)->building.silo.nuke = nullptr;
				(unit->connected_unit)->building.silo.is_ready = false;
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
			// code from hooks::remove_psi_field(Unit* unit)
			// copied because the content is hardcoded here
			if (unit->building.pylon_aura != nullptr)
			{
				unit->building.pylon_aura->free();	// should be equivalent to SpriteDestructor @ 00497B40
				unit->building.pylon_aura = nullptr;
			}

			// begin hardcoding remove_from_psi_providers_list @ 00493100 to avoid use of a helper
			if (unit->psi_link.prev != nullptr)  // untie the previous pylon from current pylon (tie it to next)
			{
				(unit->psi_link.prev)->psi_link.next = unit->psi_link.next;
			}
			if (unit->psi_link.next != nullptr)  // untie the next pylon from current pylon (tie it to previous)
			{
				(unit->psi_link.next)->psi_link.prev = unit->psi_link.prev;
			}

			if (unit == *first_psi_field_provider)
			{
				*first_psi_field_provider = unit->psi_link.next;
			}
			unit->psi_link.prev = nullptr;
			unit->psi_link.next = nullptr;
			// end hardcoding remove_from_psi_providers_list @ 00493100 to avoid use of a helper
			*can_update_powered_status = true;
		} break;
		case UnitId::ZergNydusCanal:
		{
			Unit* nydus_exit = unit->building.nydus_exit;
			if (nydus_exit != nullptr)
			{
				unit->building.nydus_exit = nullptr;
				nydus_exit->building.nydus_exit = nullptr;
				nydus_exit->remove();
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
				globals.create_effect(EffectId_Criticality, 10 * 24, { unit->get_x(), unit->get_y() }, 2 * 32, SpriteId::Criticality, 5, unit->player, unit);
			} break;
			case UnitId::ProtossClarion:
			{
				// Phase Link
				scbw::clear_clarion_links(unit);
			} break;
			case UnitId::ProtossVassal:
			{
				// Terminal Surge
				u32 range = 5 * 32;
				scbw::UnitFinder(
					unit->get_x() - range,
					unit->get_y() - range,
					unit->get_x() + range,
					unit->get_y() + range
				).for_each([&](Unit* target)
					{
						UnitExt* targetExt = globals.get_unit_ext(target);
						if (!scbw::is_allied_to(unit->player, target->player))
						{
							return;
						}
						targetExt->values[ValueTerminalSurge]++;
						target->update_speed();
						
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
				if (unit->hangar_unit.parent != nullptr)
				{
					if (unit->hangar_unit.hangar_link.next != nullptr) // untie the next hangar_unit/scarab from current hangar_unit/scarab (tie it to previous)
					{
						(unit->hangar_unit.hangar_link.next)->hangar_unit.hangar_link.prev = unit->hangar_unit.hangar_link.prev;
					}
					else
					{
						if (unit->hangar_unit.is_outside_hangar)
						{
							unit->hangar_unit.parent->unit_with_hangar.out_hangar_child = unit->hangar_unit.hangar_link.prev;
						}
						else
						{
							unit->hangar_unit.parent->unit_with_hangar.in_hangar_child = unit->hangar_unit.hangar_link.prev;
						}
					}
					if (unit->hangar_unit.is_outside_hangar)
					{
						unit->hangar_unit.parent->unit_with_hangar.out_hangar_count--;
					}
					else
					{
						unit->hangar_unit.parent->unit_with_hangar.in_hangar_count--;
					}

					if (unit->hangar_unit.hangar_link.prev != nullptr)  // untie the previous hangar_unit/scarab from current hangar_unit/scarab (tie it to next)
					{
						(unit->hangar_unit.hangar_link.prev)->hangar_unit.hangar_link.next = unit->hangar_unit.hangar_link.next;
					}
				}
				else
				{
					unit->hangar_unit.hangar_link.prev = nullptr;
					unit->hangar_unit.hangar_link.next = nullptr;
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
				augur_sacrifice(unit);
			} break;
		}
	}

	{ // Spellcaster - restore energy on ally kill
		EntryArray unit_array = unit_search_radial(unit->position, 11 * 32);
		unit_search_filter(&unit_array, unit, FilterEnemy | FilterSpellcaster);

		for (u32 i = 0; i < unit_array.count; ++i)
		{
			Unit* caster = unit_array.units[i];
			if (scbw::get_distance_fast(unit->position, caster->position) <= caster->get_sight_range() * 32)
			{
				caster->heal_energy(5 * 256);
			}
		}
	}

	// Nathrokor - Adrenal Frenzy
	for (u32 i = 0; i < 8; i++)
	{
		u32 stacks = unitExt->nathrokor_target_stacks[i];
		if (stacks)
		{
			const u32 radius = 3 * 32;
			scbw::UnitFinder(
				unit->get_x() - radius,
				unit->get_y() - radius,
				unit->get_x() + radius,
				unit->get_y() + radius
			).for_each([&](Unit* target)
			{
				if (scbw::is_allied_to(i, target->player))
				{
					UnitExt* targetExt = globals.get_unit_ext(target);
					targetExt->timers[TimerNathrokorBuff] = NathrokorAdrenalFrenzyTimer;
					targetExt->values[ValueNathrokorBuffStacks] += stacks;
					target->update_speed();
				}
			});
		}
	}

	// Alkajelisk Parasitic Spores behavior
	if (unitExt->values[ValueAlkajeliskParasite] == 1)
	{
		unit->order_target.pt.x = unit->position.x + 1;
		unit->order_target.pt.y = unit->position.y;
		auto bullet = helper::create_bullet(WeaponId::AlkajeliskParasiticSpores, unit, unit->position.x, unit->position.y, unitExt->values[ValueAlkajeliskParasitePlayer], 0);
		if (bullet != nullptr)
		{
			bullet->time_remaining = 255;
		}
	}

	// Madrigal - Dirge
	if (unitExt->timers[TimerDirgeFuse])
	{
		scbw::madrigal_dirge_fuse(unit, unitExt);
	}

	// Apostle Lazarus Agent - imply revive
	if (unitExt->values[ValueLazarusAgent] || unitExt->timers[TimerAnticthonRevive])
	{
		scbw::create_and_init_thingy(SpriteId::ReconstitutionConsumption, unit->sprite->position.x, unit->sprite->position.y - 12);
	}

	// Nuclear Silo - death
	if (unit->id == UnitId::TerranNuclearSilo && unit->building.silo.is_ready && !unitExt->timers[TimerControlledDemolition])
	{
		scbw::create_and_init_thingy(SpriteId::Nuke_Hit, unit->get_x(), unit->get_y(), 11, 12);
		helper::create_bullet(WeaponId::NuclearMissile, unit, unit->get_x(), unit->get_y(), unit->player, 0);
	}

	// Apostle Last Breath behavior
	{
		u32 range = 4 * 32;
		scbw::UnitFinder(
			unit->get_x() - range,
			unit->get_y() - range,
			unit->get_x() + range,
			unit->get_y() + range
		).for_each([&](Unit* target)
		{
			if (target->id != UnitId::TerranApostle)
			{
				return;
			}
			if (target == unit)
			{
				return;
			}
			if (!scbw::is_allied_to(unit->player, target->player))
			{
				return;
			}
			target->heal_energy(ApostleLastBreathEnergyGain);
			target->sprite->create_underlay(ImageId::SmallExplosion_Unused);
		});
	}

	// Parasite - spawn broodlings on death
	if (unit->parasite_flags != 0)
	{
		u32 supplyCount = scbw::get_broodling_count_from_supply(unit);
		u16 type = UnitId::ZergRilirokor;
		if (units_dat::base_property[unit->id] & UnitProperty::Flyer)
		{
			type = UnitId::ZergRililisk;
		}
		for (u32 player = 0; player < PLAYER_COUNT; player++)
		{
			if (unit->parasite_flags & (1 << player) && unitExt->timers[TimerParasiteArm] <= 1)
			{
				scbw::create_unit_from_ability(type, supplyCount, player, unit->position.x, unit->position.y, 720);
			}
		}
	}

	// Metastasis - Plague spreads when a unit dies
	if (unit->plague_timer)
	{
		helper::create_bullet(WeaponId::Plague, unit, unit->position.x, unit->position.y, unit->player, unit->current_direction_1);
		u32 range = 48;
		Box16 coords = {};
		coords.left   = unit->position.x - range;
		coords.top    = unit->position.y - range;
		coords.right  = unit->position.x + range;
		coords.bottom = unit->position.y + range;
		helper::iterate_units_at_location_target_proc_plague(unit, &coords);
	}

	// Update Custom Creep Disappearance
	scbw::update_custom_creep_disappearance(unit);

	// Globals memory management
	globals.remove_unit_from_pariah_table(unit, false);
	globals.unit_death(unit);
}

void clear_tile_flags_from_area(u32 flags, u32 tile_width, u32 tile_height, s32 tile_x, s32 tile_y) {
	for (s32 y = tile_height - 1; y >= 0; --y) {
		for (s32 x = tile_width - 1; x >= 0; --x) {
			s32 fx = tile_x + x;
			s32 fy = tile_y + y;
			if (fx >= 0 && fx < map_tile_size->width && fy >= 0 && fy < map_tile_size->height) {
				(*activeTileArrayInt)[fx + fy * map_tile_size->width] &= ~flags;
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
	void inject_unit_destructor_special_hooks()
	{
		jump_patch(unit_destructor_special_cases_wrapper,	0x0049F4A0, 2);
		jump_patch(clear_tile_flags_from_area_wrapper,		0x00472E50, 2);
	}
}