/// This is where the magic happens; program your plug-in's core behavior here.

#include "game_hooks.h"
#include <graphics/graphics.h>
#include <SCBW/api.h>
#include <SCBW/ExtendSightLimit.h>
#include "psi_field.h"
#include <cstdio>
#include "CUnitExtended.h"
#include "hooks/interface/uifeatures/screen_hotkeys.h"
#include "hooks/interface/uifeatures/uifeatures.h"
#include "hooks/interface/hpbars/hpbars.h"
#include "Utils/captivating_claws.h"
#include "hooks/hydraFinder.h"
#include "globals.h"
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
		if (unit == observed
		|| scbw::isAlliedTo(observed->playerId, unit->playerId)
		|| unit->id != UnitId::TerranScienceVessel) {
			return false;
		}

		return true;
	}
};

class observanceVictimMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CUnit* vessel;
public:
	observanceVictimMatch(CUnit* vessel)
		: vessel(vessel) {}

	bool match(CUnit* unit) {
		if (unit == vessel
		|| scbw::isAlliedTo(vessel->playerId, unit->playerId)
		|| scbw::get_generic_timer(unit, ValueId::ObservanceTimerDebuff) <= 1) {
			return false;
		}

		return true;
	}
};
class skyfallMatch : public scbw::UnitFinderCallbackMatchInterface {
private:
	CUnit* paladin;
public:
	skyfallMatch(CUnit* paladin)
		: paladin(paladin) {}

	bool match(CUnit* unit) {

		if (unit == paladin) {
			return false;
		}
		if (scbw::isAlliedTo(paladin->playerId, unit->playerId)) {
			return false;
		}
		if (unit->status & UnitStatus::Invincible) {
			return false;
		}
		return true;
	}
};
class corrosiveSporeProc : public scbw::UnitFinderCallbackProcInterface
{

private:
	CUnit* victim;
	
public:
	corrosiveSporeProc(CUnit* victim)
		: victim(victim) {}
	void proc(CUnit* unit)
	{
		if (unit == victim) {
			return;
		}
		if (unit->status & UnitStatus::Burrowed) {
			return;
		}
		if (unit->status & UnitStatus::Invincible) {
			return;
		}
//		scbw::printText("Spend");
		auto count = std::min(4, (int)victim->acidSporeCount);
		for (int i = 0; i < count; i++) {
			if (unit->acidSporeCount < 9) {
				scbw::ApplySpores(unit);
			}
//			scbw::printText("SetSpores");
		}
	}
};

class penumbraProc : public scbw::UnitFinderCallbackProcInterface
{

private:
	CUnit* penumbra;

public:
	penumbraProc(CUnit* penumbra)
		: penumbra(penumbra) {}
	void proc(CUnit* unit)
	{
		if (unit == penumbra) {
			return;
		}
		if (unit->status & UnitStatus::Invincible) {
			return;
		}
		if (unit->status & UnitStatus::InAir) {
			return;
		}
		auto size = units_dat::SizeType[unit->id];
		if (size != UnitSize::Small) {
			return;
		}
		if (penumbra->killCount < 255) {
			penumbra->killCount++;
		}
		unit->remove();
		

	}
};
class shamanProc : public scbw::UnitFinderCallbackProcInterface
{

private:
	CUnit* shaman;
	bool organic;
	u32 heal;

public:
	shamanProc(CUnit* shaman, bool organic, u32 heal)
		: shaman(shaman), organic(organic), heal(heal) {}
	void proc(CUnit* unit)
	{
		if (unit->playerId != shaman->playerId) {
			if (!scbw::isAlliedTo(unit->playerId, shaman->playerId)) {
				return;
			}
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Building)
			return;
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Organic) && organic)
			return;
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical) && !organic)
			return;
		if (unit->stasisTimer) {
			return;
		}
		if (unit == shaman) {
			return;
		}

		u32 dist = scbw::getDistanceFast(unit->position.x, unit->position.y,
			shaman->position.x, shaman->position.y);
		//unit->isBeingHealed = true;
		bool reqHeal = false;

		if (unit->getCurrentHpInGame() < unit->getMaxHpInGame()) {
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
			if (organic) {
				(tgt->sprite)->createTopOverlay(overlayImageId, 0, 0, 0);
			}
			else {
				(tgt->sprite)->createTopOverlay(1158, 0, 0, 0);
			}

		}
