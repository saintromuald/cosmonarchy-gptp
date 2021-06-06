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

	s8 createUnitAllowed_Partial(s16 unitId, Unit* unit, s32 playerId)
	{
		*lastInternalErr = InternalErrors::UnknownError0;
		if (unitId >= UnitId::None)
		{
			*lastInternalErr = InternalErrors::UnknownError16;
			return TECH_HIDDEN;
		}
		if (playerId != unit->playerId)
		{
			*lastInternalErr = InternalErrors::UnknownError1;
			return TECH_HIDDEN;
		}
		if (!(unit->status & UnitStatus::Completed))
		{
			*lastInternalErr = InternalErrors::UnknownError20;
			return TECH_HIDDEN;
		}
		if (unit->isFrozen())
		{
			*lastInternalErr = InternalErrors::UnitFrozen;
			return TECH_HIDDEN;
		}
		if (!UnitAvailability->available[unit->playerId][unitId])
		{
			*lastInternalErr = InternalErrors::UnknownError2;
			return TECH_HIDDEN;
		}
		if (unit->isQueueSlotActive(4))
		{
			*lastInternalErr = InternalErrors::UnknownError3;
			return TECH_HIDDEN;
		}
		if (unit->status & UnitStatus::IsHallucination)
		{
			*lastInternalErr = InternalErrors::UnknownError22;
			return TECH_HIDDEN;
		}

		switch (unit->id)
		{
			// TERRAN

			
			// PROTOSS

			
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
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergLiivralPond)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergQuazrokPool))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::ZergHydralisk:
					case UnitId::ZergSkithrokor:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergGorgralSwamp)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergHydrokDen))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::ZergMutalisk:
					case UnitId::ZergNathrokor:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergVithrathHaunt)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergMuthrokSpire))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::SharedCohort:
					case UnitId::SharedAxitrilisk:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
							|| PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::ZergZoryusthaleth:
					case UnitId::ZergLakizilisk:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergUltrakCavern)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergZorkizShroud))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::ZergMatraleth:
					case UnitId::ZergKonvilisk:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlmakisAntre)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergMatravilNest))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::SharedTarasque:
					case UnitId::SharedCharlatan:
					case UnitId::SharedEmpress:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::ZergKeskathalor:
					case UnitId::ZergGeszithalor:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlkajChasm)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergGeszkathGrotto))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::ZergIsthrathaleth:
					case UnitId::ZergSovroleth:
					{
						if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergSovthrathMound))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::SharedAnticthon:
					{
						if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlaszilArbor))
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
					if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
						|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergLiivralPond))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergLiivralPond))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergGorgralSwamp))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergGorgralSwamp))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergVithrathHaunt))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergVithrathHaunt))
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
					if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergUltrakCavern))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlmakisAntre))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlmakisAntre))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlaszilArbor))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlkajChasm))
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
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
							|| PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
						{
							return TECH_ACTIVE;
						}
						return TECH_GRAYED;
					}
					case UnitId::ZergNydusCanal:
					case UnitId::ZergGeszkathGrotto:
					case UnitId::ZergSovthrathMound:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
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
					if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
						|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
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
					if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
						|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
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
					if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
						|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
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
					if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
						|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
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
					if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
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
					if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
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
					if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlaszilArbor))
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
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlaszilArbor))
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