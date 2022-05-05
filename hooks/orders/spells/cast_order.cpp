#include "cast_order.h"
#include <SCBW/api.h>
#include "SCBW/stat_txt_spec.h"
#include "hooks/orders/pathfinding.h"

namespace hooks
{
	// Hooked to change order conditions and actions for custom spells - Pr0nogo
	s32 spellOrder(s32 defaultOrderString, Unit* caster, u16 techId, s32 range, u16* energyCost) // 004926D0
	{
		Unit* orderTarget = caster->orderTarget.unit;
		u16 meme = (u16)defaultOrderString;
		if (techId == TechId::Hallucination
		||	techId == TechId::LuminaryAuralAssault
		||	(orderTarget && hooks::canTargetSpell(techId, orderTarget, 228, caster, orderTarget->sprite->position.x, orderTarget->sprite->position.y, &meme)))
		{
			char cheatFlags = *CHEAT_STATE;
			u16 techCost = techdata_dat::EnergyCost[techId] << 8;
			*energyCost = techCost;
			if ((cheatFlags & 8 || caster->energy >= techCost))
			{
				if (techId == TechId::Hallucination && !orderTarget) {
					Point16 target_pos = caster->orderTarget.pt;
					if (scbw::getDistanceFast(caster->position, target_pos) > range) {
						helper::setMoveTargetXY(target_pos.x, target_pos.y, caster);
						return 0;
					}
					helper::setMoveTargetXY(caster->position.x, caster->position.y, caster);

					if (!helper::isAngleInAttackAngle(target_pos.y, target_pos.x, caster, TechId::Hallucination)) {
						helper::turnUnit(caster, scbw::getAngle(caster->position, target_pos));
						return 0;
					}

					return 1;
				}
				return helper::spellOrder_MoveTowardsTarget(caster, range);
			}

			GroupFlag casterGroupFlags = units_dat::GroupFlags[caster->id];
			char casterPlayer = caster->playerId;
			u8 casterRace;
			if (casterGroupFlags.isZerg)
			{
				casterRace = 0;
			}
			else if (casterGroupFlags.isTerran)
			{
				casterRace = 1;
			} else {
			casterRace = (casterGroupFlags.isProtoss) != 0 ? 2 : 4;
			}

			char* string = helper::getStatTxtString(casterRace + 864);
			if (string) {
				if (*ACTIVE_PLAYER_ID == *LOCAL_HUMAN_ID) {
					helper::setUnitStatTxtErrorMsg((s32) string);
				}
			}
			helper::toIdle(caster);
			return 0;
		}

		s16 orderStringIndex = defaultOrderString - 1;
		char* finalString;
		if ((u16) defaultOrderString) {
			if (orderStringIndex < *statTxtTbl) {
				finalString = (char*) statTxtTbl + statTxtTbl[orderStringIndex + 1];
			} else {
				finalString = "";
			}
		} else {
			finalString = 0;
		}
		helper::get_statTxt_Str_0(caster, (u8) caster->playerId, finalString);

		s32 result;
		if (caster->orderQueueHead) {
			caster->userActionFlags |= 1u;
			helper::orders_Nothing2(caster);
			result = 0;
		} else {
			if (caster->pAI) {
				helper::issue_order_targeting_nothing(caster, 0x9Cu);
			} else {
				helper::issue_order_targeting_nothing(caster, units_dat::ReturnToIdleOrder[caster->id]);
			}
			result = 0;
		}
		return result;
	}

