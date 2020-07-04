/// This is where the magic happens; program your plug-in's core behavior here.

#include "game_hooks.h"
#include <graphics/graphics.h>
#include <SCBW/api.h>
#include <SCBW/ExtendSightLimit.h>
#include "psi_field.h"
#include <cstdio>
#include "CUnitExtended.h"


const int hyperTrigger = 0x006509A0;
const u32 Func_Sub4A01F0 = 0x004A01F0;
void function_004A01F0g(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub4A01F0
		POPAD
	}

}

;



const u32 Func_98A10= 0x00498A10;
void makeSpriteInvisible(CSprite* sprite) {

	__asm {
		PUSHAD
		MOV EAX, sprite
		CALL Func_98A10
		POPAD
	}

}
;



const u32 Func_UpdateUnitStrength = 0x0049FA40;
void updateUnitStrengthg(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_UpdateUnitStrength
		POPAD
	}

}
;


const u32 Func_CreateUnit = 0x004A09D0;
CUnit* CreateUnit(u32 unitId, int x, int y, u32 playerId) {
	static CUnit* unit_created;
	__asm {
		PUSHAD
		PUSH playerId
		PUSH y
		MOV ECX, unitId
		MOV EAX, x
		CALL Func_CreateUnit
		MOV unit_created, EAX
		POPAD
	}
	return unit_created;
}
;

const u32 Func_Sub4148F0 = 0x004148F0;
void function_004148F0_(int x, int y, u32 unitId) {

	__asm {
		PUSHAD
		PUSH y
		PUSH x
		PUSH unitId
		CALL Func_Sub4148F0
		POPAD
	}

}

;

const u32 Func_Sub47D770 = 0x0047D770;
void func_0047D770(CUnit* unit) {

	__asm {
		PUSHAD
		PUSH unit
		CALL Func_Sub47D770
		POPAD
	}

}

;


const u32 Func_updateNewUnitVision = 0x0045CE90;
void funcupdateNewUnitVision(CUnit* unit, int x, int y) {
	static u32 id = unit->id;
	__asm {
		PUSHAD
		MOV ECX, id
		PUSH y
		PUSH x
		CALL Func_updateNewUnitVision
		POPAD
	}

}

;

const u32 Func_ReplaceUnitWithType = 0x0049FED0;
void replaceUnitWithType(CUnit* unit, u16 newUnitId) {

	u32 newUnitId_ = newUnitId;

	__asm {
		PUSHAD
		PUSH newUnitId_
		MOV EAX, unit
		CALL Func_ReplaceUnitWithType
		POPAD
	}

}

;

const u32 Func_Irradiate = 0x00454E00;
void spawnCreeper(CUnit* unit) {

	int width = mapTileSize->width * 32;
	int height = mapTileSize->height * 32;
	if (unit->position.y >= 64 && unit->position.x >= 64 &&
		unit->position.y < height - 64 && unit->position.x < width - 64) {
		CUnit* creep_spawned = CreateUnit(UnitId::BasiliskCreeper,
			unit->position.x, unit->position.y,
			unit->playerId);
		if (creep_spawned == NULL) {
			//scbw::printText("Error: Creeper");
		}
		else {
			function_004A01F0g(creep_spawned);
			updateUnitStrengthg(creep_spawned);

			u32 tile_x = creep_spawned->sprite->position.x / 32;
			u32 tile_y = creep_spawned->sprite->position.y / 32;

			function_004148F0_(
				tile_x * 32,
				tile_y * 32,
				UnitId::nydus_canal
			);
			func_0047D770(creep_spawned);

			creep_spawned->status |= UnitStatus::Invincible;
			creep_spawned->status |= UnitStatus::NoCollide;
			creep_spawned->sprite->elevationLevel = 0;
			creep_spawned->status |= UnitStatus::NoCollide;
			creep_spawned->sprite->images.head->flags |= 0x20;
			creep_spawned->sprite->images.head->flags |= 0x40;
			creep_spawned->sprite->images.tail->flags |= 0x20;
			creep_spawned->sprite->images.tail->flags |= 0x40;
			//unit->connectedUnit = creep_spawned;
			//scbw::printText("Burrow basilisk");
			__asm {
				PUSHAD
				MOV EDI, creep_spawned
				PUSH 50000
				PUSH unit
				CALL Func_Irradiate
				POPAD
			}
		}

	}
}


#include "SCBW\UnitFinder.h"

class observanceMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CUnit* observed;
public:
	observanceMatch(CUnit* observed)
		: observed(observed) {}

	bool match(CUnit* unit) {
		
		if (unit == observed) {
			return false;
		}
		if (scbw::isAlliedTo(observed->playerId, unit->playerId)) {
			return false;
		}
		if (unit->id != UnitId::science_vessel) {
			return false;
		}
		
		return true;
	}
};