//		unit->setHp(unit->hitPoints + (6 * 256));
		unit->setHp(unit->hitPoints + (heal * 256));
		if (unit->getCurrentHpInGame() == unit->getMaxHpInGame() && reqHeal && organic) {
			if (!unit->getOverlay(971)) {
				unit->sprite->createTopOverlay(971);
			}
			scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericTimer, ValueId::Adrenaline, 1 + (2 * 24), 0);
			return;
		}
	}
};

class treasuryProc : public scbw::UnitFinderCallbackProcInterface {
	u8 _playerId;

public:
	treasuryProc(u8 playerId) {
		_playerId = playerId;
	}

	void proc(CUnit* unit) {
		if (units_dat::SupplyProvided[unit->id] == 0) return;
		if (unit->playerId != _playerId && !scbw::isAlliedTo(_playerId, unit->playerId)) return;
		if (!(unit->status & UnitStatus::Completed)) return;

		u32 stacks = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::TreasuryStacks, 0);
		if (*elapsedTimeFrames % 36 == 0 && stacks == 0) {
			unit->sprite->createUnderlay(1060, 0, 0, 0);
		}

		scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::AddGenericValue, ValueId::TreasuryStacks, 1, 0);
		int supply = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::SupplyProvidedValue, 0);
		unit->setSupplyProvided(supply + 4);
	}
};

#include "logger.h"
#include "limits/sfx_extender.h"
#include "hooks/weapons/wpnspellhit.h"
#include "globals.h"
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
				hooks::initializeScreenLocations();
				hooks::clearActions();

			}
			hooks::handleScreenHotkeys();
			scbw::refreshScreen(); // refreshes screen, needed for game clock and apm timer to update every frame
			
			//framework example, rework and use later
			/*if (*elapsedTimeFrames == 1) {
				char* name = "samase\\sprites_ext.txt";
				const char* data = NULL;
				u32 len = 0;
				auto data_ptr = &data;
				auto len_ptr = &len;
				u32 val = reinterpret_cast<u32>(name);
				auto result = scbw::get_aise_value(NULL, reinterpret_cast<CUnit*>(name), AiseId::ReadSamaseFile, reinterpret_cast<int>(data_ptr), reinterpret_cast<int>(len_ptr));				
				auto result2 = scbw::get_aise_value(NULL, NULL, AiseId::ReadSamaseFile_Ptr, 0, 0);
			}*/
			
			// Order debugger
/*			if ((*activePortraitUnit) != NULL) {
				char buf[32];
				sprintf(buf, "Current index: %d, order: %d",(*activePortraitUnit)->getIndex(),
					(*activePortraitUnit)->mainOrderId);
				scbw::printText(buf);
			}*/

			// Tileset debugger
