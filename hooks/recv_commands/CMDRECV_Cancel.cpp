#include "CMDRECV_Cancel.h"
#include <SCBW/api.h>
#include "hooks\orders\building_making\building_morph.h"

namespace hooks
{
	// hooked to allow non-ghosts units to cancel nukes
	void CMDRECV_CancelNuke()
	{
		Unit* current_unit;
		*selectionIndexStart = 0;
		current_unit = getActivePlayerNextSelection();
		while (current_unit != nullptr)
		{
			// Why is it like this? We should instead check if the unit has a connected nuke, rather than doing insipid hardcode.
			if (current_unit->id == UnitId::TerranEidolon
				|| current_unit->id == UnitId::TerranSeraph
				)
			{
				Unit* nuke = current_unit->connectedUnit;
				if (nuke != nullptr && nuke->id == UnitId::TerranNuclearMissile)
				{
					current_unit->connectedUnit = nullptr;
					nuke->connectedUnit = nullptr;
				}
			}
			current_unit = getActivePlayerNextSelection();
		}
	}

	// hooked because the default code didn't allow for multiple selection
	void CMDRECV_CancelAddon()
	{
		Unit* unit;
		*selectionIndexStart = 0;
		unit = getActivePlayerNextSelection();
		if (unit != nullptr &&
			getActivePlayerNextSelection() == nullptr &&
			unit->secondaryOrderId == OrderId::BuildAddon &&
			unit->status & UnitStatus::GroundedBuilding
			)
		{
			Unit* addon = unit->currentBuildUnit;
			if (addon != nullptr &&
				!(addon->status & UnitStatus::Completed) &&
				unit->status & UnitStatus::Completed &&
				unit->playerId == *ACTIVE_NATION_ID
				)
			{
				helper::CancelAddon_Proc(unit);
				scbw::refreshConsole();
			}
		}
	}

	// hooked because the default code didn't allow for multiple selection
	void CMDRECV_CancelUpgrade()
	{
		Unit* unit;
		*selectionIndexStart = 0;
		unit = getActivePlayerNextSelection();
		if (unit != nullptr &&
			getActivePlayerNextSelection() == nullptr &&
			unit->status & UnitStatus::Completed &&
			unit->playerId == *ACTIVE_NATION_ID
			)
		{
			helper::CancelUpgrade_Proc(unit);
			scbw::refreshConsole();
		}
	}

	// hooked because the default code didn't allow for multiple selection
	void CMDRECV_CancelResearch()
	{
		Unit* unit;
		*selectionIndexStart = 0;
		unit = getActivePlayerNextSelection();
		if (unit != nullptr &&
			getActivePlayerNextSelection() == nullptr &&
			units_dat::BaseProperty[unit->id] & UnitProperty::Building &&
			unit->status & UnitStatus::Completed &&
			unit->playerId == *ACTIVE_NATION_ID
			)
		{
			helper::CancelResearch_Proc(unit);
			scbw::refreshConsole();
		}
	}

	// hooked because the default code didn't allow for multiple selection
	void CMDRECV_CancelTrain(u16 wUnitID)
	{
		Unit* unit;

		*selectionIndexStart = 0;
		unit = getActivePlayerNextSelection();
		if (unit != nullptr &&
			getActivePlayerNextSelection() == nullptr &&
			unit->id != UnitId::ZergLarva &&
			unit->id != UnitId::ZergMutalisk &&
			unit->id != UnitId::ZergHydralisk &&
			unit->id != UnitId::ZergHatchery &&
			unit->id != UnitId::ZergLair &&
			unit->id != UnitId::ZergMuthrokSpire &&
			unit->id != UnitId::ZergCreepColony &&
			unit->status & UnitStatus::Completed &&
			unit->playerId == *ACTIVE_NATION_ID
			)
		{
			if (wUnitID == 0xFE)
			{
				helper::function_00466E40(unit);
				scbw::refreshConsole();
			}
			else
				if (wUnitID == 0xFF)
					scbw::refreshConsole();
				else
				{
					helper::refundQueueSlot(unit, wUnitID);
					scbw::refreshConsole();
				}
		}
	}

	void CMDRECV_CancelUnitMorph()
	{
		Unit* current_unit;
		*selectionIndexStart = 0;
		current_unit = getActivePlayerNextSelection();
		while (current_unit != nullptr)
		{
			if (current_unit->playerId == *ACTIVE_NATION_ID)
				helper::AI_CancelStructure(current_unit);
			current_unit = getActivePlayerNextSelection();
		}
	}

	// hooked because the default code didn't allow for multiple selection
	void CMDRECV_CancelConstruction()
	{
		Unit* unit;
		*selectionIndexStart = 0;
		unit = getActivePlayerNextSelection();
		if (unit != nullptr)
		{
			if (unit->id == 158 || unit->id == 161)
			{
				if (unit->getOverlay(ImageId::WarpAnchor))
				{
					unit->removeOverlay(ImageId::WarpAnchor);
				}
			}
		}
		if (unit != nullptr &&
			getActivePlayerNextSelection() == nullptr &&
			unit->playerId == *ACTIVE_NATION_ID
			)
		{
			helper::AI_CancelStructure(unit);
		}
	}
}