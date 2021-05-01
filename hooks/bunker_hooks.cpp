#include "bunker_hooks.h"
#include <SCBW/scbwdata.h>
#include <SCBW\api.h>
#include "../SCBW/enumerations.h"
#include "../SCBW/structures.h"

namespace hooks {
	/// Checks whether the unit can attack from inside a bunker.
	void unitAttackFromInsideBunkerHook(CUnit* unit) {
		if (unit->id == UnitId::TerranMaverick
			|| unit->id == UnitId::TerranHarakan
			|| unit->id == UnitId::TerranCyprian
			|| unit->id == UnitId::TerranMadcap
			|| unit->id == UnitId::TerranApostle
			|| unit->id == UnitId::TerranEidolon
			|| unit->id == UnitId::TerranSavant
			|| unit->id == UnitId::ProtossEcclesiast
			|| unit->id == UnitId::ZergQuazilisk
			|| unit->id == UnitId::ZergHydralisk
			|| unit->id == UnitId::ZergBactalisk
			|| unit->id == UnitId::ZergLakizilisk
			|| unit->id == UnitId::ZergKeskathalor
			) {
			bool32 resultApplyCooldown;
			unit->status |= UnitStatus::HoldingPosition;
			if (unit->subunit != nullptr)
				(unit->subunit)->status |= UnitStatus::HoldingPosition;
			resultApplyCooldown = attackApplyCooldown(unit);
			if (resultApplyCooldown != 0) {
				if (unit->orderTarget.pt.x != unit->nextTargetWaypoint.x
					|| unit->orderTarget.pt.y != unit->nextTargetWaypoint.y) {
					unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
					unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
				}
			} else {
				if (unit->mainOrderTimer == 0) {
					unit->mainOrderTimer = 15;
					unit->orderTarget.unit = findRandomAttackTarget(unit);
					if (unit->orderTarget.unit != nullptr)
						unit->orderQueueTimer = 0;
				} //if(unit->mainOrderTimer == 0)
			} //if(resultApplyCooldown == 0)
		} //if valid id
	}

	//not checked against original code by UndeadStar
	void createBunkerAttackThingyHook(CUnit* unit) {
		CImage* bunkerImage = unit->connectedUnit->sprite->mainGraphic;
		uint8 frameDirection = (unit->currentDirection1 + 16) / 32 % 8;
		//	const LO_Header *loFile = lo_files->attackOverlays[bunkerImage->id];
		const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlayAttackPtr, bunkerImage->id));
		Point8 offset = loFile->getOffset(bunkerImage->frameIndex, frameDirection);

		if (bunkerImage->flags & CImage_Flags::Mirrored) //Is inverted
			offset.x = -offset.x;

		uint8 frameAngle;
		uint16 spriteId;

		if (unit->id == UnitId::TerranHarakan) {
			frameAngle = ((unit->currentDirection1 + 8) / 16 % 16) * 16;
			spriteId = 378; //Firebat flamethrower graphics
		} else {
			frameAngle = frameDirection * 32;
			spriteId = 377; //Bunker attack overlay
		}

		CThingy* bunkerAttackEffect =
			createThingy(spriteId, offset.x + unit->getX(), offset.y + unit->getY(), unit->playerId);

		if (!bunkerAttackEffect)
			return;

		bunkerAttackEffect->sprite->elevationLevel = unit->sprite->elevationLevel + 1;

		for (
			CImage* image = bunkerAttackEffect->sprite->images.head;
			image != nullptr;
			image = image->link.next
			) {
			setImageDirection(image, frameAngle);
		}

		setThingyVisibilityFlags(bunkerAttackEffect);

	}
}