	// Same as above
	bool canTargetSpell(u16 techId, Unit* target, s16 fowUnitId, Unit* caster, s16 x, s16 y, u16* error) // 00492020
	{
		switch (techId) {
			// Targeting ground
			case TechId::EMPShockwave:
			case TechId::ScannerSweep:
			case TechId::DarkSwarm:
			case TechId::Plague:
			case TechId::Ensnare:
			case TechId::PsionicStorm:
			case TechId::Hallucination: // New
			case TechId::Recall:
			case TechId::StasisField:
			case TechId::DisruptionWeb:
			case TechId::Maelstrom:
			case TechId::OpticalFlare: // New, Aural Assault for Luminary
			{
				return 1;
			}
			default:
			{
				if (techId != TechId::SpiderMines
					&& techId != TechId::EidolonLobotomyMine) {
					bool hasTarget = false;
					u16 errorStatString = 0;
					if (target) {
						if (!(target->status & UnitStatus::Invincible)) {
						LABEL_20:
							if (fowUnitId == UnitId::None)
							{
								errorStatString = (u32)helper::getTargetUnitAbilityErrorStatString(target, caster->playerId, techId);
							}
							else
							{
								errorStatString = helper::canTargetSpellOnFOWUnit((s32)techId, fowUnitId);
							}
							
							if (errorStatString)
							{
								if (error)
								{
									*error = errorStatString;
								}
							}
							return errorStatString == 0;
						}
						hasTarget = fowUnitId < UnitId::None;
					}
					else
					{
						hasTarget = fowUnitId < UnitId::None;
						if (fowUnitId == UnitId::None)
						{
							if (error)
							{
								*error = 878; // Must target units.<0>
							}
							return 0;
						}
					}
					if (!hasTarget || !(~(units_dat::BaseProperty[fowUnitId] >> 29) & 1))
					{
						if (error)
						{
							*error = 876; // Invalid target.<0>
						}
						return 0;
					}
					goto LABEL_20;
				}
				if (hooks::getActiveTileFlag(y, x, 0x410000, 0))
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
			else // 00492886
			{
				if (target != nullptr)
				{
					unit->orderTarget.pt.x = target->sprite->position.x;
					unit->orderTarget.pt.y = target->sprite->position.y;

					if (target->sprite->position.x != unit->nextTargetWaypoint.x
					||	target->sprite->position.y != unit->nextTargetWaypoint.y) // 004928C1
					{
						unit->nextTargetWaypoint.x = target->sprite->position.x;
						unit->nextTargetWaypoint.y = target->sprite->position.y;
					}
				}
			}
		}
		else // 004928CF
		{
			if (!helper::canCastSpell_0(unit))
			{
				char* message = (char*)statTxtTblNew->getString(876); // Invalid target
				// do the actual transmission corresponding to the message
				helper::get_statTxt_Str_0(unit, unit->playerId, message);
				// make the unit stop since the order failed
				helper::setNextWaypoint(unit);
				unit->orderToIdle();
			}
			else // 0049291A
			{
				u8 techUsed = orders_dat::TechUsed[unit->mainOrderId];
				s32 spellCost;
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
					u8 raceId = unit->getRace();
					char* message = (char*)statTxtTblNew->getString(864 + raceId); // Not enough energy
					if (message != nullptr)
					{
						// display an error message only for the owner of the unit
						if (*LOCAL_NATION_ID == unit->playerId
						&&	*ACTIVE_PLAYER_ID == *LOCAL_HUMAN_ID)
						{
							helper::setUnitStatTxtErrorMsg(message);
						}
					}
					unit->orderToIdle();
				}
				else // 004929C2
				{
					u32 result_00492620;
					// may initiate movement from unit depending on mainOrderState
					if (unit->mainOrderState == 0)
					{
						result_00492620 = helper::OrderedSpell_ProgressMoving(unit, 1);
					}
					else
					{
						result_00492620 = helper::OrderedSpell_ProgressMoving(unit, 0);
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
						if (unit->getMovableState() == 2) // unmovable
						{
							unit->orderToIdle();
						}
					}
					else // 00492A01
					{

						if (unit->getSpellCountdown() == 0
						&&	gcd_condition // new
						&&	helper::function_00476640(unit, orderWeaponId) // is ready to fire weapon (angle,range...) maybe?
						&&	!(unit->movementFlags & MovementFlags::Accelerating)
						&&	unit->getMovableState() != 0)
						{ // 00492A38
							bool bStopHere = false;
							// in case it was modified by a previous function, relinking
							target = unit->orderTarget.unit;
							if (unit->pAI != nullptr && target != nullptr)
							{
								if (orderWeaponId <= WeaponId::Maelstrom) // Maelstrom is the last spell-related weapon
								{
									// array bound to the function
									const u8* const array_00492B44 = (u8*)(0x00492B44);

									// This code would not work for weapons id of NuclearMissile, YamatoGun, and weapons id before those, they must be excluded somewhere else
									// Maybe the switch and array use could be removed in a future version since enough elements use default value (switch 6)
									u32 switch_value = array_00492B44[orderWeaponId - 0x20];
									if (switch_value != 6)
									{
										// no idea why irradiate is so important
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

							if (!bStopHere) // 00492AA3
							{
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
								unit->orderSignal &= ~2; //&= -3
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

	// Hooked to safeguard against crashes
	void issueOrderTargetingGround(Unit* unit, u8 id, u16 coordX, u16 coordY) // 00475260
	{
		if (!unit)
		{
			return;
		}
		unit->userActionFlags |= 1u;
		unit->order(id, coordX, coordY, nullptr, UnitId::None, true);
		helper::orders_Nothing2(unit);
	}
}