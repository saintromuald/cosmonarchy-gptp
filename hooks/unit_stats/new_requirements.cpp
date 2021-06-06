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

		s32 addonId = -1; 
		if (unit->building.addon)
		{
			addonId = unit->building.addon->id;
		}

		switch (unit->id)
		{
			// TERRAN
			case UnitId::TerranMason:
			{
				switch (unitId)
				{
					case UnitId::TerranCommandCenter:
					case UnitId::TerranSupplyDepot:
					case UnitId::TerranBarracks:
					case UnitId::TerranFactory:
					case UnitId::TerranStarport:
					case UnitId::TerranScrapyard:
					case UnitId::TerranWatchdog:
					case UnitId::TerranBunker:
					{
						return TECH_ACTIVE;
					}
					case UnitId::TerranTreasury:
					case UnitId::TerranCaptaincy:
					case UnitId::TerranScienceFacility:
					{
						if (TierUnlocked[RaceId::Terran][Tier1])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranIronFoundry:
					case UnitId::TerranNaniteAssembly:
					{
						if (TierUnlocked[RaceId::Terran][Tier2])
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
					case UnitId::TerranComsatStation:
					{
						return TECH_ACTIVE;
					}
					case UnitId::TerranNuclearSilo:
					case UnitId::TerranQuarry:
					{
						if (TierUnlocked[RaceId::Terran][Tier1])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
				}
			} break;
			case UnitId::TerranTreasury:
			{
				switch (unitId)
				{
					case UnitId::TerranComsatStation:
					{
						return TECH_ACTIVE;
					}
					case UnitId::TerranNuclearSilo:
					case UnitId::TerranQuarry:
					{
						if (TierUnlocked[RaceId::Terran][Tier1])
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
					case UnitId::TerranMaverick:
					case UnitId::TerranHarakan:
					case UnitId::TerranCyprian:
					case UnitId::TerranMedbay:
					{
						return TECH_ACTIVE;
					}
					case UnitId::TerranCovertOps:
					{
						if (TierUnlocked[RaceId::Terran][Tier1])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranCleric:
					case UnitId::TerranShaman:
					{
						if (addonId == UnitId::TerranMedbay)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranEidolon:
					case UnitId::TerranSavant:
					{
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
					case UnitId::TerranVulture:
					case UnitId::TerranGoliath:
					case UnitId::TerranCyclops:
					{
						return TECH_ACTIVE;
					}
					case UnitId::TerranMachineShop:
					case UnitId::TerranSalvoYard:
					{
						if (TierUnlocked[RaceId::Terran][Tier1])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranPhalanxTankMode:
					case UnitId::TerranSouthpaw:
					{
						if (addonId == UnitId::TerranMachineShop)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranPaladin:
					case UnitId::TerranMadrigal:
					{
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
					case UnitId::TerranWraith:
					case UnitId::TerranGorgon:
					case UnitId::TerranValkyrie:
					case UnitId::TerranTrojan:
					{
						return TECH_ACTIVE;
					}
					case UnitId::TerranMunitionsBay:
					case UnitId::TerranControlTower:
					{
						if (TierUnlocked[RaceId::Terran][Tier1])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranAzazel:
					case UnitId::TerranSeraph:
					{
						if (addonId == UnitId::TerranControlTower)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranSalamander:
					case UnitId::TerranWyvern:
					{
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
					case UnitId::SharedCohort:
					case UnitId::SharedAspirant:
					case UnitId::TerranBlackjack:
					{
						return TECH_ACTIVE;
					}
					case UnitId::TerranFutureStation:
					{
						if (TierUnlocked[RaceId::Terran][Tier1])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranClaymore:
					case UnitId::TerranSiren:
					{
						if (addonId == UnitId::TerranFutureStation)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::SharedStriga:
					case UnitId::SharedLuminary:
					{
						if (addonId == UnitId::TerranTinkerersTower2)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::SharedTarasque:
					case UnitId::SharedAnticthon:
					{
						if (addonId == UnitId::TerranBioticBase2)
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
					case UnitId::TerranMadcap:
					case UnitId::TerranHeracles:
					case UnitId::TerranApostle:
					{
						return TECH_ACTIVE;
					}
					case UnitId::TerranGuildhall:
					{
						if (TierUnlocked[RaceId::Terran][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranOlympian:
					case UnitId::TerranAutocrat:
					{
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
					case UnitId::TerranPenumbra:
					case UnitId::TerranPazuzu:
					{
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
					return TECH_HIDDEN;
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
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossDidactTribune))
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
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ProtossSolarion:
			{
				if (unitId == UnitId::ProtossSteward)
				{
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
					case UnitId::ProtossDidactTribune:
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
					case UnitId::SharedAxitrilisk:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
							|| Player::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergZoryusthaleth:
					case UnitId::ZergLakizilisk:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergUltrakCavern)
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
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
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
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlaszilArbor))
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
					if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| Player::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
						|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
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
					if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergZoryusthaleth:
			{
				if (unitId == UnitId::ZergUltrakor)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergUltrakCavern))
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
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlaszilArbor))
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
					case UnitId::ZergIrolIris:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ZergZorkizShroud:
					case UnitId::ZergMatravilNest:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
							|| Player::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergNydusCanal:
					case UnitId::ZergGeszkathGrotto:
					case UnitId::ZergSovthrathMound:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
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
					if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| Player::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
						|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
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
					if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| Player::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
						|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
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
					if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| Player::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
						|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
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
					if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| Player::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
						|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
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
					if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
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
					if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergZorkizShroud:
			{
				if (unitId == UnitId::ZergUltrakCavern)
				{
					if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
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
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlaszilArbor))
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
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlaszilArbor))
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