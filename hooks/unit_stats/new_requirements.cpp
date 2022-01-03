#include "new_requirements.h"
#include <hook_tools.h>

namespace hooks
{
	enum
	{
		TECH_LOCKED = -1,
		TECH_HIDDEN = 0,
		TECH_ACTIVE = 1,
	};

	s32 createUnitAllowed_Partial(s16 unitId, Unit* unit, s32 playerId)
	{
		*lastInternalError = InternalErrors::UnknownError0;
		if (unitId >= UnitId::None)
		{
			*lastInternalError = InternalErrors::UnknownError16;
			return TECH_HIDDEN;
		}
		if (playerId != unit->playerId)
		{
			*lastInternalError = InternalErrors::UnknownError1;
			return TECH_HIDDEN;
		}
		if (!(unit->status & UnitStatus::Completed))
		{
			*lastInternalError = InternalErrors::UnknownError20;
			return TECH_HIDDEN;
		}
		if (unit->isFrozen())
		{
			*lastInternalError = InternalErrors::UnitFrozen;
			return TECH_HIDDEN;
		}
		if (!UnitAvailability->available[unit->playerId][unitId])
		{
			*lastInternalError = InternalErrors::UnknownError2;
			return TECH_HIDDEN;
		}
		if (unit->isQueueSlotActive(4))
		{
			*lastInternalError = InternalErrors::UnknownError3;
			return TECH_HIDDEN;
		}
		if (unit->status & UnitStatus::IsHallucination)
		{
			*lastInternalError = InternalErrors::UnknownError22;
			return TECH_HIDDEN;
		}

		enum
		{
			Tier1 = 0,
			Tier2,
			Tier3,
			Tier4
		};

		bool TierUnlocked[3][4];
		TierUnlocked[RaceId::Zerg][Tier1] = true;
		TierUnlocked[RaceId::Zerg][Tier2] = Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris)
			|| Player::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
			|| Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor);
		TierUnlocked[RaceId::Zerg][Tier3] = Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform)
			|| Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor);
		TierUnlocked[RaceId::Zerg][Tier4] = Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlaszilArbor);

		TierUnlocked[RaceId::Terran][Tier1] = true;
		TierUnlocked[RaceId::Terran][Tier2] = Player::numberOfCompletedUnitsOfType(playerId, UnitId::TerranEngineeringBay);
		TierUnlocked[RaceId::Terran][Tier3] = Player::numberOfCompletedUnitsOfType(playerId, UnitId::TerranScienceFacility);
		TierUnlocked[RaceId::Terran][Tier4] = true;

		TierUnlocked[RaceId::Protoss][Tier1] = true;
		TierUnlocked[RaceId::Protoss][Tier2] = true;
		TierUnlocked[RaceId::Protoss][Tier3] = true;
		TierUnlocked[RaceId::Protoss][Tier4] = true;

		if (helper::isConstructingAddon(unit))
		{
			return TECH_HIDDEN;
		}

		bool hasAddon = false;
		s32 addonId = -1; 
		if (unit->building.addon)
		{
			hasAddon = true;
			addonId = unit->building.addon->id;
		}
		bool inAir = unit->status & UnitStatus::InAir;
		bool isTraining = unit->buildQueue[unit->buildQueueSlot] != UnitId::None;

		switch (unit->id)
		{
			// TERRAN
			case UnitId::TerranMason:
			{
				switch (unitId)
				{
					case UnitId::TerranCommandCenter:
					case UnitId::TerranSupplyDepot:
					case UnitId::TerranReservoir:
					case UnitId::TerranWatchdog:
					case UnitId::TerranBunker:
					{
						return TECH_ACTIVE;
					}
					case UnitId::TerranBarracks:
					case UnitId::TerranFactory:
					case UnitId::TerranStarport:
					case UnitId::TerranScrapyard:
					case UnitId::TerranEngineeringBay:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::TerranCommandCenter))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranTreasury:
					case UnitId::TerranCaptaincy:
					case UnitId::TerranScienceFacility:
					{
						if (TierUnlocked[RaceId::Terran][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranIronFoundry:
					case UnitId::TerranNaniteAssembly:
					case UnitId::TerranIonCannon:
					{
						if (TierUnlocked[RaceId::Terran][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
				}
			}
			case UnitId::TerranCommandCenter:
			{
				switch (unitId)
				{
					case UnitId::TerranMason:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranComsatStation:
					{
						if (hasAddon || isTraining)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranNuclearSilo:
					case UnitId::TerranQuarry:
					{
						if (hasAddon || isTraining)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
				}
			} break;
			case UnitId::TerranNuclearSilo:
			{
				switch (unitId)
				{
					case UnitId::TerranNuclearMissile:
					{
						if (!unit->building.silo.nuke && unit->buildQueue[unit->buildQueueSlot] == UnitId::None)
						{
							return TECH_ACTIVE;
						}
					}
				}
			} break;
			case UnitId::TerranQuarry:
			{
				switch (unitId)
				{
					case UnitId::TerranMason:
					{
						return TECH_ACTIVE;
					}
				}
			} break;
			case UnitId::TerranTreasury:
			{
				switch (unitId)
				{
					case UnitId::TerranComsatStation:
					{
						if (hasAddon || isTraining)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranNuclearSilo:
					case UnitId::TerranQuarry:
					{
						if (hasAddon || isTraining)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
				}
			} break;
			case UnitId::TerranBarracks:
			{
				switch (unitId)
				{
					case UnitId::TerranMedbay:
					{
						if (hasAddon || isTraining)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranCovertOps:
					{
						if (hasAddon || isTraining)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranMaverick:
					case UnitId::TerranHarakan:
					case UnitId::TerranCyprian:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranCleric:
					case UnitId::TerranShaman:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						if (addonId == UnitId::TerranMedbay)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranEidolon:
					case UnitId::TerranSavant:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						if (addonId == UnitId::TerranCovertOps)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
				}
				return TECH_HIDDEN;
			} break;
			case UnitId::TerranFactory:
			{
				switch (unitId)
				{
					case UnitId::TerranMachineShop:
					case UnitId::TerranSalvoYard:
					{
						if (hasAddon || isTraining)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranVulture:
					case UnitId::TerranGoliath:
					case UnitId::TerranCyclops:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranPhalanxTankMode:
					case UnitId::TerranSouthpaw:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						if (addonId == UnitId::TerranMachineShop)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranPaladin:
					case UnitId::TerranMadrigal:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						if (addonId == UnitId::TerranSalvoYard)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
				}
			} break;
			case UnitId::TerranStarport:
			{
				switch (unitId)
				{
					case UnitId::TerranMunitionsBay:
					case UnitId::TerranControlTower:
					{
						if (hasAddon || isTraining)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranWraith:
					case UnitId::TerranGorgon:
					case UnitId::TerranValkyrie:
					case UnitId::TerranTrojan:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranAzazel:
					case UnitId::TerranSeraph:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						if (addonId == UnitId::TerranControlTower)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranSalamander:
					case UnitId::TerranWyvern:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						if (addonId == UnitId::TerranMunitionsBay)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
				}
			} break;
			case UnitId::TerranScrapyard:
			{
				switch (unitId)
				{
					case UnitId::TerranFutureStation:
					{
						if (hasAddon || isTraining)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranBioticBase:
					case UnitId::TerranTinkerersTower:
					{
						if (hasAddon || isTraining)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranBlackjack:
					case UnitId::SharedAspirant:
					case UnitId::SharedCohort:
					case UnitId::TerranSiren:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::SharedStriga:
					case UnitId::SharedLuminary:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						if (addonId == UnitId::TerranTinkerersTower)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::SharedTarasque:
					case UnitId::SharedAnticthon:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						if (addonId == UnitId::TerranBioticBase)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
				}
			} break;
			case UnitId::TerranCaptaincy:
			{
				switch (unitId)
				{
					case UnitId::TerranGuildhall:
					{
						if (hasAddon || isTraining)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranMadcap:
					case UnitId::TerranHeracles:
					case UnitId::TerranApostle:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranOlympian:
					case UnitId::TerranAutocrat:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						if (addonId == UnitId::TerranGuildhall)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
				}
			} break;
			case UnitId::TerranIronFoundry:
			{
				switch (unitId)
				{
					case UnitId::TerranPazuzu:
					case UnitId::TerranClaymore:
					case UnitId::TerranPenumbra:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
				}
			} break;
			case UnitId::TerranNaniteAssembly:
			{
				switch (unitId)
				{
					case UnitId::TerranCentaur:
					case UnitId::TerranMinotaur:
					case UnitId::TerranPhobos:
					{
						if (inAir)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
				}
			} break;
			
			// PROTOSS
			// Units
			case UnitId::ProtossNexus:
			{
				if (unitId == UnitId::ProtossScribe)
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ProtossForge:
			{
				if (unitId == UnitId::ProtossScribe
					|| unitId == UnitId::ProtossEnvoy
					|| unitId == UnitId::ProtossWitness)
				{
					return TECH_ACTIVE;
				}
				else if (unitId == UnitId::ProtossClarion)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossStridentStratum))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ProtossGateway:
			{
				switch (unitId)
				{
					case UnitId::ProtossZealot:
					case UnitId::ProtossDracadin:
					case UnitId::ProtossEcclesiast:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ProtossLegionnaire:
					case UnitId::ProtossHierophant:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossCenotaph))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossGrandLibrary:
			{
				switch (unitId)
				{
					case UnitId::ProtossCantavis:
					case UnitId::ProtossAmaranth:
					case UnitId::ProtossAtreus:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ProtossArchon:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossArchonArchives))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossRoboticsFacility:
			{
				switch (unitId)
				{
					case UnitId::ProtossVassal:
					case UnitId::ProtossSimulacrum:
					case UnitId::ProtossManifold:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ProtossGolem:
					case UnitId::ProtossIdol:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossAutomatonRegister))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossRoboticsAuthority:
			{
				switch (unitId)
				{
					case UnitId::ProtossAccantor:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ProtossArchitect:
					case UnitId::ProtossDemiurge:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossSyntheticSynod))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossStargate:
			{
				switch (unitId)
				{
					case UnitId::ProtossAurora:
					case UnitId::ProtossPanoptus:
					case UnitId::ProtossLanifect:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ProtossGladius:
					case UnitId::ProtossExemplar:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossAstralOmen))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossArgosy:
			{
				switch (unitId)
				{
					case UnitId::ProtossMagister:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ProtossDidact:
					case UnitId::ProtossEmpyrean:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossCelestialTribune))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ProtossStarSovereign:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossCosmicAltar))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ProtossSolarion:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossFleetBeacon))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossProstrationStage:
			{
				switch (unitId)
				{
					case UnitId::ProtossCabalist:
					case UnitId::ProtossBarghest:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ProtossMindTyrant:
					case UnitId::ProtossPariah:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossSanctumOfSorrow))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ProtossAnthelion:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossMonumentOfSin))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossRogueGallery:
			{
				switch (unitId)
				{
					case UnitId::SharedAspirant:
					case UnitId::SharedCharlatan:
					case UnitId::SharedAxitrilisk:
					{
						return TECH_ACTIVE;
					}
					case UnitId::SharedStriga:
					case UnitId::SharedLuminary:
					case UnitId::SharedEmpress:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossWaywardLure))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					return TECH_HIDDEN;
				}
			}

			// Morphs and subunits
			case UnitId::ProtossCantavis:
			{
				if (unitId == UnitId::ProtossAugur)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossArchonArchives))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ProtossAccantor:
			{
				if (unitId == UnitId::ProtossPlasmaShell)
				{
					u32 shellCount = unit->carrier.inHangarCount + unit->carrier.outHangarCount;
					for (u32 i = 0; i < 5; ++i)
					{
						shellCount += unit->buildQueue[i] != UnitId::None;
					}
					if (shellCount >= 5)
					{
						return TECH_HIDDEN;
					}
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ProtossSolarion:
			{
				if (unitId == UnitId::ProtossSteward)
				{
					u32 stewardCount = unit->carrier.inHangarCount + unit->carrier.outHangarCount;
					for (u32 i = 0; i < 5; ++i)
					{
						stewardCount += unit->buildQueue[i] != UnitId::None;
					}
					if (stewardCount >= 12)
					{
						return TECH_HIDDEN;
					}
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}

			// Structures
			case UnitId::ProtossScribe:
			{
				switch (unitId)
				{
					case UnitId::ProtossNexus:
					case UnitId::ProtossPylon:
					case UnitId::ProtossAquifer:
					case UnitId::ProtossPhotonCannon:
					case UnitId::ProtossShieldBattery:
					case UnitId::ProtossForge:
					case UnitId::ProtossGateway:
					case UnitId::ProtossCenotaph:
					case UnitId::ProtossRoboticsFacility:
					case UnitId::ProtossAutomatonRegister:
					case UnitId::ProtossStargate:
					case UnitId::ProtossAstralOmen:
					case UnitId::ProtossStridentStratum:
					case UnitId::ProtossCrucible:
					case UnitId::ProtossPennant:
					case UnitId::ProtossGrandLibrary:
					case UnitId::ProtossRoboticsAuthority:
					case UnitId::ProtossSyntheticSynod:
					case UnitId::ProtossArgosy:
					case UnitId::ProtossCelestialTribune:
					case UnitId::ProtossProstrationStage:
					case UnitId::ProtossRogueGallery:
					case UnitId::ProtossArchonArchives:
					case UnitId::ProtossCosmicAltar:
					case UnitId::ProtossFleetBeacon:
					case UnitId::ProtossSanctumOfSorrow:
					case UnitId::ProtossWaywardLure:
					case UnitId::ProtossMonumentOfSin:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			
			// ZERG
			// Units
			case UnitId::ZergLarva:
			{
				switch (unitId)
				{
					case UnitId::ZergDroleth:
					case UnitId::ZergOvileth:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ZergZethrokor:
					case UnitId::ZergQuazilisk:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergLiivralPond)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergQuazrokPool))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergHydralisk:
					case UnitId::ZergSkithrokor:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergGorgralSwamp)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergHydrokDen))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergMutalisk:
					case UnitId::ZergNathrokor:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergVithrathHaunt)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergMuthrokSpire))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::SharedCohort:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOrbothOmlia))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::SharedAxitrilisk:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOrbothOmlia)
							&& TierUnlocked[RaceId::Zerg][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergZoryusthaleth:
					case UnitId::ZergLakizilisk:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergUltrokCavern)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergZorkizShroud))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergMatraleth:
					case UnitId::ZergKonvilisk:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlmakisAntre)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergMatravilNest))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::SharedTarasque:
					case UnitId::SharedCharlatan:
					case UnitId::SharedEmpress:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOrbothOmlia)
							&& TierUnlocked[RaceId::Zerg][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergKeskathalor:
					case UnitId::ZergGeszithalor:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlkajChasm)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergGeszkathGrotto))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergIsthrathaleth:
					case UnitId::ZergSovroleth:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergSovthrathMound))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::SharedAnticthon:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOrbothOmlia)
							&& TierUnlocked[RaceId::Zerg][Tier4])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
				}
				return TECH_HIDDEN;
			}
			
			// Morphs
			case UnitId::ZergOvileth:
			{
				if (unitId == UnitId::ZergIroleth)
				{
					if (TierUnlocked[RaceId::Zerg][Tier2]
						&& scbw::transportIsEmpty(unit))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergZethrokor:
			{
				if (unitId == UnitId::ZergVorvrokor)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergLiivralPond))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergQuazilisk:
			{
				if (unitId == UnitId::ZergLiiralisk)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergLiivralPond))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergHydralisk:
			{
				if (unitId == UnitId::ZergBactalisk)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergGorgralSwamp))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergSkithrokor:
			{
				if (unitId == UnitId::ZergGorgrokor)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergGorgralSwamp))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergMutalisk:
			{
				if (unitId == UnitId::ZergVithrilisk)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergVithrathHaunt))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergNathrokor:
			{
				if (unitId == UnitId::ZergKalkalisk)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergVithrathHaunt))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergIroleth:
			{
				if (unitId == UnitId::ZergOthstoleth)
				{
					if (TierUnlocked[RaceId::Zerg][Tier3]
						&& scbw::transportIsEmpty(unit))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergZoryusthaleth:
			{
				if (unitId == UnitId::ZergUltrokor)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergUltrokCavern))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergMatraleth:
			{
				if (unitId == UnitId::ZergAkistrokor)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlmakisAntre))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergKonvilisk:
			{
				if (unitId == UnitId::ZergAlmaksalisk)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlmakisAntre))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergOthstoleth:
			{
				if (unitId == UnitId::ZergAlaszileth)
				{
					if (TierUnlocked[RaceId::Zerg][Tier4]
						&& scbw::transportIsEmpty(unit))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergKeskathalor:
			{
				if (unitId == UnitId::ZergAlkajelisk)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlkajChasm))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}

			// Structures
			case UnitId::ZergDroleth:
			{
				switch (unitId)
				{
					case UnitId::ZergHatchery:
					case UnitId::ZergCreepColony:
					case UnitId::ZergExtractor:
					case UnitId::ZergQuazrokPool:
					case UnitId::ZergHydrokDen:
					case UnitId::ZergMuthrokSpire:
					case UnitId::ZergOrbothOmlia:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ZergIrolIris:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergSire)
							|| Player::numberOfUnitsOfType(playerId, UnitId::ZergHive)
							|| Player::numberOfUnitsOfType(playerId, UnitId::ZergLair)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergHatchery))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergZorkizShroud:
					case UnitId::ZergMatravilNest:
					{
						if (TierUnlocked[RaceId::Zerg][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergNydusCanal:
					case UnitId::ZergGeszkathGrotto:
					case UnitId::ZergSovthrathMound:
					{
						if (TierUnlocked[RaceId::Zerg][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
				}
				return TECH_HIDDEN;
			}

			// Structure morphs
			case UnitId::ZergCreepColony:
			{
				if (unitId == UnitId::ZergSunkenColony
					|| unitId == UnitId::ZergSporeColony
					|| unitId == UnitId::ZergLarvalColony)
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergHatchery:
			{
				if (unitId == UnitId::ZergLair)
				{
					if (TierUnlocked[RaceId::Zerg][Tier2])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergQuazrokPool:
			{
				if (unitId == UnitId::ZergLiivralPond)
				{
					if (TierUnlocked[RaceId::Zerg][Tier2])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergHydrokDen:
			{
				if (unitId == UnitId::ZergGorgralSwamp)
				{
					if (TierUnlocked[RaceId::Zerg][Tier2])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergMuthrokSpire:
			{
				if (unitId == UnitId::ZergVithrathHaunt)
				{
					if (TierUnlocked[RaceId::Zerg][Tier2])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergIrolIris:
			{
				if (unitId == UnitId::ZergOthstolOviform)
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergLair:
			{
				if (unitId == UnitId::ZergHive)
				{
					if (TierUnlocked[RaceId::Zerg][Tier3])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergMatravilNest:
			{
				if (unitId == UnitId::ZergAlmakisAntre)
				{
					if (TierUnlocked[RaceId::Zerg][Tier3])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergZorkizShroud:
			{
				if (unitId == UnitId::ZergUltrokCavern)
				{
					if (TierUnlocked[RaceId::Zerg][Tier3])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergHive:
			{
				if (unitId == UnitId::ZergSire)
				{
					if (TierUnlocked[RaceId::Zerg][Tier4])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergOthstolOviform:
			{
				if (unitId == UnitId::ZergAlaszilArbor)
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergGeszkathGrotto:
			{
				if (unitId == UnitId::ZergAlkajChasm)
				{
					if (TierUnlocked[RaceId::Zerg][Tier4])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
		}
		return TECH_HIDDEN;
	}
}