/*			char b[32];
			sprintf(b, "Tiles: %d %d %d %d %d", (*tileGroup + 0x2)[1].megatiles[0],
												(*tileGroup + 0x2)[1].megatiles[1],
				(*tileGroup + 0x2)[1].megatiles[2],
				(*tileGroup + 0x2)[1].megatiles[3],
				(*tileGroup + 0x2)[1].megatiles[4]

				);
			scbw::printText(b);*/

			if (*elapsedTimeFrames == 0) {
				Globals.reset();
			}
			Globals.frameHook();
			
			// Reset Treasury Stacks, before unit loop
			for (CUnit* unit = *firstVisibleUnit; unit; unit = unit->link.next) {
				int supply = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::SupplyProvidedValue, 0);
				int stacks = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::TreasuryStacks, 0);
				scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::TreasuryStacks, 0, 0);
				unit->setSupplyProvided(supply - stacks * 2);
			}

			for (int player = 0; player < 8; player++) { // Synchronize upgrades
				int level = 0;

				// Terran Upgrades
				level = scbw::getUpgradeLevel(player, UpgradeId::TerranInfantryWeapons);
				scbw::setUpgradeLevel(player, UpgradeId::TerranVehicleWeapons, level);
				scbw::setUpgradeLevel(player, UpgradeId::TerranShipWeapons, level);
				level = scbw::getUpgradeLevel(player, UpgradeId::TerranInfantryArmor);
				scbw::setUpgradeLevel(player, UpgradeId::TerranVehiclePlating, level);
				scbw::setUpgradeLevel(player, UpgradeId::TerranShipPlating, level);

				// Protoss Upgrades
				level = scbw::getUpgradeLevel(player, UpgradeId::ProtossGroundWeapons);
				scbw::setUpgradeLevel(player, UpgradeId::ProtossAirWeapons, level);
				level = scbw::getUpgradeLevel(player, UpgradeId::ProtossArmor);
				scbw::setUpgradeLevel(player, UpgradeId::ProtossPlating, level);

				// Zerg Upgrades
				level = scbw::getUpgradeLevel(player, UpgradeId::ZergMeleeAttacks);
				scbw::setUpgradeLevel(player, UpgradeId::ZergMissileAttacks, level);
				scbw::setUpgradeLevel(player, UpgradeId::ZergFlyerAttacks, level);
				level = scbw::getUpgradeLevel(player, UpgradeId::ZergCarapace);
				scbw::setUpgradeLevel(player, UpgradeId::ZergFlyerCaparace, level);
			}

			//for (CBullet* bullet = *firstBullet; bullet; bullet = bullet->next) {
			//}

			// Loop through all visible units in the game.
			for (CUnit* unit = *firstVisibleUnit; unit; unit = unit->link.next) {
				CUnitExtended::get(unit)->tick();
				Globals.unitFrameHook(unit);
				//hooks::ShowHealthBar(unit);
				if (units_dat::BaseProperty[unit->id] & UnitProperty::Organic &&
					scbw::get_generic_value(unit, ValueId::LazarusNoHeal)==1) {
					if (unit->sprite && unit->sprite->mainGraphic
						&& unit->sprite->mainGraphic->animation != IscriptAnimation::Unused1) {
						//clear revival no-heal flag
						scbw::set_generic_value(unit, ValueId::LazarusNoHeal, 0);
					}
				}

				// Star Sovereign Intervention Missiles behavior
				if (unit->id == UnitId::ProtossStarSovereign) {
					if (scbw::get_generic_timer(unit, ValueId::PlanetCrackerChannel) == 1) {
						Globals.createPlanetCracker(unit->position.x, unit->position.y, unit->playerId, unit->sprite->elevationLevel);
					}
					if (scbw::get_generic_timer(unit, ValueId::StarcallerInterceptionMissiles)==0) {
						auto r = 5*32;
						auto n = unitRangeCount(unit, CFlags::Enemy | CFlags::FrontalArc, {});
						scbw::UnitFinder unitsInFinder(unit->getX() - r, unit->getY() - r,
							unit->position.x + r, unit->position.y + r);
						bool enemiesDetected = false;
						for (int i = 0; i < unitsInFinder.getUnitCount(); i++) {
							auto picked_unit = unitsInFinder.getUnit(i);
							if (n.match(picked_unit)) {
								enemiesDetected = true;
								break;
							}
						}
						if (enemiesDetected) {
							scbw::set_generic_timer(unit, ValueId::StarcallerInterceptionMissiles, 
												weapons_dat::Cooldown[WeaponId::InterventionMissiles]);
							
							for (int arc = 0; arc < 6; arc++) {
								const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlaySpecialPtr,
									unit->sprite->mainGraphic->id));
								u8 frameDirection = unit->sprite->mainGraphic->direction;
								Point8 offset = loFile->getOffset(frameDirection, 5-arc);
								if (unit->sprite->mainGraphic->flags & CImage_Flags::Mirrored) {
									offset.x = -offset.x;
								}
								auto X = unit->getX() + offset.x;
								auto Y = unit->getY() + offset.y;
								auto target_unit = unit->orderTarget.unit;
								auto target_pos = unit->orderTarget.pt;
								auto offset_direction = -40 + arc * 16;
								auto maxRange = weapons_dat::MaxRange[WeaponId::InterventionMissiles];
								auto polar_x = unit->getX() + scbw::getPolarX(maxRange, scbw::dir_saturate(unit->currentDirection1, offset_direction));
								auto polar_y = unit->getY() + scbw::getPolarY(maxRange, scbw::dir_saturate(unit->currentDirection1, offset_direction));
								unit->orderTarget.unit = NULL;
								unit->orderTarget.pt.x = polar_x;
								unit->orderTarget.pt.y = polar_y;
								scbw::createBullet(WeaponId::InterventionMissiles, unit, X, Y, unit->playerId, 
									scbw::dir_saturate(unit->currentDirection1, offset_direction));
								scbw::playSound(80, unit);
								unit->orderTarget.unit = target_unit;
								unit->orderTarget.pt = target_pos;

								//scbw::spreadFire(WeaponId::InterventionMissiles, unit, X, Y, -40 + arc * 16);
								//-40 -24 -8 8 24 40
							}
						}
					}
				}
				if (unit->id == UnitId::Special_IonCannon) {
					auto turret = unit->getOverlay(1256);
					if (unit->sprite && turret) {
						
						const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlaySpecialPtr,
							1256));
						u8 frameDirection = turret->direction;
						Point8 offset = loFile->getOffset(frameDirection, 0);
						if (turret->flags & CImage_Flags::Mirrored) {
							offset.x = -offset.x;
						}
						auto turretOverlay = unit->getOverlay(1257);
						if (!turretOverlay) {
							CImage* cannonOverlay = unit->sprite->createOverlay(1257, offset.x, offset.y, turret->direction);
						}
						else {
							turretOverlay->direction = turret->direction;
							turretOverlay->horizontalOffset = offset.x;
							turretOverlay->verticalOffset = offset.y;
						}
						
					}
				}
				if (unit->id == UnitId::TerranApostle) {

					if (scbw::get_generic_value(unit, ValueId::CastLazarusAgent) == 1) {
						scbw::set_generic_value(unit, ValueId::CastLazarusAgent, 0);
						//cast lazarus agent
						
						unit->spendUnitEnergy(techdata_dat::EnergyCost[TechId::LazarusAgent]*256);
						auto r = 128;//radius
						auto n = unitRangeCount(NULL, CFlags::NoBuilding | CFlags::Ally | 
							CFlags::OnlyOrganic | CFlags::IncludingSelf, {});
						scbw::UnitFinder unitsInSplash(unit->getX() - r, unit->getY() - r,
							unit->position.x + r, unit->position.y + r);
						for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
							auto picked_unit = unitsInSplash.getUnit(i);
							if (n.match(picked_unit)) {
								//apply lazarus agent
								scbw::set_generic_value(picked_unit, ValueId::LazarusAgent, 1);
								scbw::set_generic_unit(picked_unit, ValueId::LazarusAgentApostleSrc, unit);
							}
						}
					}
				}
				if (unit->id == UnitId::ProtossCorsair) {
					auto web = scbw::get_generic_unit(unit, ValueId::AttachedDisruptionWeb);
					if (scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::DisruptionWeb, 0)) {
						
						if (web == NULL) {
							web = hooks::createDisruptionWeb(unit->playerId, unit->getX(), unit->getY());
							if (web == NULL) {
								//CCMU case
								scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::DisruptionWeb, 0, 0);
							}
							else {
								scbw::sendGPTP_aise_cmd(unit, web, GptpId::SetGenericUnit, ValueId::AttachedDisruptionWeb, 0, 0);
							}
						}
						else {
							//web attached
							scbw::moveUnit(web, unit->getX(), unit->getY());
						}
					}
					else {
						if (web != NULL) {
							web->remove();
						}
					}
					if (scbw::get_generic_value(unit, ValueId::DisruptionWebDamage)) {
						scbw::set_generic_value(unit, ValueId::DisruptionWebDamage, 0);
						//scbw::printFormattedText("Disruption - Energy Decay - Inflict damage %d", techdata_dat::EnergyCost[TechId::DisruptionWeb]);
						unit->inflictDamage(techdata_dat::EnergyCost[TechId::DisruptionWeb]*256, unit, unit->playerId, false, true);
					}
				}

				//
				if (unit->id == UnitId::TerranPenumbra && unit->movementFlags & MovementFlags::Accelerating) {
					auto spores = penumbraProc(unit);
					scbw::UnitFinder unitFinder(unit->getLeft()-16,
						unit->getTop()-16,
						unit->getRight()+16,
						unit->getBottom()+16);
					unitFinder.forEach(spores);
				}

				//Madcap stacks
				if (unit->id == UnitId::TerranMadcap && unit->movementFlags & MovementFlags::Accelerating) {
					scbw::set_generic_value(unit, ValueId::MadcapStacks, 0);
				}

				// Legionnaire - Blade Vortex
				if (unit->id == UnitId::ProtossLegionnaire) {
					//teleport jump
					if (unit->orderTarget.unit != NULL && (unit->mainOrderId==OrderId::Attack1 || unit->mainOrderId==OrderId::AttackUnit)) {
						auto target = unit->orderTarget.unit;
						auto distance = unit->getDistanceToTarget(target);
						
						if (distance > 15 && distance<=96) {
							if (scbw::get_aise_value(unit, target, AiseId::GetLegionnaireMarks, 0, 0) > 0) {
								bool teleport = scbw::teleport(unit, target->sprite->position, 0, true);
								if (!unit->getOverlay(276) && teleport) {
									unit->sprite->createTopOverlay(276);
								}
							}
						}
					}
				}
				// Paladin - Skyfall
				if (unit->id == UnitId::TerranPaladin
					&& scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::SkyfallTimer, 0) == 1 &&
					!unit->isFrozen()) {
					u8 missile = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::SkyfallStacks, 0);
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::AddGenericValue, ValueId::SkyfallStacks, 1, 0);
					if (missile == 20) {
						scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::SkyfallStacks, 0, 0);
					}
					else {
						scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericTimer, ValueId::SkyfallTimer, 2 + 1, 0);
						u32 tgt_x = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::SkyfallX, 0);
						u32 tgt_y = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::SkyfallY, 0);
						//scbw::printFormattedText("Read X Y %d %d [a]", tgt_x, tgt_y);
						auto column = missile / 4;
						auto row = missile % 4;
						auto distance = scbw::getDistanceFast(unit->getX(), unit->getY(), tgt_x, tgt_y);
						auto angle = scbw::bw_angle_to_normal(scbw::getAngle(unit->getX(), unit->getY(), tgt_x, tgt_y));
						Point32 rotation_pt = Point32{ 0,0 };
						rotation_pt.x += 16 + (row - 2) * 32;
						rotation_pt.y -= (column - 2) * 32;
						auto product = scbw::rotate_point(Point32{ 0,0 }, rotation_pt, angle);
						Point16 missile_target;
						missile_target.x = tgt_x + product.x;
						missile_target.y = tgt_y + product.y;
						s32 forwardOffset = weapons_dat::ForwardOffset[10];
						auto source_x = unit->getX() + scbw::getPolarX(forwardOffset, unit->currentDirection1);
						auto source_y = unit->getY() + scbw::getPolarY(forwardOffset, unit->currentDirection1)
							- weapons_dat::VerticalOffset[10];
						unit->orderTarget.unit = NULL;
						unit->orderTarget.pt = missile_target;
						auto range = 64;
						//scbw::printFormattedText("Target? %d %d", tgt_x, tgt_y);
						auto skyfall = skyfallMatch(unit);
						auto targ = scbw::UnitFinder::getNearestTarget(
							missile_target.x - range, missile_target.y - range,
							missile_target.x + range, missile_target.y + range,
							unit, skyfall
						);
						if (targ != NULL) {
							unit->orderTarget.unit = targ;
						}
						scbw::createBullet(10, unit, source_x, source_y, unit->playerId, unit->currentDirection1);

					}
				}

				// Devourer - Critical Mass
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::CorrosiveSporeDelay, 0)==1) {
					int count = unit->acidSporeCount;
					unit->acidSporeCount -= 2;
					int range = 160;
					auto spores = corrosiveSporeProc(unit);
					scbw::UnitFinder unitFinder(unit->getX() - 96,
						unit->getY() - 96,
						unit->getX() + 96,
						unit->getY() + 96);
					unitFinder.forEach(spores);
					unit->acidSporeCount = count;
					unit->removeAcidSpores();
					CThingy* thingy = scbw::createThingy(SpriteId::Corrosive_Acid_Hit, unit->getX(), unit->getY(), 11);
					if (thingy != NULL) {
						thingy->sprite->elevationLevel = unit->sprite->elevationLevel + 1;
						scbw::setThingyVisibilityFlags(thingy);
					}
					unit->inflictDamage(30 * 256, NULL,-1, false, true,false,false);		
				}

				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::EnergyDecay, 0) == 1) {
					u32 overlayImageId;
					if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
						overlayImageId = 1108;
					else
						if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
							overlayImageId = 1109;
						else
							overlayImageId = 1107;
					if (unit->subunit != NULL)
						unit->subunit->sprite->removeOverlay(overlayImageId);
					else
						unit->sprite->removeOverlay(overlayImageId);
				}

				// Lurker - Seismic Spines
				if (unit->id == UnitId::ZergLurker && unit->sprite != NULL && unit->mainOrderId != OrderId::Die 
					&& scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::LurkerETA, 0) == 1) {
					//placeholder for seismic spines
					auto position = unit->position;//new target
					auto spr_position = unit->sprite->position;
					Point16 pos;
					pos.x = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::SpineX, 0);
					pos.y = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::SpineY, 0);
					unit->position = pos;
					unit->sprite->position = pos;
					scbw::artificalBulletFrom(WeaponId::SubterraneanSpines, unit->playerId, unit, NULL, spr_position);
					unit->position = position;
					unit->sprite->position = spr_position;
				}

				// Science Vessel Observance - ability
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::ObservanceTimerDebuff, 0) == 1) {
					auto range = 192;
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
						unit->removeOverlay(1231, 1233);
					}
				}

				// Science Vessel Observance - persistant overlay creation
				if (scbw::get_generic_timer(unit, ValueId::ObservanceOverlayTimer) <= 1) {
					auto range = 192;
					auto victim = observanceVictimMatch(unit);
					auto a = scbw::UnitFinder::getNearestTarget(
						unit->getX() - range, unit->getY() - range,
						unit->getX() + range, unit->getY() + range,
						unit, victim
					);
					if (a != NULL
					&&	unit->id == UnitId::TerranScienceVessel) {
						unit->sprite->createOverlay(1230);
						scbw::set_generic_timer(unit, ValueId::ObservanceOverlayTimer, 18);
					}
				}

				// Star Sovereign Planet Cracker - blast effects
				if (unit->id == UnitId::ProtossStarSovereign) {
					u32 player = unit->playerId;
					u16 X = unit->position.x;
					u16 Y = unit->position.y;
					if (unit->orderSignal & 8) {
						unit->orderSignal &= ~8;
						scbw::createThingy(635, X + 32, Y + 32, player);
						scbw::createThingy(635, X - 32, Y + 32, player);
						scbw::createThingy(635, X + 32, Y - 32, player);
						scbw::createThingy(635, X - 32, Y - 32, player);
					}
					else if (unit->orderSignal & 16) {
						unit->orderSignal &= ~16;
						scbw::createThingy(635, X + 80, Y + 80, player);
						scbw::createThingy(635, X - 80, Y + 80, player);
						scbw::createThingy(635, X + 80, Y - 80, player);
						scbw::createThingy(635, X - 80, Y - 80, player);
						scbw::createThingy(635, X + 144, Y + 80, player);
						scbw::createThingy(635, X - 144, Y + 80, player);
						scbw::createThingy(635, X + 144, Y - 80, player);
						scbw::createThingy(635, X - 144, Y - 80, player);
						scbw::createThingy(635, X + 144, Y - 0, player);
						scbw::createThingy(635, X - 144, Y - 0, player);
						scbw::createThingy(635, X + 0, Y - 144, player);
						scbw::createThingy(635, X - 0, Y + 144, player);
						scbw::createThingy(635, X + 80, Y + 144, player);
						scbw::createThingy(635, X - 80, Y + 144, player);
						scbw::createThingy(635, X + 80, Y - 144, player);
						scbw::createThingy(635, X - 80, Y + 144, player);
						scbw::createThingy(635, X + 80, Y + 0, player);
						scbw::createThingy(635, X - 80, Y + 0, player);
						scbw::createThingy(635, X + 0, Y + 80, player);
						scbw::createThingy(635, X - 0, Y + 80, player);
					}
					else if (unit->orderSignal & 32) {
						unit->orderSignal &= ~32;
						scbw::createThingy(635, X + 144, Y + 144, player);
						scbw::createThingy(635, X - 144, Y + 144, player);
						scbw::createThingy(635, X + 144, Y - 144, player);
						scbw::createThingy(635, X - 144, Y - 144, player);
						scbw::createThingy(635, X + 208, Y + 144, player);
						scbw::createThingy(635, X - 208, Y + 144, player);
						scbw::createThingy(635, X + 208, Y - 144, player);
						scbw::createThingy(635, X - 208, Y - 144, player);
						scbw::createThingy(635, X + 208, Y - 0, player);
						scbw::createThingy(635, X - 208, Y - 0, player);
						scbw::createThingy(635, X + 0, Y - 208, player);
						scbw::createThingy(635, X - 0, Y + 208, player);
						scbw::createThingy(635, X + 144, Y + 208, player);
						scbw::createThingy(635, X - 144, Y + 208, player);
						scbw::createThingy(635, X + 144, Y - 208, player);
						scbw::createThingy(635, X - 144, Y - 208, player);
					}
					else if (unit->orderSignal & 64) {
						unit->orderSignal &= ~64;
					}
				}

				// Dark Archon - Malediction (Maelstrom)
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::MaledictionSlow, 0) == 1) {
					u32 overlayImageId;
					if (units_dat::BaseProperty[unit->id] & UnitProperty::MediumOverlay)
						overlayImageId = ImageId::Malediction_Medium;
					else
						if (units_dat::BaseProperty[unit->id] & UnitProperty::LargeOverlay)
							overlayImageId = ImageId::Malediction_Large;
						else
							overlayImageId = ImageId::Malediction_Small;
					while (unit->sprite->getOverlay(overlayImageId)) {
						unit->sprite->removeOverlay(overlayImageId);
					}
					unit->updateSpeed();
				}

				// Lobotomy Mine
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::LobotomySlow, 0) > 0) {
					unit->updateSpeed();
				}

				// Zergling - Adrenal Glands
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::AdrenalTimer, 0) == 1) {
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::AdrenalStacks, 0, 0);
				}				

				// Vorvaling - Captivating Claws
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::CaptivatingClawsTimer, 0) == 1) {
					captivating_claws::clearCaptivatingClaws(unit);
				}

				// GAME RULE
				// Workers do not provide collision while harvesting, moving to harvest, or moving to return resources
				if (unit->mainOrderId == OrderId::Harvest1
					|| unit->mainOrderId == OrderId::HarvestGas1
					|| unit->mainOrderId == OrderId::ReturnMinerals
					|| unit->mainOrderId == OrderId::ReturnGas)
				{
					if (unit->status & UnitStatus::IsGathering) {
						unit->status |= UnitStatus::NoCollide;
					}
					else {
						unit->status &= ~UnitStatus::NoCollide;
					}
				}

				// TODO: Reavers remain at full stocks

				// Shaman - Administer Medstims ability
				if (unit->id == UnitId::TerranShaman) {
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
							shamanProc Heal = shamanProc(unit,true,6);
							scbw::UnitFinder unitFinder(unit->getX() - 128,
								unit->getY() - 128,
								unit->getX() + 128,
								unit->getY() + 128);
							unitFinder.forEach(Heal);
						}
					}
					if (scbw::get_generic_value(unit, ValueId::NaniteField)) {
						auto timer = scbw::get_generic_timer(unit, ValueId::NaniteFieldTimer);
						unit->_padding_0x133 = 0;
						unit->_padding_0x132 = 0;
						if (timer == 0) {
							scbw::set_generic_timer(unit, ValueId::NaniteFieldTimer, 24);
							unit->sprite->playIscriptAnim(IscriptAnimation::CastSpell);
							shamanProc Heal = shamanProc(unit,false,4);
							scbw::UnitFinder unitFinder(unit->getX() - 128,
								unit->getY() - 128,
								unit->getX() + 128,
								unit->getY() + 128);
							unitFinder.forEach(Heal);
						}
					}
				}

				// Basilisk - Biogenesis
				if (unit->id == UnitId::ZergBasilisk) {
					if (unit->status & UnitStatus::Burrowed) {
						if (!(unit->_padding_0x132 & HydraFlags1::BasiliskBurrowed))
						{
							unit->_padding_0x132 |= HydraFlags1::BasiliskBurrowed;
							spawnCreeper(unit);
						}
					}
				}

				// Centaur - Sunchaser Missiles animation
				if (unit->id == UnitId::TerranCentaur && unit->orderSignal & 0x20) {
					unit->orderSignal &= ~0x20;
					if (unit->orderTarget.unit != NULL) {
						if (unit->getDistanceToTarget(unit->orderTarget.unit) <= 128) {
							unit->playIscriptAnim(IscriptAnimation::Unused1);
						}
					}
				}

				// Phobos - Acheron Missiles animation
				if (unit->id == UnitId::TerranPhobos && unit->orderSignal & 0x20) {
					unit->orderSignal &= ~0x20;
					if (unit->orderTarget.unit != NULL) {
						if (unit->getDistanceToTarget(unit->orderTarget.unit) <= 192) {
							unit->playIscriptAnim(IscriptAnimation::Unused1);
						}
					}
				}

				// Hallucinating Visual Effect
				u32 hallucinatingTimer = scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::HallucinatingTimer, 0);
				if (unit != NULL && unit->sprite != NULL) {
					if (hallucinatingTimer % 24 == 1) {
						CSprite* sprite;
						if (unit->subunit != NULL) {
							sprite = unit->subunit->sprite;
						}
						else {
							sprite = unit->sprite;
						}
						sprite->createOverlay(1035, 0, 0, 0);
					}
				}

				// Southpaw - Knockout Drivers behavior
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::KnockoutDriversTimer, 0) == 1) {
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::KnockoutDriversValue, 0, 0);
					unit->updateSpeed();
				}

				// Nexus behavior: increase psi generated by Pylons
				if (unit->id == UnitId::ProtossPylon
					&& (unit->status & UnitStatus::Completed)) {
					u32 nexusCount = unitCountTable[UnitId::ProtossNexus].player[unit->playerId];
					unit->setSupplyProvided((7 + nexusCount) * 2);
				}
				
				// Treasury behavior: increase supply generated by nearby allied supply providing units/structures
				if (unit->id == UnitId::TerranTreasury
					&& (unit->status & UnitStatus::Completed)) {
					Point16 pos = unit->position;
					int radius = (10 * 32) / 2;
					scbw::UnitFinder(pos.x - radius, pos.y - radius, pos.x + radius, pos.y + radius).forEach(treasuryProc(unit->playerId));
				}

				// Dark Templar - Decloak on attack
				if (unit->id == UnitId::ProtossDarkTemplar) {
					auto timer = scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::DarkTemplarTimer, 0);
					if (timer == 1) {
						for (int i = 0; i < 8; i++) {
							if (!scbw::isAlliedTo(unit->playerId, i)) {
								if (unit->visibilityStatus & (1 << i)) {
									unit->visibilityStatus &= ~(1 << i);
								}
							}
						}
					}
				}

				// Apply Ecclesiast - Astral Blessing stacks and timer
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::EcclesiastTimer, 0) == 1) {
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::EcclesiastStack, 0, 0);
				}

				// Update flingy speed when Adrenal Frenzy timer expires
				if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::NathrokorTimer, 0) == 1) {
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericTimer, ValueId::NathrokorTimer, 0, 0);
					unit->updateSpeed();
				}

				// Radiation Shockwave 5-second timer
				if (scbw::get_generic_timer(unit, ValueId::RadiationShockwaveTimer) == 1) {
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericTimer, ValueId::RadiationShockwaveTimer, 0, 0);
					scbw::sendGPTP_aise_cmd(unit, NULL, GptpId::SetGenericValue, ValueId::RadiationShockwaveCount, 1, 0);
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