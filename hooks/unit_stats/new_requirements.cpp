#include "new_requirements.h"
#include <hook_tools.h>

namespace hooks
{
	s8 createUnitAllowed_Partial(s16 unitId, Unit* unit, s32 playerId)
	{
		s8 result = 0;

		*lastInternalErr = InternalErrors::UnknownError0;
		if (unitId >= UnitId::None)
		{
			*lastInternalErr = InternalErrors::UnknownError16;
			return 0;
		}
		if (playerId != unit->playerId)
		{
			*lastInternalErr = InternalErrors::UnknownError1;
			return 0;
		}
		if (!(unit->status & UnitStatus::Completed))
		{
			*lastInternalErr = InternalErrors::UnknownError20;
			return 0;
		}
		if (unit->isFrozen())
		{
			*lastInternalErr = InternalErrors::UnitFrozen;
			return 0;
		}
		if (!UnitAvailability->available[unit->playerId][unitId])
		{
			*lastInternalErr = InternalErrors::UnknownError2;
			return 0;
		}
		if (unit->isQueueSlotActive(4))
		{
			*lastInternalErr = InternalErrors::UnknownError3;
			return 0;
		}
		if (unit->status & UnitStatus::IsHallucination)
		{
			*lastInternalErr = InternalErrors::UnknownError22;
			return 0;
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
						return 1;
					}
					case UnitId::ZergZethrokor:
					case UnitId::ZergQuazilisk:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergLiivralPond)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergQuazrokPool))
						{
							return 1;
						}
						return -1;
					}
					case UnitId::ZergHydralisk:
					case UnitId::ZergSkithrokor:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergGorgralSwamp)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergHydrokDen))
						{
							return 1;
						}
						return -1;
					}
					case UnitId::ZergMutalisk:
					case UnitId::ZergNathrokor:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergVithrathHaunt)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergMuthrokSpire))
						{
							return 1;
						}
						return -1;
					}
					case UnitId::SharedCohort:
					case UnitId::SharedAxitrilisk:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
							|| PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergOthstolOviform)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergIrolIris))
						{
							return 1;
						}
						return -1;
					}
					case UnitId::ZergZoryusthaleth:
					case UnitId::ZergLakizilisk:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergUltrakCavern)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergZorkizShroud))
						{
							return 1;
						}
						return -1;
					}
					case UnitId::ZergMatraleth:
					case UnitId::ZergKonvilisk:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlmakisAntre)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergMatravilNest))
						{
							return 1;
						}
						return -1;
					}
					case UnitId::SharedTarasque:
					case UnitId::SharedCharlatan:
					case UnitId::SharedEmpress:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
						{
							return 1;
						}
						return -1;
					}
					case UnitId::ZergKeskathalor:
					case UnitId::ZergGeszithalor:
					{
						if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlkajChasm)
							|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergGeszkathGrotto))
						{
							return 1;
						}
						return -1;
					}
					case UnitId::ZergIsthrathaleth:
					case UnitId::ZergSovroleth:
					{
						if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergSovthrathMound))
						{
							return 1;
						}
						return -1;
					}
					case UnitId::SharedAnticthon:
					{
						if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlaszilArbor))
						{
							return 1;
						}
						return -1;
					}
					default:
					{
						return 0;
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
						return 1;
					}
					return -1;
				}
				return 0;
			}
			case UnitId::ZergZethrokor:
			{
				if (unitId == UnitId::ZergVorvrokor)
				{
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergLiivralPond))
					{
						return 1;
					}
					return -1;
				}
				return 0;
			}
			case UnitId::ZergQuazilisk:
			{
				if (unitId == UnitId::ZergLiiralisk)
				{
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergLiivralPond))
					{
						return 1;
					}
					return -1;
				}
				return 0;
			}
			case UnitId::ZergHydralisk:
			{
				if (unitId == UnitId::ZergBactalisk)
				{
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergGorgralSwamp))
					{
						return 1;
					}
					return -1;
				}
				return 0;
			}
			case UnitId::ZergSkithrokor:
			{
				if (unitId == UnitId::ZergGorgrokor)
				{
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergGorgralSwamp))
					{
						return 1;
					}
					return -1;
				}
				return 0;
			}
			case UnitId::ZergMutalisk:
			{
				if (unitId == UnitId::ZergVithrilisk)
				{
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergVithrathHaunt))
					{
						return 1;
					}
					return -1;
				}
				return 0;
			}
			case UnitId::ZergNathrokor:
			{
				if (unitId == UnitId::ZergKalkalisk)
				{
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergVithrathHaunt))
					{
						return 1;
					}
					return -1;
				}
				return 0;
			}
			case UnitId::ZergIroleth:
			{
				if (unitId == UnitId::ZergOthstoleth)
				{
					if (PLAYER::numberOfUnitsOfType(playerId, UnitId::ZergAlaszilArbor)
						|| PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergOthstolOviform))
					{
						return 1;
					}
					return -1;
				}
				return 0;
			}
			case UnitId::ZergZoryusthaleth:
			{
				if (unitId == UnitId::ZergUltrakor)
				{
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergUltrakCavern))
					{
						return 1;
					}
					return -1;
				}
				return 0;
			}
			case UnitId::ZergMatraleth:
			{
				if (unitId == UnitId::ZergAkistrokor)
				{
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlmakisAntre))
					{
						return 1;
					}
					return -1;
				}
				return 0;
			}
			case UnitId::ZergKonvilisk:
			{
				if (unitId == UnitId::ZergAlmaksalisk)
				{
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlmakisAntre))
					{
						return 1;
					}
					return -1;
				}
				return 0;
			}
			case UnitId::ZergOthstoleth:
			{
				if (unitId == UnitId::ZergAlaszileth)
				{
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlaszilArbor))
					{
						return 1;
					}
					return -1;
				}
				return 0;
			}
			case UnitId::ZergKeskathalor:
			{
				if (unitId == UnitId::ZergAlkajelisk)
				{
					if (PLAYER::numberOfCompletedUnitsOfType(playerId, UnitId::ZergAlkajChasm))
					{
						return 1;
					}
					return -1;
				}
				return 0;
			}
			default:
			{
				return 0;
			}
		}
	}
}