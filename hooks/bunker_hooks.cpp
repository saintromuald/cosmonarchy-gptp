#include "bunker_hooks.h"
#include <SCBW/scbwdata.h>
#include <SCBW\api.h>
#include "../SCBW/enumerations.h"
#include "../SCBW/structures.h"
#include "globals.h"
#include "scbw/UnitFinder.h"

namespace hooks
{
	static inline bool isWithinAttackRange(Unit* attacker, Unit* target)
	{
		if (!target)
		{
			return false;
		}
		if (target->status & UnitStatus::InAir)
		{
			return !attacker->isTargetWithinMinRange(target, weapons_dat::MinRange[attacker->getAirWeapon()])
				&& attacker->isTargetWithinMinRange(target, weapons_dat::MaxRange[attacker->getAirWeapon()]);
		}
		else
		{
			return !attacker->isTargetWithinMinRange(target, weapons_dat::MinRange[attacker->getGroundWeapon()])
				&& attacker->isTargetWithinMinRange(target, weapons_dat::MaxRange[attacker->getGroundWeapon()]);
		}
	}

	// Checks whether the unit can attack from inside a bunker.
	void unitAttackFromInsideBunkerHook(Unit* unit)
	{
		if (scbw::unit_is_ranged_attacker(unit))
		{
			b32 resultApplyCooldown;
			unit->status |= UnitStatus::HoldingPosition;
			if (unit->subunit != nullptr)
			{
				(unit->subunit)->status |= UnitStatus::HoldingPosition;
			}
			resultApplyCooldown = helper::attackApplyCooldown(unit);
			if (resultApplyCooldown != 0)
			{
				if (unit->orderTarget.pt.x != unit->nextTargetWaypoint.x
					|| unit->orderTarget.pt.y != unit->nextTargetWaypoint.y)
				{
					unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
					unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
				}
			}
			else
			{
				if (unit->mainOrderTimer == 0)
				{
					unit->mainOrderTimer = 15;
					
					Unit* target = unit->orderTarget.unit;
					if (!target || !unit->canAttackTarget(target) || !isWithinAttackRange(unit, target))
					{
						if (unit->connectedUnit)
						{
							UnitExt* bunkerExt = globals.get_unit_ext(unit->connectedUnit);
							Unit* target = bunkerExt->units[UnitBunkerLockOn];
							if (target && unit->canAttackTarget(target) && isWithinAttackRange(unit, target))
							{
								unit->orderTarget.unit = bunkerExt->units[UnitBunkerLockOn]; // TODO: Find a way to set bunker lock on from attack order
							}
						}

						if (!target || !unit->canAttackTarget(target) || !isWithinAttackRange(unit, target))
						{
							const u32 radius = std::max(unit->getMaxWeaponRange(unit->getGroundWeapon()), unit->getMaxWeaponRange(unit->getAirWeapon()));
							unit->orderTarget.unit = scbw::UnitFinder::getNearestTarget(
								unit->get_x() - radius,
								unit->get_y() - radius,
								unit->get_x() + radius,
								unit->get_y() + radius,
								unit,
								[&](Unit* target)
							{
								if (scbw::is_allied_to(unit->player, target->player)
									|| !unit->canAttackTarget(target)
									|| !isWithinAttackRange(unit, target))
								{
									return false;
								}
								return true;
							});
						}
					}

					if (target != nullptr)
					{
						unit->orderQueueTimer = 0;
					}
				}
			}
		}
	}

	//not checked against original code by UndeadStar
	void createBunkerAttackThingyHook(Unit* unit)
	{
		// Fixes units with subunits not attacking
		Unit* unitVar = unit;
		if (unit->subunit)
		{
			unitVar = unit->subunit;
		}

		Image* bunkerImage = unitVar->connectedUnit->sprite->mainGraphic;
		u8 frameDirection = (unitVar->currentDirection1 + 16) / 32 % 8;
		const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlayAttackPtr, bunkerImage->id));
		Point8 offset = loFile->getOffset(bunkerImage->frameIndex, frameDirection);

		if (bunkerImage->flags & ImageFlags::Mirrored) // Is inverted
		{
			offset.x = -offset.x;
		}

		u8 frameAngle;
		u16 spriteId;
		if (unitVar->id == UnitId::TerranHarakan)
		{
			frameAngle = ((unitVar->currentDirection1 + 8) / 16 % 16) * 16;
			spriteId = 378; // Firebat flamethrower graphics
		}
		else
		{
			frameAngle = frameDirection * 32;
			spriteId = 377; // Bunker attack overlay
		}

		scbw::decloakProc(unitVar->connectedUnit);

		// TODO: Fix effect nonsense with subunits
		Thingy* bunkerAttackEffect = helper::createThingy(spriteId, offset.x + unitVar->get_x(), offset.y + unitVar->get_y(), unitVar->player);
		if (!bunkerAttackEffect)
		{
			return;
		}

		bunkerAttackEffect->sprite->elevationLevel = unitVar->sprite->elevationLevel + 1;
		for (Image* image = bunkerAttackEffect->sprite->images.head;
			image != nullptr;
			image = image->link.next)
		{
			helper::setImageDirection(image, frameAngle);
		}
		helper::setThingyVisibilityFlags(bunkerAttackEffect);
	}
}