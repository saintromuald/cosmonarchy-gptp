#include "bunker_hooks.h"
#include <SCBW/scbwdata.h>
#include <SCBW\api.h>
#include "../SCBW/enumerations.h"
#include "../SCBW/structures.h"
#include "globals.h"
#include "scbw/UnitFinder.h"

namespace hooks
{
	/// Checks whether the unit can attack from inside a bunker.
	void unitAttackFromInsideBunkerHook(Unit* unit)
	{
		if (scbw::unitIsRangedAttacker(unit))
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
					
					if (!unit->orderTarget.unit || !unit->canAttackTarget(unit->orderTarget.unit))
					{
						if (unit->connectedUnit)
						{
							UnitExt* bunkerExt = globals.GetUnitExt(unit->connectedUnit);
							Unit* target = bunkerExt->units[UnitBunkerLockOn];
							if (target && unit->canAttackTarget(target))
							{
								unit->orderTarget.unit = bunkerExt->units[UnitBunkerLockOn]; // TODO: Find a way to set bunker lock on from attack order
							}
						}

						if (!unit->orderTarget.unit || !unit->canAttackTarget(unit->orderTarget.unit))
						{
							const u32 radius = std::max(unit->getMaxWeaponRange(unit->getGroundWeapon()), unit->getMaxWeaponRange(unit->getAirWeapon()));
							unit->orderTarget.unit = scbw::UnitFinder::getNearestTarget(
								unit->getX() - radius,
								unit->getY() - radius,
								unit->getX() + radius,
								unit->getY() + radius,
								unit,
								[&](Unit* target)
							{
								if (scbw::isAlliedTo(unit->playerId, target->playerId)
									|| !unit->canAttackTarget(target))
								{
									return false;
								}
								return true;
							});
						}
					}

					if (unit->orderTarget.unit != nullptr)
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

		if (bunkerImage->flags & CImage_Flags::Mirrored) //Is inverted
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
		Thingy* bunkerAttackEffect = helper::createThingy(spriteId, offset.x + unitVar->getX(), offset.y + unitVar->getY(), unitVar->playerId);
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