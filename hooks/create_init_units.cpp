#include "create_init_units.h"
#include <SCBW/api.h>
#include <SCBW/UnitFinder.h>
#include <vector>

//helper functions def

namespace {

Bool32 spreadsCreep(u32 unitId, Bool32 defaultReturn);		//13870
void function_004148F0(int x, int y, u32 unitId);			//148F0
void CreateBuildingLarva(CUnit* main_building);				//9D660
Bool32 function_0049EC30(CUnit* unit);						//9EC30
void updateUnitStrength(CUnit* unit);						//9FA40
void function_004A01F0(CUnit* unit);						//A01F0
CUnit* createUnit(u32 unitId, int x, int y, u32 playerId);	//A09D0
CUnit* FindBestUnit(Box16* coords, u32 searchProc);			//E8830
	
} //unnamed namespace

u8 GetRaceTownhall(u8 raceId) {
	if (raceId == RaceId::Zerg)
		return UnitId::ZergHatchery;
	else if (raceId == RaceId::Terran)
		return UnitId::TerranCommandCenter;
	else if (raceId == RaceId::Protoss)
		return UnitId::ProtossNexus;
	else
		return UnitId::None;
}

u8 GetRaceWorker(u8 raceId) {
	if (raceId == RaceId::Zerg)
		return UnitId::ZergDrone;
	else if (raceId == RaceId::Terran)
		return UnitId::TerranSCV;
	else if (raceId == RaceId::Protoss)
		return UnitId::ProtossProbe;
	else
		return UnitId::None;
}

namespace hooks {

void CreateInitialMeleeBuildings(u8 raceId, u32 playerId) {
	u8 buildingId = GetRaceTownhall(raceId);
	s32 buildingSizeX, buildingSizeY;
	Box16 searchBox; 
	CUnit* created_unit;

	buildingSizeX = units_dat::BuildingDimensions[buildingId].x;
	buildingSizeY = units_dat::BuildingDimensions[buildingId].y;

	if (buildingSizeX < 0)
		buildingSizeX++;
	if (buildingSizeY < 0)
		buildingSizeY++;

	searchBox.left = (startPositions[playerId].x - (s16)(buildingSizeX / 2)) & 0x0000FFE0;
	searchBox.top = (startPositions[playerId].y - (s16)(buildingSizeY / 2)) & 0x0000FFE0;
	searchBox.bottom = buildingSizeY / 2 + ((startPositions[playerId].y - (s16)(buildingSizeY / 2)) & 0x0000FFE0)  + buildingSizeY / 2 - 1;
	searchBox.right = buildingSizeX / 2 + ((startPositions[playerId].x - (s16)(buildingSizeX / 2)) & 0x0000FFE0) + buildingSizeX / 2 - 1;

	//yes, the return value is unused for some reason
	FindBestUnit(&searchBox,0x0049D640);

	created_unit = 
		createUnit(
			buildingId,
			((startPositions[playerId].x - (buildingSizeX / 2)) & 0x0000FFE0) + buildingSizeX / 2,
			((startPositions[playerId].y - (buildingSizeY / 2)) & 0x0000FFE0)  + buildingSizeY / 2,
			playerId
		);

	if (created_unit != NULL) {

		function_004A01F0(created_unit);

		if (function_0049EC30(created_unit) != 0)
			updateUnitStrength(created_unit);

		if (spreadsCreep(created_unit->id, 1)
			|| units_dat::BaseProperty[created_unit->id] & UnitProperty::CreepBuilding)
		{
			function_004148F0(
				created_unit->sprite->position.x,
				created_unit->sprite->position.y,
				created_unit->id
			);
			//CreateBuildingLarva(created_unit);
			//CreateBuildingLarva(created_unit);
			//Starting larva is 1 - HYDRA
		}
	}
}

;

class MineralFieldAveragePosProc : public scbw::UnitFinderCallbackProcInterface
{
private:
	const u16 refX;
	const u16 refY;
	u8 cliffLevel;
	int& sumX;
	int& sumY;
	u32& count;

public:
	MineralFieldAveragePosProc(const u16& refX, const u16& refY, int& sumX, int& sumY, u32& count) :
		refX(refX), refY(refY), sumX(sumX), sumY(sumY), count(count)
	{
		cliffLevel = scbw::getActiveTileAt(refX, refY).groundHeight;
	}

