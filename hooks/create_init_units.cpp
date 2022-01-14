#include "create_init_units.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include <vector>

u8 GetRaceTownhall(u8 raceId)
{
	if (raceId == RaceId::Zerg)
		return UnitId::ZergHatchery;
	else if (raceId == RaceId::Terran)
		return UnitId::TerranCommandCenter;
	else if (raceId == RaceId::Protoss)
		return UnitId::ProtossNexus;
	else
		return UnitId::None;
}

u8 GetRaceWorker(u8 raceId)
{
	if (raceId == RaceId::Zerg)
		return UnitId::ZergDroleth;
	else if (raceId == RaceId::Terran)
		return UnitId::TerranMason;
	else if (raceId == RaceId::Protoss)
		return UnitId::ProtossScribe;
	else
		return UnitId::None;
}

namespace hooks
{

	void CreateInitialMeleeBuildings(u8 raceId, u32 playerId)
	{
		u8 buildingId = GetRaceTownhall(raceId);
		s32 buildingSizeX, buildingSizeY;
		Box16 searchBox;
		Unit* created_unit;

		buildingSizeX = units_dat::BuildingDimensions[buildingId].x;
		buildingSizeY = units_dat::BuildingDimensions[buildingId].y;
		if (buildingSizeX < 0)
		{
			buildingSizeX++;
		}
		if (buildingSizeY < 0)
		{
			buildingSizeY++;
		}

		searchBox.left = (startPositions[playerId].x - (s16)(buildingSizeX / 2)) & 0x0000FFE0;
		searchBox.top = (startPositions[playerId].y - (s16)(buildingSizeY / 2)) & 0x0000FFE0;
		searchBox.bottom = buildingSizeY / 2 + ((startPositions[playerId].y - (s16)(buildingSizeY / 2)) & 0x0000FFE0) + buildingSizeY / 2 - 1;
		searchBox.right = buildingSizeX / 2 + ((startPositions[playerId].x - (s16)(buildingSizeX / 2)) & 0x0000FFE0) + buildingSizeX / 2 - 1;

		// yes, the return value is unused for some reason
		helper::FindBestUnit(&searchBox, 0x0049D640);
		created_unit = helper::createUnit(buildingId,
				((startPositions[playerId].x - (buildingSizeX / 2)) & 0x0000FFE0) + buildingSizeX / 2,
				((startPositions[playerId].y - (buildingSizeY / 2)) & 0x0000FFE0) + buildingSizeY / 2,
				playerId);

		if (created_unit != nullptr)
		{

			helper::updateUnitStatsFinishBuilding(created_unit);

			if (helper::show_finished_unit(created_unit) != 0)
			{
				helper::updateUnitStrength(created_unit);
			}
			if (helper::spreadsCreep(created_unit->id, 1)
				|| units_dat::BaseProperty[created_unit->id] & UnitProperty::CreepBuilding)
			{
				helper::applyCreepAtLocationFromUnitType(created_unit->sprite->position.x, created_unit->sprite->position.y, created_unit->id);
			}
		}
	}

	f64 Sign(f64 x)
	{
		return x < 0.0 ? -1.0 : x > 0.0 ? 1.0 : 0.0;
	}

	Point16 FindWorkerSpawnPositions(s32 townhallId, const u16& startX, const u16& startY, bool& horizontal)
	{
		std::vector<Point16> positions;
		s32 sumX = 0;
		s32 sumY = 0;
		u32 count = 0;

		scbw::UnitFinder unitFinder;
		unitFinder.search(
			startX - 10 * 32,
			startY - 10 * 32,
			startX + 10 * 32,
			startY + 10 * 32
		);
		u8 cliffLevel = scbw::getActiveTileAt(startX, startY).groundHeight;
		unitFinder.forEach([&](Unit* unit)
		{
			if ((unit->id == UnitId::ResourceMineralField
				|| unit->id == UnitId::ResourceMineralFieldType2
				|| unit->id == UnitId::ResourceMineralFieldType3)
				&& scbw::getActiveTileAt(unit->position.x, unit->position.y).groundHeight == cliffLevel)
			{
				Point16 pos = unit->position;
				sumX += pos.x - startX;
				sumY += pos.y - startY;
				count += 1;
			}
		});

		if (count == 0)
		{ // default to bottom horizontal
			count = 1;
			sumX = 0;
			sumY = 1;
		}

		f64 averageX = (f64)sumX / count;
		f64 averageY = (f64)sumY / count;
		f64 distance = pow(pow(averageX, 2) + pow(averageY, 2), 0.5);

		f64 offsetX = averageX / distance;
		f64 offsetY = averageY / distance;

		const f64 distanceMultiplier = 1.4;
		f64 width = distanceMultiplier;
		f64 height = distanceMultiplier;

		if (offsetX < 0)
		{
			width *= units_dat::UnitBounds[townhallId].left;
		}
		else
		{
			width *= units_dat::UnitBounds[townhallId].right;
		}

		if (offsetY < 0)
		{
			height *= units_dat::UnitBounds[townhallId].top;
		}
		else
		{
			height *= units_dat::UnitBounds[townhallId].bottom;
		}

		offsetX = offsetX * width;
		offsetY = offsetY * height;

		const f64 diagonalThreshold = 0.5; // 1 - full diagonal, 0 - straight line
		bool forceHorizontal = false;
		f64 max = std::max(abs(offsetX), abs(offsetY));
		f64 min = std::min(abs(offsetX), abs(offsetY));
		if (min != 0 && diagonalThreshold != 0)
		{
			forceHorizontal = max / min < 1 / diagonalThreshold;
		}

		if (forceHorizontal ||
			abs(offsetX) < abs(offsetY))
		{
			offsetX = -units_dat::UnitBounds[townhallId].left * distanceMultiplier;
			offsetY = Sign(offsetY) * height;
			horizontal = true;
		}
		else
		{
			offsetX = Sign(offsetX) * width;
			offsetY = units_dat::UnitBounds[townhallId].bottom * distanceMultiplier * 1.5;
			horizontal = false;
		}

		return Point16{(u16)(startX + offsetX), (u16)(startY + offsetY)};
	}

