#include "cast_order.h"
#include <SCBW/api.h>
#include "SCBW/stat_txt_spec.h"

namespace hooks
{
	// Hooked to change order conditions and actions for Hallucination - Pr0nogo
	s32 spellOrder(s32 defaultOrderString, Unit* caster, u16 techId, s32 range, s32* energyCost) // 004926D0
	{		
		u16 v6;
		char v7;
		GroupFlag v8;
		char v9;
		u8 v10;
		char* v11;
		u8 v12; // not written to
		s16 v14;
		char* v15;
		s32 v16;

		Unit* orderTarget = caster->orderTarget.unit;
		v16 = defaultOrderString;
		if (orderTarget)
		{
			if (helper::getOrderTargetErrorString(techId, orderTarget, 228, caster, orderTarget->sprite->position.x, orderTarget->sprite->position.y, (s16*)&v16))
			{
				v7 = *CHEAT_STATE;
				v6 = techdata_dat::EnergyCost[techId] << 8;
				*energyCost = v6;
				if (v7 & 8 || caster->energy >= v6)
				{
					return helper::spellOrder_MoveTowardsTarget(caster, range);
				}
				v8 = units_dat::GroupFlags[caster->id];
				v9 = caster->playerId;
				if (v8.isZerg)
				{
					v10 = 0;
				}
				else if (v8.isTerran)
				{
					v10 = 1;
				}
				else
				{
					v10 = (v8.isProtoss) != 0 ? 2 : 4;
				}
				v11 = helper::getStatTxtString(v10 + 864);
				if (v11)
				{
					if (v12 == *LOCAL_NATION_ID && ACTIVE_PLAYER_ID == LOCAL_HUMAN_ID)
					{
						helper::setUnitStatTxtErrorMsg((s32)v11);
					}
				}
				helper::toIdle(caster);
				return 0;
			}
			defaultOrderString = v16;
		}
		v14 = defaultOrderString - 1;
		if ((s16)defaultOrderString)
		{
			if (v14 < *statTxtTbl)
			{
				v15 = (char*)statTxtTbl + statTxtTbl[v14 + 1];
			}
			else
			{
				v15 = "";
			}
		}
		else
		{
			v15 = 0;
		}
		helper::get_statTxt_Str_0(caster, (u8)caster->playerId, v15);

		s32 result;
		if (caster->orderQueueHead)
		{
			caster->userActionFlags |= 1u;
			helper::orders_Nothing2(caster);
			result = 0;
		}
		else
		{
			if (caster->pAI)
			{
				helper::orderComputerClear(caster, 0x9Cu);
			}
			else
			{
				helper::orderComputerClear(caster, units_dat::ReturnToIdleOrder[caster->id]);
			}
			result = 0;
		}
		return result;
	}

	// Same as above
	bool canTargetSpell(u16 techId, Unit* target, s16 fowUnitId, Unit* caster, s16 x, s16 y, s16* error) // 00492020
	{
		bool v9;
		s16 v10;

		switch (techId)
		{
			case OrderId::Guard:
			case OrderId::TurretGuard:
			case OrderId::AttackMove:
			case OrderId::InfestMine1:
			case OrderId::Powerup1:
			case OrderId::TowerAttack:
			case OrderId::StayinRange:
			case OrderId::TurretAttack:
			case OrderId::DroneStartBuild:
			case OrderId::BuildProtoss1:
			{
				return 1;
			}
			default:
			{
				if (techId != OrderId::PlayerGuard)
				{
					if (target)
					{
						if (!(target->status & UnitStatus::Invincible))
						{
						LABEL_20:
							if (fowUnitId == UnitId::None)
							{
								if (techId == TechId::Hallucination)
								{
									v10 = 0;
								}
								else
								{
									v10 = (u32)helper::getTargetUnitAbilityErrorStatString(target, caster->playerId, techId);
								}
							}
							else
							{
								v10 = helper::canTargetSpellOnFOWUnit(techId, fowUnitId);
							}
							if (v10)
							{
								if (error)
								{
									*error = v10;
								}
							}
							return v10 == 0;
						}
						v9 = fowUnitId < UnitId::None;
					}
					else
					{
						v9 = fowUnitId < UnitId::None;
						if (fowUnitId == UnitId::None)
						{
							if (error)
							{
								*error = 878; // Must target units.<0>
							}
							return 0;
						}
					}
					if (!v9 || !(~(units_dat::BaseProperty[fowUnitId] >> 29) & 1))
					{
						if (error)
							*error = 876; // Invalid target.<0>
						return 0;
					}
					goto LABEL_20;
				}
				if (helper::getActiveTileFlag(y, x, 0x410000, 0))
				{
					return 1;
				}
				if (error)
				{
					*error = 902; // Must target passable terrain.
				}
				return 0;
			}
		}
	}