class shamanProc : public scbw::UnitFinderCallbackProcInterface
{

private:
	CUnit* shaman;

public:
	shamanProc(CUnit* shaman)
		: shaman(shaman) {}
	void proc(CUnit* unit)
	{
		if (unit->playerId != shaman->playerId) {
			if (!scbw::isAlliedTo(unit->playerId, shaman->playerId)) {
				return;
			}
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Building)
			return;
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Organic))
			return;
		if (unit->stasisTimer) {
			return;
		}
		if (unit == shaman) {
			return;
		}/*
		if (unit->isBeingHealed) {
			return;
		}*/
		if (unit->parasiteFlags) {
			return;
		}

		u32 dist = scbw::getDistanceFast(unit->position.x, unit->position.y,
			shaman->position.x, shaman->position.y);
		//unit->isBeingHealed = true;
		bool reqHeal = false;

		if (unit->hitPoints < units_dat::MaxHitPoints[unit->id]) {
			reqHeal = true;
			u32 overlayImageId;
			if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
				overlayImageId = ImageId::RestorationHit_Large;
			else
				overlayImageId = ImageId::RestorationHit_Small;
			CUnit* tgt = NULL;
			if (unit->subunit != NULL)
				tgt = unit->subunit;
			else
				tgt = unit;
			(tgt->sprite)->createTopOverlay(overlayImageId, 0, 0, 0);

		}
		unit->setHp(unit->hitPoints + (6 * 256));
		if (unit->hitPoints == units_dat::MaxHitPoints[unit->id] && reqHeal) {
			if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::AdrenalinePacks) > 0) {
				if (!unit->getOverlay(971)) {
					unit->sprite->createTopOverlay(971);
				}
				scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericTimer, ValueId::Adrenaline, 1 + (2 * 24), 0);
			}
			return;
		}


		/*if (dist <= 35) {
			unit->setHp(unit->hitPoints + (6 * 256));
		}
		else if (dist <= 50) {
			unit->setHp(unit->hitPoints + (4 * 256));
		}
		else  {
			unit->setHp(unit->hitPoints + (2 * 256));
		}*/
	}
};

#include "logger.h"

namespace hooks {