	void CreateInitialMeleeUnits()
	{
		u32 playerId;
		u8* raceIdOffset;

		playerId = 8;
		raceIdOffset = (u8*)0x0057F009;

		while (raceIdOffset != (u8*)0x0057EEE9)
		{

			raceIdOffset -= 0x24;
			playerId--;

			if (*(raceIdOffset - 1) == 2 || *(raceIdOffset - 1) == 1)
			{
				//0 is use map settings,1 is Workers, 2 is Workers + Main Building
				u8 startingUnits = (u8)(*(u16*)(0x00596871));

				static u8* victory_conditions = (u8*)0x0059686D;
				static u8* tournament_mode = (u8*)0x00596877;
				static u8* user_select_slots = (u8*)0x0059BDA8;

				if (*victory_conditions == 0					// check triggers for conditions
					&& startingUnits == 0 					// use map units
					&& *tournament_mode == 0					// tournament mode off
					&& playerId < 8
					&& (user_select_slots[playerId]) != 0	// race selected instead of using map settings?
					)
				{
					startingUnits = 2;
				}

				if (startingUnits == 2)
				{
					CreateInitialMeleeBuildings(*raceIdOffset, playerId);

					if (*raceIdOffset == RaceId::Zerg)
						CreateInitialOverlord(playerId);
				}

				if (startingUnits == 1 || startingUnits == 2)
				{

					u8 unitId = GetRaceWorker(*raceIdOffset);
					u8 townhallId = GetRaceTownhall(*raceIdOffset);
					Unit* created_unit;

					if (unitId != UnitId::None)
					{
						Point16 startPos = startPositions[playerId];
						bool horizontal = true;
						Point16 pos = FindWorkerSpawnPositions(townhallId, startPos.x, startPos.y, horizontal);

						for (s32 i = 0; i < 6; i++)
						{
							const s32 correctionOffset = 1;
							const s32 unitWidth = units_dat::UnitBounds[unitId].left + units_dat::UnitBounds[unitId].right + correctionOffset;
							const s32 unitHeight = units_dat::UnitBounds[unitId].bottom + units_dat::UnitBounds[unitId].top + correctionOffset;

							if (horizontal) pos.x += unitWidth;
							else pos.y -= unitHeight;

							created_unit = helper::createUnit(unitId, pos.x, pos.y, playerId);
							scbw::teleport(created_unit, pos);

							if (created_unit != nullptr)
							{
								helper::updateUnitStatsFinishBuilding(created_unit);

								if (helper::show_finished_unit(created_unit) != 0)
									helper::updateUnitStrength(created_unit);
							}
						}
					}
				}
			}

		}

	}

	;

	void CreateInitialOverlord(u8 playerId)
	{
		static u16* const maxBoxRightValue = (u16*)0x00628450;
		static u16* const maxBoxBottomValue = (u16*)0x006284B4;

		s32 overlordStartX, overlordStartY;
		Unit* created_unit;

		if (startPositions[playerId].x < *maxBoxRightValue / 2)
			overlordStartX = startPositions[playerId].x + units_dat::UnitBounds[UnitId::ZergHatchery].right + units_dat::UnitBounds[UnitId::ZergHatchery].left + 1;
		else
			overlordStartX = startPositions[playerId].x - (units_dat::UnitBounds[UnitId::ZergHatchery].right + units_dat::UnitBounds[UnitId::ZergHatchery].left + 1);

		if (startPositions[playerId].y < *maxBoxBottomValue / 2)
			overlordStartY = startPositions[playerId].y + units_dat::UnitBounds[UnitId::ZergHatchery].bottom + units_dat::UnitBounds[UnitId::ZergHatchery].top + 1;
		else
			overlordStartY = startPositions[playerId].y - (units_dat::UnitBounds[UnitId::ZergHatchery].bottom + units_dat::UnitBounds[UnitId::ZergHatchery].top + 1);

		created_unit = helper::createUnit(UnitId::ZergOvileth, overlordStartX, overlordStartY, playerId);

		if (created_unit != nullptr)
		{

			helper::updateUnitStatsFinishBuilding(created_unit);

			if (helper::show_finished_unit(created_unit) != 0)
				helper::updateUnitStrength(created_unit);

		}
	}
}