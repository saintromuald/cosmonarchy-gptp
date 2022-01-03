#include <SCBW/api.h>
//copied from unhooked
#include <SCBW/scbwdata.h>
#include <SCBW/enumerations/TechId.h>
#include <SCBW/api.h>
#include <cassert>
#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include "hooks\orders\ai\ai_orders.h"
#include "globals.h"

namespace hooks
{
	void orders_ZergBirth(Unit* unit)
	{
		if (!(unit->orderSignal & 4))
		{
			Image* current_image = unit->sprite->mainGraphic;
			if (current_image->animation != IscriptAnimation::SpecialState1)
			{
				current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
			}
		}

		if (unit->orderSignal & 4)
		{
			bool bTwinEggOrCocoon = true;
			Point16 pos;

			unit->orderSignal &= ~4;
			bool vorvaling = false;
			auto origin = unit->eggOrigin;
			if (origin == UnitId::ZergZethrokor)
			{
				vorvaling = true;
			}
			if (units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg
				|| unit->previousUnitType == UnitId::ZergCocoon)
			{
				bTwinEggOrCocoon = false;
			}
			Unit* unit2 = nullptr;
			if (units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg && !vorvaling)
			{
				unit2 = helper::zergDualBirth(unit); // zerg dual birth
				if (unit2)
				{
					helper::AI_TrainingUnit(unit2, unit);
				}
			}
			if (unit->id != UnitId::ZergDroleth
				&& unit->id != UnitId::ZergOvileth)
			{
				helper::AI_TrainingUnit(unit, unit);
			}
			helper::initBuilding(unit, 0);
			// update various stuff (set hp, set shield...) not finished on Morph
			helper::updateUnitStatsFinishBuilding(unit);
			helper::updateUnitStrength(unit);
			pos.x = unit->sprite->position.x;
			pos.y = unit->sprite->position.y;

			if (bTwinEggOrCocoon)
			{
				if (units_dat::MovementFlags[unit->id] == MovementFlags::HoverUnit)
				{
					pos.y -= 7;
				}
				else if (unit->status & UnitStatus::InAir)
				{
					pos.y -= 42;
				}
			}
			helper::fixTargetLocation(&pos, unit->id);
			if (pos.x != unit->sprite->position.x ||
				pos.y != unit->sprite->position.y)
			{
				scbw::setUnitPosition(unit, pos.x, pos.y);
			}
			if (!scbw::isZergEggNew(unit, true, false, unit->previousUnitType))
			{
				if (unit->eggOrigin == UnitId::ZergLarva)
				{
					helper::orderNewUnitToRally(unit, unit->connectedUnit);
					if (unit2 != nullptr)
					{
						helper::orderNewUnitToRally(unit2, unit->connectedUnit);
					}
				}
			}
			helper::actUnitReturnToIdle(unit);
			unit->connectedUnit = nullptr;
		}
	}

