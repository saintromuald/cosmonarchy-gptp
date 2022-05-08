#include "game_hooks.h"
#include <graphics/graphics.h>
#include <SCBW/api.h>
#include <SCBW/ExtendSightLimit.h>
#include "psi_field.h"
#include <cstdio>
#include "hooks/interface/uifeatures/screen_hotkeys.h"
#include "hooks/interface/uifeatures/ui_features.h"
#include "hooks/interface/hpbars/hpbars.h"
#include "Utils/captivating_claws.h"
#include "hooks/cosmonarchy_finder.h"
#include "globals.h"
#include "target_related.h"
#include "hooks/orders/ai/ai_orders.h"
#include "hooks/weapons/weapon_cooldown.h"
#include "hooks/weapons/weapon_fire.h"
#include "logger.h"
#include "limits/sfx_extender.h"
#include "hooks/weapons/weapon_spellhit.h"
#include "globals.h"
#include "unit_search.h"
#include "player_color.h"
#include "unit_ai.h"
#include "tactical_ai.h"
#include "scenario.h"
#include "trigger_system.h"
#include "base_layouts.h"
#include "token_parser.h"
#include "hooks/interface/select_larva.h"

const u32 hyperTrigger = 0x006509A0;

#include "SCBW\UnitFinder.h"
void shamanProc(Unit* shaman, u32 heal, Unit* target)
{
	if (target->playerId != shaman->playerId)
	{
		if (!scbw::isAlliedTo(target->playerId, shaman->playerId))
		{
			return;
		}
	}
	if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
	{
		return;
	}
	if (target->stasisTimer)
	{
		return;
	}
	if (target == shaman)
	{
		return;
	}
	u32 dist = scbw::getDistanceFast(target->position, shaman->position);
	if (dist > 128) {
		return;
	}

	bool reqHeal = false;
	if (target->getCurrentHpInGame() < target->getMaxHpInGame())
	{
		reqHeal = true;
		u32 overlayImageId = ImageId::ShamanNaniteField;
		Unit* tgt = nullptr;
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

	u32 new_heal = heal;
	if (dist <= 32) {
		new_heal *= 2.5;
	}
	else if (dist <= 64) {
		new_heal *= 2;
	}
	else if (dist <= 96) {
		new_heal *= 1.5;
	}
	target->heal(new_heal * 256, shaman);
}

struct CorpseFinder
{
	static bool sortCompareXPos(Thingy* a, Thingy* b)
	{
		return a->sprite->position.x > b->sprite->position.x;
	}

	static bool sortCompareYPos(Thingy* a, Thingy* b)
	{
		return a->sprite->position.y > b->sprite->position.y;
	}

	static bool lowerboundCompareXPos(Thingy* thingy, s32 value)
	{
		return thingy->sprite->position.x < value;
	}

	static bool lowerboundCompareYPos(Thingy* thingy, s32 value)
	{
		return thingy->sprite->position.y < value;
	}

	static std::vector<Thingy*> corpsesX;
	static std::vector<Thingy*> corpsesY;

	static std::vector<Thingy*> findCorpses(s32 left, s32 top, s32 right, s32 bottom)
	{
		auto foundCorpsesXStart = std::lower_bound(corpsesX.begin(), corpsesX.end(), left, lowerboundCompareXPos);
		auto foundCorpsesXEnd = std::lower_bound(foundCorpsesXStart, corpsesX.end(), right, lowerboundCompareXPos);
		auto foundCorpsesX = std::vector<Thingy*>(foundCorpsesXStart, foundCorpsesXEnd);
		std::sort(foundCorpsesX.begin(), foundCorpsesX.end());

		auto foundCorpsesYStart = std::lower_bound(corpsesY.begin(), corpsesY.end(), top, lowerboundCompareYPos);
		auto foundCorpsesYEnd = std::lower_bound(foundCorpsesYStart, corpsesY.end(), bottom, lowerboundCompareYPos);
		auto foundCorpsesY = std::vector<Thingy*>(foundCorpsesYStart, foundCorpsesYEnd);
		std::sort(foundCorpsesY.begin(), foundCorpsesY.end());

		std::vector<Thingy*> foundCorpses(foundCorpsesX.size());
		auto it = std::set_intersection(foundCorpsesX.begin(), foundCorpsesX.end(), foundCorpsesY.begin(), foundCorpsesY.end(), foundCorpses.begin());
		foundCorpses.resize(it - foundCorpses.begin());
		return foundCorpses;
	}

	static void findCorpses()
	{
		corpsesX.clear();
		corpsesY.clear();
		for (Thingy* ls = *first_lone_sprite; ls; ls = ls->next)
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
					{
						corpsesX.push_back(ls);
						corpsesY.push_back(ls);
					} break;
				}
			}
		}
		std::sort(corpsesX.begin(), corpsesX.end(), sortCompareXPos);
		std::sort(corpsesY.begin(), corpsesY.end(), sortCompareYPos);
	}
};

std::vector<Thingy*> CorpseFinder::corpsesX;
std::vector<Thingy*> CorpseFinder::corpsesY;

namespace hooks
{
	void tickUnit(Unit* unit);

	static bool game_inited;

	void nextUiFrame() {
		if (!game_inited) {
			game_inited = true;

			/*
			Parser parser = {};
			parser.at = ...;
			while (parser_get_word(&parser)) {
				if (parser_eq(&parser, "-map")) {
					Token token = parser_get_string(&parser);
					helper::LoadScenarioSingle(token.data, 0, "Some Error Message", "Player Name", 6);
				}
			}
			*/
		}

		tick_trigger_system(false);
	}

