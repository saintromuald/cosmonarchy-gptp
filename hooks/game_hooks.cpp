#include "game_hooks.h"
#include <graphics/graphics.h>
#include <SCBW/api.h>
#include <SCBW/ExtendSightLimit.h>
#include "psi_field.h"
#include <cstdio>
#include "hooks/interface/uifeatures/screen_hotkeys.h"
#include "hooks/interface/uifeatures/uifeatures.h"
#include "hooks/interface/hpbars/hpbars.h"
#include "Utils/captivating_claws.h"
#include "hooks/hydraFinder.h"
#include "globals.h"
const int hyperTrigger = 0x006509A0;
const uint32 Func_Sub4A01F0 = 0x004A01F0;
void function_004A01F0g(CUnit* unit) {
	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub4A01F0
		POPAD
	}
}

const uint32 Func_98A10 = 0x00498A10;
void makeSpriteInvisible(CSprite* sprite) {
	__asm {
		PUSHAD
		MOV EAX, sprite
		CALL Func_98A10
		POPAD
	}
}

const uint32 Func_UpdateUnitStrength = 0x0049FA40;
void updateUnitStrengthg(CUnit* unit) {
	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_UpdateUnitStrength
		POPAD
	}
}

const uint32 Func_CreateUnit = 0x004A09D0;
CUnit* CreateUnit(uint32 unitId, int x, int y, uint32 playerId) {
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

const uint32 Func_Sub4148F0 = 0x004148F0;
void function_004148F0_(int x, int y, uint32 unitId) {
	__asm {
		PUSHAD
		PUSH y
		PUSH x
		PUSH unitId
		CALL Func_Sub4148F0
		POPAD
	}
}

const uint32 Func_Sub47D770 = 0x0047D770;
void func_0047D770(CUnit* unit) {
	__asm {
		PUSHAD
		PUSH unit
		CALL Func_Sub47D770
		POPAD
	}
}

const uint32 Func_updateNewUnitVision = 0x0045CE90;
void funcupdateNewUnitVision(CUnit* unit, int x, int y) {
	static uint32 id = unit->id;
	__asm {
		PUSHAD
		MOV ECX, id
		PUSH y
		PUSH x
		CALL Func_updateNewUnitVision
		POPAD
	}
}

const uint32 Func_ReplaceUnitWithType = 0x0049FED0;
void replaceUnitWithType(CUnit* unit, uint16 newUnitId) {
	uint32 newUnitId_ = newUnitId;
	__asm {
		PUSHAD
		PUSH newUnitId_
		MOV EAX, unit
		CALL Func_ReplaceUnitWithType
		POPAD
	}
}

const uint32 Func_497ED0 = 0x00497ED0;
static void makeSpriteVisible(CSprite* sprite) {
	__asm {
		PUSHAD
		MOV EAX, sprite
		CALL Func_497ED0
		POPAD
	}

}

const uint32 Func_498A90 = 0x00498A90;
void UpdateSpriteImagesForCloak(CSprite* sprite) {
	__asm {
		PUSHAD
		MOV EDI, sprite
		CALL Func_498A90
		POPAD
	}

}

const uint32 Func_Irradiate = 0x00454E00;
void spawnCreeper(CUnit* unit) {
	int width = mapTileSize->width * 32;
	int height = mapTileSize->height * 32;
	if (unit->position.y >= 64 && unit->position.x >= 64 &&
		unit->position.y < height - 64 && unit->position.x < width - 64) {
		CUnit* creep_spawned = CreateUnit(UnitId::BasiliskCreeper,
			unit->position.x, unit->position.y,
			unit->playerId);
		if (creep_spawned == nullptr) {
			//scbw::print("Error: Creeper");
		} else {
			function_004A01F0g(creep_spawned);
			updateUnitStrengthg(creep_spawned);

			uint32 tile_x = creep_spawned->sprite->position.x / 32;
			uint32 tile_y = creep_spawned->sprite->position.y / 32;

			function_004148F0_(
				tile_x * 32,
				tile_y * 32,
				UnitId::ZergNydusCanal
			); // apply creep
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
			//scbw::print("Burrow basilisk");
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
void shamanProc(CUnit* shaman, bool organic, uint32 heal, CUnit* target) {
	if (target->playerId != shaman->playerId) {
		if (!scbw::isAlliedTo(target->playerId, shaman->playerId)) {
			return;
		}
	}
	if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
		return;
	if (!(units_dat::BaseProperty[target->id] & UnitProperty::Organic) && organic)
		return;
	if (!(units_dat::BaseProperty[target->id] & UnitProperty::Mechanical) && !organic)
		return;
	if (target->stasisTimer) {
		return;
	}
	if (target == shaman) {
		return;
	}
	uint32 dist = scbw::getDistanceFast(target->position.x, target->position.y,
		shaman->position.x, shaman->position.y);
	
	bool reqHeal = false;
	if (target->getCurrentHpInGame() < target->getMaxHpInGame()) {
		reqHeal = true;
		uint32 overlayImageId;
		CUnit* tgt = nullptr;
		if (target->subunit != nullptr) {
			tgt = target->subunit;
		}
		else {
			tgt = target;
		}
		if (organic) {
			overlayImageId = ImageId::RestorationHit_Small + scbw::getOverlayOffset(target->id);
		} else {
			overlayImageId = ImageId::NaniteField;
		}
		(tgt->sprite)->createTopOverlay(overlayImageId, 0, 0, 0);
	}
	target->heal(heal * 256, shaman);
}

#include "logger.h"
#include "limits/sfx_extender.h"
#include "hooks/weapons/wpnspellhit.h"
#include "globals.h"
class CorpseFinder {
private:
	static bool sortCompareXPos(CThingy* a, CThingy* b) {
		return a->sprite->position.x > b->sprite->position.x;
	}

	static bool sortCompareYPos(CThingy* a, CThingy* b) {
		return a->sprite->position.y > b->sprite->position.y;
	}

	static bool lowerboundCompareXPos(CThingy* thingy, int value) {
		return thingy->sprite->position.x < value;
	}

	static bool lowerboundCompareYPos(CThingy* thingy, int value) {
		return thingy->sprite->position.y < value;
	}

public:
	static std::vector<CThingy*> corpsesX;
	static std::vector<CThingy*> corpsesY;

	static std::vector<CThingy*> findCorpses(int left, int top, int right, int bottom) {
		auto foundCorpsesXStart = std::lower_bound(corpsesX.begin(), corpsesX.end(), left, lowerboundCompareXPos);
		auto foundCorpsesXEnd = std::lower_bound(foundCorpsesXStart, corpsesX.end(), right, lowerboundCompareXPos);
		auto foundCorpsesX = std::vector<CThingy*>(foundCorpsesXStart, foundCorpsesXEnd);
		std::sort(foundCorpsesX.begin(), foundCorpsesX.end());

		auto foundCorpsesYStart = std::lower_bound(corpsesY.begin(), corpsesY.end(), top, lowerboundCompareYPos);
		auto foundCorpsesYEnd = std::lower_bound(foundCorpsesYStart, corpsesY.end(), bottom, lowerboundCompareYPos);
		auto foundCorpsesY = std::vector<CThingy*>(foundCorpsesYStart, foundCorpsesYEnd);
		std::sort(foundCorpsesY.begin(), foundCorpsesY.end());

		std::vector<CThingy*> foundCorpses(foundCorpsesX.size());
		auto it = std::set_intersection(foundCorpsesX.begin(), foundCorpsesX.end(), foundCorpsesY.begin(), foundCorpsesY.end(), foundCorpses.begin());
		foundCorpses.resize(it - foundCorpses.begin());
		return foundCorpses;
	}

	static void findCorpses() {
		corpsesX.clear();
		corpsesY.clear();
		for (CThingy* ls = *first_lone_sprite; ls; ls = ls->next) {
			if (ls->sprite) {
				switch (ls->sprite->spriteId) {
					case SpriteId::Corpse_Apostle:
					case SpriteId::Corpse_Aspirant:
					case SpriteId::Corpse_Bactalisk:
					case SpriteId::Corpse_Broodling:
					case SpriteId::Corpse_Cleric:
					case SpriteId::Corpse_Dracadin:
					case SpriteId::Corpse_Droleth:
					case SpriteId::Corpse_Egg:
					case SpriteId::Corpse_Eidolon:
					case SpriteId::Corpse_Harakan:
					case SpriteId::Corpse_Hydralisk:
					case SpriteId::Corpse_Isthrathaleth:
					case SpriteId::Corpse_Konvilisk:
					case SpriteId::Corpse_Lakizilisk:
					case SpriteId::Corpse_Larva:
					case SpriteId::Corpse_Maverick:
					case SpriteId::Corpse_Scribe:
					case SpriteId::Corpse_Ultrakor:
					case SpriteId::Corpse_Zergling:
					case SpriteId::Corpse_TerranBuildingLarge:
					case SpriteId::Corpse_TerranBuildingSmall:
					case SpriteId::Corpse_ProtossBuildingLarge:
					case SpriteId::Corpse_ProtossBuildingSmall:
					case SpriteId::Corpse_ZergBuildingLarge:
					case SpriteId::Corpse_ZergBuildingSmall:
						corpsesX.push_back(ls);
						corpsesY.push_back(ls);
						break;
				}
			}
		}
		std::sort(corpsesX.begin(), corpsesX.end(), sortCompareXPos);
		std::sort(corpsesY.begin(), corpsesY.end(), sortCompareYPos);
	}
};

std::vector<CThingy*> CorpseFinder::corpsesX;
std::vector<CThingy*> CorpseFinder::corpsesY;

namespace hooks {
	void firstFrame();
	void tickUnit(CUnit* unit);

	/// This hook is called every frame
	bool nextFrame() {

		if (!scbw::isGamePaused()) { // If the game is not paused

			scbw::setInGameLoopState(true); // Needed for scbw::random() to work
			graphics::resetAllGraphics();
			hooks::updatePsiFieldProviders();
			*(int*)hyperTrigger = 0; // Single frame hypertriggers

			//This block is executed once every game
			if (*elapsedTimeFrames == 0) {
				firstFrame();
			}
			hooks::handleScreenHotkeys();
			scbw::refreshScreen(); // Needed for game clock and APM period to update every frame
			scbw::refreshConsole();
			globals.frameHook();

			// Reset Treasury Stacks, before unit loop
			for (CUnit* unit = *firstVisibleUnit; unit; unit = unit->link.next) {
				int supply = scbw::get_generic_value(unit, ValueId::SupplyProvided);
				int stacks = scbw::get_generic_value(unit, ValueId::TreasuryStacks);
				scbw::set_generic_value(unit, ValueId::TreasuryStacks, 0);
				unit->setSupplyProvided(supply - stacks * TreasuryIncrease);
			}

			// Loop through bullets
			for (CBullet* bullet = *firstBullet; bullet; bullet = bullet->next) {
				if (bullet->weaponType == WeaponId::KalkaliskViscousAcid) {
					if (bullet->orderSignal & 0x80) {
						bullet->orderSignal &= ~0x80;
						globals.remove_multihit_bullet(bullet);
					}
				}
			}

			// Find corpses
			CorpseFinder::findCorpses();

			// Loop through all visible units in the game.
			for (CUnit* unit = *firstVisibleUnit; unit; unit = unit->link.next) {
				tickUnit(unit);
			}
			scbw::setInGameLoopState(false);
		}
		return true;
	}

	void firstFrame() {
		//Write your code here
		hooks::initializeScreenLocations();
		hooks::clearActions();
		globals.reset();
		for (int i = 0; i < 8; i++) {
			scbw::setUpgradeLevel(i, UpgradeId::VentralSacs, 1);
		}
	}

	void tickUnit(CUnit* unit) {
		globals.unitFrameHook(unit);
		//hooks::ShowHealthBar(unit);

		// Reviving from Lazarus Agent
		if (scbw::get_generic_value(unit, ValueId::IsReviving) == 1 &&
			unit->sprite->mainGraphic->animation != IscriptAnimation::Unused1) {
			//					unit->status &= ~UnitStatus::Invincible;
			scbw::set_generic_value(unit, ValueId::IsReviving, 0);
		}

		// Mind Tyrant Rescind Control
		if (unit->id == UnitId::ProtossMindTyrant) {
			if (scbw::get_generic_value(unit, ValueId::RescindMindControl) == 1) {
				scbw::sendGPTP_aise_cmd(unit, nullptr, GptpId::FreeMindControlled, 0, 0, 0);
				scbw::set_generic_value(unit, ValueId::RescindMindControl, 0);
			}
		}

		// Zoryusthaleth - Reconstitution
		else if (unit->id == UnitId::ZergZoryusthaleth && scbw::get_generic_timer(unit, ValueId::ReconstitutionCooldown) == 0) {
			auto radius = 128;
			std::vector<CThingy*> corpseList = CorpseFinder::findCorpses(
				unit->position.x - radius,
				unit->position.y - radius,
				unit->position.x + radius,
				unit->position.y + radius
			);
			for (auto corpse : corpseList) {
				if (unit->energy < 5 * 256) {
					break;
				}
				if (corpse->sprite->flags & CSprite_Flags::Flag04) {
					continue;
				}
				unit->spendUnitEnergy(5 * 256);
				scbw::UnitFinder(
					unit->getX() - radius,
					unit->getY() - radius,
					unit->getX() + radius,
					unit->getY() + radius
				).forEach([&](CUnit* target) {
					if (!(scbw::isAlliedTo(target->playerId, unit->playerId))) {
						return;
					}
					if (units_dat::BaseProperty[target->id] & UnitProperty::Building) {
						return;
					}
					if (!(units_dat::BaseProperty[target->id] & UnitProperty::Organic)) {
						return;
					}
					target->heal(0.1 * units_dat::MaxHitPoints[target->id], unit);
					//scbw::createAndInitThingy(SpriteId::ReconstitutionHeal, target->position.x, target->position.y);
				});
				scbw::createAndInitThingy(SpriteId::ReconstitutionConsumption, corpse->sprite->position.x, corpse->sprite->position.y);
				corpse->sprite->flags |= CSprite_Flags::Flag04;
				corpse->sprite->playIscriptAnim(IscriptAnimation::Death);
				scbw::set_generic_timer(unit, ValueId::ReconstitutionCooldown, 1 * 24);
				break;
			}
		}

		// Warp Flash overlay: begin 3 seconds before the graphic finishes warping in
		/*
		else if (unit->getRace() == RaceId::Protoss
			&& units_dat::BaseProperty[unit->id] & UnitProperty::Building
			&& unit->remainingBuildTime == 1) {
			unit->sprite->createTopOverlay(ImageId::WarpFlashFade);
		}
		*/

		// Gimmick to allow the Ultrakor morph cancellation animation to play correctly
		else if (unit->id == UnitId::ZergUltrakor || unit->id == UnitId::ZergEgg) {
			if (scbw::get_generic_timer(unit, ValueId::UltrakorCancelGimmick)) {
				if (unit->sprite->mainGraphic->animation != IscriptAnimation::CastSpell) {
					unit->playIscriptAnim(IscriptAnimation::CastSpell);
				}
			}
		}

		// Sublime Shepherd overlay
		else if (unit->id == UnitId::TerranAzazel) {
			if (scbw::get_generic_rust_value(unit, ValueId::SublimeShepherd) == 1) {
				if (!unit->getOverlay(1267)) {
					unit->sprite->createTopOverlay(1267);
				}
			} else {
				if (unit->getOverlay(1267)) {
					unit->sprite->removeOverlay(1267);
				}
			}
		}

		// Exemplar overlay
		else if (unit->id == UnitId::ProtossExemplar) {
			if (scbw::get_generic_rust_value(unit, ValueId::ExemplarSwitch) == 1) {
				if (!unit->getOverlay(ImageId::KhaydarinEclipseOverlayMedium)) {
					unit->sprite->createTopOverlay(ImageId::KhaydarinEclipseOverlayMedium);
				}
			} else {
				if (unit->getOverlay(ImageId::KhaydarinEclipseOverlayMedium)) {
					unit->sprite->removeOverlay(ImageId::KhaydarinEclipseOverlayMedium);
				}
			}
		}

		// Star Sovereign Intervention Missiles behavior + Grief of All Gods
		else if (unit->id == UnitId::ProtossStarSovereign) {
			if (scbw::get_generic_timer(unit, ValueId::PlanetCrackerChannel) == 1) {
				globals.createGenericPersistImpact(ImpactId::GriefOfAllGods, 360, {unit->position.x, unit->position.y}, 8 * 32, -1, unit->sprite->elevationLevel, unit->playerId);
			}
			if (scbw::get_generic_timer(unit, ValueId::StarcallerInterceptionMissiles) == 0) {
				auto r = 5 * 32;
				auto n = unitRangeCount(unit, CFlags::Enemy | CFlags::FrontalArc, {});
				scbw::UnitFinder unitsInFinder(unit->getX() - r, unit->getY() - r,
					unit->position.x + r, unit->position.y + r);
				bool enemiesDetected = false;
				for (int i = 0; i < unitsInFinder.getUnitCount(); i++) {
					auto picked_unit = unitsInFinder.getUnit(i);
					if (n(picked_unit)) {
						enemiesDetected = true;
						break;
					}
				}
				if (enemiesDetected) {
					scbw::set_generic_timer(unit, ValueId::StarcallerInterceptionMissiles,
						weapons_dat::Cooldown[WeaponId::StarSovereignInterventionMissiles]);

					for (int arc = 0; arc < 6; arc++) {
						const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlaySpecialPtr,
							unit->sprite->mainGraphic->id));
						uint8 frameDirection = unit->sprite->mainGraphic->direction;
						Point8 offset = loFile->getOffset(frameDirection, 5 - arc);
						if (unit->sprite->mainGraphic->flags & CImage_Flags::Mirrored) {
							offset.x = -offset.x;
						}
						auto X = unit->getX() + offset.x;
						auto Y = unit->getY() + offset.y;
						auto target_unit = unit->orderTarget.unit;
						auto target_pos = unit->orderTarget.pt;
						auto offset_direction = -40 + arc * 16;
						auto maxRange = weapons_dat::MaxRange[WeaponId::StarSovereignInterventionMissiles];
						auto polar_x = unit->getX() + scbw::getPolarX(maxRange, scbw::dir_saturate(unit->currentDirection1, offset_direction));
						auto polar_y = unit->getY() + scbw::getPolarY(maxRange, scbw::dir_saturate(unit->currentDirection1, offset_direction));
						unit->orderTarget.unit = nullptr;
						unit->orderTarget.pt.x = polar_x;
						unit->orderTarget.pt.y = polar_y;
						scbw::createBullet(WeaponId::StarSovereignInterventionMissiles, unit, X, Y, unit->playerId,
							scbw::dir_saturate(unit->currentDirection1, offset_direction));
						scbw::playSound(80, unit);
						unit->orderTarget.unit = target_unit;
						unit->orderTarget.pt = target_pos;
					}
				}
			}

			// Star Sovereign - blast effects
			uint32 player = unit->playerId;
			uint16 X = unit->position.x;
			uint16 Y = unit->position.y;
			if (unit->orderSignal & 8) {
				unit->orderSignal &= ~8;
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 32, Y + 32, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 32, Y + 32, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 32, Y - 32, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 32, Y - 32, player);
			} else if (unit->orderSignal & 16) {
				unit->orderSignal &= ~16;
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y + 80, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y + 80, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y - 80, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y - 80, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y + 80, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y + 80, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y - 80, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y - 80, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y - 0, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y - 0, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 0, Y - 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 0, Y + 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y + 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y + 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y - 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y + 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y + 0, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y + 0, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 0, Y + 80, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 0, Y + 80, player);
			} else if (unit->orderSignal & 32) {
				unit->orderSignal &= ~32;
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y + 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y + 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y - 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y - 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 208, Y + 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 208, Y + 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 208, Y - 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 208, Y - 144, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 208, Y - 0, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 208, Y - 0, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 0, Y - 208, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 0, Y + 208, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y + 208, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y + 208, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y - 208, player);
				scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y - 208, player);
			} else if (unit->orderSignal & 64) {
				unit->orderSignal &= ~64;
			}
		}

		// Apostle - Lazarus Agent cast
		else if (unit->id == UnitId::TerranApostle) {
			if (scbw::get_generic_rust_value(unit, ValueId::CastLazarusAgent) == 1) {
				scbw::set_generic_rust_value(unit, ValueId::CastLazarusAgent, 0);
				// Cast Lazarus Agent						
				unit->spendUnitEnergy(techdata_dat::EnergyCost[TechId::LazarusAgent] * 256);
				auto r = 4 * 32;
				auto n = unitRangeCount(unit, CFlags::NoBuilding | CFlags::Ally |
					CFlags::OnlyOrganic | CFlags::IncludingSelf, {});
				scbw::UnitFinder unitsInSplash(unit->getX() - r, unit->getY() - r,
					unit->position.x + r, unit->position.y + r);
				for (int i = 0; i < unitsInSplash.getUnitCount(); i++) {
					auto picked_unit = unitsInSplash.getUnit(i);
					if (n(picked_unit)) {
						// Apply Lazarus Agent
						scbw::set_generic_value(picked_unit, ValueId::LazarusAgent, 1);
						scbw::set_generic_value(picked_unit, ValueId::LazarusAgentApostleSrc, (uint32)unit);
						picked_unit->sprite->createTopOverlay(1265);
					}
				}
			}
		}

		// Disruption Web - Lanifect ability
		else if (unit->id == UnitId::ProtossLanifect) {
			auto web = scbw::get_generic_unit(unit, ValueId::AttachedDisruptionWeb);
			if (scbw::get_generic_rust_value(unit, ValueId::DisruptionWeb)) {
				if (web == nullptr) {
					web = hooks::createDisruptionWeb(unit->playerId, unit->getX(), unit->getY());
					if (web == nullptr) {
						//CCMU case
						scbw::set_generic_rust_value(unit, ValueId::DisruptionWeb, 0);
					} else {
						scbw::set_generic_unit(unit, ValueId::AttachedDisruptionWeb, web);
					}
				} else {
					//web attached
					scbw::moveUnit(web, unit->getX(), unit->getY());
				}
			} else {
				if (web != nullptr) {
					web->remove();
				}
			}
			if (scbw::get_generic_rust_value(unit, ValueId::DisruptionWebDamage)) {
				scbw::set_generic_value(unit, ValueId::DisruptionWebDamage, 0);
				unit->inflictDamage(techdata_dat::EnergyCost[TechId::DisruptionWeb] * 256, unit, unit->playerId, false, true);
			}
		}

		// Madcap Cabeirus Feeders - subtract stacks while moving
		else if (unit->id == UnitId::TerranMadcap) {
			if (unit->movementFlags & MovementFlags::Accelerating || unit->movementFlags & MovementFlags::Moving) {
				if (scbw::get_generic_value(unit, ValueId::MadcapStacks) > 0) {
					scbw::add_generic_value(unit, ValueId::MadcapStacks, -1);
				}
			}
		}

		else if (unit->id == UnitId::ProtossLegionnaire) {
			// Legionnaire - Blade Vortex
			//teleport jump
			if (unit->orderTarget.unit != nullptr && (unit->mainOrderId == OrderId::Attack1 || unit->mainOrderId == OrderId::AttackUnit)) {
				auto target = unit->orderTarget.unit;
				auto distance = unit->getDistanceToTarget(target);

				if (distance > 15 && distance <= 96) {
					if (scbw::get_aise_value(unit, target, AiseId::GetLegionnaireMarks, 0, 0) > 0) {
						bool teleport = scbw::teleport(unit, target->sprite->position, 0, true);
						if (!unit->getOverlay(ImageId::LegionnaireTeleportEffect) && teleport) {
							unit->sprite->createTopOverlay(ImageId::LegionnaireTeleportEffect);
						}
					}
				}
			}
		}

		// Paladin - Skyfall
		else if (unit->id == UnitId::TerranPaladin) {
			if (scbw::get_generic_timer(unit, ValueId::SkyfallTimer) == 1
				&& scbw::get_generic_timer(unit, ValueId::SkyfallTimer) == 1
				&& !unit->isFrozen()) {
				uint8 missile = scbw::get_generic_value(unit, ValueId::SkyfallStacks);
				scbw::add_generic_value(unit, ValueId::SkyfallStacks, 1);
				if (missile == 20) {
					scbw::set_generic_value(unit, ValueId::SkyfallStacks, 0);
				} else {
					scbw::set_generic_timer(unit, ValueId::SkyfallTimer, 2 + 1);
					uint32 tgt_x = scbw::get_generic_value(unit, ValueId::SkyfallX);
					uint32 tgt_y = scbw::get_generic_value(unit, ValueId::SkyfallY);

					auto column = missile / 4;
					auto row = missile % 4;
					auto distance = scbw::getDistanceFast(unit->getX(), unit->getY(), tgt_x, tgt_y);
					auto angle = scbw::bw_angle_to_normal(scbw::getAngle(unit->getX(), unit->getY(), tgt_x, tgt_y));
					Point32 rotation_pt = Point32{0,0};
					rotation_pt.x += 16 + (row - 2) * 32;
					rotation_pt.y -= (column - 2) * 32;
					auto product = scbw::rotate_point(Point32{0,0}, rotation_pt, angle);
					Point16 missile_target;
					missile_target.x = tgt_x + product.x;
					missile_target.y = tgt_y + product.y;

					int32 forwardOffset = weapons_dat::ForwardOffset[10];
					auto source_x = unit->getX() + scbw::getPolarX(forwardOffset, unit->currentDirection1);
					auto source_y = unit->getY() + scbw::getPolarY(forwardOffset, unit->currentDirection1)
						- weapons_dat::VerticalOffset[10];
					unit->orderTarget.unit = nullptr;
					unit->orderTarget.pt = missile_target;
					auto range = 64;

					auto targ = scbw::UnitFinder::getNearestTarget(
						missile_target.x - range,
						missile_target.y - range,
						missile_target.x + range,
						missile_target.y + range,
						unit,
						[&](CUnit* target) {
						if (target == unit) {
							return false;
						}
						if (scbw::isAlliedTo(unit->playerId, target->playerId)) {
							return false;
						}
						if (target->status & UnitStatus::Invincible) {
							return false;
						}
						return true;
					}
					);
					if (targ != nullptr) {
						unit->orderTarget.unit = targ;
					}
					scbw::createBullet(WeaponId::PaladinCrusaderMissiles, unit, source_x, source_y, unit->playerId, unit->currentDirection1);
				}
			}
		}

		else if (unit->id == UnitId::ZergLakizilisk) {
			if (unit->sprite != nullptr && unit->mainOrderId != OrderId::Die
				&& scbw::get_generic_timer(unit, ValueId::LurkerETA) == 1) {
				auto position = unit->position; // new target
				auto spr_position = unit->sprite->position;
				Point16 pos;
				pos.x = scbw::get_generic_value(unit, ValueId::SpineX);
				pos.y = scbw::get_generic_value(unit, ValueId::SpineY);
				unit->position = pos;
				unit->sprite->position = pos;
				scbw::artificalBulletFrom(WeaponId::SubterraneanSpines, unit->playerId, unit, nullptr, spr_position);
				unit->position = position;
				unit->sprite->position = spr_position;
				auto spineCount = 0;
				globals.clear_lakizilisk(unit);
			}
		}

		// Shaman - Medstims ability
		else if (unit->id == UnitId::TerranShaman) {
			if (unit->_padding_0x132 & 0x8) {
				unit->groundWeaponCooldown = 8;
			}
			if (scbw::get_generic_rust_value(unit, ValueId::NaniteField)) {
				auto timer = scbw::get_generic_timer(unit, ValueId::NaniteFieldTimer);
				unit->_padding_0x133 = 0;
				unit->_padding_0x132 = 0;
				if (timer == 0) {
					scbw::set_generic_timer(unit, ValueId::NaniteFieldTimer, 24);
					unit->sprite->playIscriptAnim(IscriptAnimation::CastSpell);
					scbw::UnitFinder(
						unit->getX() - 128,
						unit->getY() - 128,
						unit->getX() + 128,
						unit->getY() + 128
					).forEach([&](CUnit* target) {
						shamanProc(unit, false, ShamanHealMechanical, target);
					});
				}
			} else {
				auto timer = scbw::get_generic_timer(unit, ValueId::NewMedstimTimer);
				if (timer == 0) {
					scbw::set_generic_timer(unit, ValueId::NewMedstimTimer, 24);
					unit->sprite->playIscriptAnim(IscriptAnimation::CastSpell);
					scbw::UnitFinder(
						unit->getX() - 128,
						unit->getY() - 128,
						unit->getX() + 128,
						unit->getY() + 128
					).forEach([&](CUnit* target) {
						shamanProc(unit, true, ShamanHealOrganic, target);
					});
				}
			}
		}

		// TODO: Fix!!!
		// Basilisk - Biogenesis
		/*
		else if (unit->id == UnitId::ZergBactalisk) {
			if (unit->status & UnitStatus::Burrowed) {
				if (!(unit->_padding_0x132 & HydraFlags1::BasiliskBurrowed)) {
					unit->_padding_0x132 |= HydraFlags1::BasiliskBurrowed;
					spawnCreeper(unit);
				}
			}
		}
		*/

		// Repentent Chorus - Barghest behavior
		else if (unit->id == UnitId::ProtossBarghest) {
			scbw::set_generic_value(unit, ValueId::RepententChorus, hydraUnitCount(unit, unit->position, 5 * 32, CFlags::Ally | CFlags::NotSelf, {UnitId::ProtossBarghest}));
		}

		// Centaur - Sunchaser Missiles animation
		else if (unit->id == UnitId::TerranCentaur) {
			if (unit->orderSignal & 0x20) {
				unit->orderSignal &= ~0x20;
				if (unit->orderTarget.unit != nullptr) {
					if (unit->getDistanceToTarget(unit->orderTarget.unit) <= 128) {
						unit->playIscriptAnim(IscriptAnimation::Unused1);
					}
				}
			}
		}

		// Apostle - Lazarus Agent revive
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Organic &&
			scbw::get_generic_value(unit, ValueId::LazarusNoHeal) == 1) {
			if (unit->sprite && unit->sprite->mainGraphic
				&& unit->sprite->mainGraphic->animation != IscriptAnimation::Unused1) {
				//clear revival no-heal flag
				scbw::set_generic_value(unit, ValueId::LazarusNoHeal, 0);
			}
		}

		// Savant - Entropy Gauntlets: remove overlay at end of effect
		if (scbw::get_generic_timer(unit, ValueId::EntropyGauntlets) == 1) {
			uint32 overlayOffset = scbw::getOverlayOffset(unit->id);
			uint32 overlayImageId = ImageId::EntropyGauntletsOverlaySmall - overlayOffset;
			if (unit->subunit != nullptr)
				unit->subunit->sprite->removeOverlay(overlayImageId);
			else
				unit->sprite->removeOverlay(overlayImageId);
		}

		{ // Seraph - Observence
			auto timer = scbw::get_generic_timer(unit, ValueId::ObservanceTimerDebuff);
			if (timer > 1) {
				const auto range = 192;
				auto seraphs = scbw::UnitFinder(
					unit->getX() - range,
					unit->getY() - range,
					unit->getX() + range,
					unit->getY() + range
				).filter([&](CUnit* target) {
					if (unit->playerId == target->playerId
						|| scbw::isAlliedTo(unit->playerId, target->playerId)
						|| target->id != UnitId::TerranSeraph) {
						return false;
					}
					return true;
				}).getAllUnits();

				if (!seraphs.empty()) {
					// Seraph - Observance period
					scbw::set_generic_timer(unit, ValueId::ObservanceTimerDebuff, (5 * 24) + 1);

					// Seraph - Observance ability
					if (scbw::get_generic_timer(unit, ValueId::ObservanceTimerPeriod) == 1) {
						scbw::set_generic_timer(unit, ValueId::ObservanceTimerPeriod, (5 * 24) + 1);
						scbw::add_generic_value(unit, ValueId::ObservanceRend, 1);
					}

					// Seraph - Observance persistant overlay creation
					for (auto seraph : seraphs) {
						if (scbw::get_generic_timer(seraph, ValueId::ObservanceOverlayTimer) == 0) {
							seraph->sprite->createOverlay(ImageId::SeraphRedOverlay);
							scbw::set_generic_timer(seraph, ValueId::ObservanceOverlayTimer, 18);
						}
					}
				}
			} else if (timer == 1) {
				unit->sprite->removeOverlay(ImageId::ObservanceOverlayLarge, ImageId::ObservanceOverlaySmall);
			}
		}

		// Dark Archon - Malediction (Maelstrom)
		if (scbw::get_generic_timer(unit, ValueId::MaledictionSlow) == 1) {
			uint32 overlayImageId = ImageId::Malediction_Small + scbw::getOverlayOffset(unit->id);
			while (unit->sprite->getOverlay(overlayImageId)) {
				unit->sprite->removeOverlay(overlayImageId);
			}
			unit->updateSpeed();
		}

		// Lobotomy Mine
		if (scbw::get_generic_timer(unit, ValueId::LobotomySlow) > 0) {
			unit->updateSpeed();
		}

		// Vithrilisk - Corrosive Touch Clear
		if (scbw::get_generic_timer(unit, ValueId::CorrosiveTouchTimer) == 1) {
			scbw::set_generic_value(unit, ValueId::CorrosiveTouchStacks, 0);
			unit->removeOverlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
		}

		// Zergling - Adrenal Glands
		if (scbw::get_generic_timer(unit, ValueId::AdrenalTimer) == 1) {
			scbw::set_generic_value(unit, ValueId::AdrenalStacks, 0);
		}

		// Vorvaling - Captivating Claws
		if (scbw::get_generic_timer(unit, ValueId::CaptivatingClawsTimer) == 1) {
			scbw::get_generic_unit_vector(unit, ValueId::CaptivatingClawsUnitVector) = {};
		}

		// Game Rule -  Workers do not provide collision while harvesting, moving to harvest, or moving to return resources
		if (unit->mainOrderId == OrderId::Harvest1
			|| unit->mainOrderId == OrderId::HarvestGas1
			|| unit->mainOrderId == OrderId::ReturnMinerals
			|| unit->mainOrderId == OrderId::ReturnGas) {
			if (unit->status & UnitStatus::IsGathering) {
				unit->status |= UnitStatus::NoCollide;
			} else {
				unit->status &= ~UnitStatus::NoCollide;
			}
		}

		// Hallucinating Visual Effect
		uint32 hallucinatingTimer = scbw::get_generic_timer(unit, ValueId::HallucinatingTimer);
		if (unit != nullptr && unit->sprite != nullptr) {
			if (hallucinatingTimer % 24 == 1) {
				CSprite* sprite;
				if (unit->subunit != nullptr) {
					sprite = unit->subunit->sprite;
				} else {
					sprite = unit->sprite;
				}
				sprite->createOverlay(ImageId::HallucinationOverlay, 0, 0, 0);
			}
		}

		// Southpaw - Knockout Drivers behavior
		if (scbw::get_generic_timer(unit, ValueId::KnockoutDriversTimer) == 1) {
			scbw::set_generic_value(unit, ValueId::KnockoutDriversStacks, 0);
			unit->updateSpeed();
		}

		// Pylon - Warp Matrix behavior: increase supply based on number of Nexuses
		if (unit->id == UnitId::ProtossPylon
			&& (unit->status & UnitStatus::Completed)) {
			uint32 nexusCount = unitCountTable[UnitId::ProtossNexus].player[unit->playerId];
			unit->setSupplyProvided((7 + nexusCount) * 2);
		}

		// Treasury behavior: increase supply generated by nearby allied supply providing units/structures
		if (unit->id == UnitId::TerranTreasury
			&& (unit->status & UnitStatus::Completed)
			&& (unit->status & UnitStatus::GroundedBuilding)) {
			Point16 pos = unit->position;
			int radius = (10 * 32) / 2;
			scbw::UnitFinder(
				pos.x - radius,
				pos.y - radius,
				pos.x + radius,
				pos.y + radius
			).forEach([&](CUnit* target) {
				if (units_dat::SupplyProvided[target->id] == 0) {
					return;
				}
				if (target->playerId != unit->playerId && !scbw::isAlliedTo(unit->playerId, target->playerId)) {
					return;
				}
				if (!(target->status & UnitStatus::Completed)) {
					return;
				}

				uint32 stacks = scbw::get_generic_value(target, ValueId::TreasuryStacks);
				if (*elapsedTimeFrames % 36 == 0 && stacks == 0) {
					target->sprite->createUnderlay(ImageId::TreasurySupplyOverlay, 0, 0, 0);
				}
				scbw::add_generic_value(target, ValueId::TreasuryStacks, 1);
				int supply = scbw::get_generic_value(target, ValueId::SupplyProvided);
				target->setSupplyProvided(supply + TreasuryIncrease);
			});
		}

		// Permanently Cloaked units recloak after 2 seconds of being out of combat
		if ((units_dat::BaseProperty[unit->id] & UnitProperty::PermanentCloak || unit->status & UnitStatus::CloakingForFree)
			&& !(units_dat::BaseProperty[unit->id] & UnitProperty::Burrowable)) {
			auto timer = scbw::get_generic_timer(unit, ValueId::PermanentCloakTimer);
			if (timer > 1) {
				makeSpriteVisible(unit->sprite);
				unit->status &= ~UnitStatus::Cloaked;
				for (int i = 0; i < 8; i++) {
					unit->visibilityStatus |= (1 << i);
				}
			} else if (timer == 1) {
				unit->status |= UnitStatus::Cloaked;
				if (unit->playerId != *LOCAL_HUMAN_ID && !scbw::isAlliedTo(unit->playerId, *LOCAL_HUMAN_ID)) {
					makeSpriteInvisible(unit->sprite);
				}
			} else {
				for (int i = 0; i < 8; i++) {
					if (!scbw::isAlliedTo(unit->playerId, i)) {
						unit->visibilityStatus &= ~(1 << i);
					} else {
						unit->visibilityStatus |= (1 << i);
					}
				}
			}
		}

		// Remove all Nemean Reactors stacks after not attacking for 5 seconds
		if (scbw::get_generic_timer(unit, ValueId::NemeanReactorsTimer) == 1) {
			scbw::set_generic_value(unit, ValueId::NemeanReactorsStack, 0);
			// remove overlay later
		}

		// Apply Ecclesiast - Astral Blessing stacks and period
		if (scbw::get_generic_timer(unit, ValueId::EcclesiastTimer) == 1) {
			scbw::set_generic_value(unit, ValueId::EcclesiastStack, 0);
		}

		// Update flingy speed when Adrenal Frenzy period expires
		if (scbw::get_generic_timer(unit, ValueId::NathrokorTimer) == 1) {
			scbw::set_generic_timer(unit, ValueId::NathrokorTimer, 0);
			unit->updateSpeed();
		}

		// TODO: Remove? Don't think this code runs as Phase Link is perpetually active
		if (unit->id == UnitId::ProtossClarion && scbw::get_aise_value(unit, nullptr, AiseId::SendClarionValue, 0, 0) == 0) {
			scbw::sendGPTP_aise_cmd(unit, nullptr, GptpId::SetPhaseLinkStatus, 1, 0, 0);
			unit->updateSpeed();
		}

		// Autovitality - Simulacrum behavior
		if (scbw::get_generic_timer(unit, ValueId::SimulacrumCloneTimer) == 1) {
			scbw::createUnitFromAbility(UnitId::ProtossSimulacrum, 1, unit->playerId, unit->position.x, unit->position.y, 0, unit);
		}

		// Matraleth - Ensnaring Brood ability
		if (unit->id == UnitId::ZergMatraleth
			&& scbw::get_generic_rust_value(unit, ValueId::EnsnaringBroodAbility) == 1) {
			Point16 pos = unit->position;
			int radius = 6 * 32;
			scbw::set_generic_rust_value(unit, ValueId::EnsnaringBroodAbility, 0);
			scbw::UnitFinder(
				pos.x - radius,
				pos.y - radius,
				pos.x + radius,
				pos.y + radius
			).forEach([&](CUnit* target) {
				if (!(scbw::isAlliedTo(target->playerId, unit->playerId))) {
					return;
				}
				if (units_dat::BaseProperty[target->id] & UnitProperty::Building
					|| !(target->status & UnitStatus::Completed)) {
					return;
				}
				// Overlays
				if (target->getOverlay(ImageId::EnsnaringBroodOverlay)) {
					target->removeOverlay(ImageId::EnsnaringBroodOverlay);
				}
				scbw::set_generic_timer(target, ValueId::EnsnaringBroodTimer, 12 * 24);
				target->sprite->createTopOverlay(ImageId::EnsnaringBroodOverlay);
			});
			unit->spendUnitEnergy(techdata_dat::EnergyCost[TechId::EnsnaringBrood] * 256);
		}

		// Remove Ensnaring Brood overlay
		if (scbw::get_generic_timer(unit, ValueId::EnsnaringBroodTimer) == 1) {
			unit->removeOverlay(ImageId::EnsnaringBroodOverlay);
		}

		// Zoryusthaleth - Swarming Omen ability
		if (unit->id == UnitId::ZergZoryusthaleth
			&& !(scbw::get_generic_timer(unit, ValueId::SwarmingOmenTimer))
			&& unit->getCurrentHpInGame() < unit->getMaxHpInGame() / 2) {
			scbw::set_generic_timer(unit, ValueId::SwarmingOmenTimer, 15 * 24);
		}

		// Remove Swarming Omen overlay
		if (scbw::get_generic_timer(unit, ValueId::SwarmingOmenTimer) == 1) {
			//			unit->removeOverlay();
		}

		// TODO: Finish
		// Controlled Demolition - initial cast
		/*
		if (conditional) { // ability cast
			scbw::set_generic_timer(unit, ValueId::ControlledDemolition, 10*24);
			int32 overlayOffset = scbw::getOverlayOffset(unit->id);
		//	unit->sprite->createOverlay(ImageId::ControlledDemolitionOverlay_Small + overlayOffset);
		}
		*/

		// Controlled Demolition - destroy structure
		if (scbw::get_generic_timer(unit, ValueId::ControlledDemolitionTimer) == 1) {
			// Do we need more than this for the sake of calculating score screen counts and etc?
			unit->remove();
		}

		// Emergency Detonation - initial cast
		/*
		if (conditional) { // ability cast
			scbw::set_generic_timer(unit, ValueId::EmergencyDetonation, (uint32)(0.5 * scbw::get_generic_timer(unit, ValueId::ControlledDemolition)));
			int32 overlayOffset = scbw::getOverlayOffset(unit->id);
		//	unit->sprite->createOverlay(ImageId::EmergencyDetonationOverlay_Small + overlayOffset);
		}
		*/

		// Emergency Detonation - destroy structure
		if (scbw::get_generic_timer(unit, ValueId::EmergencyDetonationTimer) == 1) {
			//scbw::createBullet(WeaponId::EmergencyDetonation, unit, unit->position.x, unit->position.y, unit->playerId);
			unit->remove();
		}

		// Demiurge - Gods Made Flesh - initial cast
		/*
		if (conditional) { // corpse linked Demiurge casts the ability
			// unit finder?
		}
		*/

		// Demiurge - Gods Made Flesh - kill unit when the effect ends
		if (scbw::get_generic_timer(unit, ValueId::GodsMadeFlesh) == 1) {
			unit->remove();
		}
	}

	bool gameOn() {
		return true;
	}

	bool gameEnd() {
		return true;
	}

} //hooks