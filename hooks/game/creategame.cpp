#include "creategame.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace
{/*
// For singleplayerMeleeCreate
void switchComputerToOpen(); //004DBBE0
bool setLastOpenSlotToComputer(char a1); //004DBB70

// For loadGameCreate
s32 loadSave(); // 004CF5F0
s32 initSingleplayerHuman(); // 004DBE50
s32 createGame(s32 a1, char a2); // 004D3FC0
bool leaveGame(s32 a1); // 004C3D20
char setNetTblError(s32 a1, s32 a2, s32 a3, char a4, s32 a5); // 004BB300
bool nextLeaveGameMenu(void *a1); // 004DCC50
s32 sStrCopy(char a1, const char *a2, s32 a3); // 0041008E
s32 readMapData(s32 a1, const char *a2, s32 *a3, s32 a4); // 004BF5D0*/
}

namespace hooks
{
	/*s32 singleplayerMeleeCreate() {
		s32 v1; // esi

		if (multiPlayerMode)
			return 0;
		if (customSingleplayer[0])
		{
			if (gameData->victoryCondition || gameData->starting_units || gameData->tournamentModeEnabled)
			{
				switchComputerToOpen();
				playerTable[*LOCAL_NATION_ID].race = selectedSingleplayerRace[0];
				v1 = 0;
				do
				{
					if (selectedSingleplayerRace[v1 + 1] != 7) // RACE_Inactive
						setLastOpenSlotToComputer(selectedSingleplayerRace[v1 + 1]);
					++v1;
				} while (v1 < 8);
			}
		}
		return 1;
	};

	s32 loadGameCreate(s32 a1, char a2) { //004EE520 --
		s32 result; // eax
		void* v3; // ecx

		if (!loadGameFileHandle)
			return 1;
		if (loadSave() && initSingleplayerHuman())
		{
			isHost = 0;
			result = createGame((s32)&gameData, a2);
		}
		else
		{
			if (!outOfGame)
			{
				leaveGame(3);
				outOfGame = 1;
				setNetTblError(0, 0, 0, a2, 97);
				if (scMainState == 3)
				{
					gameActive = 0;
					nextSCMainState = 4;
					if (!is_in_replay)
						replayEndFrame = elapsed_time_frames;
				}
				nextLeaveGameMenu(v3);
			}
			result = 0;
		}
		return result;
	};

	s32 levelCheatCreate(char a1, s32 a2) { //004EE5B0 -- UNFINISHED
		s32 result; // eax
		const char *v3; // eax
		void *v4; // ecx
		s32 v5; // ecx
		__int8 *v6; // eax
		void *v7; // eax
		void *v8; // ecx
		void *v9; // ecx
		void *v10; // ecx
		char source; // [esp+0h] [ebp-1D4h]
		char a1; // [esp+104h] [ebp-D0h]
		char dest; // [esp+108h] [ebp-CCh]
		char v14; // [esp+128h] [ebp-ACh]
		char v15; // [esp+129h] [ebp-ABh]
		char v16; // [esp+12Ah] [ebp-AAh]
		char dst; // [esp+151h] [ebp-83h]
		char v18; // [esp+171h] [ebp-63h]
		char v19; // [esp+190h] [ebp-44h]
		__int8 v20[3]; // [esp+191h] [ebp-43h]
		char v21; // [esp+194h] [ebp-40h]
		char v22; // [esp+1B4h] [ebp-20h]

		if ( !levelCheatUnlocked )
		return 1;
		levelCheatUnlocked = 0;
		if ( !player_name[0] )
		{
		if ( *network_table > 0x47u )
			v3 = (char *)network_table + network_table[72];
		else
			v3 = "";
		sStrCopy("", v3, 0x19u);
		}
		if ( !campaignIndex )
		{
		sStrCopy(&source, current_map_filename, 0x104u);
		if ( !readMapData((s32)&v21, &source, (s32 *)&v21, 0) )
		{
			if ( !outOfGame )
			{
			leaveGame(3);
			outOfGame = 1;
			setNetTblError(0, 0, 0, a1, 97);
			if ( gwGameMode == 3 )
			{
				gameState = 0;
				gamePosition = 4;
				if ( !is_in_replay )
					Replay_FrameCount = elapsedTime;
			}
			nextLeaveGameMenu(v4);
			}
			return 0;
		}
		v5 = 0;
		v6 = &playerStructs[0].nType;
		do {
			if ( *v6 == 6 && ++v5 != 1 )
				*v6 = 5;
			v6 += 36;
		} while ( (s32)v6 < (s32)&playerStructs[8].nType );
		goto LABEL_21;
		}
		if ( readMapData((s32)&v21, *(const char **)&mapdataTable[4 * campaignIndex], (s32 *)&v21, 1) )
		{
		LABEL_21:
		memset(&a1, 0, 0x8Cu);
		v19 = 0;
		sStrCopy(&dest, "", 0x18u);
		sStrCopy(&dst, mapName, 0x20u);
		v14 = 1;
		v15 = 1;
		v16 = optionList.speed;
		v7 = readTemplate((s32)"Use Map Settings(1)", a2, &v22, &v22);
		if ( v7 )
		{
			qmemcpy(&v18, v7, 0x20u);
			SMemFree(v7, "Starcraft\\SWAR\\lang\\game.cpp", 342, 0);
			if ( sub_4DBE50() )
			{
			isHost = 0;
			result = CreateGame((s32)&a1, v20);
			}
			else
			{
			if ( !outOfGame )
			{
				leaveGame(3);
				outOfGame = 1;
				setNetTblError(0, 0, 0, v20, 97);
				if ( gwGameMode == 3 )
				{
				gameState = 0;
				gamePosition = 4;
				if ( !is_in_replay )
					Replay_FrameCount = elapsedTime;
				}
				nextLeaveGameMenu(v10);
			}
			result = 0;
			}
		}
		else
		{
			if ( !outOfGame )
			{
			leaveGame(3);
			outOfGame = 1;
			setNetTblError(0, 0, 0, v20, 102);
			if ( gwGameMode == 3 )
			{
				gameState = 0;
				gamePosition = 4;
				if ( !is_in_replay )
				{
				v8 = (void *)elapsedTime;
				Replay_FrameCount = elapsedTime;
				}
			}
			nextLeaveGameMenu(v8);
			}
			result = 0;
		}
		return result;
		}
		if ( !outOfGame )
		{
		leaveGame(3);
		outOfGame = 1;
		setNetTblError(0, 0, 0, a1, 97);
		if ( gwGameMode == 3 )
		{
			gameState = 0;
			gamePosition = 4;
			if ( !is_in_replay )
			{
			v9 = (void *)elapsedTime;
			Replay_FrameCount = elapsedTime;
			}
		}
		nextLeaveGameMenu(v9);
		}
		return 0;
		};*/
}

namespace
{/*
const u32 Func_switchComputerToOpen = 0x004DBBE0;
void switchComputerToOpen() {
	__asm {
		PUSHAD
		CALL Func_switchComputerToOpen
		POPAD
	}
};

const u32 Func_setLastOpenSlotToComputer = 0x004DBB70;
bool setLastOpenSlotToComputer(char a1) {
	static char a1;
	static bool result;
	__asm {
		PUSHAD
		CALL Func_setLastOpenSlotToComputer
		MOV EAX, result
		POPAD
	}
	return result;
};*/
}