	/// This hook is called every frame
	bool nextFrame()
	{
		tick_trigger_system(true);
		tick_base_layouts();

		// This block is executed once every game
		if (*elapsedTimeFrames == 0)
		{
#ifdef CM_DEBUG
			helper::playSoundGlobal(SoundId::Misc_Buzz_wav_1);
			scbw::print("\tGPTP was compiled in debug!\n\tThis will make the game run slower.", GameTextColor::Yellow);
#endif

			hooks::initializeScreenLocations();
			hooks::clearActions();
			globals.reset();

			// Default maps to "semi-explored", from DarkenedFantasies
			for (s32 i = 0; i < 4092; i++) {
				if (fog_of_war_mask->unexplored[i] < 5)
					fog_of_war_mask->unexplored[i] = 5;
			}
			scbw::refreshScreen();

			for (s32 i = 0; i < 8; i++)
			{
				// Allow score screen player name overrides, from DarkenedFantasies
				//if (colorTable->playerColorId[i] > 11)
				//{
				//	strcpy_s(playerNameList->playerName[i], sizeof(playerNameList->playerName[i]), network_table_DF->getString(colorTable->playerColorId[i] + 157));
				//}
				// Set AI-required upgrades
				scbw::setUpgradeLevel(i, UpgradeId::VentralSacs, 1);
			}
		}

		if (*elapsedTimeFrames % 2 == 0)
		{
			for (u32 i = 0; i < 16; ++i)
			{
				Ping* ping = PingArray + i;
				if (ping->timer < 8)
				{
					ping->timer++;
				}
			}
		}

		// Map Features
		scbw::setInGameLoopState(true);
		if (*elapsedTimeFrames == 1) {
			load_player_colors();
		}
		if (*elapsedTimeFrames == 0 || *elapsedTimeFrames == 24) {
			load_player_names();
		}
		scbw::setInGameLoopState(false);

		if (*elapsedTimeFrames == 2)
		{
			// Check deaths
			u8 reveal_flag = 0x00;
			bool do_ping[PLAYABLE_PLAYER_COUNT] = {};
			for (u32 player_id = 0; player_id < PLAYABLE_PLAYER_COUNT; ++player_id)
			{
				if (unit_deaths[UnitId::ResourceVespeneGeyser].player[player_id] & VespeneDeaths_Reveal)
				{
					reveal_flag |= 1 << player_id;
				}
				if (unit_deaths[UnitId::ResourceVespeneGeyser].player[player_id] & VespeneDeaths_Ping)
				{
					do_ping[player_id] = true;
				}
			}

			// Explores the entire map, requires a trigger that sets 1 death of Vespene Geyser for players in question
			if (reveal_flag)
			{
				// Explore map
				/*
				for (u32 x = 0; x < mapTileSize->width; x++)
				{
					for (u32 y = 0; y < mapTileSize->height; y++)
					{
						ActiveTile& tile = scbw::getActiveTileAt(x * 32, y * 32);
						tile.exploredFlags &= ~reveal_flag;
					}
				}
				*/

				// Reveal resources
				for (Unit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
				{
					if (unit->playerId == 11 && units_dat::BaseProperty[unit->id] & UnitProperty::Building)
					{
						unit->sprite->visibilityFlags |= reveal_flag;
					}
				}
			}

			for (u32 target = 0; target < PLAYABLE_PLAYER_COUNT; ++target)
			{
				if (do_ping[target])
				{
					u32 color = 0; // red
					if (scbw::isAlliedTo(*LOCAL_NATION_ID, target))
					{
						color = 8; // green
					}

					scbw::minimapPing(startPositions[target].x, startPositions[target].y, color, *LOCAL_NATION_ID);
				}
			}
		}

		optionsList->speed = 6; // Lock game speed at fastest
		if (!scbw::isGamePaused()) // If the game is not paused
		{
			*ai_update_cooldown = 0;
			for (u32 i = 0; i < 8; ++i) {
				raceSupply[RaceId::Terran].provided[i] = 60000;
			}

			UpdateUnitGrid();

			scbw::setInGameLoopState(true); // Needed for scbw::random() to work
			graphics::resetAllGraphics();
			hooks::updatePsiFieldProviders();
			*(s32*)hyperTrigger = 0; // Single frame hypertriggers

			// Scenario Features
			if (*elapsedTimeFrames == 1) {
				init_scenario();
			}
			if (*elapsedTimeFrames >= 1) {
				tick_scenario();
			}

			hooks::handleScreenHotkeys();
			scbw::refreshScreen();

			// Needed for extended tooltips
			static bool lastShift = 0;
			bool newShift = GetAsyncKeyState(VK_SHIFT) & 0x1;
			if (lastShift != newShift)
			{
				scbw::refreshConsole();
			}
			lastShift = newShift;

			globals.frameHook();

			// Loop through bullets
			for (Bullet* bullet = *firstBullet; bullet; bullet = bullet->next)
			{
				if (bullet->weaponId == WeaponId::KalkaliskViscousAcid)
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
			for (Unit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
			{
				tickUnit(unit);
			}
			scbw::setInGameLoopState(false);
		}

		if (*activePortraitUnit)
		{
			// TODO: update to work with multi building select
			Unit* selected = *activePortraitUnit;

			if (selected
				&& selected->id == UnitId::ZergNydusCanal
				&& selected->building.nydusExit
				&& (scbw::isAlliedTo(selected->playerId, *LOCAL_HUMAN_ID) || helper::getDetectingPlayers(selected) & (1 << *LOCAL_HUMAN_ID)))
			{
				Unit* exit = selected->building.nydusExit;
				graphics::drawLine(
					selected->getX(), selected->getY(),
					exit->getX(), exit->getY(),
					graphics::GREEN, graphics::ON_MAP);
			}
		}
		return true;
	}

	static void clear_gatherer_queue(Unit* unit) {
		if (unit) {
			clear_gatherer_queue(unit->worker.harvest_link.next);
			unit->worker.harvest_link.next = 0;
		}
	}

	void tickUnit(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		TacticalAi(unit, unitExt);

		if ((unit->id == UnitId::ProtossPariah || unit->id == UnitId::SharedStriga) && unit->orderSignal & 0x20)
		{
			unit->orderSignal &= ~0x20;
			globals.remove_unit_from_pariah_table(unit, true);
		}
		//hooks::ShowHealthBar(unit);

		switch (unit->id)
		{
			case UnitId::TerranReservoir:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExcisant:
			{
				// Fixes an issue where geyser would freeze, when upgrading the geyser while a harvester was mining
				if (!(unit->status & UnitStatus::Completed)) {
					helper::RemoveHarvesters(unit);
				}
			} break;
			case UnitId::TerranTalos:
			{
				if (unit->mainOrderId == OrderId::TalosSalvage) {
					Unit* target = unit->orderTarget.unit;
					if (target) {
						if (unit->isTargetWithinMinRange(target, unit->getMaxWeaponRange(unit->getGroundWeapon()))) {
							UnitExt* targetExt = globals.get_unit_ext(target);
							targetExt->timers[TimerSalvageStunTimer] = 5 * 24;
							
							targetExt->salvage_time += 1;
							
							targetExt->salvage_minerals += (f32)units_dat::MineralCost[target->id] / units_dat::TimeCost[target->id];
							while (targetExt->salvage_minerals > 1) {
								targetExt->salvage_minerals -= 1;
								resources->minerals[unit->playerId] += 1;
							}
							
							targetExt->salvage_gas += (f32)units_dat::GasCost[target->id] / units_dat::TimeCost[target->id];
							while (targetExt->salvage_gas > 1) {
								targetExt->salvage_gas -= 1;
								resources->gas[unit->playerId] += 1;
							}
							
							if (targetExt->salvage_time >= units_dat::TimeCost[target->id]) {
								resources->minerals[unit->playerId] += ceilf(targetExt->salvage_minerals);
								resources->gas[unit->playerId] += ceilf(targetExt->salvage_gas);
								target->remove();
							}

							if (!(unit->movementFlags & MovementFlags::Accelerating || unit->movementFlags & MovementFlags::Moving)
								&& unitExt->values[ValueTalosAnimationTimer] % 12 == 0) {
								target->sprite->createTopOverlay(ImageId::SalvagePeriodicZap);
							}

							if (unitExt->values[ValueTalosAnimationTimer] == 0
								|| unitExt->values[ValueTalosAnimationTimer] % 128 == 0) {
								Point16 pos = target->sprite->position;
								switch (scbw::random() % 4) {
									case 1:
									{
										pos.x -= units_dat::UnitBounds[target->id].left * 0.75;
										pos.y -= units_dat::UnitBounds[target->id].top * 0.75;
									} break;
									case 2:
									{
										pos.x += units_dat::UnitBounds[target->id].right * 0.75;
										pos.y -= units_dat::UnitBounds[target->id].top * 0.75;
									} break;
									case 3:
									{
										pos.x -= units_dat::UnitBounds[target->id].left * 0.75;
										pos.y += units_dat::UnitBounds[target->id].bottom * 0.75;
									} break;
									case 4:
									{
										pos.x += units_dat::UnitBounds[target->id].right * 0.75;
										pos.y += units_dat::UnitBounds[target->id].bottom * 0.75;
									} break;
								}
								
								helper::setMoveTargetXY(pos.x, pos.y, unit);
							}

							unitExt->values[ValueTalosAnimationTimer] += 1;
						} else {
							unitExt->values[ValueTalosAnimationTimer] = 0;
						}
					}
				}
			} break;
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
					auto radius = 160;
					std::vector<Thingy*> corpseList = CorpseFinder::findCorpses(
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
						if (corpse->sprite->flags & SpriteFlags::Flag04)
						{
							continue;
						}
						unit->spendUnitEnergy(5 * 256);
						scbw::UnitFinder(
							unit->getX() - radius,
							unit->getY() - radius,
							unit->getX() + radius,
							unit->getY() + radius
						).forEach([&](Unit* target)
						{
							if (!(scbw::isAlliedTo(target->playerId, unit->playerId)))
							{
								return;
							}
							target->heal(ZoryusthalethReconstitutionHealing, unit);
							//scbw::createAndInitThingy(SpriteId::ReconstitutionHeal, target->position.x, target->position.y);
						});
						scbw::createAndInitThingy(SpriteId::ReconstitutionConsumption, corpse->sprite->position.x, corpse->sprite->position.y);
						corpse->sprite->flags |= SpriteFlags::Flag04;
						corpse->sprite->playIscriptAnim(IscriptAnimation::Death);
						unitExt->timers[TimerReconstitutionCooldown] = 1 * 24;
						break;
					}
				}
			} break;
			case UnitId::ZergEgg:
			case UnitId::ZergCocoon:
			case UnitId::ZergQuazilisk:
			{
				// Gimmick to allow morph cancellation animations to play correctly
				if (unitExt->timers[TimerQuaziliskCancel])
				{
					if (unit->sprite->mainGraphic->animation != IscriptAnimation::CastSpell)
					{
						unit->playIscriptAnim(IscriptAnimation::CastSpell);
					}
				}
				if ((unit->id == UnitId::ZergEgg && !scbw::isLarvalEgg(unit)) || unit->id == UnitId::ZergCocoon) {
					if (scbw::getActiveTileAt(unit->position.x, unit->position.y).hasCreep
						&& !unit->getOverlay(ImageId::CreepMorphOverlay)) {
						unit->sprite->createUnderlay(ImageId::CreepMorphOverlay, 0, 2);
					}
				}
			} break;
			case UnitId::ProtossStarSovereign:
			{
				// Star Sovereign - Intervention Missiles
				if (unitExt->timers[TimerStarSovereignInterventionMissiles] == 0)
				{
					auto r = 5 * 32;
					auto n = cosmonarchyFinderMatch(unit, Flags::Enemy | Flags::FrontalArc, {});
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
						unitExt->timers[TimerStarSovereignInterventionMissiles] = weapons_dat::Cooldown[WeaponId::StarSovereignInterventionMissiles];
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
							helper::playSoundFromSource(80, unit, 1, 0);
							unit->orderTarget.unit = target_unit;
							unit->orderTarget.pt = target_pos;
						}
					}
				}

				// Star Sovereign - Grief of All Gods
				if (unitExt->timers[TimerPlanetCrackerChannel] == 1)
				{
					globals.createEffect(EffectId::StarSovereignTheGriefOfAllGods, 360, {unit->position.x, unit->position.y}, 8 * 32, -1, unit->sprite->elevationLevel, unit->playerId, unit, unit);
				}

				// blast effects
				u32 player = unit->playerId;
				u16 X = unit->position.x;
				u16 Y = unit->position.y;
				if (unit->orderSignal & 8)
				{
					unit->orderSignal &= ~8;
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 32, Y + 32, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 32, Y + 32, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 32, Y - 32, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 32, Y - 32, player);
				}
				else if (unit->orderSignal & 16)
				{
					unit->orderSignal &= ~16;
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y + 80, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y + 80, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y - 80, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y - 80, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y + 80, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y + 80, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y - 80, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y - 80, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y - 0, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y - 0, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 0, Y - 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 0, Y + 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y + 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y + 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y - 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y + 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 80, Y + 0, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 80, Y + 0, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 0, Y + 80, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 0, Y + 80, player);
				}
				else if (unit->orderSignal & 32)
				{
					unit->orderSignal &= ~32;
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y + 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y + 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y - 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y - 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 208, Y + 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 208, Y + 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 208, Y - 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 208, Y - 144, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 208, Y - 0, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 208, Y - 0, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 0, Y - 208, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 0, Y + 208, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y + 208, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y + 208, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X + 144, Y - 208, player);
					helper::createThingy(SpriteId::PlanetCrackerBlastLoop, X - 144, Y - 208, player);
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
					unit->spendUnitEnergy(techdata_dat::EnergyCost[TechId::ApostleLazarusAgent] * 256);
					auto r = 4 * 32;
					auto n = cosmonarchyFinderMatch(unit, Flags::NoBuilding | Flags::Ally | Flags::IncludingSelf | Flags::NoTrap | Flags::NoHallucination | Flags::NoEgg | Flags::NoLarva | Flags::NoHangar, {});
					scbw::UnitFinder unitsInSplash(unit->getX() - r, unit->getY() - r,
						unit->position.x + r, unit->position.y + r);
					for (s32 i = 0; i < unitsInSplash.getUnitCount(); i++)
					{
						auto picked_unit = unitsInSplash.getUnit(i);
						if (n(picked_unit))
						{
							// Apply Lazarus Agent
							UnitExt* pickedUnitExt = globals.get_unit_ext(picked_unit);
							pickedUnitExt->values[ValueLazarusAgent] += 1;
							pickedUnitExt->values[ValueLazarusAgentApostleSrc] = (u32)unit; // TODO: port as a unit
							picked_unit->sprite->createTopOverlay(ImageId::LazarusAgentCast);
//							helper::playSoundFromSource(SoundId::LazarusAgentCast, unit);
						}
					}
				}
			} break;
			case UnitId::ProtossLanifect:
			{
				// Disruption Field - Lanifect ability
				if (scbw::get_generic_rust_value(unit, RustId::ToggleDisruptionField))
				{
					if (!unitExt->values[ValueDisruptionFieldOn])
					{
						unitExt->values[ValueDisruptionFieldOn] = 1;
						globals.createEffect(EffectId::DisruptionWeb, -1, {unit->position.x, unit->position.y}, 0, SpriteId::Disruption_Web, 0, unit->playerId, unit, unit);
					}
					else if (unit->isFrozen())
					{
						scbw::set_generic_rust_value(unit, RustId::ToggleDisruptionField, 0);
					}
					scbw::decloakProc(unit);
				}
				else
				{
					unitExt->values[ValueDisruptionFieldOn] = 0;
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
			case UnitId::TerranHeracles:
			{
				// Heracles Nemean Reactors - remove stacks when out of combat
				if (unitExt->values[ValueNemeanReactorsStack] > 0
					&& unitExt->timers[TimerNemeanReactors] <= 1)
				{
					unitExt->values[ValueNemeanReactorsStack] = 0;
				}
			} break;
			case UnitId::TerranCuirass:
			{
				if (unitExt->values[ValueIsSelfReviving])
				{
					if (unit->hitPoints + CuirassSelfHeal >= unit->getMaxHpRaw())
					{
						unitExt->values[ValueIsSelfReviving] = 0;
						unit->hitPoints = unit->getMaxHpRaw();
						scbw::restoreFunc(unit);
						unit->orderComputer(OrderId::Stop);
						unit->status &= ~(UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack);
						helper::playSoundFromSource(SoundId::Terran_GOLIATH_TGoWht03_WAV, unit, 1, 0);
					}
					else
					{
						unit->heal(CuirassSelfHeal);
						if (!unit->sprite->getOverlay(ImageId::StonewallProtocolOverlay))
						{
							unit->sprite->createTopOverlay(ImageId::StonewallProtocolOverlay, -3, -8);
						}
					}
				}
			}
			case UnitId::SharedStriga:
			case UnitId::ProtossLegionnaire:
			{
				// Striga - Transient Travels
				// Legionnaire - Blade Vortex
				if (unit->orderTarget.unit != nullptr
					&& (unit->mainOrderId == OrderId::Attack1 || unit->mainOrderId == OrderId::AttackUnit))
				{
					Unit* target = unit->orderTarget.unit;
					u32 distance = unit->getDistanceToTarget(target);
					bool doTeleport = false;
					u32 imageId;
					Point16 position;

					if (unit->id == UnitId::ProtossLegionnaire)
					{
						if (distance > 15 && distance <= 3 * 32 && scbw::get_aise_value(unit, target, AiseId::GetLegionnaireMarks, 0, 0) > 0)
						{
							doTeleport = true;
							imageId = ImageId::LegionnaireTeleportEffect;
							position = target->sprite->position;
						}
					}
					else if (unit->id == UnitId::SharedStriga)
					{
						if (distance > 3.5 * 32 && distance <= 5 * 32 && scbw::isUnitIsolated(target))
						{
							doTeleport = true;
							imageId = ImageId::LegionnaireTeleportEffect;
							Vector32 diff = scbw::getVectorBetweenPoints(target->sprite->position, unit->sprite->position);
							Vector32f displacement = scbw::getNormalizedVector(diff);
							displacement.x *= 3 * 32;
							displacement.y *= 3 * 32;
							position = target->position;
							position.x += displacement.x;
							position.y += displacement.y;
						}
					}

					if (doTeleport)
					{
						position = scbw::clampPoint(position);
						bool teleport = scbw::teleport(unit, position, 0, true);
						if (!unit->getOverlay(imageId) && teleport)
						{
							unit->sprite->createTopOverlay(imageId);
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
						Vector32 rotation_pt = Vector32{ 0,0 };
						rotation_pt.x += 16 + (row - 2) * 32;
						rotation_pt.y -= (column - 2) * 32;
						auto product = scbw::rotate_point(Vector32{ 0,0 }, rotation_pt, angle);
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
							[&](Unit* target)
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
				if (unit->isFrozen() || unitExt->values[ValueReviveStarted])
				{
					break;
				}

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
					).forEach([&](Unit* target)
						{
							shamanProc(unit, ShamanHeal, target);
						});
				}
			} break;
			case UnitId::ProtossBarghest:
			{
				// Barghest - Repentent Chorus
				unitExt->values[ValueRepententChorus] = cosmonarchyUnitCount(unit, unit->position, 5 * 32, Flags::Ally | Flags::NotSelf, { UnitId::ProtossBarghest });
			} break;
			case UnitId::TerranCentaur:
			{
				// Centaur - Sunchaser Missiles
				if (unit->orderSignal & 0x20)
				{
					unit->orderSignal &= ~0x20;
					if (unit->orderTarget.unit != nullptr)
					{
						if (unit->getDistanceToTarget(unit->orderTarget.unit) <= CentaurMissileDistance)
						{
							unit->playIscriptAnim(IscriptAnimation::Unused2);
						}
					}
				}
			} break;
			case UnitId::TerranOlympian:
			{
				if (unit->orderSignal & 0x20)
				{
					unit->orderSignal &= ~0x20;
					if (unit->orderTarget.unit != nullptr && !(units_dat::BaseProperty[unit->orderTarget.unit->id] & UnitProperty::Flyer))
					{
						if (unit->getDistanceToTarget(unit->orderTarget.unit) <= OlympianMeleeDistance)
						{
							unit->playIscriptAnim(IscriptAnimation::SpecialState1);
						}
					}
				}
			}
			case UnitId::ZergMatraleth:
			{
				// Matraleth - Ensnaring Brood
				if (scbw::get_generic_rust_value(unit, RustId::CastEnsnaringBrood) == 1)
				{
					scbw::decloakProc(unit);
					Point16 pos = unit->position;
					s32 radius = 6 * 32;
					scbw::set_generic_rust_value(unit, RustId::CastEnsnaringBrood, 0);
					scbw::UnitFinder(
						pos.x - radius,
						pos.y - radius,
						pos.x + radius,
						pos.y + radius
					).forEach([&](Unit* target)
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
						UnitExt* targetExt = globals.get_unit_ext(target);
						targetExt->timers[TimerEnsnaringBrood] = 12 * 24;
						target->sprite->createTopOverlay(ImageId::EnsnaringBroodOverlay);
					});
					unit->spendUnitEnergy(techdata_dat::EnergyCost[TechId::MatralethEnsnaringBrood] * 256);
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
				if (scbw::get_generic_rust_value(unit, RustId::CancelMindControl) == 1)
				{
					scbw::sendGPTP_aise_cmd(unit, nullptr, GptpId::FreeMindControlled, 0, 0, 0);
					scbw::set_generic_rust_value(unit, RustId::CancelMindControl, 0);
				}
			} break;
			case UnitId::ProtossDemiurge:
			{
				// Demiurge - Gods Made Flesh - initial cast
				if (scbw::get_generic_rust_value(unit, RustId::CastGodsMadeFlesh) && !unitExt->values[ValueGodsMadeFleshActive])
				{
					scbw::decloakProc(unit);
					scbw::set_generic_rust_value(unit, RustId::CastGodsMadeFlesh, 0);
					unitExt->values[ValueGodsMadeFleshActive] = 1;
					helper::playSoundFromSource(scbw::randBetween(SoundId::DemiurgeGodsMadeFleshCast01, SoundId::DemiurgeGodsMadeFleshCast02), unit, 1, 0);
				}
				// Demiurge - Gods Made Flesh - remove effect from caster
				if (unitExt->values[ValueGodsMadeFleshActive] && unit->energy == 0)
				{
					unitExt->values[ValueGodsMadeFleshActive] = 0;
				}
			} break;
			case UnitId::TerranMaverick:
			{
				// Decloak on Stimpack activation
				if (unit->stimTimer == StimpackTimer)
				{
					scbw::decloakProc(unit);
				}
			} break;
			case UnitId::TerranWyvern:
			{
				if (scbw::get_generic_rust_value(unit, RustId::CancelLandGrab))
				{
					scbw::set_generic_rust_value(unit, RustId::CancelLandGrab, 0);
					unitExt->values[ValueWyvernLandGrabOn] = 0;
				}
			} break;
			case UnitId::SharedTarasque:
			{
				if (unitExt->timers[TimerTarasqueSecondaryWeaponCooldown] == 0)
				{
					if (fireMultiple(unit, unitExt, WeaponId::TarasqueAcceleratedPlasmid, 5, Flags::Enemy, true))
					{
						unitExt->timers[TimerTarasqueSecondaryWeaponCooldown] = weapons_dat::Cooldown[WeaponId::TarasqueAcceleratedPlasmid];
					}
					else
					{
						unitExt->timers[TimerTarasqueSecondaryWeaponCooldown] = 3; // performance
					}
				}
			} break;
			case UnitId::SharedAnticthon:
			{
				// Failure Mode Cast
				if (scbw::get_generic_rust_value(unit, RustId::CastFailureMode) == 1)
				{
					scbw::set_generic_rust_value(unit, RustId::CastFailureMode, 0);
					if (!unitExt->timers[TimerAnticthonFailureMode])
					{
						unitExt->timers[TimerAnticthonFailureMode] = 30 * 24 + 1;
					}
				}

				// Failure Mode End
				if (unitExt->timers[TimerAnticthonFailureMode] == 1)
				{
					unit->heal(5 * 256 * unitExt->values[ValueAnticthonStacks], unit);
					unitExt->values[ValueAnticthonStacks] = 0;
				}

				// Revive Status
				u32 flags = Flags::NotSelf | Flags::Ally | Flags::NoBuilding | Flags::NoTrap | Flags::NoHallucination | Flags::NoEgg | Flags::NoLarva | Flags::NoHangar;
				u32 radius = 4 * 32;
				if (unitExt->timers[TimerAnticthonFailureMode]) 
				{
					radius = 8 * 32;
					flags &= ~Flags::Ally;
				}
				if (!unitExt->values[ValueReviveStarted])
				{
					auto match = cosmonarchyFinderMatch(unit, flags, {});
					scbw::UnitFinder(
						unit->getX() - radius,
						unit->getY() - radius,
						unit->getX() + radius,
						unit->getY() + radius
					).forEachInMatch(match, [&](Unit* target)
						{
							UnitExt* targetExt = globals.get_unit_ext(target);
							targetExt->timers[TimerAnticthonRevive] = 2;
						});
				}
			} break;
			case UnitId::ProtossAnthelion:
			{
				u32 attackRadius = 10 * 32;
				auto match = cosmonarchyFinderMatch(unit, Flags::Enemy, {});
				bool attacked = false;
				scbw::UnitFinder(
					unit->getX() - attackRadius,
					unit->getY() - attackRadius,
					unit->getX() + attackRadius,
					unit->getY() + attackRadius
				).forEachInMatch(match, [&](Unit* target)
				{
					u32 dist = scbw::getDistanceFast(unit->position, target->position);
					if (dist <= weapons_dat::OuterSplashRadius[WeaponId::AnthelionEventHorizon])
					{
						UnitExt* targetExt = globals.get_unit_ext(target);
						if (!(units_dat::BaseProperty[target->id] & UnitProperty::Building)
							&& targetExt->timers[TimerAnthelionGravityWell])
						{
							targetExt->timers[TimerAnthelionGravityWell] = 2;
							targetExt->units[UnitAnthelionGravityWellSource] = unit;
						}

						if (unitExt->timers[TimerAnthelionAttackCooldown] == 0)
						{
							attacked = true;

							u32 divider = 1;
							if (dist > weapons_dat::MediumSplashRadius[WeaponId::AnthelionEventHorizon])
							{
								divider = 4;
							}
							else if (dist > weapons_dat::InnerSplashRadius[WeaponId::AnthelionEventHorizon])
							{
								divider = 2;
							}

							target->damageWith(WeaponId::AnthelionEventHorizon, -1, unit, unit->playerId, 0, divider);
							helper::createThingy(SpriteId::NeutronFlareOrange_Damageless, target->sprite->position.x, target->sprite->position.y, unit->playerId);
						}
					}
				});
				if (attacked)
				{
					unitExt->timers[TimerAnthelionAttackCooldown] = hooks::getModifiedWeaponCooldownHook(unit, WeaponId::AnthelionEventHorizon);
					unit->sprite->createUnderlay(ImageId::OpticalFlareHit_Large);
					helper::playSoundFromSource(scbw::randBetween(SoundId::AnthelionWeaponFire01, SoundId::AnthelionWeaponFire04), unit, 1, 0);
				}

				if (unit->mainOrderId == OrderId::Follow || unit->mainOrderId == OrderId::AttackUnit)
				{
					unit->orderTo(OrderId::Move, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
				}

				if (unit->mainOrderId == OrderId::Move)
				{
					if (unitExt->anthelionLastMoveTarget != unit->moveTarget.pt)
					{
						if (unitExt->timers[TimerAnthelionEndlessEclipse] > 1)
						{
							helper::playSoundFromSource(scbw::randBetween(SoundId::AnthelionMoveCharge01, SoundId::AnthelionMoveCharge02), unit, 1, 0);
						}
						unitExt->timers[TimerAnthelionEndlessEclipse] = 5 * 24 + 1;
					}
					unitExt->anthelionLastMoveTarget = unit->moveTarget.pt;
					s16 oldX = unit->sprite->position.x;
					s16 oldY = unit->sprite->position.y;

					if (unitExt->timers[TimerAnthelionEndlessEclipse] == 1)
					{
						bool teleport = scbw::teleport(unit, unit->moveTarget.pt, 0, true);
						if (teleport)
						{
							scbw::createAndInitThingy(SpriteId::AnthelionTeleportDepart, oldX, oldY, unit->playerId, 11);
							helper::playSoundFromSource(scbw::randBetween(SoundId::AnthelionArrive01, SoundId::AnthelionArrive03), unit, 1, 0);
							unit->sprite->createTopOverlay(ImageId::AnthelionTeleportArrival, 0, 0, 0);
						}
					}
				}
				else
				{
					unitExt->timers[TimerAnthelionEndlessEclipse] = 5 * 24 + 1;
				}
			} break;
			case UnitId::TerranSeraph:
			{
				// Observance - Seraph ability
				u32 radius = 6 * 32;
				auto match = cosmonarchyFinderMatch(unit, Flags::Enemy, {});
				bool attacked = false;
				scbw::UnitFinder(
					unit->getX() - radius,
					unit->getY() - radius,
					unit->getX() + radius,
					unit->getY() + radius
				).forEachInMatch(match, [&](Unit* target)
				{
					if ((!scbw::isAlliedTo(unit->playerId, target->playerId)))
					{
						UnitExt* targetExt = globals.get_unit_ext(target);
						if (!targetExt->timers[TimerObservanceDebuff])
						{
							unitExt->timers[TimerObservanceOverlay] = 2;
							targetExt->timers[TimerObservanceDebuff] = 5 * 24 + 1;
							targetExt->timers[TimerObservancePeriod] = 5 * 24 + 1;
							if (targetExt->values[ValueObservanceRend] == 0)
							{
								s32 overlay = ImageId::ObservanceOverlaySmall - scbw::getOverlayOffset(target->id);
								target->sprite->createOverlay(overlay);
							}
							targetExt->values[ValueObservanceRend] += 1;
						}
					}
				});
			} break;
			case UnitId::TerranAzazel:
			{
				// Tag nearby units for Sublime Shepherd
				scbw::UnitFinder(
					unit->getX() - 128,
					unit->getY() - 128,
					unit->getX() + 128,
					unit->getY() + 128
				).forEach([&](Unit* target)
					{
						if (target == unit)
						{
							return;
						}
						if (scbw::unitIdIsResource(target->id, false))
						{
							return;
						}
						UnitExt* targetExt = globals.get_unit_ext(target);
						targetExt->values[ValueHasSublimeShepherd] = 1;
					});
			} break;
			case UnitId::SharedAxitrilisk:
			{
				// Axitrilisk - play sound on shield loss, timer of 5 seconds per unit
				if (unit->shields < 1
					&& !unitExt->timers[TimerAxitriliskShieldLossSFX])
				{
					unitExt->timers[TimerAxitriliskShieldLossSFX] = AxitriliskShieldLossSFXTimer;
					helper::playSoundFromSource(scbw::randBetween(SoundId::AxitriliskShieldLoss01, SoundId::AxitriliskShieldLoss03), unit, 1, 0);
				}
			} break;
			case UnitId::TerranPhalanxSiegeMode:
			{
				// Fix AI-controlled Phalanxes sometimes failing to acquire targets while in siege mode, courtesy of DarkenedFantasies.
				/*
				if (playerTable[unit->playerId].type == PlayerType::Computer && unit->mainOrderId == OrderId::WatchTarget) {
					helper::issue_order_targeting_nothing(unit, OrderId::ComputerAI);
				}
				*/
			} break;
		}

		// Fix animation bug with acceleration movement flag
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::FlyingBuilding)
			&& unit->movementFlags & MovementFlags::Accelerating
			&& unit->sprite->mainGraphic->animation != IscriptAnimation::Walking
			&& unit->sprite->mainGraphic->animation != IscriptAnimation::WalkingToIdle)
		{
			unit->movementFlags &= ~MovementFlags::Accelerating;
		}

		// Sublime Shepherd - Check tagged units for nearby Azazels
		unitExt->values[ValueSublimeShepherd] = 0;
		if (unitExt->values[ValueHasSublimeShepherd])
		{
			unitExt->values[ValueHasSublimeShepherd] = 0;
			scbw::UnitFinder(
				unit->getX() - 128,
				unit->getY() - 128,
				unit->getX() + 128,
				unit->getY() + 128
			).forEach([&](Unit* target)
				{
					if (target == unit)
					{
						return;
					}
					if (target->id != UnitId::TerranAzazel)
					{
						return;
					}
					unitExt->values[ValueSublimeShepherd]++;
				});
		}

		// Azazel - Sublime Shepherd overlay
		if (unitExt->values[ValueSublimeShepherd] && !unit->getOverlay(ImageId::SublimeShepherdOverlaySmall - scbw::getOverlayOffset(unit->id)))
		{
			unit->sprite->createTopOverlay(ImageId::SublimeShepherdOverlaySmall - scbw::getOverlayOffset(unit->id));
		}
		else if (unit->getOverlay(ImageId::SublimeShepherdOverlaySmall - scbw::getOverlayOffset(unit->id)) && !unitExt->values[ValueSublimeShepherd])
		{
			unit->sprite->removeOverlay(ImageId::SublimeShepherdOverlaySmall - scbw::getOverlayOffset(unit->id));
		}

		// Fix target clearing bug
		/*
		if (unit->orderTarget.unit
			&& !(unit->orderTarget.unit->visibilityStatus & (1 << unit->playerId)))
		{
			scbw::print("lard");
			unit->orderToIdle();
		}
		*/

		// Phase Link Overlay
		if (*elapsedTimeFrames % 10 == 0
			&& unitExt->unitVectors[UnitVectorPhaseLinkClarions].size())
		{
			unit->sprite->createOverlay(ImageId::PhaseLinkSmall + scbw::getOverlayOffset(unit->id), 0, 0, 0);
		}

		// Disruption Web Clear
		if (unitExt->timers[TimerUnderDisruptionWeb] == 1)
		{
			unit->status &= ~UnitStatus::CanNotAttack;
		}

		// Madrigal - Dirge
		if (unitExt->timers[TimerDirgeFuse] == 1)
		{
			scbw::dirgeFuse(unit, unitExt);
		}

		// Hierophant Signify Reset
		if (unitExt->timers[TimerSignify] == 1)
		{
			unitExt->values[ValueSignifyStacks] = 0;
			unitExt->valueVectors[ValueVectorSignifySourcePlayer].clear();
			scbw::updateSignifyStatus(unit, unitExt);
		}

		// Decloak on stun
		if (unit->isFrozen() || unitExt->timers[TimerStrigaRepulsion] || unitExt->timers[TimerAnthelionGravityWell])
		{
			scbw::decloakProc(unit);
		}

		/*
		// Play addon work animations when related units are being trained by parent structure
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
				if (!(unit->_padding_0x132 & cosmonarchyFlags1::BasiliskBurrowed)) {
					unit->_padding_0x132 |= cosmonarchyFlags1::BasiliskBurrowed;
					spawnCreeper(unit);
				}
			}
		}
		*/

		// Correct vanilla timers - THIS IS TEMPORARY!
		if (unit->defensiveMatrixTimer > AzazelDefensiveMatrixTimer)
		{
			unit->defensiveMatrixTimer = AzazelDefensiveMatrixTimer;
		}
		if (unit->stasisTimer > DidactStasisFieldTimer)
		{
			unit->stasisTimer = DidactStasisFieldTimer;
		}
		if (unit->maelstromTimer > MindTyrantMaelstromTimer)
		{
			unit->maelstromTimer = MindTyrantMaelstromTimer;
		}
		if (unit->plagueTimer > IsthrathalethPlagueTimer)
		{
			unit->plagueTimer = IsthrathalethPlagueTimer;
		}

		// Striga - Fleeting Focus
		if (unitExt->timers[TimerStrigaRepulsion])
		{
			Point16 finalPos = unit->position;
			finalPos.x += unitExt->repulsionVector.x;
			finalPos.y += unitExt->repulsionVector.y;
			finalPos = scbw::clampPoint(finalPos);

			if (!scbw::moveUnitImproved(unit, finalPos.x, finalPos.y))
			{
				unitExt->timers[TimerStrigaRepulsion] = 0;
			}
		}

		// Set Command Card
		if (unitExt->values[ValueSetCommandCard]) {
			if (unit == *activePortraitUnit) {
				helper::BTNSACT_ChangeButtons(unitExt->values[ValueSetCommandCard]);
			}
			unitExt->values[ValueSetCommandCard] = 0;
		}

		// Larva Selection
		u32 larva_value = scbw::get_generic_rust_value(unit, RustId::SelectLarvaType);
		if (larva_value) {
			scbw::set_generic_rust_value(unit, RustId::SelectLarvaType, 0);
			BTNSACT_SelectLarva();
			helper::updateSelectedUnitData(); // Required to update *activePortraitUnit
			if (*activePortraitUnit && (*activePortraitUnit)->id == UnitId::ZergLarva) {
				UnitExt* activeExt = globals.get_unit_ext(*activePortraitUnit);
				activeExt->values[ValueSetCommandCard] = 205 + larva_value; // This has to be a two step process, because BW is stupid
			}
		}

		// Magnetar - Law of Attraction
		// Todo - fix lag time between multiple pulls
		// Todo - fix delay before moved unit is able to do stuff
		if (unitExt->timers[TimerMagnetarAttraction])
		{
			Point16 finalPos = unit->position;
			finalPos.x += unitExt->repulsionVector.x;
			finalPos.y += unitExt->repulsionVector.y;
			finalPos = scbw::clampPoint(finalPos);

			if (!scbw::moveUnitImproved(unit, finalPos.x, finalPos.y))
			{
				unitExt->timers[TimerMagnetarAttraction] = 0;
			}
		}

		// Anthelion - Gravity Well
		if (unitExt->timers[TimerAnthelionGravityWell] && unitExt->units[UnitAnthelionGravityWellSource])
		{
			Point16 targetPos = unitExt->units[UnitAnthelionGravityWellSource]->position;
			if (unit->position != targetPos)
			{
				Vector32 diff = scbw::getVectorBetweenPoints(unit->position, targetPos);

				if (scbw::getVectorDistance(diff) < 2)
				{
					scbw::moveUnitImproved(unit, targetPos.x, targetPos.y);
				}
				else
				{
					Vector32f displacement = scbw::getNormalizedVector(diff);
					displacement.x *= 2;
					displacement.y *= 2;

					Point16 finalPos = unit->position;
					finalPos.x += displacement.x;
					finalPos.y += displacement.y;
					finalPos = scbw::clampPoint(finalPos);

					scbw::moveUnitImproved(unit, finalPos.x, finalPos.y);
				}
			}
			s32 angle = scbw::getAngle(unit->position, targetPos);
			unit->currentDirection1 = angle;
		}

		// Cyclops - Flak Cannon Fuse
		for (u32 fuse_timer : unitExt->timerVectors[TimerVectorCyclopsFuse])
		{
			if (fuse_timer == 1)
			{
				// Todo: make thingy
				helper::createThingy(SpriteId::FragmentaryExplosion, unit->getX(), unit->getY(), 11);
				helper::playSoundFromSource(scbw::randBetween(SoundId::CyclopsPassive01, SoundId::CyclopsPassive04), unit, 1, 0);
				const u32 radius = 1.5 * 32;
				scbw::UnitFinder(
					unit->getX() - radius,
					unit->getY() - radius,
					unit->getX() + radius,
					unit->getY() + radius
				).forEachInMatch(cosmonarchyFinderMatch(unit, Flags::NotSelf, {}), [&](Unit* target)
				{
					target->damageWithCustom(5 * 256, DamageType::Explosive, nullptr, 0, 0, 1, false);
				});
			}
		}

		// Gorgon - Petrification
		if (unitExt->timers[TimerPetrification] == 1) {
			unitExt->values[ValuePetrificationStacks] = 0;
		}

		// Blissful Embrace - Creep Spread
		if (unitExt->values[ValueBlissfulEmbraceOriginalPlayer] &&
			units_dat::BaseProperty[unit->id] & UnitProperty::Building &&
			*elapsedTimeFrames % 15 == 0) {
			helper::spreadCreep(UnitId::ZergHive, unit->position.x, unit->position.y);
		}

		// Akistrokor - Blissful Embrace ability
		if (unit->mainOrderId == OrderId::BlissfulEmbrace)
		{
			Unit* target = unit->orderTarget.unit;
			if (target && target->hitPoints <= target->getMaxHpRaw() / 2 && target->status & UnitStatus::Completed
				&& !(units_dat::BaseProperty[target->id] & UnitProperty::Addon) && unit->playerId != target->playerId) {
				UnitExt* targetExt = globals.get_unit_ext(target);
				if (helper::isUnitInWeaponRange(unit, target)) {
					if (unitExt->values[ValueBlissfulEmbraceChannel] < 48) {
						unitExt->values[ValueBlissfulEmbraceChannel] += 100;
					} else {
						helper::force_order_done(unit);
						unitExt->values[ValueBlissfulEmbraceChannel] = 0;
						if (!targetExt->values[ValueBlissfulEmbraceOriginalPlayer]) {
							targetExt->values[ValueBlissfulEmbraceOriginalPlayer] = target->playerId + 1;
							targetExt->values[ValueBlissfulEmbraceOriginalHealth] = target->hitPoints;
						}
						target->giveTo(unit->playerId);

						// Make this a sprite
						u32 image_id = ImageId::ZergAirDeathExplosion_Small;
						if (units_dat::BaseProperty[target->id] & UnitProperty::LargeOverlay) {
							image_id = ImageId::ZergBuildingExplosion;
						} else if (units_dat::BaseProperty[target->id] & UnitProperty::MediumOverlay) {
							image_id = ImageId::ZergAirDeathExplosion_Large;
						}
						target->sprite->createOverlay(image_id);

						if (units_dat::BaseProperty[target->id] & UnitProperty::Building) {
							//helper::modifyCreepState(target->position.x / 32, target->position.y / 32, (helper::CreepChange*)0x00413AA0, 1);
						}

						// Create busy overlays for buildings only
						if (units_dat::BaseProperty[target->id] & UnitProperty::Building) {
							u32 image_id = ImageId::InfestedOverlay_Large;
							if (scbw::unit_id_is_smaller_building(target->id)) {
								image_id = ImageId::InfestedOverlay_Small;
							}
							if (!target->getOverlay(image_id)) {
								target->sprite->createTopOverlay(image_id);
							}
						}
					}
				}
			}
			else
			{
				helper::force_order_done(unit);
			}
		}
		else
		{
			unitExt->values[ValueBlissfulEmbraceChannel] = 0;
		}

		// Reviving
		if (((unitExt->timers[TimerReviveFlagClearMin] == 0 && !(unit->status & UnitStatus::NoBrkCodeStart)) || unitExt->timers[TimerReviveFlagClearMax] == 0)
			&& unitExt->values[ValueReviveStarted])
		{
			unit->status &= ~(UnitStatus::Invincible | UnitStatus::Unmovable | UnitStatus::CanNotReceiveOrders | UnitStatus::CanNotAttack);
			unitExt->timers[TimerReviveFlagClearMin] = 0;
			unitExt->timers[TimerReviveFlagClearMax] = 0;
			unitExt->values[ValueReviveStarted] = 0;

			if (playerTable[unit->playerId].type == PlayerType::Computer)
			{
				if (units_dat::BaseProperty[unit->id] & (UnitProperty::Worker | UnitProperty::Building)
					|| scbw::unitIdIsZergSupply(unit->id))
				{
					AiTown* town = (AiTown*)unitExt->values[ValueReviveTownPointer];
					if (town)
					{
						helper::AI_AddWorkerStructureAI(unit, town);
					}
					else
					{
						unit->remove();
					}
				}
				else
				{
					hooks::AI_TrainingSimpleMilitary(unit, unit);
				}
			}
		}

		// Alaszileth - Dermal Accelerant
		Point16 pos = unit->position;
		s32 radius = 7 * 32;
		scbw::UnitFinder(
			pos.x - radius,
			pos.y - radius,
			pos.x + radius,
			pos.y + radius
		).forEach([&](Unit* foundUnit)
		{
			if (!(scbw::isAlliedTo(foundUnit->playerId, unit->playerId)))
			{
				return;
			}
			if (foundUnit->id != UnitId::ZergAlaszileth)
			{
				return;
			}
			if (scbw::unitIdIsResource(unit->id, false))
			{
				return;
			}
			unitExt->timers[TimerAlaszilethRegenBoost] = 2;

			// Create overlay
			s32 overlay = ImageId::DermalAccelerantSmall + scbw::getOverlayOffset(unit->id, true);
			if (!unit->getOverlay(overlay))
			{
				unit->sprite->createTopOverlay(overlay);
			}
		});

		// Seraph - Observance
		if (unitExt->timers[TimerObservanceDebuff] > 1)
		{
			const auto range = 6 * 32;
			auto seraphs = scbw::UnitFinder(
				unit->getX() - range,
				unit->getY() - range,
				unit->getX() + range,
				unit->getY() + range
			).filter([&](Unit* target)
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
				unitExt->timers[TimerObservanceDebuff] = 5 * 24 + 1;

				// Seraph - Observance ability
				if (unitExt->timers[TimerObservancePeriod] == 1)
				{
					unitExt->timers[TimerObservancePeriod] = 5 * 24 + 1;
					unitExt->values[ValueObservanceRend] += 1;
				}

				// Seraph - Observance persistant overlay creation
				for (auto seraph : seraphs)
				{
					if (!seraph)
					{
						continue;
					}
					UnitExt* seraphExt = globals.get_unit_ext(seraph);
					if (seraphExt->timers[TimerObservanceOverlay] == 0)
					{
						seraph->sprite->createOverlay(ImageId::SeraphRedOverlay);
						seraphExt->timers[TimerObservanceOverlay] = 18;
					}
				}
			}
		}
		else if (unitExt->timers[TimerObservanceDebuff] == 1)
		{
			unit->sprite->removeOverlay(ImageId::ObservanceOverlayLarge, ImageId::ObservanceOverlaySmall);
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
				Sprite* sprite;
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

		// Luminary - Wax Wings Clear
		if (unitExt->timers[TimerWaxWingsDebuff] == 1)
		{
			unit->updateSpeed();
		}

		// Luminary - Aural Assault
		if (unitExt->timers[TimerAuralAssaultBlind] > 1)
		{
			if (unit->mainOrderId != OrderId::AttackUnit)
			{
				auto match = cosmonarchyFinderMatch(unit, Flags::NotSelf, {});
				const u32 radius = unit->getSeekRange() * 32;
				Unit* target = scbw::UnitFinder::getNearestTarget(
					unit->getX() - radius,
					unit->getY() - radius,
					unit->getX() + radius,
					unit->getY() + radius,
					unit, match
				);
				if (target)
				{
					unit->orderTo(OrderId::AttackUnit, target);
				}
			}
		}

		// Talos - Salvage Stun
		if (unitExt->timers[TimerSalvageStunTimer] > 1) {
			unit->freeze();

			u32 offset = scbw::getOverlayOffset(unit->id);
			if (!unit->getOverlay(ImageId::SalvageDisableSmall + offset)) {
				unit->sprite->createTopOverlay(ImageId::SalvageDisableSmall + offset);
			}
		}
		if (unitExt->timers[TimerSalvageStunTimer] == 1) {
			unit->unfreeze();

			u32 offset = scbw::getOverlayOffset(unit->id);
			if (unit->getOverlay(ImageId::SalvageDisableSmall + offset)) {
				unit->sprite->removeOverlay(ImageId::SalvageDisableSmall + offset);
			}
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
				unit->visibilityStatus = 0xff;
				helper::updateVisibility(unit); // didact cloak needs special handling
			}
		}

		// Remove all Nemean Reactors stacks after not attacking for 5 seconds
		if (unitExt->timers[TimerNemeanReactors] == 1)
		{
			unitExt->values[ValueNemeanReactorsStack] = 0;
			// remove overlay later
		}

		// Update flingy speed when Adrenal Frenzy period expires
		if (unitExt->timers[TimerNathrokorBuff] == 1)
		{
			unitExt->timers[TimerNathrokorBuff] = 0;
			unitExt->values[ValueNathrokorBuffStacks] = 0;
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
			// unit->removeOverlay();
		}

		// Crucible Stacks Clear
		{
			auto& crucibles = unitExt->unitVectors[UnitVectorCrucibles];
			for (auto it = crucibles.begin(); it != crucibles.end(); ++it)
			{
				Unit* target = *it;
				if (scbw::getDistanceFast(unit->position, target->position) > 8 * 32)
				{
					target->removeOverlay(ImageId::TreasurySupplyOverlay);
					crucibles.erase(it);
					--it;
				}
			}
		}

		// Crucible - Khaydarin Charge stacks
		if (unit->id == UnitId::ProtossCrucible && unit->status & UnitStatus::Completed)
		{
			UnitArray arr = UnitSearchRadial(unit->position, 8 * 32);
			UnitArrayFilter(&arr, unit, FilterAlly | FilterBuilding);
			for (u32 i = 0; i < arr.size; i++)
			{
				Unit* target = arr.units[i];
				UnitExt* targetExt = globals.get_unit_ext(target);
				auto& vec = targetExt->unitVectors[UnitVectorCrucibles];
				if (std::find(vec.begin(), vec.end(), unit) == vec.end())
				{
					vec.push_back(unit);
				}
				if (!target->getOverlay(ImageId::TreasurySupplyOverlay)) {
					target->sprite->createUnderlay(ImageId::TreasurySupplyOverlay, 0, 0, 0);
				}
			}
		}

		// Emergency Detonation - initial cast
		if (scbw::get_generic_rust_value(unit, RustId::CastEmergencyDetonation)
			&& !unitExt->timers[TimerEmergencyDetonation]
			&& !unitExt->timers[TimerControlledDemolition])
		{
			scbw::set_generic_rust_value(unit, RustId::CastEmergencyDetonation, 0);
			unitExt->timers[TimerEmergencyDetonation] = EmergencyDetonationTimer;
			s32 overlayOffset = scbw::getOverlayOffset(unit->id);
//			unit->sprite->createOverlay(ImageId::EmergencyDemolitionOverlay_Small + overlayOffset);
		}

		// Controlled Demolition - initial cast
		if (scbw::get_generic_rust_value(unit, RustId::CastControlledDemolition)
			&& !unitExt->timers[TimerEmergencyDetonation] && !unitExt->timers[TimerControlledDemolition])
		{
			scbw::set_generic_rust_value(unit, RustId::CastControlledDemolition, 0);
			unitExt->timers[TimerControlledDemolition] = ControlledDemolitionTimer;
			s32 overlayOffset = scbw::getOverlayOffset(unit->id);
//			unit->sprite->createOverlay(ImageId::ControlledDemolitionOverlay_Small + overlayOffset);
		}

		// Controlled Demolition - destroy structure
		if (unitExt->timers[TimerControlledDemolition] == 1)
		{
			unit->damageHp(unit->hitPoints, unit, unit->playerId, false);
		}

		// Emergency Detonation - destroy structure
		if (unitExt->timers[TimerEmergencyDetonation] == 1)
		{
			auto match = cosmonarchyFinderMatch(unit, Flags::NotSelf, {});
			const u32 radius = 3 * 32;
			scbw::UnitFinder(
				unit->getX() - radius,
				unit->getY() - radius,
				unit->getX() + radius,
				unit->getY() + radius
			).forEachInMatch(match, [&](Unit* target)
			{
				if (scbw::getDistanceFast(unit->position, target->position) <= radius)
				{
					target->damageWithCustom(250 * 256, DamageType::Explosive, unit, unit->playerId, 0, 1, false);
				}
			});
			helper::createThingy(SpriteId::ArcliteShockCannon_Damageless, unit->sprite->position.x, unit->sprite->position.y, unit->playerId);
			unit->damageHp(unit->hitPoints, unit, unit->playerId, false);
		}

		// Emergency Detonation - initial cast
		if (scbw::get_generic_rust_value(unit, RustId::CastEmergencyDetonation))
		{
			scbw::set_generic_rust_value(unit, RustId::CastEmergencyDetonation, 0);
			unitExt->timers[TimerEmergencyDetonation] = (u32)(0.5 * unitExt->timers[TimerControlledDemolition]);
			unitExt->timers[TimerControlledDemolition] = 0;
			s32 overlayOffset = scbw::getOverlayOffset(unit->id);
//			unit->sprite->createOverlay(ImageId::EmergencyDetonationOverlay_Small + overlayOffset);
		}

		// Demiurge - Gods Made Flesh - return units to original owner
		if (unitExt->values[ValueWorshipStack]
			&& unitExt->units[UnitMindController] == nullptr)
		{
			helper::playSoundFromSource(SoundId::DemiurgeWorship01, unit, 1, 0);
			u32 overlayId = ImageId::MindControlHit_Small + scbw::getOverlayOffset(unit->id);
			unit->sprite->createTopOverlay(overlayId, 0, 0, 0);

			unitExt->values[ValueWorshipStack] = 0;
			unitExt->timers[TimerGodsMadeFlesh] = 0;

			scbw::recede_mind_control(unit);
		}

		// Demiurge - Gods Made Flesh - kill unit when the effect ends
		if (unitExt->timers[TimerGodsMadeFlesh] == 1
			&& unitExt->units[UnitMindController])
		{
			unit->remove();
		}

		// Alaszileth - Dermal Accelerant - remove overlays
		if (unitExt->timers[TimerAlaszilethRegenBoost] == 0)
		{
			s32 overlay = ImageId::DermalAccelerantSmall + scbw::getOverlayOffset(unit->id, true);
			if (unit->getOverlay(overlay))
			{
				unit->removeOverlay(overlay);
			}
		}

		if (unitExt->values[ValueAIWorkerChosenForExpansion] && unit->resourceType == ResourceType::None) {
			unit->orderTo(OrderId::Move, unitExt->values[ValueAIWorkerChosenX], unitExt->values[ValueAIWorkerChosenY]);
			unitExt->values[ValueAIWorkerChosenForExpansion] = 0;
			unitExt->values[ValueAIWorkerChosenX] = 0;
			unitExt->values[ValueAIWorkerChosenY] = 0;
		}
	}

	bool gameOn()
	{
		chk_cache_size = 0;
		setMaxSightRange<20>();
		return true;
	}

	bool gameEnd()
	{
		end_scenario();
		return true;
	}
}