	void proc(CUnit* unit) override {
		if ((unit->id == UnitId::ResourceMineralField
		  || unit->id == UnitId::ResourceMineralFieldType2
		  || unit->id == UnitId::ResourceMineralFieldType3)
		  && scbw::getActiveTileAt(unit->position.x, unit->position.y).groundHeight == cliffLevel) {
			Point16 pos = unit->position;
			sumX += pos.x - refX;
			sumY += pos.y - refY;
			count += 1;
		}
	}
};

double Sign(double x) {
	return x < 0.0 ? -1.0 : x > 0.0 ? 1.0 : 0.0;
}

Point16 FindWorkerSpawnPositions(int townhallId, const u16& startX, const u16& startY, bool& horizontal)
{
	std::vector<Point16> positions;
	int sumX = 0;
	int sumY = 0;
	u32 count = 0;

	scbw::UnitFinder unitFinder;
	unitFinder.search(startX - 10 * 32, startY - 10 * 32, startX + 10 * 32, startY + 10 * 32);
	unitFinder.forEach(MineralFieldAveragePosProc(startX, startY, sumX, sumY, count));

	if (count == 0) { // default to bottom horizontal
		count = 1;
		sumX = 0;
		sumY = 1;
	}

	double averageX = (double)sumX / count;
	double averageY = (double)sumY / count;
	double distance = pow(pow(averageX, 2) + pow(averageY, 2), 0.5);

	double offsetX = averageX / distance;
	double offsetY = averageY / distance;

	const double distanceMultiplier = 1.4;
	double width = distanceMultiplier;
	double height = distanceMultiplier;

	if (offsetX < 0) {
		width *= units_dat::UnitBounds[townhallId].left;
	}
	else {
		width *= units_dat::UnitBounds[townhallId].right;
	}

	if (offsetY < 0) {
		height *= units_dat::UnitBounds[townhallId].top;
	}
	else {
		height *= units_dat::UnitBounds[townhallId].bottom;
	}

	offsetX = offsetX * width;
	offsetY = offsetY * height;

	const double diagonalThreshold = 0.5; // 1 - full diagonal, 0 - straight line
	bool forceHorizontal = false;
	double max = std::max(abs(offsetX), abs(offsetY));
	double min = std::min(abs(offsetX), abs(offsetY));
	if (min != 0 && diagonalThreshold != 0) {
		forceHorizontal = max / min < 1 / diagonalThreshold;
	}

	if (forceHorizontal ||
		abs(offsetX) < abs(offsetY)) {
		offsetX = -units_dat::UnitBounds[townhallId].left * distanceMultiplier;
		offsetY = Sign(offsetY) * height;
		horizontal = true;
	} else {
		offsetX = Sign(offsetX) * width;
		offsetY = units_dat::UnitBounds[townhallId].bottom * distanceMultiplier * 1.5;
		horizontal = false;
	}

	return Point16{ (u16)(startX + offsetX), (u16)(startY + offsetY) };
}

void CreateInitialMeleeUnits() {
	u32 playerId;
	u8* raceIdOffset;

	playerId = 8;
	raceIdOffset = (u8*)0x0057F009;

	while(raceIdOffset != (u8*)0x0057EEE9) {

		raceIdOffset -= 0x24;
		playerId--;

		if(*(raceIdOffset-1) == 2 || *(raceIdOffset-1) == 1) {
			//0 is use map settings,1 is Workers, 2 is Workers + Main Building
			u8 startingUnits = (u8)(*(u16*)(0x00596871));

			static u8* victory_conditions = (u8*)0x0059686D;
			static u8* tournament_mode = (u8*)0x00596877;
			static u8* user_select_slots = (u8*)0x0059BDA8;

			if (*victory_conditions == 0					// check triggers for conditions
			  && startingUnits == 0 					// use map units
			  &&*tournament_mode == 0					// tournament mode off
			  && playerId < 8
			  && (user_select_slots[playerId]) != 0	// race selected instead of using map settings?
			) {
				startingUnits = 2;
			}

			if (startingUnits == 2) {
				CreateInitialMeleeBuildings(*raceIdOffset, playerId);

				if (*raceIdOffset == RaceId::Zerg)
					CreateInitialOverlord(playerId);
			}

			if (startingUnits == 1 || startingUnits == 2) {

				u8 unitId = GetRaceWorker(*raceIdOffset);
				u8 townhallId = GetRaceTownhall(*raceIdOffset);
				CUnit* created_unit;
				
				if (unitId != UnitId::None) {
					Point16 startPos = startPositions[playerId];
					bool horizontal = true;
					Point16 pos = FindWorkerSpawnPositions(townhallId, startPos.x, startPos.y, horizontal);

					for (int i = 0; i < 6; i++) {
						const int correctionOffset = 1;
						const int unitWidth = units_dat::UnitBounds[unitId].left + units_dat::UnitBounds[unitId].right + correctionOffset;
						const int unitHeight = units_dat::UnitBounds[unitId].bottom + units_dat::UnitBounds[unitId].top + correctionOffset;

						if (horizontal) pos.x += unitWidth;
						else pos.y -= unitHeight;

						created_unit = createUnit(unitId, pos.x, pos.y, playerId);
						scbw::teleport(created_unit, pos);

						if (created_unit != NULL) {
							function_004A01F0(created_unit);

							if (function_0049EC30(created_unit) != 0)
								updateUnitStrength(created_unit);
						}
					}
				}
			}
		}

	}

}

;

void CreateInitialOverlord(u8 playerId) {
	static u16* const maxBoxRightValue = (u16*) 0x00628450;
	static u16* const maxBoxBottomValue = (u16*) 0x006284B4;

	int overlordStartX, overlordStartY;
	CUnit* created_unit;

	if(startPositions[playerId].x < *maxBoxRightValue / 2)
		overlordStartX = startPositions[playerId].x + units_dat::UnitBounds[UnitId::ZergHatchery].right + units_dat::UnitBounds[UnitId::ZergHatchery].left + 1;
	else
		overlordStartX = startPositions[playerId].x - (units_dat::UnitBounds[UnitId::ZergHatchery].right + units_dat::UnitBounds[UnitId::ZergHatchery].left + 1);

	if(startPositions[playerId].y < *maxBoxBottomValue / 2)
		overlordStartY = startPositions[playerId].y + units_dat::UnitBounds[UnitId::ZergHatchery].bottom + units_dat::UnitBounds[UnitId::ZergHatchery].top + 1;
	else
		overlordStartY = startPositions[playerId].y - (units_dat::UnitBounds[UnitId::ZergHatchery].bottom + units_dat::UnitBounds[UnitId::ZergHatchery].top + 1);

	created_unit = createUnit(UnitId::ZergOverlord,overlordStartX,overlordStartY,playerId);

	if(created_unit != NULL) {

		function_004A01F0(created_unit);

		if(function_0049EC30(created_unit) != 0)
			updateUnitStrength(created_unit);

	}
}

;

} //namespace hooks

