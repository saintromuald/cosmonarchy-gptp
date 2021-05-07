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
#include "target_related.h"

const u32 hyperTrigger = 0x006509A0;
const u32 Func_Irradiate = 0x00454E00;

void spawnCreeper(CUnit* unit)
{
	s32 width = mapTileSize->width * 32;
	s32 height = mapTileSize->height * 32;
	if (unit->position.y >= 64 && unit->position.x >= 64 &&
		unit->position.y < height - 64 && unit->position.x < width - 64)
	{
		CUnit* creep_spawned = helper::createUnit(UnitId::BasiliskCreeper,
			unit->position.x, unit->position.y,
			unit->playerId);
		if (creep_spawned == nullptr)
		{
//			scbw::print("Error: Creeper");
		}
		else
		{
			helper::updateUnitStatsFinishBuilding(creep_spawned);
			helper::updateUnitStrength(creep_spawned);

			u32 tile_x = creep_spawned->sprite->position.x / 32;
			u32 tile_y = creep_spawned->sprite->position.y / 32;

			helper::function_004148F0(
				tile_x * 32,
				tile_y * 32,
				UnitId::ZergNydusCanal
			); // apply creep
			helper::stopCreepDisappearingAtBuilding(creep_spawned);

			creep_spawned->status |= UnitStatus::Invincible;
			creep_spawned->status |= UnitStatus::NoCollide;
			creep_spawned->sprite->elevationLevel = 0;
			creep_spawned->status |= UnitStatus::NoCollide;
			creep_spawned->sprite->images.head->flags |= 0x20;
			creep_spawned->sprite->images.head->flags |= 0x40;
			creep_spawned->sprite->images.tail->flags |= 0x20;
			creep_spawned->sprite->images.tail->flags |= 0x40;
//			unit->connectedUnit = creep_spawned;
//			scbw::print("Burrow basilisk");
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
void shamanProc(CUnit* shaman, u32 heal, CUnit* target)
{
	if (target->playerId != shaman->playerId)
	{
		if (!scbw::isAlliedTo(target->playerId, shaman->playerId))
		{
			return;
		}
	}
	if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
		return;
	if (target->stasisTimer)
	{
		return;
	}
	if (target == shaman)
	{
		return;
	}
	u32 dist = scbw::getDistanceFast(target->position.x, target->position.y,
		shaman->position.x, shaman->position.y);

	bool reqHeal = false;
	if (target->getCurrentHpInGame() < target->getMaxHpInGame())
	{
		reqHeal = true;
		u32 overlayImageId = ImageId::NaniteField;
		CUnit* tgt = nullptr;
		if (target->subunit != nullptr)
		{
			tgt = target->subunit;
		}
		else
		{
			tgt = target;
		}
		(tgt->sprite)->createTopOverlay(overlayImageId, 0, 0, 0);
	}
	target->heal(heal * 256, shaman);
}

#include "logger.h"
#include "limits/sfx_extender.h"
#include "hooks/weapons/wpnspellhit.h"
#include "globals.h"
class CorpseFinder
{
private:
	static bool sortCompareXPos(CThingy* a, CThingy* b)
	{
		return a->sprite->position.x > b->sprite->position.x;
	}

	static bool sortCompareYPos(CThingy* a, CThingy* b)
	{
		return a->sprite->position.y > b->sprite->position.y;
	}

	static bool lowerboundCompareXPos(CThingy* thingy, s32 value)
	{
		return thingy->sprite->position.x < value;
	}

	static bool lowerboundCompareYPos(CThingy* thingy, s32 value)
	{
		return thingy->sprite->position.y < value;
	}

public:
	static std::vector<CThingy*> corpsesX;
	static std::vector<CThingy*> corpsesY;

	static std::vector<CThingy*> findCorpses(s32 left, s32 top, s32 right, s32 bottom)
	{
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

	static void findCorpses()
	{
		corpsesX.clear();
		corpsesY.clear();
		for (CThingy* ls = *first_lone_sprite; ls; ls = ls->next)
		{
			if (ls->sprite)
			{
				switch (ls->sprite->spriteId)
				{
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

namespace hooks
{
	void firstFrame();
	void tickUnit(CUnit* unit);

	/// This hook is called every frame
	bool nextFrame()
	{
		if (!scbw::isGamePaused())
		{ // If the game is not paused

			scbw::setInGameLoopState(true); // Needed for scbw::random() to work
			graphics::resetAllGraphics();
			hooks::updatePsiFieldProviders();
			*(s32*)hyperTrigger = 0; // Single frame hypertriggers

			// This block is executed once every game
			if (*elapsedTimeFrames == 0)
			{
				firstFrame();
			}
			hooks::handleScreenHotkeys();
			scbw::refreshScreen(); // Needed for game clock and APM period to update every frame
			scbw::refreshConsole();
			globals.frameHook();

			// Reset Treasury Stacks, before unit loop
			for (CUnit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
			{
				UnitExt* unitExt = globals.GetUnitExt(unit);
				s32 supply = unitExt->values[ValueSupplyProvided];
				s32 stacks = unitExt->values[ValueTreasuryStacks];
				unitExt->values[ValueTreasuryStacks] = 0;
				unit->setSupplyProvided(supply - stacks * TreasuryIncrease);
			}

			// Loop through bullets
			for (CBullet* bullet = *firstBullet; bullet; bullet = bullet->next)
			{
				if (bullet->weaponType == WeaponId::KalkaliskViscousAcid)
				{
					if (bullet->orderSignal & 0x80)
					{
						bullet->orderSignal &= ~0x80;
						globals.remove_multihit_bullet(bullet);
					}
				}
			}

			// Find corpses
			CorpseFinder::findCorpses();

			// Loop through all visible units in the game.
			for (CUnit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
			{
				tickUnit(unit);
			}
			scbw::setInGameLoopState(false);
		}
		return true;
	}

	void firstFrame()
	{
		//Write your code here
		hooks::initializeScreenLocations();
		hooks::clearActions();
		globals.reset();
		for (s32 i = 0; i < 8; i++)
		{
			scbw::setUpgradeLevel(i, UpgradeId::VentralSacs, 1);
		}
	}

	void tickUnit(CUnit* unit)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);
		if (unit->id == UnitId::ProtossPariah && unit->orderSignal & 0x20)
		{
			unit->orderSignal &= ~0x20;
			globals.remove_unit_from_pariah_table(unit, true);
		}
		//hooks::ShowHealthBar(unit);

		switch (unit->id)
		{
			case UnitId::ZergZoryusthaleth:
			{
				// Zoryusthaleth - Swarming Omen
				if (!unitExt->timers[TimerSwarmingOmen]
					&& unit->getCurrentHpInGame() < unit->getMaxHpInGame() / 2)
				{
					unitExt->timers[TimerSwarmingOmen] = 15 * 24;
				}
				// Zoryusthaleth - Reconstitution
				if (unitExt->timers[TimerReconstitutionCooldown] == 0)
				{
					auto radius = 128;
					std::vector<CThingy*> corpseList = CorpseFinder::findCorpses(
						unit->position.x - radius,
						unit->position.y - radius,
						unit->position.x + radius,
						unit->position.y + radius
					);
					for (auto corpse : corpseList)
					{
						if (unit->energy < 5 * 256)
						{
							break;
						}
						if (corpse->sprite->flags & CSprite_Flags::Flag04)
						{
							continue;
						}
						unit->spendUnitEnergy(5 * 256);
						scbw::UnitFinder(
							unit->getX() - radius,
							unit->getY() - radius,
							unit->getX() + radius,
							unit->getY() + radius
						).forEach([&](CUnit* target)
						{
							if (!(scbw::isAlliedTo(target->playerId, unit->playerId)))
							{
								return;
							}
							target->heal(ZoryusthalethReconstitutionHealing, unit);
							//scbw::createAndInitThingy(SpriteId::ReconstitutionHeal, target->position.x, target->position.y);
						});
						scbw::createAndInitThingy(SpriteId::ReconstitutionConsumption, corpse->sprite->position.x, corpse->sprite->position.y);
						corpse->sprite->flags |= CSprite_Flags::Flag04;
						corpse->sprite->playIscriptAnim(IscriptAnimation::Death);
						unitExt->timers[TimerReconstitutionCooldown] = 1 * 24;
						break;
					}
				}
			} break;
			case UnitId::ZergUltrakor:
			case UnitId::ZergEgg:
			{
				// Gimmick to allow the Ultrakor morph cancellation animation to play correctly
				if (unitExt->timers[TimerUltrakorCancel])
				{
					if (unit->sprite->mainGraphic->animation != IscriptAnimation::CastSpell)
					{
						unit->playIscriptAnim(IscriptAnimation::CastSpell);
					}
				}
			} break;
			case UnitId::TerranAzazel:
			{
				if (scbw::get_generic_rust_value(unit, RustId::SublimeShepherd) == 1)
				{
					scbw::decloakProc(unit);
					if (!unit->getOverlay(1267))
					{
						unit->sprite->createTopOverlay(1267);
					}
				}
				else
				{
					if (unit->getOverlay(1267))
					{
						unit->sprite->removeOverlay(1267);
					}
				}
			} break;
			case UnitId::ProtossStarSovereign:
			{
				// Star Sovereign Intervention Missiles behavior + Grief of All Gods
				if (unitExt->timers[TimerPlanetCrackerChannel] == 1)
				{
					globals.createGenericPersistImpact(ImpactId::GriefOfAllGods, 360, { unit->position.x, unit->position.y }, 8 * 32, -1, unit->sprite->elevationLevel, unit->playerId);
				}
				if (unitExt->timers[TimerStarcallerInterceptionMissiles] == 0)
				{
					auto r = 5 * 32;
					auto n = unitRangeCount(unit, CFlags::Enemy | CFlags::FrontalArc, {});
					scbw::UnitFinder unitsInFinder(unit->getX() - r, unit->getY() - r,
						unit->position.x + r, unit->position.y + r);
					bool enemiesDetected = false;
					for (s32 i = 0; i < unitsInFinder.getUnitCount(); i++)
					{
						auto picked_unit = unitsInFinder.getUnit(i);
						if (n(picked_unit))
						{
							enemiesDetected = true;
							break;
						}
					}
					if (enemiesDetected)
					{
						unitExt->timers[TimerStarcallerInterceptionMissiles] = weapons_dat::Cooldown[WeaponId::StarSovereignInterventionMissiles];
						for (s32 arc = 0; arc < 6; arc++)
						{
							const LO_Header* loFile = reinterpret_cast<LO_Header*>(scbw::getImageData(ImagesDatEntries::OverlaySpecialPtr,
								unit->sprite->mainGraphic->id));
							u8 frameDirection = unit->sprite->mainGraphic->direction;
							Point8 offset = loFile->getOffset(frameDirection, 5 - arc);
							if (unit->sprite->mainGraphic->flags & CImage_Flags::Mirrored)
							{
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
							helper::createBullet(WeaponId::StarSovereignInterventionMissiles, unit, X, Y, unit->playerId,
								scbw::dir_saturate(unit->currentDirection1, offset_direction));
							scbw::playSound(80, unit);
							unit->orderTarget.unit = target_unit;
							unit->orderTarget.pt = target_pos;
						}
					}
				}

				// Star Sovereign - blast effects
				u32 player = unit->playerId;
				u16 X = unit->position.x;
				u16 Y = unit->position.y;
				if (unit->orderSignal & 8)
				{
					unit->orderSignal &= ~8;
					scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 32, Y + 32, player);
					scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 32, Y + 32, player);
					scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 32, Y - 32, player);
					scbw::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 32, Y - 32, player);
				}
				else if (unit->orderSignal & 16)
				{
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
				}
				else if (unit->orderSignal & 32)
				{
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
				}
				else if (unit->orderSignal & 64)
				{
					unit->orderSignal &= ~64;
				}
			} break;
			case UnitId::TerranApostle:
			{
				// Apostle - Lazarus Agent cast
				if (scbw::get_generic_rust_value(unit, RustId::CastLazarusAgent) == 1)
				{
					scbw::decloakProc(unit);
					scbw::set_generic_rust_value(unit, RustId::CastLazarusAgent, 0);
					// Cast Lazarus Agent						
					unit->spendUnitEnergy(techdata_dat::EnergyCost[TechId::LazarusAgent] * 256);
					auto r = 4 * 32;
					auto n = unitRangeCount(unit, CFlags::NoBuilding | CFlags::Ally | CFlags::IncludingSelf | CFlags::NoTrap | CFlags::NoHallucination, {});
					scbw::UnitFinder unitsInSplash(unit->getX() - r, unit->getY() - r,
						unit->position.x + r, unit->position.y + r);
					for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++)
					{
						auto picked_unit = unitsInSplash.getUnit(i);
						if (n(picked_unit))
						{
							// Apply Lazarus Agent
							UnitExt* pickedUnitExt = globals.GetUnitExt(picked_unit);
							pickedUnitExt->values[ValueLazarusAgent] += 1;
							pickedUnitExt->values[ValueLazarusAgentApostleSrc] = (u32)unit; // TODO: port as a unit
							picked_unit->sprite->createTopOverlay(ImageId::LazarusAgentCast);
//							scbw::playSound(SoundId::LazarusAgentCast, unit);
						}
					}
				}
			} break;
			case UnitId::ProtossLanifect:
			{
				// Disruption Field - Lanifect ability
				auto web = unitExt->units[UnitAttachedDisruptionWeb];
				if (scbw::get_generic_rust_value(unit, RustId::DisruptionWeb))
				{
					if (web == nullptr)
					{
						web = hooks::createDisruptionWeb(unit->playerId, unit->getX(), unit->getY());
						if (web == nullptr)
						{
							// CCMU case
							scbw::set_generic_rust_value(unit, RustId::DisruptionWeb, 0);
						}
						else
						{
							unitExt->units[UnitAttachedDisruptionWeb] = web;
						}
					}
					else
					{
						// web attached
						scbw::moveUnit(web, unit->getX(), unit->getY());
						scbw::decloakProc(unit);
					}
				}
				else
				{
					if (web != nullptr)
					{
						web->remove();
					}
				}
				if (scbw::get_generic_rust_value(unit, RustId::DisruptionWebDamage))
				{
					unitExt->values[ValueDisruptionWebDamage] = 0;
					unit->inflictDamage(techdata_dat::EnergyCost[TechId::DisruptionWeb] * 256, unit, unit->playerId, false, true);
				}
			} break;
			case UnitId::TerranMadcap:
			{
				// Madcap Cabeirus Feeders - subtract stacks while moving
				if (unit->movementFlags & MovementFlags::Accelerating || unit->movementFlags & MovementFlags::Moving)
				{
					if (unitExt->values[ValueMadcapStacks] > 0)
					{
						unitExt->values[ValueMadcapStacks]--;
					}
				}
			} break;
			case UnitId::ProtossLegionnaire:
			{
				// Legionnaire - Blade Vortex
				if (unit->orderTarget.unit != nullptr && (unit->mainOrderId == OrderId::Attack1 || unit->mainOrderId == OrderId::AttackUnit))
				{
					auto target = unit->orderTarget.unit;
					auto distance = unit->getDistanceToTarget(target);

					if (distance > 15 && distance <= 96)
					{
						if (scbw::get_aise_value(unit, target, AiseId::GetLegionnaireMarks, 0, 0) > 0)
						{
							bool teleport = scbw::teleport(unit, target->sprite->position, 0, true);
							if (!unit->getOverlay(ImageId::LegionnaireTeleportEffect) && teleport)
							{
								unit->sprite->createTopOverlay(ImageId::LegionnaireTeleportEffect);
							}
						}
					}
				}
			} break;
			case UnitId::TerranPaladin:
			{
				// Paladin - Skyfall
				if (unitExt->timers[TimerSkyfall] == 1
					&& !unit->isFrozen())
				{
					u8 missile = unitExt->values[ValueSkyfallStacks];
					unitExt->values[ValueSkyfallStacks]++;
					if (missile == 20)
					{
						unitExt->values[ValueSkyfallStacks] = 0;
					}
					else
					{
						unitExt->timers[TimerSkyfall] = 2 + 1;
						u32 tgt_x = unitExt->values[ValueSkyfallX];
						u32 tgt_y = unitExt->values[ValueSkyfallY];

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
						unit->orderTarget.unit = nullptr;
						unit->orderTarget.pt = missile_target;
						auto range = 64;

						auto targ = scbw::UnitFinder::getNearestTarget(
							missile_target.x - range,
							missile_target.y - range,
							missile_target.x + range,
							missile_target.y + range,
							unit,
							[&](CUnit* target)
							{
								if (target == unit)
								{
									return false;
								}
								if (scbw::isAlliedTo(unit->playerId, target->playerId))
								{
									return false;
								}
								if (target->status & UnitStatus::Invincible)
								{
									return false;
								}
								return true;
							}
						);
						if (targ != nullptr)
						{
							unit->orderTarget.unit = targ;
						}
						helper::createBullet(WeaponId::PaladinCrusaderMissiles, unit, source_x, source_y, unit->playerId, unit->currentDirection1);
					}
				}
			} break;
			case UnitId::ZergLakizilisk:
			{
				// Lakizilisk - play sound on attack, hardcoded here to facilitate a different sound on return
				if (unit->orderSignal & 0x20) {
					unit->orderSignal &= ~0x20;
					helper::playSoundFromSource(SoundId::Bullet_ZLrkFir1_wav, unit, 1, 0);
				}
				// Lakizilisk - Seismic Spines
				if (unit->sprite != nullptr && unit->mainOrderId != OrderId::Die
					&& unitExt->timers[TimerLurkerETA] == 1)
				{
					auto position = unit->position; // new target
					auto spr_position = unit->sprite->position;
					Point16 pos;
					pos.x = unitExt->values[ValueSpineX];
					pos.y = unitExt->values[ValueSpineY];
					unit->position = pos;
					unit->sprite->position = pos;
					scbw::artificalBulletFrom(WeaponId::SubterraneanSpines, unit->playerId, unit, nullptr, spr_position);
					helper::playSoundFromSource(SoundId::LakiziliskProjectileReturn, unit, 1, 0);
					unit->position = position;
					unit->sprite->position = spr_position;
					auto spineCount = 0;
					globals.clear_lakizilisk(unit);
				}
			} break;
			case UnitId::TerranShaman:
			{
				// Shaman - Nanite Field
				auto timer = unitExt->timers[TimerNaniteField];
				unit->_padding_0x133 = 0;
				if (timer == 0)
				{
					unitExt->timers[TimerNaniteField] = 24;
					unit->sprite->playIscriptAnim(IscriptAnimation::CastSpell);
					scbw::UnitFinder(
						unit->getX() - 128,
						unit->getY() - 128,
						unit->getX() + 128,
						unit->getY() + 128
					).forEach([&](CUnit* target)
						{
							shamanProc(unit, ShamanHeal, target);
						});
				}
			} break;
			case UnitId::ProtossBarghest:
			{
				// Barghest - Repentent Chorus
				unitExt->values[ValueRepententChorus] = hydraUnitCount(unit, unit->position, 5 * 32, CFlags::Ally | CFlags::NotSelf, { UnitId::ProtossBarghest });
			} break;
			case UnitId::TerranCentaur:
			{
				// Centaur - Sunchaser Missiles
				if (unit->orderSignal & 0x20)
				{
					unit->orderSignal &= ~0x20;
					if (unit->orderTarget.unit != nullptr)
					{
						if (unit->getDistanceToTarget(unit->orderTarget.unit) <= 128)
						{
							unit->playIscriptAnim(IscriptAnimation::Unused1);
						}
					}
				}
			} break;
			case UnitId::TerranTreasury:
			{
				// Treasury - Expanded Storage
				if ((unit->status & UnitStatus::Completed)
					&& (unit->status & UnitStatus::GroundedBuilding))
				{
				Point16 pos = unit->position;
				s32 radius = (10 * 32) / 2;
				scbw::UnitFinder(
					pos.x - radius,
					pos.y - radius,
					pos.x + radius,
					pos.y + radius
				).forEach([&](CUnit* target)
					{
						if (units_dat::SupplyProvided[target->id] == 0)
						{
							return;
						}
						if (target->playerId != unit->playerId && !scbw::isAlliedTo(unit->playerId, target->playerId))
						{
							return;
						}
						if (!(target->status & UnitStatus::Completed))
						{
							return;
						}

						u32 stacks = unitExt->values[ValueTreasuryStacks];
						if (*elapsedTimeFrames % 36 == 0 && stacks == 0)
						{
							target->sprite->createUnderlay(ImageId::TreasurySupplyOverlay, 0, 0, 0);
						}
						UnitExt* targetExt = globals.GetUnitExt(target);
						targetExt->values[ValueTreasuryStacks]++;
						s32 supply = targetExt->values[ValueSupplyProvided];
						target->setSupplyProvided(supply + TreasuryIncrease);
					});
				}
			} break;
			case UnitId::ZergMatraleth:
			{
				// Matraleth - Ensnaring Brood
				if (scbw::get_generic_rust_value(unit, RustId::EnsnaringBroodAbility) == 1)
				{
					scbw::decloakProc(unit);
					Point16 pos = unit->position;
					s32 radius = 6 * 32;
					scbw::set_generic_rust_value(unit, RustId::EnsnaringBroodAbility, 0);
					scbw::UnitFinder(
						pos.x - radius,
						pos.y - radius,
						pos.x + radius,
						pos.y + radius
					).forEach([&](CUnit* target)
					{
						if (!(scbw::isAlliedTo(target->playerId, unit->playerId)))
						{
							return;
						}
						if (units_dat::BaseProperty[target->id] & UnitProperty::Building
							|| !(target->status & UnitStatus::Completed))
						{
							return;
						}
						// Overlays
						if (target->getOverlay(ImageId::EnsnaringBroodOverlay))
						{
							target->removeOverlay(ImageId::EnsnaringBroodOverlay);
						}
						UnitExt* targetExt = globals.GetUnitExt(target);
						targetExt->timers[TimerEnsnaringBrood] = 12 * 24;
						target->sprite->createTopOverlay(ImageId::EnsnaringBroodOverlay);
					});
					unit->spendUnitEnergy(techdata_dat::EnergyCost[TechId::EnsnaringBrood] * 256);
				}
			} break;
			case UnitId::ProtossPylon:
			{
				// Pylon - Warp Matrix
				if (unit->status & UnitStatus::Completed)
				{
					u32 nexusCount = unitCountTable[UnitId::ProtossNexus].player[unit->playerId];
					unit->setSupplyProvided((7 + nexusCount) * 2);
				}
			} break;
			case UnitId::ProtossClarion:
			{
				// Clarion Harmony Drive
				unit->updateSpeed();

				// Clarion Phase Link
				if (scbw::get_aise_value(unit, nullptr, AiseId::SendClarionValue, 0, 0) == 0)
				{
					scbw::sendGPTP_aise_cmd(unit, nullptr, GptpId::SetPhaseLinkStatus, 1, 0, 0);
				}
			} break;
			case UnitId::ProtossMindTyrant:
			{
				// Rescind Mind Control
				if (scbw::get_generic_rust_value(unit, RustId::RescindMindControl) == 1)
				{
					scbw::sendGPTP_aise_cmd(unit, nullptr, GptpId::FreeMindControlled, 0, 0, 0);
					scbw::set_generic_rust_value(unit, RustId::RescindMindControl, 0);
				}
			} break;
			case UnitId::ProtossDemiurge:
			{
				// Demiurge - Gods Made Flesh - initial cast
				if (scbw::get_generic_rust_value(unit, RustId::GodsMadeFleshCast))
				{
					scbw::decloakProc(unit);
					scbw::set_generic_rust_value(unit, RustId::GodsMadeFleshCast, 0);
					unitExt->values[ValueGodsMadeFleshActive] = 1;
				}
				// Demiurge - Gods Made Flesh - remove effect from caster
				if (unitExt->values[ValueGodsMadeFleshActive] && unit->energy == 0)
				{
					unitExt->values[ValueGodsMadeFleshActive] = 0;
				}
			} break;
			case UnitId::ZergLarva:
			{
				// Set Larva type
				if (unit->connectedUnit != nullptr
					&& unit->connectedUnit->id == UnitId::ZergOcsalColony
					&& unitExt->values[ValueLarvaType] == 0)
				{
					unitExt->values[ValueLarvaType] = 1;
				}
			} break;
			case UnitId::ZergAlaszileth:
			{
				// Alaszileth - Dermal Accelerant
				Point16 pos = unit->position;
				s32 radius = 7 * 32;
				scbw::UnitFinder(
					pos.x - radius,
					pos.y - radius,
					pos.x + radius,
					pos.y + radius
				).forEach([&](CUnit* foundUnit)
					{
						if (!(scbw::isAlliedTo(foundUnit->playerId, unit->playerId)))
						{
							return;
						}
						if (!(units_dat::BaseProperty[foundUnit->id] & UnitProperty::RegeneratesHP))
						{
							return;
						}
						UnitExt* foundUnitExt = globals.GetUnitExt(foundUnit);
						if (!foundUnitExt->values[ValueAlaszilethRegenBoost])
						{
							foundUnitExt->values[ValueAlaszilethRegenBoost] = 1;
						}
					});
			} break;
			case UnitId::TerranMaverick:
			{
				if (unit->stimTimer == StimpackTimer)
				{
					scbw::decloakProc(unit);
				}
			} break;
			default:
			{
			}
		}

		/*
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Addon)
		{
			if (unit->connectedUnit != nullptr
				&& scbw::isStandardTerranUnit(unit->connectedUnit->buildQueue[unit->connectedUnit->buildQueueSlot]))
			{
				scbw::print("u");
				scbw::playWorkAnimation(unit);
			}
			else
			{
				unit->sprite->playIscriptAnim(IscriptAnimation::WorkingToIdle);
			}
		}
		*/

		// Warp Flash overlay: begin 3 seconds before the graphic finishes warping in
		/*
		else if (unit->getRace() == RaceId::Protoss
			&& units_dat::BaseProperty[unit->id] & UnitProperty::Building
			&& unit->remainingBuildTime == 1) {
			unit->sprite->createTopOverlay(ImageId::WarpFlashFade);
		}
		*/

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

		// Apostle - Lazarus Agent revive
		if (unitExt->values[ValueLazarusNoHeal])
		{
			if (unit->sprite && unit->sprite->mainGraphic
				&& unit->sprite->mainGraphic->animation != IscriptAnimation::Unused1)
			{
				// clear revival no-heal flag
				unitExt->values[ValueLazarusNoHeal] = 0;
			}
		}

		// Reviving from Lazarus Agent
		if (unitExt->values[ValueIsReviving]
			&& unit->sprite->mainGraphic->animation != IscriptAnimation::Unused1)
		{
			unitExt->values[ValueIsReviving] = 0;
		}

		// Savant - Entropy Gauntlets: remove overlay at end of effect
		if (unitExt->timers[TimerEntropyGauntlets] == 1)
		{
			u32 overlayOffset = scbw::getOverlayOffset(unit->id);
			u32 overlayImageId = ImageId::EntropyGauntletsOverlaySmall - overlayOffset;
			if (unit->subunit != nullptr)
			{
				unit->subunit->sprite->removeOverlay(overlayImageId);
			}
			else
			{
				unit->sprite->removeOverlay(overlayImageId);
			}
		}

		{ // Seraph - Observance
			auto timer = unitExt->timers[TimerObservanceDebuff];
			if (timer > 1)
			{
				const auto range = 192;
				auto seraphs = scbw::UnitFinder(
					unit->getX() - range,
					unit->getY() - range,
					unit->getX() + range,
					unit->getY() + range
				).filter([&](CUnit* target)
				{
					if (unit->playerId == target->playerId
						|| scbw::isAlliedTo(unit->playerId, target->playerId)
						|| target->id != UnitId::TerranSeraph)
					{
						return false;
					}
					return true;
				}).getAllUnits();

				if (!seraphs.empty())
				{
					// Seraph - Observance period
					unitExt->timers[TimerObservanceDebuff] = (5 * 24) + 1;

					// Seraph - Observance ability
					if (unitExt->timers[TimerObservancePeriod] == 1)
					{
						unitExt->timers[TimerObservancePeriod] = (5 * 24) + 1;
						unitExt->values[ValueObservanceRend] += 1;
					}

					// Seraph - Observance persistant overlay creation
					for (auto seraph : seraphs)
					{
						if (!seraph)
						{
							continue;
						}
						UnitExt* seraphExt = globals.GetUnitExt(seraph);
						if (seraphExt->timers[TimerObservanceOverlay] == 0)
						{
							seraph->sprite->createOverlay(ImageId::SeraphRedOverlay);
							seraphExt->timers[TimerObservanceOverlay] = 18;
						}
					}
				}
			}
			else if (timer == 1)
			{
				unit->sprite->removeOverlay(ImageId::ObservanceOverlayLarge, ImageId::ObservanceOverlaySmall);
			}
		}

		// Vithrilisk - Corrosive Touch Clear
		if (unitExt->timers[TimerCorrosiveTouch] == 1)
		{
			unitExt->values[ValueCorrosiveTouchStacks] = 0;
			unit->removeOverlay(ImageId::AcidSpores_1_Overlay_Small, ImageId::AcidSpores_6_9_Overlay_Large);
		}

		// Zergling - Adrenal Glands
		if (unitExt->timers[TimerAdrenal] == 1)
		{
			unitExt->values[ValueAdrenalStacks] = 0;
		}

		// Vorvaling - Captivating Claws
		if (unitExt->timers[TimerCaptivatingClaws] == 1)
		{
			unitExt->unitVectors[UnitVectorCaptivatingClaws] = {};
		}

		// Game Rule -  Workers do not provide collision while harvesting, moving to harvest, or moving to return resources
		if (unit->mainOrderId == OrderId::Harvest1
			|| unit->mainOrderId == OrderId::HarvestGas1
			|| unit->mainOrderId == OrderId::ReturnMinerals
			|| unit->mainOrderId == OrderId::ReturnGas)
		{
			if (unit->status & UnitStatus::IsGathering)
			{
				unit->status |= UnitStatus::NoCollide;
			}
			else
			{
				unit->status &= ~UnitStatus::NoCollide;
			}
		}

		// Hallucinating Visual Effect
		u32 hallucinatingTimer = unitExt->timers[TimerHallucinating];
		if (unit != nullptr && unit->sprite != nullptr)
		{
			if (hallucinatingTimer % 24 == 1)
			{
				CSprite* sprite;
				if (unit->subunit != nullptr)
				{
					sprite = unit->subunit->sprite;
				}
				else
				{
					sprite = unit->sprite;
				}
				sprite->createOverlay(ImageId::HallucinationOverlay, 0, 0, 0);
			}
		}

		// Southpaw - Knockout Drivers behavior
		if (unitExt->values[ValueKnockoutDriversStacks]
			&& unitExt->timers[TimerKnockoutDrivers] < 2)
		{
			unitExt->values[ValueKnockoutDriversStacks] = 0;
			unit->updateSpeed();
		}

		// Cloaked units recloak after 2 seconds of being out of combat
		if (unitExt->timers[TimerDecloak] == 1)
		{
			if (scbw::isDecloakable(unit) && units_dat::BaseProperty[unit->id] & UnitProperty::PermanentCloak)
			{
				helper::AnimCloak(unit, SoundId::Terran_PHOENIX_TPhClo00_WAV);
			}
			else
			{
				helper::updateVisibility(unit); // didact cloak needs special handling
			}
			unitExt->timers[TimerDecloak] = 0;
		}

		// Remove all Nemean Reactors stacks after not attacking for 5 seconds
		if (unitExt->timers[TimerNemeanReactors] == 1)
		{
			unitExt->values[ValueNemeanReactorsStack] = 0;
			// remove overlay later
		}

		// Apply Ecclesiast - Astral Blessing stacks and period
		if (unitExt->timers[TimerEcclesiast] == 1)
		{
			unitExt->values[ValueEcclesiastStack] = 0;
		}

		// Update flingy speed when Adrenal Frenzy period expires
		if (unitExt->timers[TimerNathrokor] == 1)
		{
			unitExt->timers[TimerNathrokor] = 0;
			unit->updateSpeed();
		}

		// Autovitality - Simulacrum behavior
		if (unitExt->timers[TimerSimulacrumClone] == 1)
		{
			scbw::createUnitFromAbility(UnitId::ProtossSimulacrum, 1, unit->playerId, unit->position.x, unit->position.y, 0, unit);
		}

		// Remove Ensnaring Brood overlay
		if (unitExt->timers[TimerEnsnaringBrood] == 1)
		{
			unit->removeOverlay(ImageId::EnsnaringBroodOverlay);
		}

		// Remove Swarming Omen overlay
		if (unitExt->timers[TimerSwarmingOmen] == 1)
		{
//			unit->removeOverlay();
		}

		// TODO: Finish
		// Controlled Demolition - initial cast
		/*
		if (conditional)
		{
			unitExt->ControlledDemolition = 10*24;
			s32 overlayOffset = scbw::getOverlayOffset(unit->id);
		//	unit->sprite->createOverlay(ImageId::ControlledDemolitionOverlay_Small + overlayOffset);
		}
		*/

		// Controlled Demolition - destroy structure
		if (unitExt->timers[TimerControlledDemolition] == 1)
		{
			// Do we need more than this for the sake of calculating score screen counts and etc?
			unit->remove();
		}

		// Emergency Detonation - initial cast
		/*
		if (conditional)
		{
			unitExt->EmergencyDetonationTimer = (u32)(0.5 * unitExt->ControlledDemolitionTimer);
			s32 overlayOffset = scbw::getOverlayOffset(unit->id);
		//	unit->sprite->createOverlay(ImageId::EmergencyDetonationOverlay_Small + overlayOffset);
		}
		*/

		// Emergency Detonation - destroy structure
		if (unitExt->timers[TimerEmergencyDetonation] == 1)
		{
			//scbw::createBullet(WeaponId::EmergencyDetonation, unit, unit->position.x, unit->position.y, unit->playerId);
			unit->remove();
		}

		if (unitExt->units[UnitWorshipDemiurge]) {
			UnitExt* demiurgeExt = globals.GetUnitExt(unitExt->units[UnitWorshipDemiurge]);
			// Demiurge - Gods Made Flesh - apply effect to linked units
			if (demiurgeExt->values[ValueGodsMadeFleshActive])
			{
				unitExt->timers[TimerGodsMadeFlesh] = 3;
			}

			// Demiurge - Gods Made Flesh - return units to original owner
			if (unitExt->values[ValueWorshipStack]
				&& demiurgeExt == nullptr)
			{
				unitExt->values[ValueWorshipStack] = 0;
				unitExt->timers[TimerGodsMadeFlesh] = 0;
				unit->giveTo(unitExt->values[ValueWorshipOriginalPlayer]);
			}
		}

		// Demiurge - Gods Made Flesh - kill unit when the effect ends
		if (unitExt->timers[TimerGodsMadeFlesh] == 1
			&& unitExt->units[UnitWorshipDemiurge])
		{
			unit->remove();
		}

		// Siren - Discovered Attacks - clear links on Siren death
		// We probably need to use a unit vector for handling this behavior
		if (unitExt->values[ValueDiscoveredAttacksDebuff]
			&& unitExt->units[UnitDiscoveredAttacksSiren] == nullptr)
		{
			unitExt->values[ValueDiscoveredAttacksDebuff] = 0;
		}
	}

	bool gameOn()
	{
		return true;
	}

	bool gameEnd()
	{
		return true;
	}

} //hooks