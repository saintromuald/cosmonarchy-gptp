#include "glucustom.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace {

	//For raceDropdown
	char addItemToListing(char pszString, BinDlg** pDlg, __int8 bFlags);
	void createDialogDropdown(BinDlg** a1);
	void dlgSetSelected(unsigned __int8 a1, BinDlg** a2);
	
	//For initSingleplayerCustom
	bool isMeleeGame(char a1, __int16 a2, char a3);
	PLAYER randomizePlayerSlot();

	//For initGameLoad
	HANDLE stopMusic();
	FILE loadReplayFile(int a1, int a2, int a3);
	signed int levelCheatCreate(char a1, int a2);
	signed int loadGameCreate(int a1, char a2);
	signed int gameCreateSomething(char a1);
	signed int singleplayerMeleeCreate();
	int getSystemTime(LPSYSTEMTIME* a1);
	void initScreenLayer();
	int subTwoCheckTwo(void a1, int a2);
	void loadGameFonts();
	void srand(unsigned int a1);
	dwPOINT initPathArray();
	signed int appAddExit(int a1);
	char multiplayerChatSomething();
	void getPlayerStructs(void a1);
	PLAYER initStartingRacesAndTypes();
	char initMultiplayerSavePlayers();
	FILE initPlayers();
	void setVisionIndex();
	void initAiPlayerNames();
	__int16 loadRaceUI();
	bool hotkeyRemapping();
	signed int gameInit(a1);
	int getPlayerColors(int a1);
	signed int randomizePlayerColors();
	int authorizeMap(int a1, int a2, int a3, int a4);
	char bigPacketError(int a1, char 2, int a3, int a4, int a5);
	signed int loadGameCore(int a1);
	void chooseTriggerTemplate(int a1, char a2, int a3);
	int initPlayerNames();
	int tickCounterInit();
	int CMDRECV_SetReplaySpeed(int a1, unsigned __int8 a2, unsigned int a3);
	void copyPlayerStructsToReplay(void a1, void a2);
	signed int allocReplayCommands();
}

namespace hooks {

	void raceDropdown(BinDlg** dialog) { // 004ADF20 -- https://pastebin.com/raw/rXq01xR5
		BinDlg** dlgVar = dialog;		//v2
		bStringMap* race = 0x0051AAA0;	//singleRaceSelect
		wStringMap* type = 0x0051AAA8;	//singleTypeSelect
		__int16 stringId;				//v4
		char pszRaceStr;				//v5
		unsigned __int8 listing;		//v6
		int value;						//v7
		int flags;						//v8
		int dlgEvent;					//v9
		int unused1;					//v10
		__int16 unused2;				//v11

		dialog->flags |= 0x80000000;
		do {
			stringId = race->id - 1;
			if (race->id) {
				if (stringId < *networkTable) {
					pszRaceStr = (char*)networkTable + (char*)(networkTable[v4 + 1]);
				}
				else {
					pszRaceStr = "";
				}
			}
			else {
				pszRaceStr = 0;
			}
			listing = addItemToListing(pszRaceStr, dialog, 0);
			if (listing == -1) {
				break;
			}
			value = (unsigned __int8)dlgVar->value;
			dlgVar++;
			*(_DWORD*)(*(_DWORD*)&dlgVar->*childrenDlg + 4 * listing) = value;
		} while race < type;
		flags = dialog->flags;
		if (flags < 0) {
			dlgVar->flags = flags & 0x7FFFFFFF;
			createDialogDropdown(dlgVar);
		}
		if (dlgVar->field_46 > 3u) {
			unused2 = 14;
			dlgEvent = 11;
			unused1 = 3;
			dlgVar->*fxnInteract(dlgVar, (dlgEvent*)&dlgEvent);
			dlgSetSelected(3u, dlgVar);
		}
	};