;

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

const u32 Func_spreadsCreep = 0x00413870;
Bool32 spreadsCreep(u32 unitId, Bool32 defaultReturn) {

	Bool32 bReturnValue;

	__asm {
		PUSHAD
		MOV EAX, unitId
		PUSH defaultReturn
		CALL Func_spreadsCreep
		MOV bReturnValue, EAX
		POPAD
	}

	return bReturnValue;

}

;

const u32 Func_Sub4148F0 = 0x004148F0;
void function_004148F0(int x, int y, u32 unitId) {

	__asm {
		PUSHAD
		PUSH y
		PUSH x
		PUSH unitId
		CALL Func_Sub4148F0
		POPAD
	}

}

;

const u32 Func_Sub49D660 = 0x0049D660;
void CreateBuildingLarva(CUnit* main_building) {

	__asm {
		PUSHAD
		MOV ESI, main_building
		CALL Func_Sub49D660
		POPAD
	}

}

;

const u32 Func_Sub49EC30 = 0x0049EC30;
Bool32 function_0049EC30(CUnit* unit) {

	Bool32 bResult;

	__asm {
		PUSHAD
		MOV ESI, unit
		CALL Func_Sub49EC30
		MOV bResult, EAX
		POPAD
	}

	return bResult;

}

;

const u32 Func_UpdateUnitStrength = 0x0049FA40;
void updateUnitStrength(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_UpdateUnitStrength
		POPAD
	}

}

;

const u32 Func_Sub4A01F0 = 0x004A01F0;
void function_004A01F0(CUnit* unit) {

	__asm {
		PUSHAD
		MOV EAX, unit
		CALL Func_Sub4A01F0
		POPAD
	}

}

;

const u32 Func_CreateUnit = 0x004A09D0;
CUnit* createUnit(u32 unitId, int x, int y, u32 playerId) {

	static CUnit* unit_created;

	__asm {
		PUSHAD
		PUSH playerId
		PUSH y
		MOV ECX, unitId
		MOV EAX, x
		CALL Func_CreateUnit
		MOV unit_created, EAX
		POPAD
	}

	return unit_created;

}

;

const u32 Func_FindBestUnit = 0x004E8830;
CUnit* FindBestUnit(Box16* coords, u32 searchProc) {

	static CUnit* unit;

	__asm {
		PUSHAD
		MOV EAX, coords
		MOV EDI, searchProc
		CALL Func_FindBestUnit
		MOV unit, EAX
		POPAD
	}

	return unit;

}

;

} //Unnamed namespace

//End of helper functions
