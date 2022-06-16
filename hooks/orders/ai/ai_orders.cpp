#include "ai_orders.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "logger.h"
#include "base_layouts.h"
#include "globals.h"

namespace hooks
{
	// Hooked to modify AI behavior such as cancelling structures, withdrawing hurt high-value targets, etc -- Pr0nogo
	// TODO: finish and cleanup
	void ai_unit_was_hit(Unit* self, Unit* attacker, s32 main_target_reactions, s32 dont_call_help) // 0043F320
	{
		AiRegion* v14;
		s32 v15;
		AiRegion* selfRegion;
		char v18;
		s16 v19;
		s16 child0;
		s32 child1;
		s32 child3;
		s32 child5;
		s32 child6;
		Unit* child7;
		__int64 parentAi3;
		Unit* parentAi4;
		Unit* parentAi6;
		Unit* parentAi7;
		s32 parentAi9;
		s32 childAi0;
		s32 childAi1;
		s32 childAi2;
		s32 childAi4;

		if (playerTable[self->player].type != PlayerType::Computer || self->player >= 8u)
		{
			return;
		}
		if (units_dat::base_property[(u16)self->id] & UnitProperty::Building)
		{
			if (self->unit_ai)
			{
				AIScriptController[self->player].previous_building_hit_second = *elapsed_time_seconds;
				BuildingAi* selfBuildingAi = (BuildingAi*)self->unit_ai;
				if (selfBuildingAi)
				{
					selfBuildingAi->town->inBattle = 1;
				}
				if (main_target_reactions)
				{
					if (attacker)
					{
						if (units_dat::base_property[(u16)self->id] & UnitProperty::Worker // cancel structure code
							&& !(self->main_order_id) // Original function call checks if unit has a sprite??? 00475A50
							&& !(self->status & UnitStatus::Completed))
						{
							s32 quartTotalHP = self->get_total_hp_plus_shields() / 4;
							if (self->get_current_life_ingame() < quartTotalHP)
							{
								helper::ai_cancel_structure(self); // originally unused variable v8
								return;
							}
						}
					}
				}
			}
		}
		Unit* target = helper::ai_get_actual_target(self, attacker);
		Unit* actual_target = target;
		if (target->status & UnitStatus::InBuilding)
		{
			actual_target = target->connected_unit;
			if (!actual_target)
			{
				return;
			}
		}
		if (actual_target->status & UnitStatus::CloakingForFree)
		{
			if (!(actual_target->status & UnitStatus::CloakingForFree))
			{
				actual_target = helper::check_arbiter_cloaking_range(self, UnitId::ProtossDidact);
				if (!actual_target)
				{
					return;
				}
			}
		}
		if (main_target_reactions)
		{
			helper::ai_order_unit_cloaking(self);
			helper::ai_order_lurker_burrow(self);
		}
		UnitAi* selfUnitAi = (UnitAi*)self->unit_ai;
		parentAi9 = 0;
		childAi1 = 0;
		if (!selfUnitAi)
		{
			goto LABEL_37;
		}
		if (selfUnitAi->type == 4)
		{
			MilitaryAi* selfMilitaryAi = (MilitaryAi*)selfUnitAi;
			if (selfMilitaryAi->region->state == 1)
			{
				if (self->is_unit_ai_non_building())
				{
					helper::ai_set_region_type(2, selfMilitaryAi->region);
				}
			}
			childAi1 = helper::ai_is_under_attack(self, 0);
		}
		if (!main_target_reactions || !helper::unit_is_active_transport(self) || !(self->status & UnitStatus::IsBuilding))
			goto LABEL_37;
		if (helper::is_unit_loaded(self))
		{
			if (self->main_order_id != 111)
			{
				helper::issue_order_targeting_nothing(self, 0x6Fu);
			}
			return;
		}
		if (helper::ai_try_return_home(self, 0) || !helper::ai_transport_action(self))
		{
			if (AIScriptController[(u8)self->player].AI_Flags.is_use_map_settings)
			{
				parentAi9 = 1;
			}
LABEL_37:
			v14 = &AiRegionCaptains[(u8)self->player][helper::get_region_id(self->sprite->position.x, self->sprite->position.y)];
			if (!parentAi9)
			{
				if (!dont_call_help)
				{
					helper::ai_ask_for_help(self, actual_target, childAi1);
				}
				if (!childAi1)
				{
					if (!self->unit_ai
						|| helper::ai_is_under_attack(self, 0)
						|| (childAi2 = 1, *(s8*)(v15 + 8) == 1)
						&& AIScriptController[(u8)self->player].AI_Flags.is_use_map_settings)
					{
						childAi2 = 0;
					}
					selfRegion = &AiRegionCaptains[(u8)self->player][helper::get_region_id_from_unit(actual_target)];
					if (childAi2)
					{
						v18 = v14->state;
						if (v18 == 5 || v18 == 4)
						{
							helper::ai_set_region_type(6, v14);
						}
						if (v14->state)
						{
							if (!selfRegion->state && *elapsed_time_seconds > 60)
								helper::ai_set_region_type(5, selfRegion);
						}
					}
					if (!dont_call_help)
					{
						selfRegion->enemyAirStrength = helper::get_enemy_air_strength_in_region(selfRegion->region, (u8)self->player);
						selfRegion->enemyGroundStrength = helper::get_enemy_strength_in_region(selfRegion->region, (u8)self->player, 0);
						v19 = selfRegion->neededAirStrength;
						if (v19 > v14->neededAirStrength)
						{
							v14->neededAirStrength = v19;
						}
						child0 = selfRegion->neededGroundStrength;
						if (child0 > v14->neededGroundStrength)
						{
							v14->neededGroundStrength = child0;
						}
					}
				}
			}
			v14->captainFlags |= 0x20u;
			if (self->status & UnitStatus::Completed)
			{
				child1 = (u8)self->main_order_id;
				childAi0 = (u8)self->main_order_id;
				if (child1 != OrderId::CompletingArchonSummon && child1 != OrderId::ResetCollision1 && child1 != OrderId::ConstructingBuilding)
				{
					if (main_target_reactions || !scbw::order_is_spell(child1))
					{
						child3 = (u16)self->id;
						if (self->id || child1 != OrderId::EnterTransport)
						{
							childAi4 = helper::unit_is_performing_unbreakable_code(self);
							if (childAi4 || !helper::ai_cast_spell_behavior(self, main_target_reactions))
							{
								if (child1 != OrderId::RechargeShields1 && child1 != OrderId::Move)
								{
									if (!main_target_reactions)
									{
										goto LABEL_126;
									}
									if (child3 != UnitId::ProtossSolarion
//										|| (u8)(LOBYTE(self->field_C8) + BYTE1(self->unit_with_hangar.in_hangar_count)) > 2u
										|| !helper::ai_transport_action(self))
									{
										if (actual_target && actual_target->status & UnitStatus::IsBuilding // Attempt to return to base when below an HP threshold
											|| AIScriptController[(u8)self->player].AI_Flags.is_use_map_settings
											|| ((child5 = 3 * self->get_total_hp_plus_shields() / 4, self->get_current_life_ingame() > child5)
												|| child3 != UnitId::ProtossDidact && child3 != UnitId::TerranWraith && child3 != UnitId::ZergMutalisk
												|| !helper::ai_transport_action(self))
											&& ((child6 = 2 * self->get_total_hp_plus_shields() / 3, self->get_current_life_ingame() > child6)
												|| child3 != UnitId::TerranMinotaur && child3 != UnitId::ProtossSolarion && child3 != UnitId::ProtossPanoptus
												|| !helper::ai_transport_action(self)))
										{
											if (self->id != UnitId::ZergLakizilisk
												|| self->status & UnitStatus::Burrowed
												|| childAi0 == OrderId::Burrow
												|| childAi0 == OrderId::Unburrow)
											{
												child7 = actual_target;
											}
											else
											{
												child7 = actual_target;
												if (helper::ai_flee(self, actual_target))
												{
													return;
												}
											}
											if (childAi4
												|| !(self->status & UnitStatus::IsBuilding)
												|| helper::unit_has_ground_path(self, child7->sprite->position.x, child7->sprite->position.y)
												|| !helper::ai_flee(self, child7))
											{
									LABEL_126:
												if (self->order_target.unit != actual_target)  // Responding with worker
												{
													if (!(units_dat::base_property[(u16)self->id] & UnitProperty::Worker)
														|| (units_dat::base_property[actual_target->id] & UnitProperty::Worker)
														|| !scbw::unit_is_ranged_attacker(actual_target) // previously only checked for Zethrokor id
														|| helper::ai_has_no_defense_build((u8)self->id))
													{
														if (main_target_reactions)
														{
															if (self->unit_ai)
															{
																if (helper::unit_can_see_cloaked_target(actual_target, self))
																{
																	helper::ai_respond_to_cloaked_threat(self, actual_target);
																}
															}
														}
														if (!childAi4)
														{
															if (helper::unit_can_attack_target(actual_target, self, 1))
															{
																parentAi6 = self->subunit;
																parentAi7 = self->subunit;
																self->auto_target_unit = actual_target;
																if (parentAi7->is_subunit())
																{
																	parentAi6->auto_target_unit = actual_target;
																}
																helper::ai_update_attack_target(self, 0, 1, 0);
															}
															else if (self->unit_ai)
															{
																if (childAi0 != 159
//																	|| (parentAi3 = ai_prepare_to_flee(actual_target, self), (s32)parentAi3 == *(s32*)&self->sprite->position)
																	)
																{
																	if (main_target_reactions)
																	{
																		if (self->id != 34)
																		{
																			parentAi4 = self->order_target.unit;
																			if (!parentAi4 || !helper::unit_can_attack_target(parentAi4, parentAi4, 1))
																			{
																				helper::ai_flee(self, actual_target);
																			}
																		}
																	}
																}
																else
																{
//																	order_interrupt(self, 6u, (Point32*)parentAi3, (Unit*)HIDWORD(parentAi3)); // point32 was dwPosition, I think that's the same thing
																}
															}
														}
													}
													else if (main_target_reactions)
													{
														helper::ai_flee(self, actual_target);
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Hooked to help with AI recloaking units after the cloak rewrite - Pr0nogo
	void ai_order_unit_cloaking(Unit* unit) // 0043B970
	{
		return;
		/*
		s32 unitStatus = unit->status;
		if (!(unitStatus & 0x100))
		{
			s8 techId;
			if (unit->id == UnitId::TerranEidolon)
			{
				techId = TechId::PersonnelCloaking;
			}
			else if (unit->id == UnitId::TerranWraith)
			{
				techId = TechId::CloakingField;
			}
			if (helper::tech_use_allowed(unit, techId, unit->player) == 1)
			{
				helper::applyCloakingOrder(unit);
			}
		}
		*/
	}

	// Hooked to improve functionality - Pr0nogo
	void ai_send_suicide(char randomSuicide, s32 player) // 0043E050
	{
		if (randomSuicide)
		{
			for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next)
			{
				if (unit->sprite)
				{
					if (unit->main_order_id)
					{
						if (!unit->is_subunit()
							&& unit->id != UnitId::TerranNuclearMissile
							&& unit->id != UnitId::ZergLarva
							&& !(units_dat::base_property[unit->id] & UnitProperty::Worker)
							&& !(unit->sprite->flags & SpriteFlags::Hidden)
							&& !helper::is_threat(0, unit))
						{
							helper::prepare_suicide(unit);
						}
					}
				}
			}
			return;
		}

		if (!AIScriptController[player].strategic_suicide)
		{
			AiRegion* region2 = 0;

			s32 regionIndex = 2500;
			for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next)
			{
				if (!unit->sprite)
				{
					continue;
				}
				if (!unit->main_order_id)
				{
					continue;
				}
				GuardAi* guardAI = (GuardAi*)unit->unit_ai;
				if (!guardAI
				  || helper::unit_is_active_transport(unit)
				  || unit->status & UnitStatus::InBuilding && !(AIScriptController[player].AI_Flags.is_use_map_settings))
				{
					continue;
				}
				s16 regionId = helper::get_region_id(unit->sprite->position.x, unit->sprite->position.y);
				AiRegion* region = &AiRegionCaptains[player][regionId];
				u8 aiType = guardAI->type;
				if (aiType == 1)
				{
					guardAI->parent->unit_ai = nullptr;
					helper::add_military_ai(region, guardAI->parent, 1);
					guardAI->parent->unit_ai = nullptr;
				}
				else if (aiType != 4)
				{
					continue;
				}
				MilitaryAi* unitAI2 = (MilitaryAi*)unit->unit_ai;
				if (unitAI2)
				{
					if (unitAI2->type == 4)
					{
						MilitaryAi* militaryAI = unitAI2;
						u8 regionType = militaryAI->region->state;
						if (regionType == 1 || regionType == 8 || regionType == 9 || regionType == 2)
						{
							continue;
						}
					}
				}
				if (regionIndex != 2500)
				{
					helper::remove_unit_ai(unit, nullptr);
					helper::add_military_ai(region2, unit, 1);
					continue;
				}
				memset(AIScriptController[player].attackers, 0, sizeof(AIScriptController[player].attackers));
				AIScriptController[player].ai_attack_group = 0;
				helper::ai_attack_clear(player);
				if (!helper::ai_attack_manager(player, unit->sprite->position.x, unit->sprite->position.y, 0, 1))
				{
					AIScriptController[player].strategic_suicide = 24;
					regionIndex = (u16)AIScriptController[player].ai_attack_group - 1;
					region2 = &AiRegionCaptains[player][regionIndex];
					helper::remove_unit_ai(unit, nullptr);
					helper::add_military_ai(region2, unit, 1);
				}
			}
		}
	}

	// Hooked to investigate crashes - Pr0nogo
	u32 ai_recalculate_region_strength(AiRegion* region) // 0043A390
	{
		const u32 Func_CB_AICaptainStrengthIterator = 0x00438F60;
		MilitaryAi* military_ai = region->first_military_ai;
		region->regionAirStrength = 0;
		region->fullAirStrength = 0;
		region->regionGroundStrength = 0;
		region->fullGroundStrength = 0;
			
		for (MilitaryAi* i = military_ai; i; i = i->link.prev)
		{
			Unit* townWorker = (Unit*)i->unit;
			if (!townWorker)
			{
				continue;
			}
			if (!(townWorker->status & UnitStatus::InBuilding))
			{
				s32 townUnitGroundStrength = townWorker->ground_strength;
				s16 townUnitAirStrength = townWorker->air_strength;
				region->fullAirStrength += townUnitAirStrength;
				region->fullGroundStrength += townUnitGroundStrength;
				Sprite* townUnitSprite = townWorker->sprite;
				if (!townUnitSprite)
				{
					continue;
				}
				s32 v6 = townUnitSprite->position.x;
				s32 regionTown6 = townUnitGroundStrength;
				if (helper::get_region_id(v6, townUnitSprite->position.y) == region->region)
				{
					region->regionAirStrength += townUnitAirStrength;
					region->regionGroundStrength += regionTown6;
				}
			}
		}

		if (AIScriptController[region->player].AI_Flags.is_use_map_settings)
		{
			s32 v8 = region->region << 6;
			s32 v9 = *(s32*)((char*)&(*SAI_Paths)->regions[0].regionCenterY + v8);
			s32 regionTown0 = *(s32*)((char*)&(*SAI_Paths)->regions[0].regionCenterX + v8) >> 8;
			v9 >>= 8;
			s16 a1 = regionTown0 - 256;
			s32 regionTown6 = (v9 + 256) << 16;
			s16 regionTown5 = v9 - 256;
			regionTown6 |= regionTown0 + 256;
			helper::iterateUnitsAtLocationTargetProc((Box16*)&a1, (Unit*(__fastcall*)(Unit*, Unit*))Func_CB_AICaptainStrengthIterator, (Unit*)region);
		}

		u32 regionAirStrength = region->enemyAirStrength;
		if (!(AIScriptController[region->player].AI_Flags.is_use_map_settings))
		{
			regionAirStrength += regionAirStrength >> 1;
		}
		if (regionAirStrength > region->neededAirStrength)
		{
			region->neededAirStrength = regionAirStrength;
		}

		u32 regionGroundStrength = region->enemyGroundStrength;
		if (!(AIScriptController[region->player].AI_Flags.is_use_map_settings))
		{
			regionGroundStrength += regionGroundStrength >> 1;
		}
		if (regionGroundStrength > region->neededGroundStrength)
		{
			region->neededGroundStrength = regionGroundStrength;
		}

		u16 result = AIScriptController[region->player].ai_max_force;
		if (AIScriptController[region->player].ai_max_force)
		{
			if (region->neededAirStrength > result)
			{
				region->neededAirStrength = result;
			}
			if (region->neededGroundStrength > result)
			{
				region->neededGroundStrength = result;
			}
		}
		return result;
	}	

	// Hooked to change functionality later - Pr0nogo
	Unit* get_closest_enemy_structure(s32 x, s32 y, s32 player) // 004A0F90
	{
		s16 regionId = helper::get_region_id(x, y);
		Unit* firstUnit = *firstVisibleUnit;
		s32 regionOffset = regionId << 6;
		s32 regionCenterX = *(s32*)((char*)&(*SAI_Paths)->regions[0].regionCenterX + regionOffset);
		s32 regionCenterY = *(s32*)((char*)&(*SAI_Paths)->regions[0].regionCenterY + regionOffset);

		Unit* result = nullptr;
		if (firstVisibleUnit)
		{
			// initialize vars used in loop
			Sprite* firstSprite;
			u32 fastDistance = 0;
			u32 maxDistance = 99999999;
			firstSprite = firstUnit->sprite;
			Unit* foundUnit = nullptr;
			u8 firstUnitPlayer;
			do
			{
				if (firstSprite)
				{
					if (firstUnit->main_order_id)
					{
						if (firstUnit->status & UnitStatus::GroundedBuilding)
						{
							firstUnitPlayer = firstUnit->player;
							if (firstUnitPlayer == 11)
								firstUnitPlayer = firstSprite->player;
							if (!scbw::is_allied_to(player, firstUnitPlayer))
							{
								fastDistance = helper::get_distance_fast(regionCenterX >> 8, firstUnit->sprite->position.x, regionCenterY >> 8, firstUnit->sprite->position.y);
								if (fastDistance < maxDistance)
								{
									maxDistance = fastDistance;
									foundUnit = firstUnit;
								}
							}
						}
					}
				}
				firstUnit = firstUnit->link.next;
			} while (firstUnit);
			result = foundUnit;
		}
		return result;
	}	

	s16 get_region_id_from_unit(Unit* unit) // 0049CB40
	{
		if (unit == nullptr || unit->sprite == nullptr)
		{
			return 0;
		}
		return helper::get_region_id(unit->sprite->position.x, unit->sprite->position.y);
	}

	bool get_best_worker_for_repair_proc(Unit* repairman, Unit* repairTarget) // 004402B0
	{
		bool result = false;
		WorkerAi* repairmanAI = (WorkerAi*)repairman->unit_ai;
		if (repairman->player == repairTarget->player
			&& units_dat::base_property[repairman->id] & UnitProperty::Worker
			&& !(repairman->status & UnitStatus::InTransport)
			&& !(repairman->status & UnitStatus::InBuilding) // new
			&& repairmanAI != nullptr
			&& repairmanAI->town != nullptr
			&& repairmanAI->town->building_SCV != nullptr) // new
		{
			result = repairmanAI->town->building_SCV == repairman;
		}
		return result;
	}

	// Hooked to add sprite nullcheck - Pr0nogo
	b32 ai_military_should_wait_for_group(Unit* unit) // 0043AA10
	{
		// new start
		if (unit == nullptr)
		{
			return false;
		}
		if (unit->sprite == nullptr)
		{
			return false;
		}
		// new end

		MilitaryAi* unitAI = (MilitaryAi*)unit->unit_ai;
		if (unitAI == nullptr)
		{
			return false;
		}
		if (unitAI->type != AiType::Military)
		{
			return false;
		}
		if (unitAI->unit != unit)
		{
			return false;
		}
		if (units_dat::base_property[unit->id] & UnitProperty::Building)
		{
			return false;
		}
		if (unitAI->region->state == RegionState::Attacking)
		{
			return false;
		}

		MilitaryAi* i = nullptr;
		MilitaryAi* military_ai = unitAI->region->first_military_ai;
		for (i = military_ai; military_ai; i = military_ai)
		{
			Unit* firstUnit = (Unit*)military_ai->unit;
			if (firstUnit != nullptr && firstUnit->sprite != nullptr && firstUnit != unit)
			{
				u32 distance = helper::get_distance_fast(firstUnit->sprite->position.x, unit->sprite->position.x, firstUnit->sprite->position.y, unit->sprite->position.y);
				if (distance <= 0x180 && helper::ai_are_on_connected_regions(firstUnit, unit) && !(helper::ai_check_military_grouping(firstUnit)) && distance > 0x100)
				{
					return 1;
				}
				military_ai = i;
			}
			military_ai = military_ai->link.prev;
		}

		// Not sure if this is correct
		if (military_ai)
		{
			return true;
		}
		return false;
	}

	// Hooked to add more functionality. - Pr0nogo
	// TODO: Add a conditional timeout for deleting towns, somewhere.
	AiTown* delete_ai_town_if_needed(AiTown* town) // 004347E0
	{
		AiTown* result = helper::try_begin_delete_town(town);
		if (result)
		{
			helper::delete_aiscripts_using_town(town);
			for (Unit* i = *firstVisibleUnit; i; i = i->link.next)
			{
				if (i->sprite != nullptr)
				{
					if (i->main_order_id)
					{
						if (units_dat::base_property[i->id] & UnitProperty::ResourceContainer)
						{
							i->building.resource.resource_group = 0;
						}
					}
				}
			}

			u8 resourceArea = town->resarea;
			if (resourceArea)
			{
				baseArray[resourceArea].flags &= 0xFDu;
			}

			s32 player = (u8)town->player;
			AiTown* firstTown = AiActiveTownArray[player].first;
			AiExpandList* townList = &AiActiveTownArray[player];
			if (firstTown == town)
			{
				townList->first = town->prev;
			}

			AiTown* nextTown = town->next;
			if (nextTown)
			{
				nextTown->prev = town->prev;
			}

			if (town->prev)
			{
				town->prev->next = town->next;
			}

			result = townList->last;
			town->next = 0;
			town->prev = result[100].prev;
			AiTown* previousTown = result[100].prev;
			if (previousTown)
			{
				previousTown->next = town;
			}
			result[100].prev = town;
		}
		return result;
	}

	// Hooked because why not? - Pr0nogo
	bool ai_can_mine_extra(Unit* unit) // 00447A70
	{
		if (!unit->unit_ai)
		{
			return 0;
		}
		s32 player = (u8)unit->player;
		if (AIScriptController[player].difficulty != 2)
		{
			return 0;
		}

		s32 unit_id = UnitId::None;
		u8 raceId = playerTable[player].race;
		switch(raceId)
		{
			case RaceId::Terran:
			{
				unit_id = UnitId::TerranMason;
			} break;
			case RaceId::Protoss:
			{
				unit_id = UnitId::ProtossScribe;
			} break;
			case RaceId::Zerg:
			{
				unit_id = UnitId::ZergDroleth;
			} break;
		}
		unit_id = (u16)unit_id;
		u32 completedCount = unit_count_table[(u16)unit_id].player[(u8)player];

		// Originally had an inlined function that combined sieged and unsieged tanks for unit counts
		return completedCount >= 12;
	}

	// Hooked to fix crashes - Pr0nogo
	bool ai_is_building_safe(Unit* unit) // 00438910
	{
		bool result = 0;
		if (unit != nullptr && unit->sprite != nullptr)
		{
			if (unit->main_order_id)
			{
				s16 regionId = helper::get_region_id(unit->sprite->position.x, unit->sprite->position.y);
				AiRegion* playerRegions = AiRegionCaptains[unit->player];
				char regionState = playerRegions[regionId].state;
				AiRegion* region = &playerRegions[regionId];
				if (regionState != RegionState::EnemySighted && !region->airTarget && !region->groundTarget && !(region->captainFlags & 0x20))
				{
					result = 1;
				}
			}
		}
		return result;
	}

	// Hooked to fix crashes - Pr0nogo
	bool ai_are_on_connected_regions(Unit* unit1, Unit* unit2) // 00438E10
	{
		if (!unit1 || !unit2 || !unit1->sprite || !unit2->sprite)
		{
			return 0;
		}

		u16 regionId = helper::get_region_id(unit2->sprite->position.x, unit2->sprite->position.y);
		u16 regionId2 = helper::get_region_id(unit1->sprite->position.x, unit1->sprite->position.y);
		return helper::ai_regions_are_connected(unit2->player, regionId2, regionId);
	}

	// Hooked to change default behaviors - Pr0nogo
	s32 get_bunker_placement_position(u32 placementId, u32* regionId) // 004442E0
	{
		u32 region_max = *regionIdsCount;
		if (!regionIdsCount)
		{
			return 0;
		}

		s32 result;
		if (*regionIdsCount == 1)
		{
			*regionId = regionIds[0];
			result = 1;
		}
		else
		{
			u32 iterator1 = 0;
			Point16 regionPositions[24];
			u32 tileDistances[24];
			if (regionIdsCount)
			{
				memset(tileDistances, 99999999, *regionIdsCount);
				RegionArray* _paths = *SAI_Paths;
				do
				{
					u32 regionOffset = regionIds[iterator1] << 6;
					u32 region_x = *(u32*)((char*)&_paths->regions[0].regionCenterX + regionOffset) >> 8;
					u32 region_y = *(u32*)((char*)&_paths->regions[0].regionCenterY + regionOffset) >> 8;
					regionPositions[iterator1].x = region_x;
					regionPositions[iterator1++].y = region_y;
				} while (iterator1 < region_max);
			}

			Unit* i;
			for (i = *first_player_unit[placementId].unit; i; i = i->player_link.next)
			{
				if (i->sprite)
				{
					if (i->main_order_id)
					{
						if (i->id == UnitId::TerranBunker || i->id == UnitId::ZergCreepColony || i->id == UnitId::ZergSunkenColony)
						{
							u32 iterator2 = 0;
							if (region_max)
							{
								do
								{ // Distance from unit to region position
									u32 newTileDistance = helper::getTileDistance(i->sprite->position.x, i->sprite->position.y, regionPositions[iterator2].x, regionPositions[iterator2].y);
									if (newTileDistance < tileDistances[iterator2])
									{
										tileDistances[iterator2] = newTileDistance;
									}
									region_max = *regionIdsCount;
									++iterator2;
								} while (iterator2 < *regionIdsCount);
							}
						}
					}
				}
			}

			u32 bestTileDistance = tileDistances[0];
			u32 currentRegionId = 1;
			u32 bestRegionId;
			for (bestRegionId = 0; currentRegionId < region_max; ++currentRegionId)
			{
				if (tileDistances[currentRegionId] > bestTileDistance)
				{
					bestTileDistance = tileDistances[currentRegionId];
					bestRegionId = currentRegionId;
				}
			}
			*regionId = regionIds[bestRegionId];
			result = 1;
		}
		return result;
	}

	// Hooked to change default behaviors - Pr0nogo
	void ai_get_build_location(u32 unit_id, char* placementData, u32 player, Point16 desiredPos, s32 maxRange) // 004461B0
	{
		memset(placementData, 0, 0x1000u);
		helper::ai_get_building_placement_data(maxRange, unit_id, placementData, player, desiredPos);
		helper::ai_remove_unreachable_regions(placementData, desiredPos);
		if (unit_id != UnitId::ProtossPylon)
		{
			helper::ai_remove_dangerous_regions(unit_id, placementData, player, desiredPos);
		}
		helper::ai_remove_far_tiles(placementData, desiredPos, maxRange);
		while (2)
		{
			u32 iterator1 = 0;
			do
			{
				if (placementData[iterator1])
				{
					helper::ai_remove_different_height(placementData, desiredPos); // Owned?
					helper::ai_placement_related_something(unit_id, placementData, desiredPos);
					if (unit_id == UnitId::TerranCommandCenter || unit_id == UnitId::ZergHatchery || unit_id == UnitId::ProtossNexus)
					{
						maxRange = 1;
					}
					else
					{
						maxRange = 0;
						helper::ai_remove_unfit_tiles(unit_id, placementData, player, desiredPos);
					}
					helper::ai_remove_addon_places(unit_id, placementData, player, desiredPos);
					switch (unit_id)
					{
						case UnitId::TerranWatchdog:
						case UnitId::ProtossPhotonCannon:
						{
							goto LABEL_17; 
						}
						case UnitId::TerranBunker:
						{
							if (!helper::get_bunker_placement_position(player, &maxRange))
							{
					LABEL_17:
								helper::get_turret_placement_position(placementData, player, desiredPos, unit_id, 0);
								return;
							}
						}
						case UnitId::ZergCreepColony:
						{
							if (!AIScriptController[player].AI_Flags.is_secure_finished) // SLOBYTE(AIScriptController[player].AI_Flags) < 0
							{
								goto LABEL_21;
							}

							if (!helper::get_bunker_placement_position(player, &maxRange))
							{
					LABEL_21:
								helper::get_turret_placement_position(placementData, player, desiredPos, unit_id, 0);
							}
						} break;
						case UnitId::ProtossPylon:
						{
							helper::get_pylon_placement_position(placementData, player, desiredPos);
						} break;
						default:
						{
							if (!maxRange)
							{
								helper::get_default_placement_position(unit_id, placementData, player, desiredPos);
							}
						} break;
					}
					return;
				}
				++iterator1;
			} while (iterator1 < 0x40);

			u32 iterator2 = 0;
			++iterator2;
			placementData += 64;
			if (iterator2 < 0x40)
			{
				continue;
			}
			break;
		}
	}

	// Hooked to change default behaviors - Pr0nogo
	u32 get_turret_placement_position(char* placementData, u32 player, Point16 desiredPos, u16 unit_id, u32 unkFlag) // 00443AC0
	{
		// will do this later
		return 0;
	}

	// Hooked to nullcheck sprites - Pr0nogo
	bool ai_check_military_grouping(Unit* unit) // 00439550
	{
		UnitAi* unitAi = (UnitAi*)unit->unit_ai;
		if (!unitAi || unitAi->type != 4 || units_dat::base_property[unit->id] & 1)
		{
			return 1;
		}

		MilitaryAi* militaryAi = (MilitaryAi*)unit->unit_ai;
		if (militaryAi->unit != unit)
		{
			return 1;
		}

		AiRegion* region = militaryAi->region;
		Unit* slowestUnitVar = region->slowestUnit;
		if (!slowestUnitVar
		  || !slowestUnitVar->sprite // new
		  || slowestUnitVar->status & UnitStatus::InTransport
		  || slowestUnitVar == unit
		  || region->state == 2
		  || (u16)(&(*SAI_Paths)->regionCount + 32 * (helper::get_region_id_from_unit(region->slowestUnit) + 4392)) < 60u
		  || !helper::ai_are_on_connected_regions(slowestUnitVar, unit)
		  || helper::get_distance_fast(region->slowestUnit->sprite->position.x, unit->sprite->position.x, region->slowestUnit->sprite->position.y, unit->sprite->position.y) <= 0x80)
		{
			return 1;
		}

		u32 x2 = 0;
		u32 y2 = 0;
		helper::get_region_center_from_id(region->region, &x2, &y2);
		return helper::get_distance_fast(unit->sprite->position.x, x2, unit->sprite->position.y, y2) <= 0x180;
	}

	// Hooked to fix AI height stupidity... or DUMBNESS??? -Pr0nogo
	u32 __stdcall ai_remove_different_height(char* placement_data, Point16 desired_pos) // 004436C0
	{
		return 0;

		/*
		u32 ground_height;
		char* placement_data_local;
		s32 pos_x_copy;
		u32 placement_data_iterator;
		u32 ground_height_2;
		bool iterate_done;
		u32 loop_var;
		char* placement_data_local_2;
		u32 result;
		char placement_data_buffer[4096];
		s32 pos_x;
		s32 times_to_iterate;
		s32 pos_y;

		memcpy(placement_data_buffer, placement_data, sizeof(placement_data_buffer));
		ground_height = helper::get_ground_height_at_px(desired_pos.y, desired_pos.x);
		pos_x = desired_pos.x - 1024;
		pos_y = desired_pos.y - 1024;
		placement_data_local = placement_data;
		times_to_iterate = 64;
		do
		{
			pos_x_copy = pos_x;
			placement_data_iterator = 0;
			do
			{
				if (placement_data_local[placement_data_iterator])
				{
					ground_height_2 = helper::get_ground_height_at_px(pos_y, pos_x_copy);
					if (ground_height != ground_height_2)
					{
						placement_data_local[placement_data_iterator] = 0;
					}
				}
				++placement_data_iterator;
				pos_x_copy += 32;
			} while (placement_data_iterator < 0x40);
			placement_data_local += 64;
			iterate_done = times_to_iterate == 1;
			pos_y += 32;
			--times_to_iterate;
		} while (!iterate_done);
		loop_var = 0;
		placement_data_local_2 = placement_data;
	LABEL_9:
		result = 0;
		while (!placement_data_local_2[result])
		{
			if (++result >= 0x40)
			{
				++loop_var;
				placement_data_local_2 += 64;
				if (loop_var < 0x40)
				{
					goto LABEL_9;
				}
				memcpy(placement_data, placement_data_buffer, 0x1000u);
				return result;
			}
		}
		return result;
		*/
	}

	// Hooked to investigate removing AI ally retaliation. (No cigar!) -Pr0nogo
	s32 attack_recent_attacker(Unit* unit) // 004770E0
	{
		Unit* target_unit = unit->auto_target_unit;
		if (!target_unit || unit->unit_ai)
		{
			return 0;
		}

		u8 target_player = target_unit->player;
		if (target_unit->player == 11) { // Neutral
			target_player = target_unit->sprite->player;
		}
		if (!scbw::is_allied_to(unit->player, target_player)) {
			if (helper::unit_can_attack_target(target_unit, unit, 1)) {
				helper::order_to_attack_target(unit->auto_target_unit, unit, 1, 0);
				unit->status |= UnitStatus::Disabled;
				return 1;
			}
		}
		unit->auto_target_unit = 0;
		return 0;
	}

	u32 ai_get_vespene_type(Unit* unit, u32 player_id, bool for_harvesting) { // 004326E0
		if (!unit || !unit->sprite || !unit->main_order_id) {
			return 0;
		}
		if (!for_harvesting && unit->id == UnitId::ResourceVespeneGeyser) {
			return 1;
		}
		if (unit->id == UnitId::ResourceVespeneGeyser || unit->id == UnitId::TerranReservoir || unit->id == UnitId::ProtossAquifer || unit->id == UnitId::ZergExcisant) {
			return 2 + (unit->building.resource.resource_amount != 0);
		}
		return 0;
	}

	Unit* get_ai_refinery(AiTown* town, u32 gas_state, u32 index, bool for_harvesting) { // 00432B90
		// This is stupid, but changing or even cleaning it seems to break AI having more than 1 refinery, might be undefined behavior. Please fix
		Unit* gas_building = town->gas_buildings[0];
		u32 counter = 0;
		if (gas_building && ai_get_vespene_type(town->gas_buildings[0], town->player, for_harvesting) == gas_state && (counter = 1, index == 1)
			|| (gas_building = town->gas_buildings[1]) != 0 && ai_get_vespene_type(town->gas_buildings[1], town->player, for_harvesting) == gas_state && (++counter, counter == index)
			|| (gas_building = town->gas_buildings[2]) != 0 && ai_get_vespene_type(town->gas_buildings[2], town->player, for_harvesting) == gas_state && counter + 1 == index)
		{
			return gas_building;
		}

		//if (town->gas_buildings[index - 1] && ai_get_vespene_type(town->gas_buildings[index - 1], town->player, for_harvesting) == gas_state)
		//{
		//	return town->gas_buildings[index - 1];
		//}

		return 0;
	}

	// Hooked to help with AI implementation of the vespene overhaul. -Pr0nogo
	void ai_worker_controller(Unit* worker) // 00435210
	{
		WorkerAi* worker_ai = (WorkerAi*)worker->unit_ai;
		if (worker_ai)
		{
			if (worker_ai->wait_timer)
			{
				worker_ai->wait_timer--;
				return;
			}
			Point16 worker_pos = {};
			AiTown* town = worker_ai->town;
			worker_ai->last_update_second = *elapsed_time_seconds;
			GroupFlag group_flags = units_dat::group_flags[worker->id];
			if ((group_flags.is_zerg || group_flags.is_protoss || !group_flags.is_terran || !helper::ai_scv_repair_controller(worker, town))
				&& !helper::ai_try_progress_spending_queue_worker(town, worker))
			{
				Unit* main_building = town->main_building;
				if (main_building)
				{
					Unit* town_gas = 0;
					Unit* town_gas_2 = 0;
					if (helper::unit_is_active_resource_depot(main_building))
					{
						if (worker_ai->target_resource == 1)
						{
							Unit* target_resource = worker->worker.target_resource.unit;
							if (target_resource && helper::unit_is_mineral_field(target_resource))
							{
								helper::ai_auto_harvest(target_resource, worker, 0);
							}
							else
							{
								Unit* town_mineral = town->mineral;
								if (!town_mineral || town_mineral->building.resource.resource_belongs_to_ai)
								{
									worker_pos = worker->sprite->position;
									town_mineral = helper::harvest_next_nearest_resources_ex(town->worker_count <= 0xCu, worker, worker_pos);
									if (!town_mineral)
									{
										town_mineral = town->mineral;
									}
								}
								Unit* ai_refinery = get_ai_refinery(town, 3, 1, true);
								if (!ai_refinery)
								{
									ai_refinery = get_ai_refinery(town, 2, 1, true);
								}
								helper::ai_auto_harvest(town_mineral, worker, ai_refinery);
							}
							return;
						}

						if (worker_ai->target_resource == 2)
						{
							Unit* ai_refinery = get_ai_refinery(town, 3, 1, true);
							if (ai_refinery)
							{
								helper::ai_auto_harvest(ai_refinery, worker, town->mineral);
								return;
							}
							goto exit;
						}

						u32 gas_iterator = 0;
						u32 gas_type = 0;
						town_gas = town->gas_buildings[0];
						if (worker_ai->target_resource == 3)
						{
							if (town_gas)
							{
								if (ai_get_vespene_type(town_gas, town->player, true) == 3)
								{
									gas_iterator = 1;
								}
							}
							town_gas_2 = town->gas_buildings[1];
							gas_type = ai_get_vespene_type(town->gas_buildings[1], town->player, true);
							if (town_gas_2 && gas_type == 3) {
								++gas_iterator;
							}
							if (!town_gas_2 || gas_type != 3 || gas_iterator != 2)
							{
								town_gas_2 = town->gas_buildings[2];
								gas_type = ai_get_vespene_type(town->gas_buildings[2], town->player, true);
								if (!town_gas_2 || gas_type != 3)
								{
									goto exit;
								}
								if (gas_iterator != 1)
								{
									goto exit;
								}
								if (town_gas_2)
								{
									helper::ai_auto_harvest(town_gas_2, worker, town->mineral);
									return;
								}
								goto exit;
							}
						}
						else
						{
							if (town_gas)
							{
								if (ai_get_vespene_type(town_gas, town->player, true) == 3)
								{
									gas_iterator = 1;
								}
							}
							town_gas_2 = town->gas_buildings[1];
							u32 gas_type = ai_get_vespene_type(town->gas_buildings[1], town->player, true);
							if (town_gas_2 && gas_type == 3) {
								++gas_iterator;
							}
							if (!town_gas_2 || gas_type != 3 || gas_iterator != 2)
							{
								town_gas_2 = town->gas_buildings[2];
								gas_type = ai_get_vespene_type(town->gas_buildings[2], town->player, true);
								if (!town_gas_2 || gas_type != 3)
								{
									goto exit;
								}
								if (gas_iterator != 2)
								{
									goto exit;
								}
								if (town_gas_2)
								{
									helper::ai_auto_harvest(town_gas_2, worker, town->mineral);
									return;
								}
								goto exit;
							}
						}
						if (town_gas_2)
						{
							helper::ai_auto_harvest(town_gas_2, worker, town->mineral);
							return;
						}
					exit:
						Unit* ai_refinery = get_ai_refinery(town, 3, 1, true);
						if (!ai_refinery)
						{
							Unit* town_gas_final = town->gas_buildings[0];
							u32 gas_iterator_final = 0;
							if (town_gas_final)
							{
								if (ai_get_vespene_type(town_gas_final, town->player, true) == 3)
								{
									gas_iterator_final = 1;
								}
							}
							town_gas_final = town->gas_buildings[1];
							if ((!town_gas_final
								|| ai_get_vespene_type(town->gas_buildings[1], town->player, true) != 3
								|| (++gas_iterator_final, gas_iterator_final != 2))
								&& ((town_gas_final = town->gas_buildings[2]) == 0
									|| ai_get_vespene_type(town->gas_buildings[2], town->player, true) != 3
									|| gas_iterator_final != 1)
								|| (ai_refinery = town_gas_final) == 0)
							{
								town_gas_final = town->gas_buildings[0];
								gas_iterator_final = 0;
								if (town_gas_final)
								{
									if (ai_get_vespene_type(town_gas_final, town->player, true) == 3)
									{
										gas_iterator_final = 1;
									}
								}
								town_gas_final = town->gas_buildings[1];
								if ((!town_gas_final
									|| ai_get_vespene_type(town->gas_buildings[1], town->player, true) != 3
									|| (++gas_iterator_final, gas_iterator_final != 3))
									&& ((town_gas_final = town->gas_buildings[2]) == 0
										|| ai_get_vespene_type(town->gas_buildings[2], town->player, true) != 3
										|| gas_iterator_final != 2)
									|| (ai_refinery = town_gas_final) == 0)
								{
									ai_refinery = get_ai_refinery(town, 2, 1, true);
									if (!ai_refinery)
									{
										town_gas_final = town->gas_buildings[0];
										gas_iterator_final = 0;
										if (town_gas_final)
										{
											if (ai_get_vespene_type(town_gas_final, town->player, true) == 2)
											{
												gas_iterator_final = 1;
											}
										}
										town_gas_final = town->gas_buildings[1];
										if ((!town_gas_final
											|| ai_get_vespene_type(town->gas_buildings[1], town->player, true) != 2
											|| (++gas_iterator_final, gas_iterator_final != 2))
											&& ((town_gas_final = town->gas_buildings[2]) == 0
												|| ai_get_vespene_type(town->gas_buildings[2], town->player, true) != 2
												|| gas_iterator_final != 1)
											|| (ai_refinery = town_gas_final) == 0)
										{
											gas_iterator_final = 0;
											if (town_gas_final)
											{
												if (ai_get_vespene_type(town_gas_final, town->player, true) == 2)
												{
													gas_iterator_final = 1;
												}
											}
											Unit* town_gas_2_copy_4 = town->gas_buildings[1];
											if (town_gas_2_copy_4
												&& ai_get_vespene_type(town->gas_buildings[1], town->player, true) == 2
												&& (++gas_iterator_final, gas_iterator_final == 3)
												|| (town_gas_2_copy_4 = town->gas_buildings[2]) != 0
												&& ai_get_vespene_type(town->gas_buildings[2], town->player, true) == 2
												&& gas_iterator_final == 2)
											{
												ai_refinery = town_gas_2_copy_4;
											}
											else
											{
												ai_refinery = 0;
											}
										}
									}
								}
							}
						}
						helper::ai_auto_harvest(ai_refinery, worker, town->mineral);
					}
				}
			}
		}
	}

	static bool unit_has_order(Unit* unit, u32 order_id) {
		if (unit->main_order_id == order_id) {
			return true;
		}
		for (Order* order = unit->order_queue_head; order; order = order->next) {
			if (order->order_id == order_id) {
				return true;
			}
		}
		return false;
	}

	static bool unit_has_uninterruptable_order(Unit* unit) {
		if (!orders_dat::CanBeInterrupted[unit->main_order_id]) {
			return true;
		}
		for (Order* order = unit->order_queue_head; order; order = order->next) {
			if (!orders_dat::CanBeInterrupted[order->order_id]) {
				return true;
			}
		}
		return false;
	}

	// Hooked as part of the vespene overhaul. -Pr0nogo
	char ai_reassign_worker_resources(AiTown* town) // 00435900, unused int arg
	{
		u8 result = 0;
		if (town->inited)
		{
			if (!town->resarea)
			{
				town->resarea = helper::get_closest_base_from_startloc(town->player, town->position.x, town->position.y);
				town->resource_units_not_set = 1;
			}

			u32 resarea = town->resarea;
			if (town->resource_units_not_set)
			{
				town->resource_units_not_set = 0;
				helper::get_resource_area_gas(resarea, &town->gas_buildings[0], &town->gas_buildings[1], &town->gas_buildings[2]);
				if (town->gas_buildings[0])
				{
					helper::ai_make_guard(town->player, town->gas_buildings[0]);
				}
				if (town->gas_buildings[1])
				{
					helper::ai_make_guard(town->player, town->gas_buildings[1]);
				}
				if (town->gas_buildings[2])
				{
					helper::ai_make_guard(town->player, town->gas_buildings[2]);
				}
			}
			if (!town->mineral)
			{
				Unit* resarea_minerals = helper::get_resource_area_mineral(town->resarea);
				town->mineral = resarea_minerals;
				if (resarea_minerals)
				{
					helper::ai_make_guard(town->player, resarea_minerals);
				}
			}

			if (resarea)
			{
				ResourceArea* ai_resarea = &baseArray[resarea];
				u32 new_worker_limit = 0;
				if (resarea && resarea < 250u && ai_resarea != 0)
				{
					new_worker_limit = 3 * ai_resarea->geyser_count + helper::get_optimal_mineral_workers(resarea, town->player);
				}
				town->worker_limit = new_worker_limit;
			}
			u8 mineral_value = 1;
			u8 gas_value = 2;
			if (get_ai_refinery(town, 3, 1, true) || get_ai_refinery(town, 2, 1, true))
			{
				if (!town->mineral)
				{
					mineral_value = 2;
				}
			}
			else
			{
				gas_value = 1;
			}

			u32 town_player = town->player;
			u8 minerals_percentage = 0;
			u8 gas_percentage = 0;
			helper::ai_resource_need_split(&gas_percentage, town_player, &minerals_percentage);
			u32 gas_count = gas_percentage * town->worker_count / 100;

			u32 worker_count = 0;
			switch (playerTable[town_player].race) {
				case RaceId::Terran: {
					worker_count += unit_count_table[UnitId::TerranMason].player[town_player];
					worker_count += unit_count_table[UnitId::TerranTalos].player[town_player];
				} break;
				case RaceId::Protoss: {
					worker_count += unit_count_table[UnitId::ProtossScribe].player[town_player];
				} break;
				case RaceId::Zerg: {
					worker_count += unit_count_table[UnitId::ZergDroleth].player[town_player];
				} break;
			}

			// Lard special cases
			if (game_vespene[town_player] < 800)
			{
				if (worker_count < 20)
				{
					if (worker_count < 15)
					{
						if (worker_count < 10)
						{
							if (worker_count >= 5 && gas_count < 1)
							{
								gas_count = 1;
							}
						}
						else
						{
							if (gas_count < 2)
							{
								gas_count = 2;
							}
						}
					}
					else
					{
						if (gas_count < 3)
						{
							gas_count = 3;
						}
					}
				}
				else if (gas_count < 4)
				{
					gas_count = 4;
				}
			}

			// New limit
			u32 max_worker_count = 0;
			for (u32 i = 0; i < 3; ++i) {
				Unit* gas = town->gas_buildings[i];
				if (gas && gas->status & UnitStatus::Completed) {
					if (gas->id == UnitId::ResourceVespeneGeyser) {
						max_worker_count += 2;
					} else {
						max_worker_count += 3;
					}
				}
			}
			if (gas_count > max_worker_count)
			{
				gas_count = max_worker_count;
			}

			u32 node_worker_count = 0;
			for (WorkerAi* worker_ai = town->workers; worker_ai; worker_ai = worker_ai->link.prev)
			{
				u8* target_resource_type = gas_count ? &gas_value : &mineral_value;
				while (*target_resource_type != 1 && !node_worker_count) {
					Unit* gas = town->gas_buildings[*target_resource_type - 2];
					if (gas && gas->status & UnitStatus::Completed) {
						if (gas->id == UnitId::ResourceVespeneGeyser) {
							node_worker_count = 2;
						} else {
							node_worker_count = 3;
						}
					}
					if (!node_worker_count) {
						*target_resource_type += 1;
						if (*target_resource_type > 4) {
							*target_resource_type = 1;
						}
					}
				}

				Unit* worker = worker_ai->parent;
				u8 unit_order = worker->main_order_id;
				if (unit_order == OrderId::ConstructingBuilding
					|| unit_order == OrderId::Pickup4
					|| unit_order == OrderId::DroneBuild
					|| worker->status & UnitStatus::InTransport)
				{
					worker_ai->last_update_second = *elapsed_time_seconds;
				}
				//else if (*elapsed_time_seconds - worker_ai->last_update_second >= 60)
				//{
				//	helper::issue_order_targeting_nothing(worker, OrderId::ComputerAI);
				//	worker_ai->wait_timer = 10;
				//	worker_ai->last_update_second = *elapsed_time_seconds;
				//	worker_ai->reassign_count += 1;
				//	helper::ai_get_old_peons(worker_ai->town, 1u);
				//}
				else
				{
					u32 timer = 5;
					if (unit_has_uninterruptable_order(worker)
						|| worker->main_order_id == OrderId::CanHarvestOre
						|| worker->main_order_id == OrderId::MiningMinerals
						|| worker->main_order_id == OrderId::ReturnMinerals
						|| worker->main_order_id == OrderId::CanEnterGasMine
						|| worker->main_order_id == OrderId::HarvestGas3
						|| worker->main_order_id == OrderId::ReturnGas
						|| worker->main_order_id == OrderId::HarvestInterrupt
						|| worker->main_order_id == OrderId::HarvestFinish
						|| unit_has_order(worker, OrderId::BuildTerran)
						|| unit_has_order(worker, OrderId::BuildProtoss1)
						|| unit_has_order(worker, OrderId::BuildProtoss2)
						|| unit_has_order(worker, OrderId::DroneStartBuild)
						|| unit_has_order(worker, OrderId::DroneBuild)) {
						timer = 30;
					}

					UnitExt* workerExt = globals.get_unit_ext(worker);
					if (*elapsed_time_seconds - worker_ai->last_update_second >= timer
						&& !workerExt->timers[TimerAI_KitingLeash])
					{
						helper::issue_order_targeting_nothing(worker, OrderId::ComputerAI);
						worker_ai->wait_timer = 0;
						worker_ai->last_update_second = *elapsed_time_seconds;
						helper::ai_get_old_peons(worker_ai->town, 1);
					}
				}

				if (gas_count) {
					--gas_count;
				}

				worker_ai->target_resource = *target_resource_type;
				result = *target_resource_type;

				if (*target_resource_type != 1) {
					if (node_worker_count) {
						--node_worker_count;
					}
					if (!node_worker_count) {
						*target_resource_type += 1;
						if (*target_resource_type > 4) {
							*target_resource_type = 1;
						}
					}
				}
			}
		}
		return result;
	}

	// Hooked as part of the vespene overhaul. -Pr0nogo
	void ai_auto_harvest(Unit* resource, Unit* worker, Unit* fallback) // 00433B90
	{
		if (worker->resource_type & ResourceType::Minerals)
		{
			worker->user_action_flags |= 1u;
			if (worker->main_order_id)
			{
				while (1)
				{
					if (!worker->order_queue_tail)
					{
						break;
					}
					if (!orders_dat::CanBeInterrupted[worker->order_queue_tail->order_id]
					&&	worker->order_queue_tail->order_id != OrderId::ReturnMinerals)
					{
						break;
					}
					helper::remove_order_from_unit_queue(worker->order_queue_tail, worker);
				}
				helper::perform_another_order(OrderId::ReturnMinerals, 0, worker, 228, {}, 0);
			}
			helper::orders_nothing_2(worker);
		}
		else if (worker->resource_type & ResourceType::VespeneGas)
		{
			helper::issue_order_targeting_nothing(worker, OrderId::ReturnGas);
		}
		else
		{
			if (!resource || !(resource->status & UnitStatus::Completed))
			{
				resource = fallback;
			}
			if (resource && (resource->id == UnitId::ResourceVespeneGeyser || resource->status & UnitStatus::Completed))
			{
				helper::assign_order_with_target(resource, worker, OrderId::Harvest1);
				resource->building.resource.resource_belongs_to_ai = 1;
			}
		}
	}

	u8 ai_get_build_order_id(Unit* unit, u32 unit_id) { // 00434C31
		u32 worker_race = unit->get_race();
		if (worker_race == Unit::get_race(unit_id)) {
			switch (worker_race) {
				case RaceId::Terran: {
					return OrderId::BuildTerran;
				} break;
				case RaceId::Protoss: {
					return OrderId::BuildProtoss1;
				} break;
				case RaceId::Zerg: {
					return OrderId::DroneStartBuild;
				} break;
			}
		}
		return OrderId::None;
	}

	// Hooked to replace old functionality with new, to prioritize Taloses over Masons for AI repairs and to allow more than 1 repairer. -Pr0nogo
	Unit* ai_find_best_repair_unit(Unit* to_repair) { // 00443160
		EntryArray workers = unit_search_radial(to_repair->position, 64 * 32);
		unit_search_filter(&workers, to_repair, FilterAlly, {UnitId::TerranMason, UnitId::TerranTalos});
		Unit* best = unit_search_find_best(&workers, [=](Unit* worker) {
			if (!worker->has_path_to_unit(to_repair) ||
				to_repair->order_target.unit == worker ||
				scbw::is_allied_to(worker->player, to_repair->player) ||
				!helper::is_order_valid(OrderId::Repair1, worker, 0, to_repair, 0)) {
				return ScoreInvalidUnit;
			}
			s32 score = 0;
			score -= scbw::get_distance_fast(worker->position, to_repair->position);
			if (worker->id == UnitId::TerranTalos) {
				score += 1000;
			}
		});
		return best;
	}

	// Same as above. -Pr0nogo
	s32 ai_scv_repair_controller(Unit* worker, AiTown* town) // 00433C70
	{
		u32 best_score = 0;
		Unit* repairee = 0;
		for (BuildingAi* building_ai = town->buildings; building_ai; building_ai = building_ai->link.prev)
		{
			Unit* building = building_ai->parent;
			if (building->connected_unit || !helper::ai_is_building_safe(building))
			{
				continue;
			}
			if (!(building->status & UnitStatus::Completed))
			{
				continue; // Used to break, but it should actually continue. unless the building list is sorted a particular way, which allows to break early...?
			}
			s32 max_hp = units_dat::MaxHitPoints[building->id] / 256;
			if (!max_hp)
			{
				max_hp = (building->hitpoints + 255) / 256;
				if (!max_hp)
				{
					max_hp = 1;
				}
			}
			u32 score = max_hp - ((building->hitpoints + 255) / 256);
			if (best_score < score)
			{
				repairee = building;
				best_score = score;
			}
		}
		if (!town->buildings || !repairee)
		{
			if (!town->building_SCV) {
				town->building_SCV = worker;
			}
			if (town->building_SCV != worker) {
				return 0;
			}
			repairee = helper::ai_find_best_repair_unit(worker); // This writing to repairee seems really wrong
			if (!repairee) {
				return 0;
			}
		}
		worker->user_action_flags |= 1u;
		Point16 repairee_pos = {}; // Shouldn't we return when nothing to repair?
		if (repairee) {
			repairee_pos = repairee->sprite->position;
		}
		if (worker->main_order_id) {
			for (Order* order_tail = worker->order_queue_tail; order_tail; order_tail = worker->order_queue_tail) {
				if (!orders_dat::CanBeInterrupted[order_tail->order_id] && order_tail->order_id != OrderId::Repair1) {
					break;
				}
				helper::remove_order_from_unit_queue(order_tail, worker);
			}
			helper::perform_another_order(OrderId::Repair1, 0, worker, UnitId::None, repairee_pos, repairee);
		}
		helper::orders_nothing_2(worker);
		repairee->connected_unit = worker;
		return 1;
	}

	f32 get_resarea_score(u32 player, AiTown* main_town, ResourceArea* area, EntryArray* units) {
		f32 score = 0;

		// TODO: make trigger customizable (and probably convert all biases to 1.0, so it's always the default)
		const f32 own_bias      = -1.0;
		const f32 ally_bias     = -0.5;
		const f32 enemy_bias    =  0.5;
		const f32 distance_bias = -4.0;
		const f32 mineral_bias  =  1.0;
		const f32 gas_bias      =  1.0;

		u32 own_count = 0;
		u32 ally_count = 0;
		u32 enemy_count = 0;
		f32 average_own_distance = 0; 
		f32 average_ally_distance = 0; 
		f32 average_enemy_distance = 0; 

		for (u32 i = 0; i < PlayerLimit; ++i) {
			for (Unit* unit = first_player_unit->unit[i]; unit; unit = unit->player_link.next) {
				f32 distance = scbw::get_distance_fast(area->position, unit->position) / 32.0;
				if (player == i) {
					own_count++;
					average_own_distance += distance;
				} else if (scbw::is_allied_to(player, i)) {
					ally_count++;
					average_ally_distance += distance;
				} else {
					enemy_count++;
					average_enemy_distance += distance;
				}
			}
		}
		average_own_distance   /= own_count;
		average_ally_distance  /= ally_count;
		average_enemy_distance /= enemy_count;
		score += average_own_distance   * own_bias;
		score += average_ally_distance  * ally_bias;
		score += average_enemy_distance * enemy_bias;

		f32 distance_from_main = scbw::get_distance_fast(main_town->position, area->position) / 32;
		score += distance_from_main * distance_bias;

		f32 mineral_value = area->mineral_field_count + area->total_mineral_count / 1500.0;
		f32 gas_value     = area->geyser_count * 3    + area->total_gas_count     / 500.0;
		score += mineral_value * mineral_bias;
		score += gas_value     * gas_bias;

		for (u32 j = 0; j < units->count; ++j) {
			Unit* unit = units->units[j];
			if (unit->player < PlayerLimit) {
				if (player != unit->player && scbw::is_allied_to(player, unit->player) && units_dat::group_flags[unit->id].is_building) {
					score -= 10;
				}
			}
		}

		return score;
	}

	u32 ai_find_expansion_custom(u32 player, AiTown* main_town) {
		u32 best_resarea = 0;
		f32 best_score = INT_MIN;

		bool has_transport_or_flying_worker = false;
		for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
			if (scbw::unit_id_is_transport(unit->id)) {
				has_transport_or_flying_worker = true;
				break;
			}
			if (units_dat::base_property[unit->id] & UnitProperty::Worker && units_dat::base_property[unit->id] & UnitProperty::Flyer) {
				has_transport_or_flying_worker = true;
				break;
			}
		}

		Region* main_region = scbw::get_region(scbw::get_region_id(main_town->position));
		for (u32 i = 1; i <= resource_areas->used_count; ++i) {
			ResourceArea* area = &resource_areas->areas[i];
			if (area->flags & 3) {
				continue;
			}
			Region* area_region = scbw::get_region(scbw::get_region_id(area->position));
			if (!has_transport_or_flying_worker && main_region->group_index != area_region->group_index) {
				continue;
			}
			EntryArray units = unit_search_radial(area->position, 12 * 32);
			for (u32 j = 0; j < units.count; ++j) {
				Unit* unit = units.units[j];
				if (unit->player < PlayerLimit) {
					if (player != unit->player && scbw::is_allied_to(player, unit->player) && units_dat::group_flags[unit->id].is_factory) {
						goto exit;
					}
					if (!scbw::is_allied_to(player, unit->player) && units_dat::group_flags[unit->id].is_building) {
						goto exit;
					}
				}
			}

			f32 score = get_resarea_score(player, main_town, area, &units);
			if (score > best_score) {
				best_score = score;
				best_resarea = i;
			}

		exit:
			continue;
		}
		
		if (best_resarea) {
			resource_areas->areas[best_resarea].flags |= 2;
		}
		return best_resarea;
	}

	// Hooked to remove a ridicilus condition that would stop AI from expanding in late game and to allow base layouts to control expand order
	void ai_expand(AiScript* aiscript, u32 block, u32 amount) { // 0045B530
		default_ai_expansion_block[aiscript->player] = block;
		if (aiscript->town->resarea) { 
			u32 base_id = ai_get_next_custom_expo(aiscript->town);
			if (!base_id || !helper::ai_try_expanding_to(base_id)) {
				base_id = ai_find_expansion_custom(aiscript->player, aiscript->town);
			}
			if (base_id) {
				helper::ai_expand_to(aiscript, base_id, block);
				scbw::ai_defend(aiscript->player, scbw::get_region_id(resource_areas->areas[base_id].position), 115, true);
			}
		}
	}

	static bool unit_id_is_stackable_building(u32 unit_id) {
		switch (unit_id) {
			case UnitId::ZergCreepColony:
			case UnitId::ZergSunkenColony:
			case UnitId::ZergSporeColony:
			case UnitId::ZergLarvalColony:
			case UnitId::ZergIrolIris:
			case UnitId::ZergOthstolOviform:
			case UnitId::ZergAlaszilArbor:
			case UnitId::ProtossPylon:
			case UnitId::ProtossPennant:
			case UnitId::ProtossShieldBattery:
			case UnitId::ProtossPhotonCannon:
			case UnitId::TerranWatchdog:
			case UnitId::TerranBunker: {
				return true;
			} break;
		}
		return false;
	}

	static bool unit_id_has_addon(u32 unit_id) {
		switch (unit_id) {
			case UnitId::TerranCommandCenter:
			case UnitId::TerranTreasury:
			case UnitId::TerranBarracks:
			case UnitId::TerranFulcrum:
			case UnitId::TerranStarport: 
			case UnitId::TerranScrapyard: 
			case UnitId::TerranCaptaincy: {
				return true;
			} break;
		}
		return false;
	}

	// Hooked to add addon cases for new addon buildings and to reserve space around buildings. -Veeq7
	void __stdcall ai_remove_addon_placements(u16 unit_id, PlacementData* placement_data, u32 player, Point16 desired_position) { // 00443CC0
		for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
			if (unit->sprite && unit->main_order_id != OrderId::Die && units_dat::base_property[unit->id] & UnitProperty::Building) {
				// Map coordinates to the placement grid
				s32 unit_x = unit->sprite->position.x / 32 - desired_position.x / 32 + 32;
				s32 unit_y = unit->sprite->position.y / 32 - desired_position.y / 32 + 32;

				// Dimensions
				u32 w = units_dat::BuildingDimensions[unit->id].x / 32;
				u32 h = units_dat::BuildingDimensions[unit->id].y / 32;

				// Go to top left
				unit_x -= (w + 1) / 2;
				unit_y -= (h + 1) / 2;

				// Add padding on top and left
				// Stackable buildings don't have padding between each other
				if (!unit_id_is_stackable_building(unit_id) || !unit_id_is_stackable_building(unit->id)) {
					unit_x -= 1;
					unit_y -= 1;
					w += 1;
					h += 1;
				}

				// Handle addons
				if (unit_id_has_addon(unit->id)) {
					w += 2;
				}
				if (unit_id_has_addon(unit_id)) {
					unit_x -= 2;
					w += 2;
				}

				// Forbid placements
				for (u32 y = unit_y; y < unit_y + h; ++y) {
					for (u32 x = unit_x; x < unit_x + w; ++x) {
						if (x < 0 || x >= 64 || y < 0 || y >= 64) {
							continue;
						}
						placement_data->tile_valid[y][x] = 0;
					}
				}
			}
		}
	}

	AiRegion* find_better_region(MilitaryAi* military_ai, AiRegion* ai_region, u32 recurses) {
		if (scbw::get_region_id(military_ai->unit->position) == ai_region->region || scbw::get_ai_region_military_count(ai_region) < 15) {
			return ai_region;
		}
		if (recurses) {
			Region* region = scbw::get_region(ai_region->region);
			for (u32 i = 0; i < region->neighborCount; ++i) {
				u32 other_id = region->neighbors[i];
				Region* other_region = scbw::get_region(other_id);
				if (military_ai->unit->status & UnitStatus::InAir || region->group_index == other_region->group_index) {
					AiRegion* new_ai_region = find_better_region(military_ai, scbw::get_ai_region(ai_region->player, other_id), recurses - 1);
					if (new_ai_region) {
						return new_ai_region;
					}
				}
			}
		}
		return 0;
	}

	// Hooked to fix AI pathfinding lag and improve army spread. -Pr0nogo
	void ai_manage_military_controller(Unit* unit) // 0043D910
	{
		if (unit->unit_ai && unit->unit_ai->type == AiType::Military && unit->unit_ai->military_ai.unit == unit) {
			MilitaryAi* military_ai = (MilitaryAi*)unit->unit_ai;
			AiRegion* new_ai_region = find_better_region(military_ai, military_ai->region, 3);
			if (new_ai_region && military_ai->region != new_ai_region) {
				helper::delete_military_ai(military_ai, military_ai->region);
				unit->unit_ai = 0;
				helper::add_military_ai(new_ai_region, unit, true);
			}
		}

		if (unit->unit_ai && unit->unit_ai->type == AiType::Military)
		{
			MilitaryAi* military_ai = (MilitaryAi*)unit->unit_ai;
			if (military_ai->unit == unit)
			{
				u16 unit_id = unit->id;
				if (!(units_dat::base_property[unit_id] & UnitProperty::Building))
				{
					AiRegion* ai_region = military_ai->region;
					if (unit_id && unit_id != UnitId::TerranHarakan || !helper::ai_defender_bunker_behavior(unit, ai_region->region))
					{
						if (!helper::ai_cast_spell_behavior(unit, 0) && !helper::ai_update_attack_target(unit, 0, 1, 0))
						{
							s32 region_id = ai_region->region;
							if (ai_region->state == RegionState::ContinueAttackHere)
							{
								if (!ai_region->slowestUnit)
								{
									helper::ai_assign_region_to_slowest_unit(ai_region);
								}
								Unit* slowest_unit = ai_region->slowestUnit;
								if (slowest_unit)
								{
									if (helper::ai_has_incomplete_military_in_region(ai_region))
									{
										region_id = helper::get_region_id_from_unit(slowest_unit);
									}
								}
							}

							Region* region = scbw::get_region(region_id);
							if (helper::get_region_id(unit->sprite->position.x, unit->sprite->position.y) == region_id)
							{
								helper::make_to_hold_position(unit);
							}
							else
							{
								Point16 center = scbw::get_region_center(region);
								helper::ai_order_to_destination(unit, OrderId::AttackMove, center.x, center.y);
							}
						}
					}
				}
			}
		}
	}

	// Hooked to help AI group mixed speed armies in more intelligent ways. -Pr0nogo
	void ai_assign_region_to_slowest_unit(AiRegion* region) // 00436F70
	{
		Unit* slowest_unit = region->slowestUnit;
		Unit* slowest_ground_unit = 0;
		Unit* slowest_air_unit = 0;
		u32 slowest_ground_speed = 99999999;
		u32 slowest_air_speed = 99999999;
		if (slowest_unit)
		{
			if (slowest_unit->status & UnitStatus::InAir)
			{
				slowest_air_unit = region->slowestUnit;
				slowest_air_speed = slowest_unit->flingy_top_speed;
			}
			else
			{
				slowest_ground_unit = region->slowestUnit;
				slowest_ground_speed = slowest_unit->flingy_top_speed;
			}
		}
		for (MilitaryAi* first_military = region->first_military_ai; first_military; first_military = first_military->link.prev)
		{
			Unit* military = first_military->unit;
			if (military->status & UnitStatus::InAir)
			{
				if (military->flingy_top_speed < slowest_air_speed)
				{
					slowest_air_speed = military->flingy_top_speed;
					slowest_air_unit = first_military->unit;
				}
			}
			else if (military->flingy_top_speed < slowest_ground_speed)
			{
				slowest_ground_speed = military->flingy_top_speed;
				slowest_ground_unit = first_military->unit;
			}
		}
		if (!slowest_ground_unit)
		{
			slowest_ground_unit = slowest_air_unit;
		}
		region->slowestUnit = slowest_ground_unit;
	}

	// Hooked to improve behavior. -Pr0nogo
	void ai_respond_to_cloaked_threat(Unit* victim, Unit* threat) // 0043C580
	{
		if (victim->unit_ai)
		{
			AIScriptController[victim->player].AI_Flags.is_secure_finished;
			UnitAi* victim_ai = victim->unit_ai;
			AiRegion* victim_region = 0;
			if (victim_ai->type == AiType::Military)
			{
				MilitaryAi* military_ai = (MilitaryAi*)victim_ai;
				victim_region = military_ai->region;
			}
			else
			{
				if (victim_ai->type != AiType::Guard)
				{
				LABEL_9:
					Unit* unit = *first_player_unit[victim->player].unit;
					Unit* unit_to_order_1 = 0;
					u32 max_distance = 9999999;
					Unit* unit_2 = *first_player_unit[victim->player].unit;
					if (unit)
					{
						do
						{
							if (unit->sprite)
							{
								u8 first_order_id = unit->main_order_id;
								if (first_order_id)
								{
									switch (unit->id)
									{
										case UnitId::TerranSeraph:
										case UnitId::ProtossWitness:
										case UnitId::ZergOvileth:
										case UnitId::ZergIroleth:
										{
											if (helper::is_unit_loaded(unit))
											{
												break;
											}
											if (!unit->is_blind
											&&  (first_order_id == OrderId::ComputerAI
												|| first_order_id == OrderId::ComputerReturn
												|| first_order_id == OrderId::GuardPost
												|| first_order_id == OrderId::AIPatrol))
											{
												u32 distance = helper::get_distance_fast(victim->sprite->position.x, unit->sprite->position.x, victim->sprite->position.y, unit->sprite->position.y);
												if (distance < max_distance)
												{
													unit_to_order_1 = unit;
													max_distance = distance;
												}
											}
										} break;
										case UnitId::TerranComsatStation:
										{
											if (!(unit->status & UnitStatus::Completed))
											{
												break;
											}
											// TODO: Add check to see if victim is already detected / if there's a detector in the area?
											if (victim->id != UnitId::TerranBunker || !helper::is_unit_loaded(victim))
											{
												if (!helper::ai_unit_can_attack(victim) || units_dat::base_property[victim->id] & UnitProperty::Worker)
												{
													break;
												}
											}
											if (unit->energy >= techdata_dat::energy_cost[TechId::ScannerSweep] * 256 || unit->main_order_id == OrderId::PlaceScanner)
											{
												break;
											}
											helper::assign_order_with_target(threat, unit, OrderId::PlaceScanner);
											return;
										} break;
										default:
										{
										} break;
									}
								}
							}
							unit = unit->player_link.next;
							unit_2 = unit;
						} while (unit);
						if (unit_to_order_1)
						{
							Point16 victim_pos = {};
							victim_pos.x = victim->sprite->position.x;
							victim_pos.y = victim->sprite->position.y;
							unit_to_order_1->user_action_flags |= 1;
							if (unit_to_order_1->main_order_id)
							{
								while (1)
								{
									Order* tail_order_1 = unit_to_order_1->order_queue_tail;
									if (!tail_order_1)
									{
										break;
									}
									if (!orders_dat::CanBeInterrupted[tail_order_1->order_id] && tail_order_1->order_id != OrderId::ComputerReturn)
									{
										break;
									}
									helper::remove_order_from_unit_queue(tail_order_1, unit_to_order_1);
								}
								helper::perform_another_order(OrderId::ComputerReturn, 0, unit_to_order_1, UnitId::None, victim_pos, 0);
							}
							helper::orders_nothing_2(unit_to_order_1);
						}
					}
					return;
				}
				BuildingAi* building_ai = (BuildingAi*)victim_ai;
				victim_region = &AiRegionCaptains[victim->player][helper::get_region_id(building_ai->parent->position.x, building_ai->parent->position.y)]; // WRONG... or is it?
			}
			if (victim_region)
			{
				if (victim_region->follow_target)
				{
					helper::assign_order_with_target(victim, victim_region->follow_target, OrderId::AIPatrol);
					return;
				}
			}
			goto LABEL_9;
		}
	}

	// Hooked to remove computer return bullshit and improve kiting. -Veeq7
	u32 ai_try_return_home(Unit* unit, u32 dont_issue_order) {
		return 0;
	}
}