	int initSingleplayerCustom(BinDlg** dialog) { // 004AD850 -- https://pastebin.com/raw/SQJXUWtG
		int eax;							//v1
		int pDropdown;
		PLAYER result;
		unsigned __int8 al;					//v4
		unsigned __int8 pDropdownBig;		//v5
		char spRace;						//v6
		unsigned __int8 slot;				//v7

		if (dialog->controlType) {
			dialog = *(binDlg**)&dialog->parent;
		}
		eax = *(int)&dialog->childrenDlg;
		if (eax) {
			while (eax + 16 != 28) {
				eax = *eax;
				if (!eax) {
					// goto label 6
				}
			}
			pDropdown = eax;
		}
		else {
			//label 6
			pDropdown = 0;
		}
		result = isMeleeGame(SBYTE1(scbw::selectedGameType), scbw::selectedGameTypeParam, scbw::selectedGameType);
		if (result) {
			scbw::selectedSingleplayerRace[1] = 0x7070707;
			customSingleplayer[0] = 1;
			scbw::selectedSingleplayerRace[5] = 0x7070707;
			if (pDropDown + 70) {
				pDropdownBig = pDropdown + 72;
				if (pDropdownBig != -1) {
					scbw::selectedSingleplayerRace[0] = (pDropdown + 66) + 4 * pDropdownBig;
				}
				result = pDropdown;
				if (pDropdown) {
					spRace = scbw::selectedSingleplayerRace[1];
					do {
						if (result->name[13] & 8) {
							if (result[1].name[23]) {
								slot = result[2]->slot;
								if (slot != -1) {
									spRace++ = result[1].name[19] + 4 * slot;
								}
							}
						}
						result = result->slot;
					} while (result);
				}
			}
			else if (pDropdown[6] & 8) {
				if ((pDropdown + 70) && (al = (pDropdown + 72), al != -1)) {
					scbw::playerTable[scbw::LOCAL_NATION_ID].race = (pDropdown + 66) + 4 * al;
					scbw::IS_IN_GAME_LOOP = 1;
					result = randomizePlayerSlot();
					scbw::IS_IN_GAME_LOOP = 0;
				}
				else {
					scbw::playerTable[scbw::LOCAL_NATION_ID].race = 6;
					scbw::IS_IN_GAME_LOOP = 1;
					result = randomizePlayerSlot();
					scbw::IS_IN_GAME_LOOP = 0;
				}
			}
		}
		return result;
	};
	
	int initGameLoad(int a1, char a2) { // 004EF100 -- https://pastebin.com/raw/szn0Z6sp
		int v2; // ecx
		int v3; // eax
		int v5; // esi
		char* v6; // esi
		int v7; // edx
		_DWORD* v8; // eax
		int v9; // ecx
		char v10; // [esp+8h] [ebp-4h]

		stopMusic();
		if (scbw::IS_IN_REPLAY)
		{
			if (!scbw::scenarioChk)
			{
				a2 = 0;
				loadReplayFile((int)scbw::CurrentMapFileName, v2, 0);
			}
			if (scbw::IS_IN_REPLAY)
			{
				*(_DWORD*)scbw::playerForce = *(_DWORD*)scbw::playerForceSomethingReplay;
				*(_DWORD*)&scbw::playerForce[4] = *(_DWORD*)&scbw::playerForceSomethingReplay[4];
			}
		}
		if (!scbw::loadGameFileHandle)
			scbw::elapsedTimeFrames = 0;
		if (!scbw::multiPlayerMode)
		{
			v3 = levelCheatCreate(a2, a1);
			if (!v3 || !loadGameCreate(v3, a2) || !gameCreateSomething(a2) || !singleplayerMeleeCreate())
				return 0;
			if (scbw::IS_IN_REPLAY)
			{
				scbw::initialSeed = scbw::replaySeed;
			}
			else
			{
				scbw::initialSeed = getSystemTime(0);
				scbw::replaySeed = scbw::initialSeed;
			}
		}
		initScreenLayer();
		scbw::mouseOverCallback = (int(__stdcall*)(_DWORD))subTwoCheckTwo;
		loadGameFonts();
		memset(scbw::randomCounts, 0, sizeof(scbw::randomCounts));
		scbw::randomCountsTotal = 0;
		scbw::lastRandomNumber = scbw::initialSeed;
		srand(scbw::initialSeed);
		initPathArray();
		appAddExit((int)NullCallback);
		if (!scbw::loadGameFileHandle)
			multiplayerChatSomething();
		randomizePlayerSlot();
		v5 = scbw::IS_IN_REPLAY;
		if (scbw::IS_IN_REPLAY)
			getPlayerStructs(scbw::playerTable);
		initStartingRacesAndTypes();
		if (!v5)
		{
			if (scbw::loadGameFileHandle)
				initMultiplayerSavePlayers();
			else
				initPlayers();
		}
		setVisionIndex();
		initAiPlayerNames();
		memset(scbw::randomCounts, 0, sizeof(scbw::randomCounts));
		scbw::randomCountsTotal = 0;
		scbw::lastRandomNumber = scbw::initialSeed;
		srand(scbw::initialSeed);
		loadRaceUI();
		hotkeyRemapping();
		if (!gameInit(v5))
			return 0;
		v6 = (char*)scbw::IS_IN_REPLAY;
		if (scbw::IS_IN_REPLAY)
			getPlayerColors(scbw::factionsColorsOrdering);
		if (!scbw::loadGameFileHandle)
			randomizePlayerColors();
		if (!v6 && !scbw::campaignIndex)
		{
			v6 = scbw::CurrentMapFileName;
			if (!authorizeMap((unsigned int)scbw::CurrentMapFileName, (int)&v10, 1, (int)scbw::mapArchiveHandle)
				&& !scbw::gameData.bVictoryCondition // 0x75
				&& !scbw::gameData.bStartingUnits // 0x79
				&& !scbw::gameData.bTournamentModeEnabled) // 0x7F
			{
				if (scbw::loadGameFileHandle)
				{
					bigPacketError(99, scbw::CurrentMapFileName, 0, 0, 1);
					return 0;
				}
				bigPacketError(97, 0, 0, 0, 1);
				return 0;
			}
		}
		scbw::CHEAT_STATE &= 0x28000200u;
		if (!loadGameCore((int)v6))
			return 0;
		if (!scbw::mapStarted)
		{
			if (!chooseTriggerTemplate(v7, (char*)&scbw::randomCounts[256], (int)v6))
			{
				bigPacketError(93, 0, 0, 0, 1);
				return 0;
			}
			if (!scbw::mapStarted)
			{
				*(_DWORD*)scbw::playerHasLeft = 0;
				scbw::lossType = 0;
				*(_DWORD*)&scbw::unkVictoryVariable = 0;
			}
		}
		initPlayerNames();
		if (!scbw::multiPlayerMode)
			tickCounterInit();
		scbw::saveLoadSuccess = (unsigned __int8)scbw::mapStarted;
		scbw::elapstedTimeModifier = scbw::mapStarted != 0 ? scbw::savedElapsedSeconds : 0;
		CMDRECV_SetReplaySpeed(optionList.speed, 0, 1u);
		if (scbw::IS_IN_REPLAY)
		{
			copyPlayerStructsToReplay(scbw::playerTable, &scbw::gameData);
			v8 = scbw::replayData;
			dword_6D5BF0 = 0;
			v9 = *((_DWORD*)scbw::replayData + 2);
			*(_DWORD*)scbw::replayData = 0;
			v8[1] = 1;
			v8[7] = v9;
			scbw::replayVisions = 255;
			scbw::playerVisions = 255;
			scbw::replayShowEntireMap = 0;
			scbw::nextReplayCommandFrame = -1;
			scbw::playerExploredVisions = 0xFF00;
		}
		else
		{
			allocReplayCommands();
		}
		return 1;
	};
	
} //namespace hooks

