#include "new_requirements.h"
#include <hook_tools.h>

namespace hooks {
	enum {
		TECH_LOCKED = -1,
		TECH_HIDDEN = 0,
		TECH_ACTIVE = 1,
	};

	static bool player_has_unit_id(s32 player_id, u32 unit_id) {
		return Player::numberOfUnitsOfType(player_id, unit_id);
	}

	static bool player_has_completed_unit_id(s32 player_id, u32 unit_id) {
		return Player::numberOfCompletedUnitsOfType(player_id, unit_id);
	}

	s32 createUnitIdAllowed(s16 unitId, s32 player_id, bool ignore_creator)
	{
		*lastInternalError = InternalErrors::UnknownError0;
		if (unitId >= UnitId::None)
		{
			return false;
		}
		if (!UnitAvailability->available[player_id][unitId])
		{
			return false;
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
		TierUnlocked[RaceId::Zerg][Tier2] = player_has_completed_unit_id(player_id, UnitId::ZergIrolIris)
			|| player_has_completed_unit_id(player_id, UnitId::ZergOthstolOviform)
			|| player_has_completed_unit_id(player_id, UnitId::ZergAlaszilArbor);
		TierUnlocked[RaceId::Zerg][Tier3] = player_has_completed_unit_id(player_id, UnitId::ZergOthstolOviform)
			|| player_has_completed_unit_id(player_id, UnitId::ZergAlaszilArbor);
		TierUnlocked[RaceId::Zerg][Tier4] = player_has_completed_unit_id(player_id, UnitId::ZergAlaszilArbor);

		TierUnlocked[RaceId::Terran][Tier1] = true;
		TierUnlocked[RaceId::Terran][Tier2] = player_has_completed_unit_id(player_id, UnitId::TerranEngineeringBay);
		TierUnlocked[RaceId::Terran][Tier3] = player_has_completed_unit_id(player_id, UnitId::TerranScienceFacility);
		TierUnlocked[RaceId::Terran][Tier4] = true;

		TierUnlocked[RaceId::Protoss][Tier1] = true;
		TierUnlocked[RaceId::Protoss][Tier2] = true;
		TierUnlocked[RaceId::Protoss][Tier3] = true;
		TierUnlocked[RaceId::Protoss][Tier4] = true;

		// TERRAN
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranMason) || player_has_completed_unit_id(player_id, UnitId::TerranTalos))
		{
			switch (unitId)
			{
				case UnitId::TerranCommandCenter:
				case UnitId::TerranReservoir:
				case UnitId::TerranWatchdog:
				case UnitId::TerranBunker:
				{
					return true;
				}
				case UnitId::TerranBarracks:
				case UnitId::TerranFulcrum:
				case UnitId::TerranStarport:
				case UnitId::TerranScrapyard:
				case UnitId::TerranEngineeringBay:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranCommandCenter))
					{
						return true;
					}
				}
				case UnitId::TerranTreasury:
				case UnitId::TerranCaptaincy:
				case UnitId::TerranScienceFacility:
				{
					if (TierUnlocked[RaceId::Terran][Tier2])
					{
						return true;
					}
				}
				case UnitId::TerranIronFoundry:
				case UnitId::TerranNaniteAssembly:
				case UnitId::TerranIonCannon:
				{
					if (TierUnlocked[RaceId::Terran][Tier3])
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranCommandCenter))
		{
			switch (unitId)
			{
				case UnitId::TerranMason:
				case UnitId::TerranComsatStation:
				{
					return true;
				}
				case UnitId::TerranTalos:
				{
					if (TierUnlocked[RaceId::Terran][Tier3])
					{
						return true;
					}
				}
				case UnitId::TerranNuclearSilo:
				case UnitId::TerranQuarry:
				{
					if (TierUnlocked[RaceId::Terran][Tier2])
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranNuclearSilo))
		{
			switch (unitId)
			{
				case UnitId::TerranNuclearMissile:
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranQuarry))
		{
			switch (unitId)
			{
				case UnitId::TerranMason:
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranTreasury))
		{
			switch (unitId)
			{
				case UnitId::TerranComsatStation:
				{
					return true;
				}
				case UnitId::TerranNuclearSilo:
				case UnitId::TerranQuarry:
				{
					if (TierUnlocked[RaceId::Terran][Tier2])
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranBarracks))
		{
			switch (unitId)
			{
				case UnitId::TerranMedbay:
				{
					return true;
				}
				case UnitId::TerranCovertOps:
				{
					if (TierUnlocked[RaceId::Terran][Tier2])
					{
						return true;
					}
				}
				case UnitId::TerranMaverick:
				case UnitId::TerranHarakan:
				case UnitId::TerranCyprian:
				{
					return true;
				}
				case UnitId::TerranCleric:
				case UnitId::TerranShaman:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranMedbay))
					{
						return true;
					}
				}
				case UnitId::TerranEidolon:
				case UnitId::TerranSavant:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranCovertOps))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranFulcrum))
		{
			switch (unitId)
			{
				case UnitId::TerranMachineShop:
				case UnitId::TerranSalvoYard:
				{
					if (TierUnlocked[RaceId::Terran][Tier2])
					{
						return true;
					}
				}
				case UnitId::TerranVulture:
				case UnitId::TerranGoliath:
				case UnitId::TerranCyclops:
				{
					return true;
				}
				case UnitId::TerranPhalanxTankMode:
				case UnitId::TerranCuirass:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranMachineShop))
					{
						return true;
					}
				}
				case UnitId::TerranPaladin:
				case UnitId::TerranMadrigal:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranSalvoYard))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranStarport))
		{
			switch (unitId)
			{
				case UnitId::TerranMunitionsBay:
				case UnitId::TerranControlTower:
				{
					if (TierUnlocked[RaceId::Terran][Tier2])
					{
						return true;
					}
				}
				case UnitId::TerranWraith:
				case UnitId::TerranGorgon:
				case UnitId::TerranValkyrie:
				case UnitId::TerranTrojan:
				{
					return true;
				}
				case UnitId::TerranAzazel:
				case UnitId::TerranSeraph:
				{
					if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranControlTower))
					{
						return true;
					}
				}
				case UnitId::TerranSalamander:
				case UnitId::TerranWyvern:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranMunitionsBay))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranScrapyard))
		{
			switch (unitId)
			{
				case UnitId::TerranBioticBase:
				case UnitId::TerranTinkerersTower:
				{
					if (TierUnlocked[RaceId::Terran][Tier3])
					{
						return true;
					}
				}
				case UnitId::TerranBlackjack:
				case UnitId::SharedAspirant:
				case UnitId::SharedCohort:
				case UnitId::TerranSiren:
				{
					return true;
				}
				case UnitId::SharedStriga:
				case UnitId::SharedLuminary:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranTinkerersTower))
					{
						return true;
					}
				}
				case UnitId::SharedTarasque:
				case UnitId::SharedAnticthon:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranBioticBase))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranCaptaincy))
		{
			switch (unitId)
			{
				case UnitId::TerranGuildhall:
				{
					if (TierUnlocked[RaceId::Terran][Tier3])
					{
						return true;
					}
				}
				case UnitId::TerranMadcap:
				case UnitId::TerranHeracles:
				case UnitId::TerranApostle:
				{
					return true;
				}
				case UnitId::TerranOlympian:
				case UnitId::TerranAutocrat:
				{
					if (player_has_completed_unit_id(player_id, UnitId::TerranGuildhall))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranIronFoundry))
		{
			switch (unitId)
			{
				case UnitId::TerranPazuzu:
				case UnitId::TerranClaymore:
				case UnitId::TerranPenumbra:
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::TerranNaniteAssembly))
		{
			switch (unitId)
			{
				case UnitId::TerranCentaur:
				case UnitId::TerranMinotaur:
				case UnitId::TerranMagnetar:
				case UnitId::TerranPhobos:
				{
					return true;
				}
			}
		}

		// PROTOSS
		// Units
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossNexus))
		{
			if (unitId == UnitId::ProtossScribe)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossForge))
		{
			if (unitId == UnitId::ProtossScribe
				|| unitId == UnitId::ProtossEnvoy
				|| unitId == UnitId::ProtossWitness)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossGateway))
		{
			switch (unitId)
			{
				case UnitId::ProtossZealot:
				case UnitId::ProtossDracadin:
				case UnitId::ProtossEcclesiast:
				{
					return true;
				}
				case UnitId::ProtossLegionnaire:
				case UnitId::ProtossHierophant:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ProtossCenotaph))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossGrandLibrary))
		{
			switch (unitId)
			{
				case UnitId::ProtossCantavis:
				case UnitId::ProtossAmaranth:
				case UnitId::ProtossAtreus:
				{
					return true;
				}
				case UnitId::ProtossArchon:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ProtossArchonArchives))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossRoboticsFacility))
		{
			switch (unitId)
			{
				case UnitId::ProtossVassal:
				case UnitId::ProtossSimulacrum:
				case UnitId::ProtossManifold:
				{
					return true;
				}
				case UnitId::ProtossGolem:
				case UnitId::ProtossIdol:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ProtossAutomatonRegister))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossRoboticsAuthority))
		{
			switch (unitId)
			{
				case UnitId::ProtossAccantor:
				case UnitId::ProtossArchitect:
				case UnitId::ProtossClarion:
				{
					return true;
				}
				case UnitId::ProtossDemiurge:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ProtossSyntheticSynod))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossStargate))
		{
			switch (unitId)
			{
				case UnitId::ProtossAurora:
				case UnitId::ProtossPanoptus:
				case UnitId::ProtossLanifect:
				{
					return true;
				}
				case UnitId::ProtossGladius:
				case UnitId::ProtossExemplar:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ProtossAstralOmen))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossArgosy))
		{
			switch (unitId)
			{
				case UnitId::ProtossMagister:
				case UnitId::ProtossDidact:
				case UnitId::ProtossEmpyrean:
				{
					return true;
				}
				case UnitId::ProtossSolarion:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ProtossFleetBeacon))
					{
						return true;
					}
				}
				case UnitId::ProtossStarSovereign:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ProtossCosmicAltar))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossProstrationStage))
		{
			switch (unitId)
			{
				case UnitId::ProtossCabalist:
				case UnitId::ProtossBarghest:
				{
					return true;
				}
				case UnitId::ProtossMindTyrant:
				case UnitId::ProtossPariah:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ProtossSanctumOfSorrow))
					{
						return true;
					}
				}
				case UnitId::ProtossAnthelion:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ProtossMonumentOfSin))
					{
						return true;
					}
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossRogueGallery))
		{
			switch (unitId)
			{
				case UnitId::SharedAspirant:
				case UnitId::SharedCharlatan:
				case UnitId::SharedAxitrilisk:
				{
					return true;
				}
				case UnitId::SharedStriga:
				case UnitId::SharedLuminary:
				case UnitId::SharedEmpress:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ProtossWaywardLure))
					{
						return true;
					}
				}
			}
		}

		// Morphs and subunits
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossCantavis))
		{
			if (unitId == UnitId::ProtossAugur)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ProtossArchonArchives))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossAccantor))
		{
			if (unitId == UnitId::ProtossPlasmaShell)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossSolarion))
		{
			if (unitId == UnitId::ProtossSteward)
			{
				return true;
			}
		}

		// Structures
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ProtossScribe))
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
				case UnitId::ProtossCrucible:
				case UnitId::ProtossPennant:
				case UnitId::ProtossGrandLibrary:
				case UnitId::ProtossRoboticsAuthority:
				case UnitId::ProtossSyntheticSynod:
				case UnitId::ProtossArgosy:
				case UnitId::ProtossProstrationStage:
				case UnitId::ProtossRogueGallery:
				case UnitId::ProtossArchonArchives:
				case UnitId::ProtossCosmicAltar:
				case UnitId::ProtossFleetBeacon:
				case UnitId::ProtossSanctumOfSorrow:
				case UnitId::ProtossWaywardLure:
				case UnitId::ProtossMonumentOfSin:
				{
					return true;
				}
			}
		}

		// ZERG
		// Units
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergLarva))
		{
			switch (unitId)
			{
				case UnitId::ZergDroleth:
				case UnitId::ZergOvileth:
				{
					return true;
				}
				case UnitId::ZergOthstoleth:
				{
					if (TierUnlocked[RaceId::Zerg][Tier3])
					{
						return true;
					}
				}
				case UnitId::ZergZethrokor:
				case UnitId::ZergQuazilisk:
				{
					if (player_has_unit_id(player_id, UnitId::ZergLiivralPond)
						|| player_has_completed_unit_id(player_id, UnitId::ZergQuazrokPool))
					{
						return true;
					}
				}
				case UnitId::ZergHydralisk:
				case UnitId::ZergSkithrokor:
				{
					if (player_has_unit_id(player_id, UnitId::ZergGorgralSwamp)
						|| player_has_completed_unit_id(player_id, UnitId::ZergHydrokDen))
					{
						return true;
					}
				}
				case UnitId::ZergMutalisk:
				case UnitId::ZergNathrokor:
				{
					if (player_has_unit_id(player_id, UnitId::ZergVithrathHaunt)
						|| player_has_completed_unit_id(player_id, UnitId::ZergMuthrokSpire))
					{
						return true;
					}
				}
				case UnitId::SharedCohort:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergOrbothOmlia))
					{
						return true;
					}
				}
				case UnitId::SharedAxitrilisk:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergOrbothOmlia)
						&& TierUnlocked[RaceId::Zerg][Tier2])
					{
						return true;
					}
				}
				case UnitId::ZergIzirokor:
				{
					if (player_has_unit_id(player_id, UnitId::ZergLiivralPond))
					{
						return true;
					}
				}
				case UnitId::ZergZoryusthaleth:
				case UnitId::ZergLakizilisk:
				{
					if (player_has_unit_id(player_id, UnitId::ZergUltrokCavern)
						|| player_has_completed_unit_id(player_id, UnitId::ZergZorkizShroud))
					{
						return true;
					}
				}
				case UnitId::ZergMatraleth:
				case UnitId::ZergKonvilisk:
				{
					if (player_has_unit_id(player_id, UnitId::ZergAlmakisAntre)
						|| player_has_completed_unit_id(player_id, UnitId::ZergMatravilNest))
					{
						return true;
					}
				}
				case UnitId::SharedTarasque:
				case UnitId::SharedCharlatan:
				case UnitId::SharedEmpress:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergOrbothOmlia)
						&& TierUnlocked[RaceId::Zerg][Tier3])
					{
						return true;
					}
				}
				case UnitId::ZergKeskathalor:
				case UnitId::ZergGeszithalor:
				{
					if (player_has_unit_id(player_id, UnitId::ZergAlkajChasm)
						|| player_has_completed_unit_id(player_id, UnitId::ZergGeszkathGrotto))
					{
						return true;
					}
				}
				case UnitId::ZergIsthrathaleth:
				case UnitId::ZergSovroleth:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergSovthrathMound))
					{
						return true;
					}
				}
				case UnitId::SharedAnticthon:
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergOrbothOmlia)
						&& TierUnlocked[RaceId::Zerg][Tier4])
					{
						return true;
					}
				}
			}
		}

		// Morphs
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergOvileth))
		{
			if (unitId == UnitId::ZergIroleth)
			{
				if (TierUnlocked[RaceId::Zerg][Tier2])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergZethrokor))
		{
			if (unitId == UnitId::ZergVorvrokor)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergLiivralPond))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergQuazilisk))
		{
			if (unitId == UnitId::ZergLiiralisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergLiivralPond))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergHydralisk))
		{
			if (unitId == UnitId::ZergBactalisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergGorgralSwamp))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergSkithrokor))
		{
			if (unitId == UnitId::ZergGorgrokor)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergGorgralSwamp))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMutalisk))
		{
			if (unitId == UnitId::ZergVithrilisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergVithrathHaunt))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergNathrokor))
		{
			if (unitId == UnitId::ZergKalkalisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergVithrathHaunt))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergZoryusthaleth))
		{
			if (unitId == UnitId::ZergUltrokor)
			{
				if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergUltrokCavern))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMatraleth))
		{
			if (unitId == UnitId::ZergAkistrokor)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergKonvilisk))
		{
			if (unitId == UnitId::ZergAlmaksalisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergAlmakisAntre))
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergOthstoleth))
		{
			if (unitId == UnitId::ZergAlaszileth)
			{
				if (TierUnlocked[RaceId::Zerg][Tier4])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergKeskathalor))
		{
			if (unitId == UnitId::ZergAlkajelisk)
			{
				if (player_has_completed_unit_id(player_id, UnitId::ZergAlkajChasm))
				{
					return true;
				}
			}
		}

		// Structures
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergDroleth))
		{
			switch (unitId)
			{
				case UnitId::ZergHatchery:
				case UnitId::ZergCreepColony:
				case UnitId::ZergExcisant:
				case UnitId::ZergQuazrokPool:
				case UnitId::ZergHydrokDen:
				case UnitId::ZergMuthrokSpire:
				case UnitId::ZergOrbothOmlia:
				{
					return true;
				}
				case UnitId::ZergIrolIris:
				{
					if (player_has_unit_id(player_id, UnitId::ZergSire)
						|| player_has_unit_id(player_id, UnitId::ZergHive)
						|| player_has_unit_id(player_id, UnitId::ZergLair)
						|| player_has_completed_unit_id(player_id, UnitId::ZergHatchery))
					{
						return true;
					}
				}
				case UnitId::ZergZorkizShroud:
				case UnitId::ZergMatravilNest:
				{
					if (TierUnlocked[RaceId::Zerg][Tier2])
					{
						return true;
					}
				}
				case UnitId::ZergNydusCanal:
				case UnitId::ZergGeszkathGrotto:
				case UnitId::ZergSovthrathMound:
				{
					if (TierUnlocked[RaceId::Zerg][Tier3])
					{
						return true;
					}
				}
			}
		}

		// Structure morphs
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergCreepColony))
		{
			if (unitId == UnitId::ZergSunkenColony
				|| unitId == UnitId::ZergSporeColony
				|| unitId == UnitId::ZergLarvalColony)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergHatchery))
		{
			if (unitId == UnitId::ZergLair)
			{
				if (TierUnlocked[RaceId::Zerg][Tier2])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergQuazrokPool))
		{
			if (unitId == UnitId::ZergLiivralPond)
			{
				if (TierUnlocked[RaceId::Zerg][Tier2])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergHydrokDen))
		{
			if (unitId == UnitId::ZergGorgralSwamp)
			{
				if (TierUnlocked[RaceId::Zerg][Tier2])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMuthrokSpire))
		{
			if (unitId == UnitId::ZergVithrathHaunt)
			{
				if (TierUnlocked[RaceId::Zerg][Tier2])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergIrolIris))
		{
			if (unitId == UnitId::ZergOthstolOviform)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergLair))
		{
			if (unitId == UnitId::ZergHive)
			{
				if (TierUnlocked[RaceId::Zerg][Tier3])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergMatravilNest))
		{
			if (unitId == UnitId::ZergAlmakisAntre)
			{
				if (TierUnlocked[RaceId::Zerg][Tier3])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergZorkizShroud))
		{
			if (unitId == UnitId::ZergUltrokCavern)
			{
				if (TierUnlocked[RaceId::Zerg][Tier3])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergHive))
		{
			if (unitId == UnitId::ZergSire)
			{
				if (TierUnlocked[RaceId::Zerg][Tier4])
				{
					return true;
				}
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergOthstolOviform))
		{
			if (unitId == UnitId::ZergAlaszilArbor)
			{
				return true;
			}
		}
		if (ignore_creator || player_has_completed_unit_id(player_id, UnitId::ZergGeszkathGrotto))
		{
			if (unitId == UnitId::ZergAlkajChasm)
			{
				if (TierUnlocked[RaceId::Zerg][Tier4])
				{
					return true;
				}
			}
		}
		return false;
	}

	s32 createUnitAllowed_Partial(s16 unitId, Unit* unit, s32 player_id)
	{
		*lastInternalError = InternalErrors::UnknownError0;
		if (unitId >= UnitId::None)
		{
			*lastInternalError = InternalErrors::UnknownError16;
			return TECH_HIDDEN;
		}
		if (player_id != unit->player)
		{
			*lastInternalError = InternalErrors::UnknownError1;
			return TECH_HIDDEN;
		}
		if (!(unit->status & UnitStatus::Completed))
		{
			*lastInternalError = InternalErrors::UnknownError20;
			return TECH_HIDDEN;
		}
		if (unit->is_frozen())
		{
			*lastInternalError = InternalErrors::UnitFrozen;
			return TECH_HIDDEN;
		}
		if (!UnitAvailability->available[unit->player][unitId])
		{
			*lastInternalError = InternalErrors::UnknownError2;
			return TECH_HIDDEN;
		}
		if (unit->is_queue_slot_active(4))
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
		TierUnlocked[RaceId::Zerg][Tier2] = player_has_completed_unit_id(player_id, UnitId::ZergIrolIris)
			|| player_has_unit_id(player_id, UnitId::ZergOthstolOviform)
			|| player_has_unit_id(player_id, UnitId::ZergAlaszilArbor);
		TierUnlocked[RaceId::Zerg][Tier3] = player_has_completed_unit_id(player_id, UnitId::ZergOthstolOviform)
			|| player_has_unit_id(player_id, UnitId::ZergAlaszilArbor);
		TierUnlocked[RaceId::Zerg][Tier4] = player_has_completed_unit_id(player_id, UnitId::ZergAlaszilArbor);

		TierUnlocked[RaceId::Terran][Tier1] = true;
		TierUnlocked[RaceId::Terran][Tier2] = player_has_completed_unit_id(player_id, UnitId::TerranEngineeringBay);
		TierUnlocked[RaceId::Terran][Tier3] = player_has_completed_unit_id(player_id, UnitId::TerranScienceFacility);
		TierUnlocked[RaceId::Terran][Tier4] = true;

		TierUnlocked[RaceId::Protoss][Tier1] = true;
		TierUnlocked[RaceId::Protoss][Tier2] = true;
		TierUnlocked[RaceId::Protoss][Tier3] = true;
		TierUnlocked[RaceId::Protoss][Tier4] = true;

		if (helper::is_constructing_addon(unit))
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
			case UnitId::TerranTalos:
			{
				if (unit->mainOrderId == OrderId::ConstructingBuilding) {
					return TECH_HIDDEN;
				}

				switch (unitId)
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
					case UnitId::TerranStarport:
					case UnitId::TerranScrapyard:
					case UnitId::TerranEngineeringBay:
					{
						if (player_has_completed_unit_id(player_id, UnitId::TerranCommandCenter))
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
					case UnitId::TerranTalos:
					{
						if (inAir) {
							return TECH_HIDDEN;
						}
						if (TierUnlocked[RaceId::Terran][Tier3]) {
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
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
			case UnitId::TerranFulcrum:
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
					case UnitId::TerranCuirass:
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
					case UnitId::TerranMagnetar:
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
						if (player_has_completed_unit_id(player_id, UnitId::ProtossCenotaph))
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
						if (player_has_completed_unit_id(player_id, UnitId::ProtossArchonArchives))
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
						if (player_has_completed_unit_id(player_id, UnitId::ProtossAutomatonRegister))
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
					case UnitId::ProtossArchitect:
					case UnitId::ProtossClarion:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ProtossDemiurge:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ProtossSyntheticSynod))
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
						if (player_has_completed_unit_id(player_id, UnitId::ProtossAstralOmen))
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
					case UnitId::ProtossDidact:
					case UnitId::ProtossEmpyrean:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ProtossSolarion:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ProtossFleetBeacon))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ProtossStarSovereign:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ProtossCosmicAltar))
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
						if (player_has_completed_unit_id(player_id, UnitId::ProtossSanctumOfSorrow))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ProtossAnthelion:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ProtossMonumentOfSin))
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
						if (player_has_completed_unit_id(player_id, UnitId::ProtossWaywardLure))
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
					if (player_has_completed_unit_id(player_id, UnitId::ProtossArchonArchives))
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
					case UnitId::ProtossCrucible:
					case UnitId::ProtossPennant:
					case UnitId::ProtossGrandLibrary:
					case UnitId::ProtossRoboticsAuthority:
					case UnitId::ProtossSyntheticSynod:
					case UnitId::ProtossArgosy:
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
					{
						if (player_has_unit_id(player_id, UnitId::ZergLiivralPond)
							|| player_has_completed_unit_id(player_id, UnitId::ZergQuazrokPool))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergHydralisk:
					case UnitId::ZergSkithrokor:
					{
						if (player_has_unit_id(player_id, UnitId::ZergGorgralSwamp)
							|| player_has_completed_unit_id(player_id, UnitId::ZergHydrokDen))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergMutalisk:
					case UnitId::ZergNathrokor:
					{
						if (player_has_unit_id(player_id, UnitId::ZergVithrathHaunt)
							|| player_has_completed_unit_id(player_id, UnitId::ZergMuthrokSpire))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::SharedCohort:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergOrbothOmlia))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::SharedAxitrilisk:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergOrbothOmlia)
							&& TierUnlocked[RaceId::Zerg][Tier2])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergIzirokor:
					{
						if (player_has_unit_id(player_id, UnitId::ZergLiivralPond))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergZoryusthaleth:
					case UnitId::ZergLakizilisk:
					{
						if (player_has_unit_id(player_id, UnitId::ZergUltrokCavern)
							|| player_has_completed_unit_id(player_id, UnitId::ZergZorkizShroud))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergMatraleth:
					case UnitId::ZergKonvilisk:
					{
						if (player_has_unit_id(player_id, UnitId::ZergAlmakisAntre)
							|| player_has_completed_unit_id(player_id, UnitId::ZergMatravilNest))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::SharedTarasque:
					case UnitId::SharedCharlatan:
					case UnitId::SharedEmpress:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergOrbothOmlia)
							&& TierUnlocked[RaceId::Zerg][Tier3])
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergKeskathalor:
					case UnitId::ZergGeszithalor:
					{
						if (player_has_unit_id(player_id, UnitId::ZergAlkajChasm)
							|| player_has_completed_unit_id(player_id, UnitId::ZergGeszkathGrotto))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::ZergIsthrathaleth:
					case UnitId::ZergSovroleth:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergSovthrathMound))
						{
							return TECH_ACTIVE;
						}
						return TECH_LOCKED;
					}
					case UnitId::SharedAnticthon:
					{
						if (player_has_completed_unit_id(player_id, UnitId::ZergOrbothOmlia)
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
					if (player_has_completed_unit_id(player_id, UnitId::ZergLiivralPond))
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
					if (player_has_completed_unit_id(player_id, UnitId::ZergLiivralPond))
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
				if (unitId == UnitId::ZergGorgrokor)
				{
					if (player_has_completed_unit_id(player_id, UnitId::ZergGorgralSwamp))
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
					if (player_has_completed_unit_id(player_id, UnitId::ZergVithrathHaunt))
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
				if (unitId == UnitId::ZergUltrokor)
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
				if (unitId == UnitId::ZergAkistrokor)
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
				if (unitId == UnitId::ZergAlmaksalisk)
				{
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
					if (player_has_completed_unit_id(player_id, UnitId::ZergAlkajChasm))
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
					case UnitId::ZergExcisant:
					case UnitId::ZergQuazrokPool:
					case UnitId::ZergHydrokDen:
					case UnitId::ZergMuthrokSpire:
					case UnitId::ZergOrbothOmlia:
					{
						return TECH_ACTIVE;
					}
					case UnitId::ZergIrolIris:
					{
						if (player_has_unit_id(player_id, UnitId::ZergSire)
							|| player_has_unit_id(player_id, UnitId::ZergHive)
							|| player_has_unit_id(player_id, UnitId::ZergLair)
							|| player_has_completed_unit_id(player_id, UnitId::ZergHatchery))
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