	void orders_Spell(Unit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);
		s32 spellCost;
		u8 orderWeaponId = orders_dat::OrderWeaponId[unit->mainOrderId];
		Unit* target = unit->orderTarget.unit;

		if (unit->id == UnitId::TerranPaladin)
		{
			if (unit->orderSignal & 0x20)
			{
				unit->orderSignal &= ~0x20;
				unitExt->timers[TimerSkyfall] = 3;
				unitExt->values[ValueSkyfallStacks] = 0;
				auto pt = unit->orderTarget.pt;
				if (unit->orderTarget.unit != nullptr)
				{
					pt = unit->orderTarget.unit->position;
					unit->orderTarget.unit = nullptr;
				}
				unitExt->values[ValueSkyfallX] = unit->orderTarget.pt.x;
				unitExt->values[ValueSkyfallY] = unit->orderTarget.pt.y;
			}
		}

		if (unit->id == UnitId::TerranWyvern)
		{
			unitExt->values[ValueWyvernLandGrabOn] = 1;
			unitExt->values[ValueWyvernLandGrabX] = unit->orderTarget.pt.x;
			unitExt->values[ValueWyvernLandGrabY] = unit->orderTarget.pt.y;
		}

		if (unit->mainOrderState == 2)
		{
			if (unit->orderSignal & 2)
			{
				unit->orderSignal -= 2;
				unit->orderToIdle();

			}
			else
			{ // 00492886
				if (target != nullptr)
				{
					unit->orderTarget.pt.x = target->sprite->position.x;
					unit->orderTarget.pt.y = target->sprite->position.y;

					if (target->sprite->position.x != unit->nextTargetWaypoint.x ||
						target->sprite->position.y != unit->nextTargetWaypoint.y)
					{ // 004928C1
						unit->nextTargetWaypoint.x = target->sprite->position.x;
						unit->nextTargetWaypoint.y = target->sprite->position.y;
					}
				}
			}

		}
		else
		{ // 004928CF
			if (!helper::canCastSpell_0(unit))
			{
				char* message = (char*)statTxtTblNew->getString(876); // Invalid Target
				// do the actual transmission corresponding to the message
				helper::get_statTxt_Str_0(unit, unit->playerId, message);
				// make the unit stop since the order failed
				helper::setNextWaypoint(unit);
				unit->orderToIdle();

			}
			else
			{ // 0049291A
				u8 techUsed = orders_dat::TechUsed[unit->mainOrderId];
				if (techUsed >= TechId::None)
				{
					spellCost = 0;
				}
				else
				{
					spellCost = techdata_dat::EnergyCost[techUsed] * 256;
				}

				if (!(*CHEAT_STATE & CheatFlags::TheGathering) &&
					unit->energy < spellCost)
				{
					u8 raceId;
					char* message;
					if (units_dat::GroupFlags[unit->id].isZerg)
					{
						raceId = RaceId::Zerg;
					}
					else
						if (units_dat::GroupFlags[unit->id].isTerran)
						{
							raceId = RaceId::Terran;
						}
						else
							if (units_dat::GroupFlags[unit->id].isProtoss)
							{
								raceId = RaceId::Protoss;
							}
							else
							{
								raceId = RaceId::Neutral;
							}

					message = (char*)statTxtTblNew->getString(864 + raceId); // Not Enough Energy

					if (message != nullptr)
					{
						// display an error message only for the owner
						// of the unit
						if (*LOCAL_NATION_ID == unit->playerId &&
							*ACTIVE_PLAYER_ID == *LOCAL_HUMAN_ID)
						{
							helper::setUnitStatTxtErrorMsg(message);
						}
					}
					unit->orderToIdle();

				}
				else
				{ //929C2

					u32 result_00492620;
					// may initiate movement from unit depending on mainOrderState
					if (unit->mainOrderState == 0)
					{
						result_00492620 = helper::function_00492620(unit, 1);
					}
					else
					{
						result_00492620 = helper::function_00492620(unit, 0);
					}

					if (unit->mainOrderState == 0)
					{
						unit->mainOrderState = 1;
					}
					bool gcd_condition = true;
					if (unit->id == UnitId::TerranWyvern && unit->groundWeaponCooldown > 0)
					{
						gcd_condition = false; // new
					}
					if (result_00492620 == 0)
					{
						if (unit->getMovableState() == 2) //unmovable
						{
							unit->orderToIdle();
						}
					}
					else
					{ // 00492A01

						if (unit->getSpellCountdown() == 0 &&
							gcd_condition && // new
							helper::function_00476640(unit, orderWeaponId) && // is ready to fire weapon (angle,range...) maybe?
							!(unit->movementFlags & MovementFlags::Accelerating) &&
							unit->getMovableState() != 0
							)
						{ // 00492A38
						 // scbw::print("Conditions good");
							bool bStopHere = false;

							// in case it was modified by a previous function,
							// relinking
							target = unit->orderTarget.unit;

							if (unit->pAI != nullptr && target != nullptr)
							{

								if (orderWeaponId <= WeaponId::Maelstrom)
								{ // Maelstrom is the last spell-related weapon

									// array bound to the function
									const u8* const array_00492B44 = (u8*)(0x00492B44);

									// this code would not work for weapons id of NuclearMissile, YamatoGun 
									// and weapons id before those, they must be excluded somewhere else
									// Maybe the switch and array use could be removed in a future version
									// since enough elements use default value (switch 6)
									u32 switch_value = array_00492B44[orderWeaponId - 0x20];
									if (switch_value != 6)
									{
										//no idea why irradiate is so important
										u32 analyzed_target_status = target->irradiateTimer;
										if (switch_value == 2)
										{
											analyzed_target_status |= target->parasiteFlags;
										}
										if (switch_value == 3)
										{
											analyzed_target_status |= target->ensnareTimer;
										}
										if (switch_value == 4)
										{
											analyzed_target_status |= target->plagueTimer;
										}
										if (switch_value == 5)
										{
											analyzed_target_status |= target->isBlind;
										}
										if (switch_value == 0 && unit->isFrozen())
										{
											analyzed_target_status |= 1;
										}
										if (analyzed_target_status != 0)
										{
											unit->orderToIdle();
											bStopHere = true;
										}
									}
								}
							}

							if (!bStopHere)
							{ // 00492AA3
								u8 cooldown;
								u32 random_value;
								unit->spendUnitEnergy(spellCost);

								cooldown = helper::getUpgradedWpnCooldown(unit, orderWeaponId);
								unit->spellCooldown = cooldown;
								if (unit->id == UnitId::TerranWyvern)
								{
									unit->groundWeaponCooldown = cooldown; // new
								}

								// disable flag 2 if enabled, else has no effect
								unit->orderSignal &= ~2;	//&= -3
								unit->movementFlags |= MovementFlags::StartingAttack;
								if (unit->sprite->images.head != nullptr)
								{
									Image* current_image = unit->sprite->images.head;
									IscriptAnimation::Enum anim = (IscriptAnimation::Enum)orders_dat::OrderIscriptAnim[unit->mainOrderId];
									if (techUsed == TechId::Lockdown && unit->vulture.spiderMineCount > 0)
									{
										unit->vulture.spiderMineCount--;
									}
									while (current_image != nullptr)
									{
										current_image->playIscriptAnim(anim);
										current_image = current_image->link.next;
									}
								}
								unit->mainOrderState = 2;
								if (unit->pAI != nullptr)
								{
									AIScriptController[unit->playerId].spellcasterTimer = 1;
								}
							}
						}
					}
				}
			}
		}
	}
}