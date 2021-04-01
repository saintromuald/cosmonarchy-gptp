#include "bunker_hooks.h"
#include <SCBW/scbwdata.h>
#include <SCBW\api.h>
#include "../SCBW/enumerations.h"
#include "../SCBW/structures.h"
//Helper function declarations. Do NOT modify!
namespace {

CUnit* findRandomAttackTarget(CUnit* unit);							//0x00442FC0
Bool32 attackApplyCooldown(CUnit* unit);							//0x00478B40
void setThingyVisibilityFlags(CThingy *thingy);						//0x004878F0
CThingy* createThingy(u16 spriteId, s16 x, s16 y, u8 playerId = 0);	//0x00488210
void setImageDirection(CImage *image, s8 direction);				//0x004D5F80


} //unnamed namespace

namespace hooks {

/// Checks whether the unit can attack from inside a bunker.
void unitAttackFromInsideBunkerHook(CUnit* unit) {

	if (	unit->id == UnitId::TerranMaverick
		||	unit->id == UnitId::TerranHarakan
		||	unit->id == UnitId::TerranCyprian
		||	unit->id == UnitId::TerranMadcap
		||	unit->id == UnitId::TerranApostle
		||	unit->id == UnitId::TerranEidolon
		||	unit->id == UnitId::TerranSavant
		||	unit->id == UnitId::ProtossEcclesiast
		||	unit->id == UnitId::ZergQuazilisk
		||	unit->id == UnitId::ZergHydralisk
		||	unit->id == UnitId::ZergBactalisk
		||	unit->id == UnitId::ZergLakizilisk
		||	unit->id == UnitId::ZergKeskathalor
	) {

		Bool32 resultApplyCooldown;
		unit->status |= UnitStatus::HoldingPosition;
		if(unit->subunit != nullptr)
			(unit->subunit)->status |= UnitStatus::HoldingPosition;
		resultApplyCooldown = attackApplyCooldown(unit);
		if(resultApplyCooldown != 0) {
			if(unit->orderTarget.pt.x != unit->nextTargetWaypoint.x
			|| unit->orderTarget.pt.y != unit->nextTargetWaypoint.y) {
				unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
				unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
			}
		}
		else {
			if(unit->mainOrderTimer == 0) {
				unit->mainOrderTimer = 15;
				unit->orderTarget.unit = findRandomAttackTarget(unit);
				if(unit->orderTarget.unit != nullptr)
					unit->orderQueueTimer = 0;
			} //if(unit->mainOrderTimer == 0)
		} //if(resultApplyCooldown == 0)
	} //if valid id
}

//not checked against original code by UndeadStar
void createBunkerAttackThingyHook(CUnit* unit) {

	CImage *bunkerImage = unit->connectedUnit->sprite->mainGraphic;

	u8 frameDirection = (unit->currentDirection1 + 16) / 32 % 8;

//	const LO_Header *loFile = lo_files->attackOverlays[bunkerImage->id];

	const LO_Header *loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlayAttackPtr, bunkerImage->id));
	Point8 offset = loFile->getOffset(bunkerImage->frameIndex, frameDirection);


	if (bunkerImage->flags & CImage_Flags::Mirrored) //Is inverted
		offset.x = -offset.x;

	u8 frameAngle;
	u16 spriteId;

	if (unit->id == UnitId::TerranHarakan) {
		frameAngle = ((unit->currentDirection1 + 8) / 16 % 16) * 16;
		spriteId = 378; //Firebat flamethrower graphics
	}
	else {
		frameAngle = frameDirection * 32;
		spriteId = 377; //Bunker attack overlay
	}

	CThingy *bunkerAttackEffect = 
		createThingy( spriteId, offset.x + unit->getX(), offset.y + unit->getY() );

	if (!bunkerAttackEffect) 
		return;

	bunkerAttackEffect->sprite->elevationLevel = unit->sprite->elevationLevel + 1;

	for (	
			CImage *image = bunkerAttackEffect->sprite->images.head;
			image != nullptr; 
			image = image->link.next
		) 
	{
		setImageDirection(image, frameAngle);
	}

	setThingyVisibilityFlags(bunkerAttackEffect);

}

} //hooks


//-------- Helper function definitions. Do NOT modify! --------//

namespace {

const u32 Func_findRandomAttackTarget = 0x00442FC0;
CUnit* findRandomAttackTarget(CUnit* unit) {

	static CUnit* target;

	__asm {
		PUSHAD
		MOV ESI, unit
		CALL Func_findRandomAttackTarget
		MOV target, EAX
		POPAD
	}

	return target;

}


const u32 Func_attackApplyCooldown = 0x00478B40;
Bool32 attackApplyCooldown(CUnit* unit) {

	static u32 result;

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_attackApplyCooldown
		MOV result, EAX
		POPAD
	}

	return result;

}

const u32 Func_SetThingyVisibilityFlags = 0x004878F0;
void setThingyVisibilityFlags(CThingy *thingy) {

	__asm {
		PUSHAD
		MOV ESI, thingy
		CALL Func_SetThingyVisibilityFlags
		POPAD
	}

}

;

const u32 Func_CreateThingy = 0x00488210;
CThingy* createThingy(u16 spriteId, s16 x, s16 y, u8 playerId) {

	static CThingy* thingy;
	static s32 x_;
	static s32 playerId_;
	static u32 spriteId_;

	x_ = x;
	playerId_ = playerId;
	spriteId_ = spriteId;

	__asm {
		PUSHAD
		PUSH playerId_
		MOV DI, y
		PUSH x_
		PUSH spriteId_
		CALL Func_CreateThingy
		MOV thingy, EAX
		POPAD
	}

	return thingy;

}

;

const u32 Func_SetImageDirection = 0x004D5F80;
void setImageDirection(CImage *image, s8 direction) {

	static u32 direction_;

	direction_ = direction;

	__asm {
		PUSHAD
		PUSH direction_
		MOV ESI, image
		CALL Func_SetImageDirection
		POPAD
	}

}

;

};