;

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

	//For raceDropdown
	const u32 Func_addItemToListing = 0x0041B760;
	char addItemToListing(char pszString, BinDlg** pDlg, __int8 bFlags) {
		static char result;

		__asm {
			PUSHAD
			MOV pszString, EAX
			MOV pDlg, ECX
			CALL Func_addItemToListing
			POPAD
			MOV EAX, result
		}
		return result;
	};

	const u32 Func_createDialogDropdown = 0x0041B410;
	void createDialogDropdown(BinDlg** a1) {
		__asm {
			PUSHAD
			MOV a1, EAX
			CALL Func_createDialogDropdown
			POPAD
		}
	};

	const u32 Func_dlgSetSelected = 0x0041A600;
	void dlgSetSelected(unsigned __int8 a1, BinDlg** a2) {
		__asm {
			PUSHAD
			MOVZX a1, BX
			MOV a2, ESI
			CALL Func_dlgSetSelected
			POPAD
		}
	};

	//For initSingleplayerCustom
	const u32 Func_isMeleeGame = 0x004AAFA0;
	bool isMeleeGame(char a1, __int16 a2, char a3) {
		static char a1;
		static __int16 a2;
		static char a3;
		static bool result;

		__asm {
			PUSHAD
			MOVZX a1, DL
			MOVZX a2, CX
			MOVZX a3, BL
			CALL Func_isMeleeGame
			POPAD
			MOV EAX, result
		}
		return result;
	};

	const u32 Func_randomizePlayerSlot = 0x004A9A30;
	PLAYER randomizePlayerSlot() {
		static PLAYER result;

		__asm {
			PUSHAD
			CALL Func_randomizePlayerSlot
			POPAD
			MOV EAX, result
		}
		return result;
	};

	//For initGameLoad

}