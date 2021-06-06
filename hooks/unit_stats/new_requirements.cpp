#include "new_requirements.h"
#include <hook_tools.h>

namespace hooks
{
	enum
	{
		TECH_GRAYED = -1,
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

		switch (unit->id)
		{
			// TERRAN

			
			// PROTOSS
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
						return TECH_GRAYED;
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
						return TECH_GRAYED;
					}
					return TECH_HIDDEN;
				}
			}
			case UnitId::ProtossCantavis:
			{
				if (unitId == UnitId::ProtossAugur)
				{
					if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossArchonArchives))
					{
						return TECH_ACTIVE;
					}
					return TECH_GRAYED;
				}
				return TECH_HIDDEN;
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
						return TECH_GRAYED;
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
						return TECH_GRAYED;
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
						return TECH_GRAYED;
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
						return TECH_GRAYED;
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
						return TECH_GRAYED;
					}
					case UnitId::ProtossAnthelion:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ProtossMonumentOfSin))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
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
						return TECH_GRAYED;
					}
					return TECH_HIDDEN;
				}
			}

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
						return TECH_GRAYED;
					}
					case UnitId::ZergHydralisk:
					case UnitId::ZergSkithrokor:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergGorgralSwamp)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergHydrokDen))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::ZergMutalisk:
					case UnitId::ZergNathrokor:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergVithrathHaunt)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergMuthrokSpire))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
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
						return TECH_GRAYED;
					}
					case UnitId::ZergZoryusthaleth:
					case UnitId::ZergLakizilisk:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergUltrakCavern)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergZorkizShroud))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::ZergMatraleth:
					case UnitId::ZergKonvilisk:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlmakisAntre)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergMatravilNest))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
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
						return TECH_GRAYED;
					}
					case UnitId::ZergKeskathalor:
					case UnitId::ZergGeszithalor:
					{
						if (Player::numberOfUnitsOfType(playerId, UnitId::ZergAlkajChasm)
							|| Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergGeszkathGrotto))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::ZergIsthrathaleth:
					case UnitId::ZergSovroleth:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergSovthrathMound))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::SharedAnticthon:
					{
						if (Player::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlaszilArbor))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					default:
					{
						return TECH_HIDDEN;
					}
				}
			}
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
				}
				return TECH_HIDDEN;
			}
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
						return TECH_GRAYED;
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
						return TECH_GRAYED;
					}
				}
				return TECH_HIDDEN;
			}
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
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
					return TECH_GRAYED;
				}
				return TECH_HIDDEN;
			}
		}
		return TECH_HIDDEN;
	}
}