#include "bunker_hooks.h"
#include <SCBW/scbwdata.h>
#include <SCBW\api.h>
#include "../SCBW/enumerations.h"
#include "../SCBW/structures.h"

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
					// TODO: Remove RNG
					unit->orderTarget.unit = helper::findRandomAttackTarget(unit);
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
		Image* bunkerImage = unit->connectedUnit->sprite->mainGraphic;
		u8 frameDirection = (unit->currentDirection1 + 16) / 32 % 8;
		const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlayAttackPtr, bunkerImage->id));
		Point8 offset = loFile->getOffset(bunkerImage->frameIndex, frameDirection);

		if (bunkerImage->flags & CImage_Flags::Mirrored) //Is inverted
		{
			offset.x = -offset.x;
		}

		u8 frameAngle;
		u16 spriteId;
		if (unit->id == UnitId::TerranHarakan)
		{
			frameAngle = ((unit->currentDirection1 + 8) / 16 % 16) * 16;
			spriteId = 378; // Firebat flamethrower graphics
		}
		else
		{
			frameAngle = frameDirection * 32;
			spriteId = 377; // Bunker attack overlay
		}

		scbw::decloakProc(unit->connectedUnit);

		Thingy* bunkerAttackEffect = helper::createThingy(spriteId, offset.x + unit->getX(), offset.y + unit->getY(), unit->playerId);
		if (!bunkerAttackEffect)
		{
			return;
		}

		bunkerAttackEffect->sprite->elevationLevel = unit->sprite->elevationLevel + 1;
		for (Image* image = bunkerAttackEffect->sprite->images.head;
			image != nullptr;
			image = image->link.next)
		{
			helper::setImageDirection(image, frameAngle);
		}
		helper::setThingyVisibilityFlags(bunkerAttackEffect);
	}
}