	/// This hook is called every frame; most of your plugin's logic goes here.
	bool nextFrame() {

		if (!scbw::isGamePaused()) { //If the game is not paused

			scbw::setInGameLoopState(true); //Needed for scbw::random() to work
			graphics::resetAllGraphics();
			hooks::updatePsiFieldProviders();
			*(int*)hyperTrigger = 0; // Single frame hypertriggers

			//This block is executed once every game.
			if (*elapsedTimeFrames == 0) {
				//Write your code here
			}
			/*
			if ((*activePortraitUnit) != NULL) {
				char buf[32];
				sprintf(buf, "Current index: %d, order: %d",(*activePortraitUnit)->getIndex(),
					(*activePortraitUnit)->mainOrderId);
				scbw::printText(buf);
			}*/

			/*
			char b[32];
			sprintf(b, "Tiles: %d %d %d %d %d", (*tileGroup + 0x2)[1].megatiles[0],
												(*tileGroup + 0x2)[1].megatiles[1],
				(*tileGroup + 0x2)[1].megatiles[2],
				(*tileGroup + 0x2)[1].megatiles[3],
				(*tileGroup + 0x2)[1].megatiles[4]

				);
			scbw::printText(b);*/

			//Loop through all visible units in the game.
			for (CUnit* unit = *firstVisibleUnit; unit; unit = unit->link.next) {
				CUnitExtended::get(unit)->tick();

				if (unit->id == UnitId::ProtossLegionnaire && scbw::getUpgradeLevel(unit->playerId,UpgradeId::BladeVortex)) {
					//teleport jump
					if (unit->orderTarget.unit != NULL && (unit->mainOrderId==OrderId::Attack1 || unit->mainOrderId==OrderId::AttackUnit)) {
						auto target = unit->orderTarget.unit;
						auto distance = unit->getDistanceToTarget(target);
						
						if (distance > 15 && distance<=96) {
							if (scbw::get_aise_value(unit, target, AiseId::GetLegionnaireMarks, 0, 0) > 0) {
								bool teleport = scbw::teleport(unit, target->sprite->position, 0, true);
								if (!unit->getOverlay(971) && teleport) {
									unit->sprite->createTopOverlay(971);
								}
							}
						}
					}
				}
				/*
				if (unit->id == UnitId::observer) {
					u16 tileid = scbw::getTileIdAt(unit->position.x, unit->position.y);
					char buf[64];
					u32 offset = (unit->position.x/ 32) + mapTileSize->width*(unit->position.y/ 32);
					u32 offset_val = 0x005993C4 + (2 * offset);
	//				sprintf(buf, "Tile: %d, p%d %d, offset %02X", tileid, unit->position.x / 32, unit->position.y / 32, offset_val);
	//				scbw::printText(buf);
					u16 megatile = (*megatileArray)[(unit->position.x / 32) + mapTileSize->width*(unit->position.y / 32)];
					u16 tile_id = (*tileIdArray)[(unit->position.x / 32) + mapTileSize->width*(unit->position.y / 32)];
					u16 gval = (tile_id >> 0x4) & 0x07ff;
					u16 hval = gval * 0x2a;//half of new struct
					ActiveTile A = scbw::getActiveTileAt(unit->position.x, unit->position.y);
					sprintf(buf, "tile flags: %d %d %d %d",

						A.exploredFlags,A.groundHeight,A.hasCreep,A.hasDoodadCover
						);
					scbw::printText(buf);
				}

				*/
				/*
				//debug test for cyprians
				if(unit->id==UnitId::ultralisk && (*elapsedTimeFrames)%18 == 0){
					//cyprian back-forth test
					u32 result = scbw::get_aise_value(unit,NULL,1,0,0);
					char buf[32];
					sprintf(buf,"Get aise value: %d",result);
					scbw::printText(buf);
					if(result == 1){
						scbw::printText("Cyprian state is set");
					}
					else {
						scbw::printText("Cyprian state do not exist or not set, send aise cmd");
						scbw::sendGPTP_aise_cmd(unit,NULL,2,1,0,0);
					}
				}*/
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::DarkTemplarTimer, 0) == 1) {
					bool uncover = false;
					for (int i = 0; i < 8; i++) {
						if (!scbw::isAlliedTo(i, unit->playerId)) {
							if (unit->visibilityStatus & (1 << i)) {
								unit->visibilityStatus &= ~(1 << i);
								uncover = true;
							}
						}
					}
					
				}
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::ObservanceTimerDebuff, 0) == 1) {
					auto range = 160;
					auto observance = observanceMatch(unit);
					auto a = scbw::UnitFinder::getNearestTarget(
						unit->getX() - range, unit->getY() - range,
						unit->getX() + range, unit->getY() + range,
						unit, observance
					);

					if (a != NULL) {
						scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericTimer, ValueId::ObservanceTimerDebuff, (5 * 24) + 1, 0);
						if (scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::ObservanceRend, 0) < 5) {
							scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::AddGenericValue, ValueId::ObservanceRend, 1, 0);
						}	
					}
					else {
						scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::ObservanceRend, 0, 0);
					}
				}
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::MaledictionSlow, 0) == 1) {
					u32 overlayImageId;
					if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
						overlayImageId = ImageId::MaelstromOverlay_Medium;
					else
						if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
							overlayImageId = ImageId::MaelstromOverlay_Large;
						else
							overlayImageId = ImageId::MaelstromOverlay_Small;
					while (unit->sprite->getOverlay(overlayImageId)) {
						unit->sprite->removeOverlay(overlayImageId);
					}
					unit->updateSpeed();
				}


				

				//(*unit)._dc132 |= 0x8;
				//(*unit)._dc133 = 24;

				if (unit->id == UnitId::civilian) {
					if (unit->_padding_0x132 & 0x8) {
						unit->groundWeaponCooldown = 8;
					}
					if (unit->_padding_0x133 > 0) {
						unit->_padding_0x133--;
					}
					if (unit->_padding_0x133 == 0) {
						if (unit->_padding_0x132 & 0x8) {
							unit->_padding_0x133 = 24;
							unit->sprite->playIscriptAnim(IscriptAnimation::CastSpell);
							shamanProc Heal = shamanProc(unit);
							scbw::UnitFinder unitFinder(unit->getX() - 128,
								unit->getY() - 128,
								unit->getX() + 128,
								unit->getY() + 128);
							unitFinder.forEach(Heal);
						}
					}
				}
				if (unit->id == UnitId::Powerup_KhalisCrystal) //basilisk
				{
					if (unit->status & UnitStatus::Burrowed) {
						if (!(unit->_padding_0x132 & HydraFlags1::BasiliskBurrowed))
						{
							unit->_padding_0x132 |= HydraFlags1::BasiliskBurrowed;
							spawnCreeper(unit);
						}
					}
					else {
						/*
						if (unit->_padding_0x132 & HydraFlags1::BasiliskBurrowed) {
							if (unit->connectedUnit != NULL) {
								unit->connectedUnit->userActionFlags |= 0x4;
								unit->connectedUnit->remove();
							}
						}
						unit->_padding_0x132 &= ~HydraFlags1::BasiliskBurrowed;
											*/
					}
				}
				if (unit->id == UnitId::Powerup_KhalisCrystal && unit->orderSignal & 0x20) {
					unit->orderSignal &= ~0x20;
					unit->playIscriptAnim(IscriptAnimation::Unused1);
				}
				if (unit->id == UnitId::ZergLurker && unit->orderSignal & 0x80) {
					unit->orderSignal &= ~0x80;
					if (scbw::getUpgradeLevel(unit->playerId, UpgradeId::TectonicClaws) == 1) {
						unit->playIscriptAnim(IscriptAnimation::Unused3);
					}
				}
			}
			scbw::setInGameLoopState(false);
		}
		return true;

	}

	bool gameOn() {
		return true;
	}

	bool gameEnd() {
		return true;
	}

} //hooks
