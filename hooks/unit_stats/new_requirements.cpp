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
		}
		return TECH_HIDDEN;
	}
}