#include "new_requirements.h"
#include <hook_tools.h>
#include "hooks/rally_point.h"

namespace hooks {
	enum {
		TECH_LOCKED = -1,
		TECH_HIDDEN = 0,
		TECH_ACTIVE = 1,
	};

	static bool player_has_unit_id(s32 player_id, u32 unit_id) {
		return Player::number_of_units_of_id(player_id, unit_id);
	}

	static bool player_has_completed_unit_id(s32 player_id, u32 unit_id) {
		return Player::number_of_completed_units_of_id(player_id, unit_id);
	}

	static bool player_has_completed_landed_unit_id(s32 player_id, u32 unit_id) {
		return Player::number_of_completed_landed_units_of_id(player_id, unit_id);
	}

	s32 create_unit_id_allowed(u16 unit_id, s32 player_id, bool ignore_creator)
	{
		*g_last_internal_error = InternalErrors::UnknownError0;
		if (unit_id == UnitId::None)
		{
			return false;
		}
		if (!g_unit_availability->available[player_id][unit_id])
		{
			return false;
		}

		enum
		{
			Tier1 = 0,
			Tier2,
			Tier3,
			Tier4,
			Tier5
		};
		bool TierUnlocked[3][5];
		TierUnlocked[RaceId::Zerg][Tier1] = true;
		TierUnlocked[RaceId::Zerg][Tier2] = player_has_completed_unit_id(player_id, UnitId::ZergIrolIris)
			|| player_has_unit_id(player_id, UnitId::ZergOthstolOviform)
			|| player_has_unit_id(player_id, UnitId::ZergAlaszilArbor);
		TierUnlocked[RaceId::Zerg][Tier3] = player_has_completed_unit_id(player_id, UnitId::ZergOthstolOviform)
			|| player_has_unit_id(player_id, UnitId::ZergAlaszilArbor);
		TierUnlocked[RaceId::Zerg][Tier4] = player_has_completed_unit_id(player_id, UnitId::ZergAlaszilArbor);
		TierUnlocked[RaceId::Zerg][Tier5] = true;

		TierUnlocked[RaceId::Terran][Tier1] = true;
		TierUnlocked[RaceId::Terran][Tier2] = player_has_completed_landed_unit_id(player_id, UnitId::TerranEngineeringBay);
		TierUnlocked[RaceId::Terran][Tier3] = player_has_completed_landed_unit_id(player_id, UnitId::TerranScienceFacility);
		TierUnlocked[RaceId::Terran][Tier4] = player_has_completed_landed_unit_id(player_id, UnitId::TerranQuantumInstitute);
		TierUnlocked[RaceId::Terran][Tier5] = true;

		TierUnlocked[RaceId::Protoss][Tier1] = true;
		TierUnlocked[RaceId::Protoss][Tier2] = true;
		TierUnlocked[RaceId::Protoss][Tier3] = true;
		TierUnlocked[RaceId::Protoss][Tier4] = true;
		TierUnlocked[RaceId::Protoss][Tier5] = true;

		// TERRAN
		// @Todo: Consider making AI check for landed structures as a safety, e.g. player_has_completed_landed_unit_id(player_id, UnitId::TerranCommandCenter)
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranMason) || player_has_completed_unit_id(player_id, UnitId::TerranTalos))
		{
			switch (unit_id)
			{
				case UnitId::TerranCommandCenter:
				case UnitId::TerranReservoir:
				case UnitId::TerranWatchdog:
				case UnitId::TerranBunker:
				{
					return true;
				} break;
				case UnitId::TerranBarracks:
				case UnitId::TerranFulcrum:
				case UnitId::TerranStarpad:
				case UnitId::TerranEngineeringBay:
				{
					return player_has_completed_landed_unit_id(player_id, UnitId::TerranCommandCenter);
				} break;
				case UnitId::TerranTreasury:
				case UnitId::TerranSentinel:
				case UnitId::TerranCaptaincy:
				case UnitId::TerranMantle:
				case UnitId::TerranStarport:
				case UnitId::TerranScienceFacility:
				{
					if (TierUnlocked[RaceId::Terran][Tier2])
					{
						return true;
					}
				} break;
				case UnitId::TerranParthenon:
				case UnitId::TerranQuantumInstitute:
				case UnitId::TerranApothecary:
				case UnitId::TerranIronFoundry:
				case UnitId::TerranNaniteAssembly:
				case UnitId::TerranIonCannon:
				{
					if (TierUnlocked[RaceId::Terran][Tier3])
					{
						return true;
					}
				} break;
			//	case UnitId::TerranCapitol:
				case UnitId::TerranIronCouncil:
				{
					if (TierUnlocked[RaceId::Terran][Tier5])
					{
						return true;
					}
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranCommandCenter))
		{
			switch (unit_id)
			{
				case UnitId::TerranMason:
				case UnitId::TerranComsatStation:
				{
					return true;
				} break;
				case UnitId::TerranMissileSilo:
				case UnitId::TerranQuarry:
				{
					if (TierUnlocked[RaceId::Terran][Tier2])
					{
						return true;
					}
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranParthenon))
		{
			switch (unit_id)
			{
				case UnitId::TerranTalos:
				case UnitId::TerranArmistice:
				case UnitId::TerranPythean:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranMissileSilo))
		{
			switch (unit_id)
			{
				case UnitId::TerranNuclearMissile:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranQuarry))
		{
			switch (unit_id)
			{
				case UnitId::TerranMason:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranTreasury))
		{
			switch (unit_id)
			{
				case UnitId::TerranComsatStation:
				{
					return true;
				} break;
				case UnitId::TerranMissileSilo:
				case UnitId::TerranQuarry:
				{
					if (TierUnlocked[RaceId::Terran][Tier2])
					{
						return true;
					}
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranBarracks))
		{
			switch (unit_id)
			{
				case UnitId::TerranMedbay:
				{
					return true;
				} break;
				case UnitId::TerranCovertOps:
				{
					if (TierUnlocked[RaceId::Terran][Tier2])
					{
						return true;
					}
				} break;
				case UnitId::TerranMaverick:
				case UnitId::TerranHarakan:
				case UnitId::TerranCyprian:
				{
					return true;
				} break;
				case UnitId::TerranCleric:
				case UnitId::TerranShaman:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranMedbay))
					{
						return true;
					}
				} break;
				case UnitId::TerranEidolon:
				case UnitId::TerranSavant:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranCovertOps))
					{
						return true;
					}
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranCaptaincy))
		{
			switch (unit_id)
			{
				case UnitId::TerranGuildhall:
				{
					if (TierUnlocked[RaceId::Terran][Tier3])
					{
						return true;
					}
				} break;
				case UnitId::TerranTinkerersTower:
				case UnitId::TerranMadcap:
				case UnitId::TerranHeracles:
				case UnitId::TerranApostle:
				{
					return true;
				} break;
				case UnitId::TerranSiren:
				case UnitId::TerranStriga:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranTinkerersTower))
					{
						return true;
					}
				} break;
				case UnitId::TerranOlympian:
				case UnitId::TerranAutocrat:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranGuildhall))
					{
						return true;
					}
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranApothecary))
		{
			switch (unit_id)
			{
				case UnitId::TerranDilettante:
				case UnitId::TerranSevenstep:
				case UnitId::TerranHypnagogue:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranIronCouncil))
		{
			switch (unit_id)
			{
				case UnitId::TerranThroneroom:
				{
					if (TierUnlocked[RaceId::Terran][Tier5])
					{
						return true;
					}
				} break;
				case UnitId::TerranAmphitheater:
				case UnitId::TerranAmbassador:
				case UnitId::TerranHeir:
				case UnitId::TerranHorseman:
				{
					return true;
				} break;
				case UnitId::TerranThespian:
				case UnitId::TerranSilvertongue:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranAmphitheater))
					{
						return true;
					}
				} break;
				case UnitId::TerranEmperor:
				case UnitId::TerranJester:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranThroneroom))
					{
						return true;
					}
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranFulcrum))
		{
			switch (unit_id)
			{
				case UnitId::TerranMachineShop:
				{
					if (TierUnlocked[RaceId::Terran][Tier2])
					{
						return true;
					}
				} break;
				case UnitId::TerranScrapyard:
				case UnitId::TerranVulture:
				case UnitId::TerranGoliath:
				case UnitId::TerranCyclops:
				{
					return true;
				} break;
				case UnitId::TerranBlackjack:
				case UnitId::TerranMatador:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranScrapyard))
					{
						return true;
					}
				} break;
				case UnitId::TerranPhalanxTankMode:
				case UnitId::TerranMadrigal:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranMachineShop))
					{
						return true;
					}
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranMantle))
		{
			switch (unit_id)
			{
				case UnitId::TerranRamesses:
				case UnitId::TerranDurendal:
				case UnitId::TerranCataphract:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranIronFoundry))
		{
			switch (unit_id)
			{
				case UnitId::TerranBioticBase:
				{
					if (TierUnlocked[RaceId::Terran][Tier4])
					{
						return true;
					}
				} break;
				case UnitId::TerranFutureStation:
				case UnitId::TerranPaladin:
				case UnitId::TerranCuirass:
				case UnitId::TerranPazuzu:
				{
					return true;
				} break;
				case UnitId::TerranClaymore:
				case UnitId::TerranPenumbra:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranFutureStation))
					{
						return true;
					}
				}
				case UnitId::TerranAnticthon:
				case UnitId::TerranAion:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranBioticBase))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranStarpad))
		{
			switch (unit_id)
			{
				case UnitId::TerranWraith:
				case UnitId::TerranAncile:
				case UnitId::TerranTrojan:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranStarport))
		{
			switch (unit_id)
			{
				case UnitId::TerranMunitionsBay:
				{
					return true;
				} break;
				case UnitId::TerranControlTower:
				{
					if (TierUnlocked[RaceId::Terran][Tier3])
					{
						return true;
					}
				} break;
				case UnitId::TerranWyvern:
				case UnitId::TerranGorgon:
				case UnitId::TerranValkyrie:
				{
					return true;
				} break;
				case UnitId::TerranAzazel:
				case UnitId::TerranSeraph:
				{
					if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranControlTower))
					{
						return true;
					}
				} break;
				case UnitId::TerranSundog:
				case UnitId::TerranSalamander:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranMunitionsBay))
					{
						return true;
					}
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranNaniteAssembly))
		{
			switch (unit_id)
			{
				case UnitId::TerranParticleAccelerator:
				{
					if (TierUnlocked[RaceId::Terran][Tier4])
					{
						return true;
					}
				} break;
				case UnitId::TerranAerie:
				case UnitId::TerranCentaur:
				case UnitId::TerranMinotaur:
				case UnitId::TerranMagnetar:
				{
					return true;
				} break;
				case UnitId::TerranPegasus:
				case UnitId::TerranHippogriff:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranAerie))
					{
						return true;
					}
				} break;
				case UnitId::TerranOberon:
				case UnitId::TerranPhobos:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranParticleAccelerator))
					{
						return true;
					}
				} break;
			}
		}

		// PROTOSS
		// Units
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossNexus))
		{
			if (unit_id == UnitId::ProtossScribe)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossEmbassy))
		{
			switch (unit_id)
			{
				case UnitId::ProtossBulwark:
				case UnitId::ProtossEnvoy:
				case UnitId::ProtossWitness:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossStridentStratum))
		{
			switch (unit_id)
			{
				case UnitId::ProtossArtisan:
				case UnitId::ProtossClarion:
				case UnitId::ProtossMonogram:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossGateway))
		{
			switch (unit_id)
			{
				case UnitId::ProtossZealot:
				case UnitId::ProtossDracadin:
				case UnitId::ProtossEcclesiast:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossCenotaph))
		{
			switch (unit_id)
			{
				case UnitId::ProtossLegionnaire:
				case UnitId::ProtossHierophant:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossGrandLibrary))
		{
			switch (unit_id)
			{
				case UnitId::ProtossAmaranth:
				case UnitId::ProtossHerald:
				case UnitId::ProtossAtreus:
				case UnitId::ProtossCantavis:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossRogueGallery))
		{
			switch (unit_id)
			{
			case UnitId::ProtossVagrant:
			case UnitId::ProtossCabalist:
			case UnitId::ProtossBarghest:
			{
				return true;
			} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossProstrationStage))
		{
			switch (unit_id)
			{
			case UnitId::ProtossPariah:
			case UnitId::ProtossPotentate:
			case UnitId::ProtossSycophant:
			{
				return true;
			} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossSanctumOfSorrow))
		{
			switch (unit_id)
			{
			case UnitId::ProtossCharlatan:
			case UnitId::ProtossMindTyrant:
			{
				return true;
			} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossAncestralArchives))
		{
			switch (unit_id)
			{
				case UnitId::ProtossArchon:
				case UnitId::ProtossOptecton:
				case UnitId::ProtossPatriarch:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossHallOfHeretics))
		{
			switch (unit_id)
			{
				case UnitId::ProtossApostate:
				case UnitId::ProtossAugur:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossLattice))
		{
			switch (unit_id)
			{
				case UnitId::ProtossVassal:
				case UnitId::ProtossSimulacrum:
				case UnitId::ProtossManifold:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossAutomatonRegister))
		{
			switch (unit_id)
			{
				case UnitId::ProtossGolem:
				case UnitId::ProtossIdol:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossArdentAuthority))
		{
			switch (unit_id)
			{
				case UnitId::ProtossServitor:
				case UnitId::ProtossPositron:
				case UnitId::ProtossAnalogue:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossSequesteredIcon))
		{
			switch (unit_id)
			{
				case UnitId::ProtossAccantorOld:
				case UnitId::ProtossArchitect:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossAuxiliaryOasis))
		{
			switch (unit_id)
			{
				case UnitId::ProtossMainstay:
				case UnitId::ProtossMaven:
				case UnitId::ProtossCloudform:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossUnitaryUnion))
		{
			switch (unit_id)
			{
				case UnitId::ProtossVibrance:
				case UnitId::ProtossOscilliant:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossSyntheticSynod))
		{
			switch (unit_id)
			{
				case UnitId::ProtossDemiurge:
				case UnitId::ProtossEmpress:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossStargate))
		{
			switch (unit_id)
			{
				case UnitId::ProtossAurora:
				case UnitId::ProtossPanoptus:
				case UnitId::ProtossLanifect:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossAstralOmen))
		{
			switch (unit_id)
			{
				case UnitId::ProtossGladius:
				case UnitId::ProtossExemplar:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossArgosy))
		{
			switch (unit_id)
			{
				case UnitId::ProtossMagister:
				case UnitId::ProtossEpigraph:
				case UnitId::ProtossDidact:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossFleetBeacon))
		{
			switch (unit_id)
			{
				case UnitId::ProtossEmpyrean:
				case UnitId::ProtossSolarion:
				{
					return true;
				} break;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossMonumentOfSin))
		{
			switch (unit_id)
			{
				case UnitId::ProtossAnthelion:
				case UnitId::ProtossStarSovereign:
				{
					return true;
				} break;
			}
		}

		// Morphs and subunits
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossAccantorOld))
		{
			if (unit_id == UnitId::ProtossPlasmaShell)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossSolarion))
		{
			if (unit_id == UnitId::ProtossSteward)
			{
				return true;
			}
		}

		// Structures
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossScribe) || player_has_completed_unit_id(player_id, UnitId::ProtossArtisan))
		{
			switch (unit_id)
			{
					case UnitId::ProtossNexus:
					case UnitId::ProtossPylon:
					case UnitId::ProtossAquifer:
					case UnitId::ProtossWarden:
					case UnitId::ProtossMatrix:
					case UnitId::ProtossEmbassy:
					case UnitId::ProtossCrucible:
					case UnitId::ProtossStridentStratum:
					case UnitId::ProtossGateway:
					case UnitId::ProtossCenotaph:
					case UnitId::ProtossGrandLibrary:
					case UnitId::ProtossRogueGallery:
					case UnitId::ProtossProstrationStage:
					case UnitId::ProtossSanctumOfSorrow:
					case UnitId::ProtossAncestralArchives:
					case UnitId::ProtossHallOfHeretics:
					case UnitId::ProtossLattice:
					case UnitId::ProtossAutomatonRegister:
					case UnitId::ProtossArdentAuthority:
					case UnitId::ProtossSequesteredIcon:
					case UnitId::ProtossAuxiliaryOasis:
					case UnitId::ProtossUnitaryUnion:
					case UnitId::ProtossSyntheticSynod:
					case UnitId::ProtossStargate:
					case UnitId::ProtossAstralOmen:
					case UnitId::ProtossArgosy:
					case UnitId::ProtossCosmicAltar:
					case UnitId::ProtossFleetBeacon:
					case UnitId::ProtossMonumentOfSin:
				{
					return true;
				} break;
			}
		}

		// ZERG
		// Units
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergLarva))
		{
			switch (unit_id)
			{
				case UnitId::ZergDroleth:
				case UnitId::ZergOvileth:
				{
					return true;
				} break;
				case UnitId::ZergOthstoleth:
				{
					if (TierUnlocked[RaceId::Zerg][Tier3])
					{
						return true;
					}
				} break;
				case UnitId::ZergZethrokor:
				case UnitId::ZergQuazilisk:
				case UnitId::ZergNathrokor:
				{
					if (player_has_unit_id(player_id, UnitId::ZergLiivralPond)
						|| player_has_completed_unit_id(player_id, UnitId::ZergQuazrokPool))
					{
						return true;
					}
				} break;
				case UnitId::ZergMutalisk:
				case UnitId::ZergProtathalor:
				case UnitId::ZergCikralisk:
				{
					if (player_has_unit_id(player_id, UnitId::ZergVithrathHaunt)
						|| player_has_completed_unit_id(player_id, UnitId::ZergMuthrokSpire))
					{
						return true;
					}
				} break;
				case UnitId::ZergAxitrilisk:
				case UnitId::ZergMortothrokor:
				{
					if (player_has_unit_id(player_id, UnitId::ZergAxtothAxis)
						|| player_has_completed_unit_id(player_id, UnitId::ZergBegrozithBarrow))
					{
						return true;
					}
				} break;
				case UnitId::ZergHydralisk:
				case UnitId::ZergIzirokor:
				case UnitId::ZergSkithrokor:
				{
					if (player_has_unit_id(player_id, UnitId::ZergGorgralSwamp)
						|| player_has_completed_unit_id(player_id, UnitId::ZergHydrokDen))
					{
						return true;
					}
				} break;
				case UnitId::ZergKagralisk:
				case UnitId::ZergEvigrilisk:
				case UnitId::ZergAlmaksalisk:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergIzkagIteth))
					{
						return true;
					}
				} break;
				case UnitId::ZergZoryusthaleth:
				case UnitId::ZergGorgrokor:
				case UnitId::ZergLakizilisk:
				{
					if (player_has_unit_id(player_id, UnitId::ZergUltrokCavern)
						|| player_has_completed_unit_id(player_id, UnitId::ZergZorkizShroud))
					{
						return true;
					}
				} break;
				case UnitId::ZergKonvilisk:
				case UnitId::ZergVilgorokor:
				case UnitId::ZergMatraleth:
				{
					if (player_has_unit_id(player_id, UnitId::ZergAlmakisAntre)
						|| player_has_completed_unit_id(player_id, UnitId::ZergMatravilNest))
					{
						return true;
					}
				} break;
				case UnitId::ZergKeskathalor:
				case UnitId::ZergGeszithalor:
				{
					if (player_has_unit_id(player_id, UnitId::ZergAlkajChasm)
						|| player_has_completed_unit_id(player_id, UnitId::ZergGeszkathGrotto))
					{
						return true;
					}
				} break;
				case UnitId::ZergIsthrathaleth:
				case UnitId::ZergElthrazalisk:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergElthisthMound))
					{
						return true;
					}
				} break;
			}
		}

		// Morphs
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergDroleth) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergGosvileth)
			{
				if (TierUnlocked[RaceId::Zerg][Tier3])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergOvileth) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergIroleth)
			{
				if (TierUnlocked[RaceId::Zerg][Tier2])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergOthstoleth) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergAlaszileth)
			{
				if (TierUnlocked[RaceId::Zerg][Tier4])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergZethrokor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergVorvrokor)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergQuazilisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergLiiralisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergLiivralPond))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergNathrokor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergKalkalisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergLiivralPond))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergCikralisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergGhitorokor)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergVithrathHaunt))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergProtathalor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergSovroleth)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergVithrathHaunt))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMutalisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergTosgrilisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergVithrathHaunt))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergAxitrilisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergBegrothilisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergBegrozithBarrow))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMortothrokor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergOztukrokor)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergBegrozithBarrow))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergHydralisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergBactalisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergGorgralSwamp))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergIzirokor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergTethzorokor)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergGorgralSwamp))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergSkithrokor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergVithrilisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergGorgralSwamp))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergKagralisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergCherbrathalor)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergChokralgantChazsal))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergEvigrilisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergOkrimnilisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergChokralgantChazsal))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergAlmaksalisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergAlgoztalisk)
			{
				if (TierUnlocked[RaceId::Zerg][Tier4])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergKeskathalor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergKethrostalor)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergAlkajChasm))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergGeszithalor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergAlkajelisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergAlkajChasm))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergZoryusthaleth) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergUltrokor)
			{
				if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergUltrokCavern))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergLakizilisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergZobriolisk)
			{
				if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergUltrokCavern))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergGorgrokor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergZarcavrokor)
			{
				if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergUltrokCavern))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergKonvilisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergTetcaerokor)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergVilgorokor) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergSelkerilisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMatraleth) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergAkistrokor)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergIsthrathaleth) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergOgroszaleth)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergRakzogthRoost))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergElthrazalisk) || player_has_completed_unit_id(player_id, UnitId::ZergOcsalLarva))
		{
			if (unit_id == UnitId::ZergRakzilisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergRakzogthRoost))
				{
					return true;
				}
			}
		}

		// Structures
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergDroleth) || player_has_completed_unit_id(player_id, UnitId::ZergGosvileth))
		{
			switch (unit_id)
			{
				case UnitId::ZergHachirosk:
				case UnitId::ZergCagrantColony:
				case UnitId::ZergExcisant:
				case UnitId::ZergQuazrokPool:
				case UnitId::ZergHydrokDen:
				case UnitId::ZergZorkizShroud:
				case UnitId::ZergIrolIris:
				{
					return true;
				} break;
				case UnitId::ZergMuthrokSpire:
				case UnitId::ZergIzkagIteth:
				case UnitId::ZergMatravilNest:
				{
					if (TierUnlocked[RaceId::Zerg][Tier2])
					{
						return true;
					}
				} break;
				case UnitId::ZergNydusCesiant:
				case UnitId::ZergAxtothAxis:
				case UnitId::ZergGeszkathGrotto:
				case UnitId::ZergElthisthMound:
				{
					if (TierUnlocked[RaceId::Zerg][Tier3])
					{
						return true;
					}
				} break;
			}
		}

		// Structure morphs
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergCagrantColony))
		{
			if (unit_id == UnitId::ZergSurkithColony
				|| unit_id == UnitId::ZergSpraithColony
				|| unit_id == UnitId::ZergLarvoskColony)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergHachirosk))
		{
			if (unit_id == UnitId::ZergCaphrolosk)
			{
				if (TierUnlocked[RaceId::Zerg][Tier2])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergQuazrokPool))
		{
			if (unit_id == UnitId::ZergLiivralPond)
			{
				if (TierUnlocked[RaceId::Zerg][Tier2])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergHydrokDen))
		{
			if (unit_id == UnitId::ZergGorgralSwamp)
			{
				if (TierUnlocked[RaceId::Zerg][Tier2])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMuthrokSpire))
		{
			if (unit_id == UnitId::ZergVithrathHaunt)
			{
				if (TierUnlocked[RaceId::Zerg][Tier2])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergAxtothAxis))
		{
			if (unit_id == UnitId::ZergBegrozithBarrow)
			{
				if (TierUnlocked[RaceId::Zerg][Tier4])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergIrolIris))
		{
			if (unit_id == UnitId::ZergOthstolOviform)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergHachirosk))
		{
			if (unit_id == UnitId::ZergGathtelosk)
			{
				if (TierUnlocked[RaceId::Zerg][Tier3])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMatravilNest))
		{
			if (unit_id == UnitId::ZergAlmakisAntre)
			{
				if (TierUnlocked[RaceId::Zerg][Tier3])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergZorkizShroud))
		{
			if (unit_id == UnitId::ZergUltrokCavern)
			{
				if (TierUnlocked[RaceId::Zerg][Tier3])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergElthisthMound))
		{
			if (unit_id == UnitId::ZergRakzogthRoost)
			{
				if (TierUnlocked[RaceId::Zerg][Tier4])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergCaphrolosk))
		{
			if (unit_id == UnitId::ZergSibraelosk)
			{
				if (TierUnlocked[RaceId::Zerg][Tier4])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergGathtelosk))
		{
			if (unit_id == UnitId::ZergNephrilosk)
			{
				if (TierUnlocked[RaceId::Zerg][Tier4])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergOthstolOviform))
		{
			if (unit_id == UnitId::ZergAlaszilArbor)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergAlaszilArbor))
		{
			if (unit_id == UnitId::ZergOrbothOmlia)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergIzkagIteth))
		{
			if (unit_id == UnitId::ZergChokralgantChazsal)
			{
				if (TierUnlocked[RaceId::Zerg][Tier3])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergGeszkathGrotto))
		{
			if (unit_id == UnitId::ZergAlkajChasm)
			{
				if (TierUnlocked[RaceId::Zerg][Tier4])
				{
					return true;
				}
			}
		}
		return false;
	}

	s32 create_unit_allowed_partial(u16 unit_id, Unit* unit, s32 player_id)
	{
		*g_last_internal_error = InternalErrors::UnknownError0;
		if (unit_id == UnitId::None)
		{
			*g_last_internal_error = InternalErrors::UnknownError16;
			return TECH_HIDDEN;
		}
		if (player_id != unit->player)
		{
			*g_last_internal_error = InternalErrors::UnknownError1;
			return TECH_HIDDEN;
		}
		if (!(unit->status & UnitStatus::Completed))
		{
			*g_last_internal_error = InternalErrors::UnknownError20;
			return TECH_HIDDEN;
		}
		if (unit->is_frozen())
		{
			*g_last_internal_error = InternalErrors::UnitFrozen;
			return TECH_HIDDEN;
		}
		if (!g_unit_availability->available[unit->player][unit_id])
		{
			*g_last_internal_error = InternalErrors::UnknownError2;
			return TECH_HIDDEN;
		}
		if (unit->is_queue_slot_active(4))
		{
			*g_last_internal_error = InternalErrors::UnknownError3;
			return TECH_HIDDEN;
		}
		if (unit->status & UnitStatus::IsHallucination)
		{
			*g_last_internal_error = InternalErrors::UnknownError22;
			return TECH_HIDDEN;
		}

		enum
		{
			Tier1 = 0,
			Tier2,
			Tier3,
			Tier4,
			Tier5
		};

		bool TierUnlocked[3][5];
		TierUnlocked[RaceId::Zerg][Tier1] = true;
		TierUnlocked[RaceId::Zerg][Tier2] = player_has_completed_unit_id(player_id, UnitId::ZergIrolIris)
			|| player_has_unit_id(player_id, UnitId::ZergOthstolOviform)
			|| player_has_unit_id(player_id, UnitId::ZergAlaszilArbor)
			|| player_has_unit_id(player_id, UnitId::ZergOrbothOmlia);
		TierUnlocked[RaceId::Zerg][Tier3] = player_has_completed_unit_id(player_id, UnitId::ZergOthstolOviform)
			|| player_has_unit_id(player_id, UnitId::ZergAlaszilArbor)
			|| player_has_unit_id(player_id, UnitId::ZergOrbothOmlia);
		TierUnlocked[RaceId::Zerg][Tier4] = player_has_completed_unit_id(player_id, UnitId::ZergAlaszilArbor)
			|| player_has_unit_id(player_id, UnitId::ZergOrbothOmlia);
		TierUnlocked[RaceId::Zerg][Tier5] = player_has_completed_unit_id(player_id, UnitId::ZergOrbothOmlia);

		TierUnlocked[RaceId::Terran][Tier1] = true;
		TierUnlocked[RaceId::Terran][Tier2] = player_has_completed_landed_unit_id(player_id, UnitId::TerranEngineeringBay);
		TierUnlocked[RaceId::Terran][Tier3] = player_has_completed_landed_unit_id(player_id, UnitId::TerranScienceFacility);
		TierUnlocked[RaceId::Terran][Tier4] = player_has_completed_landed_unit_id(player_id, UnitId::TerranQuantumInstitute);
		TierUnlocked[RaceId::Terran][Tier5] = true; // player_has_completed_landed_unit_id(player_id, UnitId::TerranCapitol);

		TierUnlocked[RaceId::Protoss][Tier1] = true;
		TierUnlocked[RaceId::Protoss][Tier2] = true;
		TierUnlocked[RaceId::Protoss][Tier3] = true;
		TierUnlocked[RaceId::Protoss][Tier4] = true;
		TierUnlocked[RaceId::Protoss][Tier5] = true;

		if (helper::is_constructing_addon(unit))
		{
			return TECH_HIDDEN;
		}

		bool has_addon = false;
		s32 addon_id = -1; 
		if (unit->building.addon)
		{
			has_addon = true;
			addon_id = unit->building.addon->id;
		}
		bool in_air = unit->status & UnitStatus::InAir;
		bool is_training = unit->build_queue[unit->build_queue_slot] != UnitId::None;
		bool is_adding_on = units_dat::base_property[unit->build_queue[unit->build_queue_slot]] & UnitProperty::Addon;

		switch (unit->id)
		{
			// TERRAN
			case UnitId::TerranMason:
			case UnitId::TerranTalos:
			{
				if (unit->main_order_id == OrderId::ConstructingBuilding) {
					return TECH_HIDDEN;
				}

				switch (unit_id)
				{
					case UnitId::TerranCommandCenter:
					case UnitId::TerranReservoir:
					case UnitId::TerranWatchdog:
					case UnitId::TerranBunker:
					{
						return TECH_ACTIVE;
					}
					case UnitId::TerranBarracks:
					case UnitId::TerranFulcrum:
					case UnitId::TerranStarpad:
					case UnitId::TerranEngineeringBay:
					{
						if (player_has_completed_landed_unit_id(player_id, UnitId::TerranCommandCenter))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranTreasury:
					case UnitId::TerranSentinel:
					case UnitId::TerranCaptaincy:
					case UnitId::TerranMantle:
					case UnitId::TerranStarport:
					case UnitId::TerranScienceFacility:
					{
						if (TierUnlocked[RaceId::Terran][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranParthenon:
					case UnitId::TerranQuantumInstitute:
					case UnitId::TerranApothecary:
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
				//	case UnitId::TerranCapitol:
					case UnitId::TerranIronCouncil:
					{
						if (TierUnlocked[RaceId::Terran][Tier4])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
				}
			}
			case UnitId::TerranCommandCenter:
			{
				switch (unit_id)
				{
					case UnitId::TerranMason:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranComsatStation:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranMissileSilo:
					case UnitId::TerranQuarry:
					{
						if (has_addon || is_training)
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
			case UnitId::TerranMissileSilo:
			{
				switch (unit_id)
				{
					case UnitId::TerranNuclearMissile:
					{
						if (!unit->building.silo.nuke && unit->build_queue[unit->build_queue_slot] == UnitId::None)
						{
							return TECH_ACTIVE;
						}
					}
				}
			} break;
			case UnitId::TerranQuarry:
			{
				switch (unit_id)
				{
					case UnitId::TerranMason:
					{
						return TECH_ACTIVE;
					}
				}
			} break;
			case UnitId::TerranTreasury:
			{
				switch (unit_id)
				{
					case UnitId::TerranComsatStation:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranMissileSilo:
					case UnitId::TerranQuarry:
					{
						if (has_addon || is_training)
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
			case UnitId::TerranParthenon:
			{
				switch (unit_id)
				{
					case UnitId::TerranTalos:
					case UnitId::TerranArmistice:
					case UnitId::TerranPythean:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
				}
			} break;
			case UnitId::TerranBarracks:
			{
				switch (unit_id)
				{
					case UnitId::TerranMedbay:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranCovertOps:
					{
						if (has_addon || is_training)
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
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranCleric:
					case UnitId::TerranShaman:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranMedbay)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranEidolon:
					case UnitId::TerranSavant:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranCovertOps)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
				}
				return TECH_HIDDEN;
			} break;
			case UnitId::TerranCaptaincy:
			{
				switch (unit_id)
				{
					case UnitId::TerranGuildhall:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranTinkerersTower:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranMadcap:
					case UnitId::TerranHeracles:
					case UnitId::TerranApostle:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranSiren:
					case UnitId::TerranStriga:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranTinkerersTower)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranOlympian:
					case UnitId::TerranAutocrat:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranGuildhall)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
				}
			} break;
			case UnitId::TerranApothecary:
			{
				switch (unit_id)
					{
					case UnitId::TerranDilettante:
					case UnitId::TerranSevenstep:
					case UnitId::TerranHypnagogue:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
				}
			}
			case UnitId::TerranIronCouncil:
			{
				switch (unit_id)
				{
					case UnitId::TerranThroneroom:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier5])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranAmphitheater:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranAmbassador:
					case UnitId::TerranHeir:
					case UnitId::TerranHorseman:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranThespian:
					case UnitId::TerranSilvertongue:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranAmphitheater)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranEmperor:
					case UnitId::TerranJester:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranThroneroom)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
				}
			} break;
			case UnitId::TerranFulcrum:
			{
				switch (unit_id)
				{
					case UnitId::TerranMachineShop:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranScrapyard:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranVulture:
					case UnitId::TerranGoliath:
					case UnitId::TerranCyclops:
					{
						if (in_air || is_adding_on)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranBlackjack:
					case UnitId::TerranMatador:
					{
						if (in_air || is_adding_on)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranScrapyard)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranPhalanxTankMode:
					case UnitId::TerranMadrigal:
					{
						if (in_air || is_adding_on)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranMachineShop)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
				}
			} break;
			case UnitId::TerranMantle:
			{
				switch (unit_id)
				{
					case UnitId::TerranRamesses:
					case UnitId::TerranDurendal:
					case UnitId::TerranCataphract:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
				}
			}
			case UnitId::TerranIronFoundry:
			{
				switch (unit_id)
				{
					case UnitId::TerranBioticBase:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier4])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranFutureStation:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranPaladin:
					case UnitId::TerranCuirass:
					case UnitId::TerranPazuzu:
					{
						if (in_air || is_adding_on)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranClaymore:
					case UnitId::TerranPenumbra:
					{
						if (in_air || is_adding_on)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranFutureStation)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranAnticthon:
					case UnitId::TerranAion:
					{
						if (in_air || is_adding_on)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranBioticBase)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
				}
			} break;
			case UnitId::TerranStarpad:
			{
				switch (unit_id)
				{
					case UnitId::TerranWraith:
					case UnitId::TerranAncile:
					case UnitId::TerranTrojan:
					{
						if (in_air)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
				}
			}
			case UnitId::TerranStarport:
			{
				switch (unit_id)
				{
					case UnitId::TerranMunitionsBay:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranControlTower:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranGorgon:
					case UnitId::TerranWyvern:
					case UnitId::TerranValkyrie:
					{
						if (in_air || is_adding_on)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranSundog:
					case UnitId::TerranSalamander:
					{
						if (in_air || is_adding_on)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranMunitionsBay)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranAzazel:
					case UnitId::TerranSeraph:
					{
						if (in_air || is_adding_on)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranControlTower)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
				}
			} break;
			case UnitId::TerranNaniteAssembly:
			{
				switch (unit_id)
				{
					case UnitId::TerranParticleAccelerator:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier4])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::TerranAerie:
					{
						if (has_addon || is_training)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranCentaur:
					case UnitId::TerranMinotaur:
					case UnitId::TerranMagnetar:
					{
						if (in_air || is_adding_on)
						{
							return TECH_HIDDEN;
						}
						return TECH_ACTIVE;
					}
					case UnitId::TerranPegasus:
					case UnitId::TerranHippogriff:
					{
						if (in_air || is_adding_on)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranAerie)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
					case UnitId::TerranOberon:
					case UnitId::TerranPhobos:
					{
						if (in_air || is_adding_on)
						{
							return TECH_HIDDEN;
						}
						if (addon_id == UnitId::TerranParticleAccelerator)
						{
							return TECH_ACTIVE;
						}
						return TECH_HIDDEN;
					}
				}
			} break;
			
			// PROTOSS
			// Units
			case UnitId::ProtossNexus:
			{
				if (unit_id == UnitId::ProtossScribe)
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ProtossEmbassy:
			{
				switch (unit_id)
				{
					case UnitId::ProtossBulwark:
					case UnitId::ProtossEnvoy:
					case UnitId::ProtossWitness:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ProtossStridentStratum:
			{
				switch (unit_id)
				{
					case UnitId::ProtossArtisan:
					case UnitId::ProtossClarion:
					case UnitId::ProtossMonogram:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ProtossGateway:
			{
				switch (unit_id)
				{
					case UnitId::ProtossZealot:
					case UnitId::ProtossDracadin:
					case UnitId::ProtossEcclesiast:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossCenotaph:
			{
				switch (unit_id)
				{
					case UnitId::ProtossLegionnaire:
					case UnitId::ProtossHierophant:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossGrandLibrary:
			{
				switch (unit_id)
				{
					case UnitId::ProtossAmaranth:
					case UnitId::ProtossHerald:
					case UnitId::ProtossAtreus:
					case UnitId::ProtossCantavis:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossRogueGallery:
			{
				switch (unit_id)
				{
				case UnitId::ProtossVagrant:
				case UnitId::ProtossCabalist:
				case UnitId::ProtossBarghest:
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossProstrationStage:
			{
				switch (unit_id)
				{
				case UnitId::ProtossPariah:
				case UnitId::ProtossPotentate:
				case UnitId::ProtossSycophant:
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossSanctumOfSorrow:
			{
				switch (unit_id)
				{
				case UnitId::ProtossCharlatan:
				case UnitId::ProtossMindTyrant:
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossAncestralArchives:
			{
				switch (unit_id)
				{
					case UnitId::ProtossArchon:
					case UnitId::ProtossOptecton:
					case UnitId::ProtossPatriarch:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossHallOfHeretics:
			{
				switch (unit_id)
				{
				case UnitId::ProtossApostate:
				case UnitId::ProtossAugur:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossLattice:
			{
				switch (unit_id)
				{
					case UnitId::ProtossVassal:
					case UnitId::ProtossSimulacrum:
					case UnitId::ProtossManifold:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossAutomatonRegister:
			{
				switch (unit_id)
				{
					case UnitId::ProtossGolem:
					case UnitId::ProtossIdol:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossArdentAuthority:
			{
				switch (unit_id)
				{
					case UnitId::ProtossServitor:
					case UnitId::ProtossPositron:
					case UnitId::ProtossAnalogue:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossSequesteredIcon:
			{
				switch (unit_id)
				{
					case UnitId::ProtossAccantorOld:
					case UnitId::ProtossArchitect:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossAuxiliaryOasis:
			{
				switch (unit_id)
				{
				case UnitId::ProtossMainstay:
				case UnitId::ProtossMaven:
				case UnitId::ProtossCloudform:
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossUnitaryUnion:
			{
				switch (unit_id)
				{
				case UnitId::ProtossVibrance:
				case UnitId::ProtossOscilliant:
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossSyntheticSynod:
			{
				switch (unit_id)
				{
					case UnitId::ProtossDemiurge:
					case UnitId::ProtossEmpress:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossStargate:
			{
				switch (unit_id)
				{
					case UnitId::ProtossAurora:
					case UnitId::ProtossPanoptus:
					case UnitId::ProtossLanifect:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossAstralOmen:
			{
				switch (unit_id)
				{
					case UnitId::ProtossGladius:
					case UnitId::ProtossExemplar:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossArgosy:
			{
				switch (unit_id)
				{
					case UnitId::ProtossMagister:
					case UnitId::ProtossEpigraph:
					case UnitId::ProtossDidact:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossFleetBeacon:
			{
				switch (unit_id)
				{
					case UnitId::ProtossEmpyrean:
					case UnitId::ProtossSolarion:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossMonumentOfSin:
			{
				switch (unit_id)
				{
					case UnitId::ProtossAnthelion:
					case UnitId::ProtossStarSovereign:
					{
						return TECH_ACTIVE;
					}
					return TECH_HIDDEN;
				}
			}

			// Morphs and subunits
			case UnitId::ProtossAccantorOld:
			{
				if (unit_id == UnitId::ProtossPlasmaShell)
				{
					u32 shell_count = unit->unit_with_hangar.in_hangar_count + unit->unit_with_hangar.out_hangar_count;
					for (u32 i = 0; i < 5; ++i)
					{
						shell_count += unit->build_queue[i] != UnitId::None;
					}
					if (shell_count >= 5)
					{
						return TECH_HIDDEN;
					}
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ProtossSolarion:
			{
				if (unit_id == UnitId::ProtossSteward)
				{
					u32 steward_count = unit->unit_with_hangar.in_hangar_count + unit->unit_with_hangar.out_hangar_count;
					for (u32 i = 0; i < 5; ++i)
					{
						steward_count += unit->build_queue[i] != UnitId::None;
					}
					if (steward_count >= 12)
					{
						return TECH_HIDDEN;
					}
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}

			// Structures
			case UnitId::ProtossScribe:
			case UnitId::ProtossArtisan:
			{
				switch (unit_id)
				{
					case UnitId::ProtossNexus:
					case UnitId::ProtossPylon:
					case UnitId::ProtossAquifer:
					case UnitId::ProtossWarden:
					case UnitId::ProtossMatrix:
					case UnitId::ProtossEmbassy:
					case UnitId::ProtossCrucible:
					case UnitId::ProtossStridentStratum:
					case UnitId::ProtossGateway:
					case UnitId::ProtossCenotaph:
					case UnitId::ProtossGrandLibrary:
					case UnitId::ProtossRogueGallery:
					case UnitId::ProtossProstrationStage:
					case UnitId::ProtossSanctumOfSorrow:
					case UnitId::ProtossAncestralArchives:
					case UnitId::ProtossHallOfHeretics:
					case UnitId::ProtossLattice:
					case UnitId::ProtossAutomatonRegister:
					case UnitId::ProtossArdentAuthority:
					case UnitId::ProtossSequesteredIcon:
					case UnitId::ProtossAuxiliaryOasis:
					case UnitId::ProtossUnitaryUnion:
					case UnitId::ProtossSyntheticSynod:
					case UnitId::ProtossStargate:
					case UnitId::ProtossAstralOmen:
					case UnitId::ProtossArgosy:
					case UnitId::ProtossCosmicAltar:
					case UnitId::ProtossFleetBeacon:
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
				switch (unit_id)
				{
					case UnitId::ZergDroleth:
					case UnitId::ZergOvileth:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ZergOthstoleth:
					{
						if (TierUnlocked[RaceId::Zerg][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergZethrokor:
					case UnitId::ZergQuazilisk:
					case UnitId::ZergNathrokor:
					{
						if (player_has_unit_id(player_id, UnitId::ZergLiivralPond)
						||  player_has_completed_unit_id(player_id, UnitId::ZergQuazrokPool))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergHydralisk:
					case UnitId::ZergIzirokor:
					case UnitId::ZergSkithrokor:
					{
						if (player_has_unit_id(player_id, UnitId::ZergGorgralSwamp)
						||  player_has_completed_unit_id(player_id, UnitId::ZergHydrokDen))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergZoryusthaleth:
					case UnitId::ZergLakizilisk:
					case UnitId::ZergGorgrokor:
					{
						if (player_has_unit_id(player_id, UnitId::ZergUltrokCavern)
						||  player_has_completed_unit_id(player_id, UnitId::ZergZorkizShroud))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergKagralisk:
					case UnitId::ZergEvigrilisk:
					case UnitId::ZergAlmaksalisk:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergIzkagIteth))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergMutalisk:
					case UnitId::ZergProtathalor:
					case UnitId::ZergCikralisk:
					{
						if (player_has_unit_id(player_id, UnitId::ZergVithrathHaunt)
						||  player_has_completed_unit_id(player_id, UnitId::ZergMuthrokSpire))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergAxitrilisk:
					case UnitId::ZergMortothrokor:
					{
						if (player_has_unit_id(player_id, UnitId::ZergAxtothAxis)
						||  player_has_completed_unit_id(player_id, UnitId::ZergBegrozithBarrow))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergKonvilisk:
					case UnitId::ZergVilgorokor:
					case UnitId::ZergMatraleth:
					{
						if (player_has_unit_id(player_id, UnitId::ZergAlmakisAntre)
						||  player_has_completed_unit_id(player_id, UnitId::ZergMatravilNest))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergKeskathalor:
					case UnitId::ZergGeszithalor:
					{
						if (player_has_unit_id(player_id, UnitId::ZergAlkajChasm)
						||  player_has_completed_unit_id(player_id, UnitId::ZergGeszkathGrotto))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergIsthrathaleth:
					case UnitId::ZergElthrazalisk:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergElthisthMound))
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
				if (unit_id == UnitId::ZergIroleth)
				{
					if (TierUnlocked[RaceId::Zerg][Tier2]
						&& scbw::transport_is_empty(unit))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergZethrokor:
			{
				if (unit_id == UnitId::ZergVorvrokor)
				{
					if (unit->status & UnitStatus::Burrowed)
					{
						return TECH_HIDDEN;
					}
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergQuazilisk:
			{
				if (unit_id == UnitId::ZergLiiralisk)
				{
					if (unit->status & UnitStatus::Burrowed)
					{
						return TECH_HIDDEN;
					}
					if (player_has_completed_unit_id(player_id, UnitId::ZergLiivralPond))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergNathrokor:
			{
				if (unit_id == UnitId::ZergKalkalisk)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergLiivralPond))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergCikralisk:
			{
				if (unit_id == UnitId::ZergGhitorokor)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergVithrathHaunt))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergProtathalor:
			{
				if (unit_id == UnitId::ZergSovroleth)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergVithrathHaunt))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergMutalisk:
			{
				if (unit_id == UnitId::ZergTosgrilisk)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergVithrathHaunt))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergAxitrilisk:
			{
				if (unit_id == UnitId::ZergBegrothilisk)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergBegrozithBarrow))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergMortothrokor:
			{
				if (unit_id == UnitId::ZergOztukrokor)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergBegrozithBarrow))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergHydralisk:
			{
				if (unit_id == UnitId::ZergBactalisk)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergGorgralSwamp))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergIzirokor:
			{
				if (unit_id == UnitId::ZergTethzorokor)
				{
					if (unit->status & UnitStatus::Burrowed)
					{
						return TECH_HIDDEN;
					}
					if (player_has_completed_unit_id(player_id, UnitId::ZergGorgralSwamp))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergSkithrokor:
			{
				if (unit_id == UnitId::ZergVithrilisk)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergGorgralSwamp))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergKagralisk:
			{
				if (unit_id == UnitId::ZergCherbrathalor)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergChokralgantChazsal))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergEvigrilisk:
			{
				if (unit_id == UnitId::ZergOkrimnilisk)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergChokralgantChazsal))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergAlmaksalisk:
			{
				if (unit_id == UnitId::ZergAlgoztalisk)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergVithrathHaunt))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergZoryusthaleth:
			{
				if (unit_id == UnitId::ZergUltrokor)
				{
					if (unit->status & UnitStatus::Burrowed)
					{
						return TECH_HIDDEN;
					}
					if (player_has_completed_unit_id(player_id, UnitId::ZergUltrokCavern))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergLakizilisk:
			{
				if (unit_id == UnitId::ZergZobriolisk)
				{
					if (unit->status & UnitStatus::Burrowed)
					{
						return TECH_HIDDEN;
					}
					if (player_has_completed_unit_id(player_id, UnitId::ZergUltrokCavern))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergGorgrokor:
			{
				if (unit_id == UnitId::ZergZarcavrokor)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergUltrokCavern))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergMatraleth:
			{
				if (unit_id == UnitId::ZergAkistrokor)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergKonvilisk:
			{
				if (unit_id == UnitId::ZergTetcaerokor)
				{
					if (unit->status & UnitStatus::Burrowed)
					{
						return TECH_HIDDEN;
					}
					if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergVilgorokor:
			{
				if (unit_id == UnitId::ZergSelkerilisk)
				{
					if (unit->status & UnitStatus::Burrowed)
					{
						return TECH_HIDDEN;
					}
					if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergOthstoleth:
			{
				if (unit_id == UnitId::ZergAlaszileth)
				{
					if (TierUnlocked[RaceId::Zerg][Tier4]
						&& scbw::transport_is_empty(unit))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergKeskathalor:
			{
				if (unit_id == UnitId::ZergKethrostalor)
				{
					if (unit->status & UnitStatus::Burrowed)
					{
						return TECH_HIDDEN;
					}
					if (player_has_completed_unit_id(player_id, UnitId::ZergAlkajChasm))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergGeszithalor:
			{
				if (unit_id == UnitId::ZergAlkajelisk)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergAlkajChasm))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergIsthrathaleth:
			{
				if (unit_id == UnitId::ZergOgroszaleth)
				{
					if (unit->status & UnitStatus::Burrowed)
					{
						return TECH_HIDDEN;
					}
					if (player_has_completed_unit_id(player_id, UnitId::ZergRakzogthRoost))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergElthrazalisk:
			{
				if (unit_id == UnitId::ZergRakzilisk)
				{
					if (unit->status & UnitStatus::Burrowed)
					{
						return TECH_HIDDEN;
					}
					if (player_has_completed_unit_id(player_id, UnitId::ZergRakzogthRoost))
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergOcsalLarva:
			{
				switch (unit_id)
				{
					case UnitId::ZergIroleth:
					{
						if (TierUnlocked[RaceId::Zerg][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergGosvileth:
					{
						if (TierUnlocked[RaceId::Zerg][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergAlaszileth:
					{
						if (TierUnlocked[RaceId::Zerg][Tier4])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergVorvrokor:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ZergLiiralisk:
					case UnitId::ZergKalkalisk:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergLiivralPond))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergGhitorokor:
					case UnitId::ZergSovroleth:
					case UnitId::ZergTosgrilisk:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergVithrathHaunt))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergBegrothilisk:
					case UnitId::ZergOztukrokor:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergBegrozithBarrow))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergBactalisk:
					case UnitId::ZergTethzorokor:
					case UnitId::ZergVithrilisk:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergGorgralSwamp))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergCherbrathalor:
					case UnitId::ZergOkrimnilisk:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergChokralgantChazsal))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergAlkajelisk:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergAlkajChasm))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergUltrokor:
					case UnitId::ZergZobriolisk:
					case UnitId::ZergZarcavrokor:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergUltrokCavern))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergTetcaerokor:
					case UnitId::ZergSelkerilisk:
					case UnitId::ZergAkistrokor:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergOgroszaleth:
					case UnitId::ZergRakzilisk:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergRakzogthRoost))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
				}
				return TECH_HIDDEN;
			}

			// Structures
			case UnitId::ZergDroleth:
			{
				switch (unit_id)
				{
					case UnitId::ZergHachirosk:
					case UnitId::ZergCagrantColony:
					case UnitId::ZergExcisant:
					case UnitId::ZergQuazrokPool:
					case UnitId::ZergHydrokDen:
					case UnitId::ZergZorkizShroud:
					case UnitId::ZergIrolIris:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ZergMuthrokSpire:
					case UnitId::ZergIzkagIteth:
					case UnitId::ZergMatravilNest:
					{
						if (TierUnlocked[RaceId::Zerg][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergNydusCesiant:
					case UnitId::ZergAxtothAxis:
					case UnitId::ZergGeszkathGrotto:
					case UnitId::ZergElthisthMound:
					{
						if (TierUnlocked[RaceId::Zerg][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergGosvileth:
					{
						if (unit->status & UnitStatus::Burrowed)
						{
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Zerg][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergGosvileth:
			{
				switch (unit_id)
				{
					case UnitId::ZergHachirosk:
					case UnitId::ZergCagrantColony:
					case UnitId::ZergExcisant:
					case UnitId::ZergQuazrokPool:
					case UnitId::ZergHydrokDen:
					case UnitId::ZergZorkizShroud:
					case UnitId::ZergIrolIris:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ZergMuthrokSpire:
					case UnitId::ZergIzkagIteth:
					case UnitId::ZergMatravilNest:
					{
						if (TierUnlocked[RaceId::Zerg][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergNydusCesiant:
					case UnitId::ZergAxtothAxis:
					case UnitId::ZergGeszkathGrotto:
					case UnitId::ZergElthisthMound:
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
			case UnitId::ZergCagrantColony:
			{
				if (unit_id == UnitId::ZergSurkithColony
					|| unit_id == UnitId::ZergSpraithColony
					|| unit_id == UnitId::ZergLarvoskColony)
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergHachirosk:
			{
				if (unit_id == UnitId::ZergCaphrolosk)
				{
					if (TierUnlocked[RaceId::Zerg][Tier2])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				if (unit_id == UnitId::ZergGathtelosk)
				{
					if (TierUnlocked[RaceId::Zerg][Tier3])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergQuazrokPool:
			{
				if (unit_id == UnitId::ZergLiivralPond)
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
				if (unit_id == UnitId::ZergGorgralSwamp)
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
				if (unit_id == UnitId::ZergVithrathHaunt)
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
				if (unit_id == UnitId::ZergOthstolOviform)
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergCaphrolosk:
			{
				if (unit_id == UnitId::ZergSibraelosk)
				{
					if (TierUnlocked[RaceId::Zerg][Tier4])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergMatravilNest:
			{
				if (unit_id == UnitId::ZergAlmakisAntre)
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
				if (unit_id == UnitId::ZergUltrokCavern)
				{
					if (TierUnlocked[RaceId::Zerg][Tier3])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergGathtelosk:
			{
				if (unit_id == UnitId::ZergNephrilosk)
				{
					if (TierUnlocked[RaceId::Zerg][Tier5])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergOthstolOviform:
			{
				if (unit_id == UnitId::ZergAlaszilArbor)
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergGeszkathGrotto:
			{
				if (unit_id == UnitId::ZergAlkajChasm)
				{
					if (TierUnlocked[RaceId::Zerg][Tier4])
					{
						return TECH_ACTIVE;
					}
					return TECH_LOCKED;
				}
				return TECH_HIDDEN;
			}
			case UnitId::ZergAlaszilArbor:
			{
				if (unit_id == UnitId::ZergOrbothOmlia)
				{
					return TECH_ACTIVE;
				}
				return TECH_HIDDEN;
			}
		}
		return TECH_HIDDEN;
	}

	bool can_set_rally_opcode(Unit* unit) {
		return hooks::unit_can_rally(unit);
	}
}