	void orders_Morph1(Unit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		if (unit->mainOrderState == 0)
		{
			if (scbw::isConventionalUnitMorphSource(unit->id, true, true))
			{
				if (!helper::hasSuppliesForUnit(unit->buildQueue[unit->buildQueueSlot], unit->playerId, 1))
				{
					helper::refundAllQueueSlots(unit);
					unit->orderComputerClear(units_dat::ReturnToIdleOrder[unit->id]);
				}
				else
				{
					u16 eggId;
					switch (unit->id)
					{
						case UnitId::ZergOvileth:
						case UnitId::ZergIroleth:
						case UnitId::ZergOthstoleth:
						{
							break;
						}
						default:
						{
							helper::incrementUnitScoresEx(unit, -1, 0);
						} break;
					}

					if (unit->status & UnitStatus::Completed)
					{
						unit->status -= UnitStatus::Completed;
					}
					switch (unit->id)
					{
						case UnitId::ZergLarva:
						case UnitId::ZergZethrokor:
						case UnitId::ZergQuazilisk:
						case UnitId::ZergHydralisk:
						case UnitId::ZergZoryusthaleth:
						case UnitId::ZergKonvilisk:
						case UnitId::ZergKeskathalor:
						{
							eggId = UnitId::ZergEgg;
						} break;
						case UnitId::ZergOvileth:
						case UnitId::ZergIroleth:
						case UnitId::ZergOthstoleth:
						case UnitId::ZergNathrokor:
						case UnitId::ZergSkithrokor:
						case UnitId::ZergMutalisk:
						case UnitId::ZergMatraleth:
						{
							eggId = UnitId::ZergCocoon;
						} break;
						default:
						{
							if (unit->id != UnitId::ProtossCantavis)
							{
								return;
							}
						}  break;
					}

					unitExt->values[ValuePreMorphEnergy] = unit->energy;
					if (unit->id == UnitId::ProtossCantavis)
					{
						helper::replaceUnitWithType(unit, UnitId::ProtossAugur);
						unit->sprite->flags &= ~SpriteFlags::IscriptCode;
						if (unit->sprite->images.head != nullptr)
						{
							Image* current_image = unit->sprite->images.head;
							do
							{
								current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
								current_image = current_image->link.next;
							}
							while (current_image != nullptr);
						}
						helper::changeUnitButtonSet(unit, UnitId::None);
						scbw::refreshConsole();
						unit->status &= ~UnitStatus::IsBuilding; // disable the flag if set
						helper::setArchonPathing(unit);
						unit->orderComputerClear(OrderId::CompletingArchonSummon);
						return;
					}
					else
					{
						unit->eggOrigin = unit->id;
						// morph into egg from source unit
						helper::replaceUnitWithType(unit, eggId);
					}
					helper::changeUnitButtonSet(unit, unit->id);
					unit->remainingBuildTime = units_dat::TimeCost[unit->buildQueue[unit->buildQueueSlot]];
					unit->mainOrderState = 1;
					auto origin = unit->eggOrigin;
					if (origin == UnitId::ZergZethrokor
						|| origin == UnitId::ZergNathrokor)
					{
						unit->sprite->playIscriptAnim(IscriptAnimation::Unused1, true);
					}
					else if (origin == UnitId::ZergUltrokor
						|| origin == UnitId::ZergOvileth)
					{
						unit->sprite->playIscriptAnim(IscriptAnimation::AlmostBuilt, true);
					}
					else if (origin == UnitId::ZergQuazilisk)
					{
						unit->sprite->playIscriptAnim(IscriptAnimation::GndAttkToIdle, true);
					}
					else if (origin == UnitId::ZergLarva)
					{
						unit->sprite->playIscriptAnim(IscriptAnimation::Walking, true);
					}
				}
			}
		}
		else if (unit->mainOrderState == 1)
		{
			// advance
			if (helper::advanceRemainingBuildTime(unit))
			{
				auto anim = IscriptAnimation::SpecialState1;
				unit->sprite->playIscriptAnim(anim, true);
				unit->mainOrderState = 2;
			}
		}
		else if (unit->mainOrderState == 2)
		{
			if (unit->orderSignal & 4)
			{
				unit->orderSignal -= 4;
				auto origin = unit->eggOrigin;
				switch (origin)
				{
					case UnitId::ZergOvileth:
					{
						unitExt->values[ValueOvilethSupply] = 1;
					} break;
					case UnitId::ZergIroleth:
					{
						unitExt->values[ValueOvilethSupply] = 2;
					} break;
					case UnitId::ZergOthstoleth:
					{
						unitExt->values[ValueOvilethSupply] = 3;
					} break;
				}
				helper::replaceUnitWithType(unit, unit->buildQueue[unit->buildQueueSlot]);
				unit->energy = unitExt->values[ValuePreMorphEnergy];
				if (unit->id == UnitId::ZergDroleth
					|| unit->id == UnitId::ZergOvileth)
				{
					helper::AI_TrainingUnit(unit, unit);
				}
				helper::playMorphingCompleteSound(unit);
				unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
				if (units_dat::ConstructionGraphic[unit->id] != 0)
				{
					Image* current_image;
					helper::initBuilding(unit, 1);
					current_image = unit->sprite->images.head;
					while (current_image != nullptr)
					{
						auto anim = IscriptAnimation::SpecialState1;
						current_image->playIscriptAnim(anim);
						current_image = current_image->link.next;
					}
					unit->orderComputerClear(OrderId::ZergBirth);
				}
				else // 0045DF60
				{
					helper::updateUnitStatsFinishBuilding(unit);
					helper::updateUnitStrength(unit);
					if (unit->id == UnitId::ZergEgg && unit->eggOrigin == UnitId::ZergLarva)
					{
						helper::orderNewUnitToRally(unit, unit->connectedUnit);
					}
				}
			}
		}
	}

	// Check if @p unit can morph into @p morphUnitId.
	bool unitCanMorphHook(Unit* unit, u16 morphUnitId)
	{
		// Default StarCraft behavior
		if (unit->id == UnitId::ZergZethrokor)
		{
			if (morphUnitId == UnitId::ZergVorvrokor)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergQuazilisk)
		{
			if (morphUnitId == UnitId::ZergLiiralisk)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergHydralisk)
		{
			if (morphUnitId == UnitId::ZergBactalisk)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergSkithrokor)
		{
			if (morphUnitId == UnitId::ZergGorgrokor)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergMutalisk)
		{
			if (morphUnitId == UnitId::ZergVithrilisk)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergNathrokor)
		{
			if (morphUnitId == UnitId::ZergKalkalisk)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergZoryusthaleth)
		{
			if (morphUnitId == UnitId::ZergUltrokor)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergMatraleth)
		{
			if (morphUnitId == UnitId::ZergAkistrokor)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergKonvilisk)
		{
			if (morphUnitId == UnitId::ZergAlmaksalisk)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergKeskathalor)
		{
			if (morphUnitId == UnitId::ZergAlkajelisk)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergOvileth)
		{
			if (morphUnitId == UnitId::ZergIroleth)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergIroleth)
		{
			if (morphUnitId == UnitId::ZergOthstoleth)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergOthstoleth)
		{
			if (morphUnitId == UnitId::ZergAlaszileth)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ZergLarva)
		{
			if (unit->canMakeUnit(morphUnitId, *ACTIVE_NATION_ID) == 1)
			{
				return true;
			}
		}
		else if (unit->id == UnitId::ProtossCantavis)
		{
			if (morphUnitId == UnitId::ProtossAugur)
			{
				return true;
			}
		}
		return false;
	}

	// Check if @p unitId is an egg unit that can be rallied
	bool isRallyableEggUnitHook(u16 unitId)
	{
		if (scbw::isZergEggNew(nullptr, false, false, unitId))
		{
			return false;
		}
		return true;
	}

	// Return the ID of the egg unit to use when morphing @p unitId.
	// If the unit cannot morph, return UnitId::None.
	u16 getUnitMorphEggTypeHook(u16 unitId, Unit* unit, u16 resultId)
	{
		// Default StarCraft behavior
		if (units_dat::BaseProperty[unitId] & UnitProperty::Flyer)
		{
			return UnitId::ZergCocoon;
		}
		else
		{
			return UnitId::ZergEgg;
		}
		return UnitId::None;
	}

	// Determine the type (unit ID) of the unit to revert to when cancelling an
	// @p eggUnit while it is morphing.
	u16 getCancelMorphRevertTypeHook(Unit* eggUnit)
	{
		auto source = eggUnit->eggOrigin;
		if (eggUnit->getRace() == RaceId::Zerg
			&& eggUnit->id != UnitId::ZergInfestedTerran)
		{
			if (!(units_dat::BaseProperty[eggUnit->id] & UnitProperty::Building)
				&& source != UnitId::ZergLarva)
			{
				return source;
			}
		}
		return UnitId::None;  // Default (no revert for larvae)
	}

	// Determines the vertical (Y) offset by which the @p unit will be shifted to
	// when it finishes morphing.
	s16 getUnitVerticalOffsetOnBirth(Unit* unit)
	{
		// No offset, birth offset is handled elsewhere
		if (units_dat::BaseProperty[unit->id] & UnitProperty::TwoUnitsIn1Egg)
		{
			return 0;
		}
		// No offset, since the morphed unit should stay where it is
		if (unit->previousUnitType == UnitId::ZergCocoon)
		{
			return 0;
		}
		// Hovering units (?) f32 7 pixels above ground
		if (units_dat::MovementFlags[unit->id] == MovementFlags::HoverUnit)
		{
			return -7;
		}
		// Air units f32 42 pixels above ground
		if (unit->status & UnitStatus::InAir)
		{
			return -42;
		}
		// Default for ground units
		return 0;
	}

	Unit* beginTrain(Unit* unit, u16 unit_id, bool notify_error)
	{
		if (unit->status & UnitStatus::GroundedBuilding
			|| unit->id == UnitId::ProtossSolarion
			|| unit->id == UnitId::ProtossAccantor
			|| scbw::isLarvalEgg(unit)) // just egg
		{
			if (helper::hasSuppliesForUnit(unit_id, unit->playerId, notify_error))
			{
				Unit* result = helper::createUnit(unit_id, unit->getX(), unit->getY(), unit->playerId);
				if (result)
				{
					return result;
				}
				helper::displayLastNetErrorForPlayer(unit->playerId);
			}
		}
		return 0;
	}
}