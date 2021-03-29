#include "glucustom.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace {
	/*
	//For raceDropdown
	char addItemToListing(char pszString, BinDlg* pDlg, __int8 bFlags);							//0041B760
	void createDialogDropdown(BinDlg* a1);														//0041B410
	void dlgSetSelected(unsigned __int8 a1, BinDlg* a2);										//0041A600
	
	//For initSingleplayerCustom
	bool isMeleeGame(s8 a1, s16 a2, s8 a3);														//004AD850
	PLAYER randomizePlayerSlot();																//004A9A30

	//For initGameLoad
	void* stopMusic();																			//004A5F50
	FILE loadReplayFile(s32 a1, s32 a2, s32 a3);												//004DF570
	s32 levelCheatCreate(s8 a1, s32 a2);														//004EE5B0
	s32 loadGameCreate(s32 a1, s8 a2);															//004EE520
	s32 gameCreateSomething(s8 a1);																//004EE3D0
	s32 singleplayerMeleeCreate();																//004EE110
	s32 getSystemTime(LPSYSTEMTIME* a1);														//0040C7DF
	void initScreenLayer();																		//0041A030
	s32 subTwoCheckTwo(void* a1, int a2);														//004EE0F0
	void loadGameFonts();																		//004DAE50
	void srand(u32 a1);																			//0040C7B0
	Point32 initPathArray();																	//0042F4F0
	s32 appAddExit(s32 a1);																		//004F6100
	char multiplayerChatSomething();															//0049B500
	void getPlayerStructs(void* a1);															//004DE9A0
	PLAYER initStartingRacesAndTypes();															//0049CC40
	char initMultiplayerSavePlayers();															//004A9250
	FILE initPlayers();																			//004A9D80
	void setVisionIndex();																		//004EE180
	void initAiPlayerNames();																	//004ED2B0
	s16 loadRaceUI();																			//004EE2D0
	bool hotkeyRemapping();																		//004EEC30
	s32 gameInit(s32 a1);																		//004EEE00
	s32 getPlayerColors(s32 a1);																//004DE950
	s32 randomizePlayerColors();																//0049B060
	s32 authorizeMap(s32 a1, s32 a2, s32 a3, s32 a4);											//004CC110
	s8 bigPacketError(s32 a1, s8 a2, s32 a3, s32 a4, s32 a5);									//004BB430
	s32 loadGameCore(s32 a1);																	//004EED10
	void chooseTriggerTemplate(s32 a1, s8 a2, s32 a3);											//004ABAF0
	s32 initPlayerNames();																		//004B2DF0
	s32 tickCounterInit();																		//004A2BF0
	s32 CMDRECV_SetReplaySpeed(s32 a1, u8 a2, u32 a3);											//004DEB90
	void copyPlayerStructsToReplay(void* a1, void* a2);											//004DE9D0
	s32 allocReplayCommands();																	//004CE280
	*/
	//
	void* loadInitCreditsBIN(char* a1);															// 004D8DF0
	bool SFileOpenArchive(char* archiveName, u32 dwPriority, u32 dwFlags, void* const* handle);	// 004100B2
	bool SFileCloseArchive(void* archive);														// 004100BE
	bool SFileOpenFileEx(void* hMPQ, const char* szFileName, u32 dwSearchScope, void** phFile);	// 004100C4
	bool SFileCloseFile(void* hFile);	                                                        // 004100C4
}

namespace hooks {
	/*
	void raceDropdown(BinDlg* a1) { // 004ADF20 -- https://pastebin.com/raw/rXq01xR5
		bStringMap* v2; // edi
		BinDlg* dlgDropdown; // esi
		__int16 v4; // ax
		char pszRaceStr; // eax
		unsigned __int8 v6; // al
		int v7; // ecx
		int v8; // eax
		int v9; // [esp+Ch] [ebp-14h]
		int v10; // [esp+10h] [ebp-10h]
		__int16 v11; // [esp+18h] [ebp-8h]

		dlgDropdown = a1;
		a1->flags |= 0x80000000;
//		v2 = &singleRaceSelect;
		do
		{
			v4 = (unsigned __int8)v2->id - 1;
			if (v2->id)
			{
				if (v4 < *networkTable)
					pszRaceStr = (char*)networkTable + networkTable[v4 + 1];
				else
					pszRaceStr = "";
			}
			else
			{
				pszRaceStr = 0;
			}
			v6 = addItemToListing(pszRaceStr, dlgDropdown, 0);
			if (v6 == -1)
				break;
			v7 = (unsigned __int8)v2->value;
			++v2;
			*(int*)(*(int*)&dlgDropdown->childrenDlg + 4 * v6) = v7;
		} while ((signed int)v2 < (signed int)&singleTypeSelect);
		v8 = dlgDropdown->flags;
		if (v8 < 0)
		{
			dlgDropdown->flags = v8 & 0x7FFFFFFF;
			createDialogDropdown(dlgDropdown);
		}
		if (dlgDropdown->alternateVersion0x46 > 3u)
		{
			v11 = 14;
			v9 = 11;
			v10 = 3;
			dlgDropdown->fxnInteract(dlgDropdown, (dlgEvent*)&v9);
			dlgSetSelected(3u, dlgDropdown);
		}
	};

	int initSingleplayerCustom(BinDlg** dialog) { // 004AD850 -- https://pastebin.com/raw/SQJXUWtG
		int eax;							//v1
		BinDlg** pDropdown;
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
		result = isMeleeGame(SBYTE1(selectedGameType), selectedGameTypeParam, selectedGameType);
		if (result) {
			selectedSingleplayerRace[1] = 0x7070707;
			customSingleplayer[0] = 1;
			selectedSingleplayerRace[5] = 0x7070707;
			if (pDropDown + 70) {
				pDropdownBig = pDropdown + 72;
				if (pDropdownBig != -1) {
					selectedSingleplayerRace[0] = (pDropdown + 66) + 4 * pDropdownBig;
				}
				result = pDropdown;
				if (pDropdown) {
					spRace = selectedSingleplayerRace[1];
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
					playerTable[LOCAL_NATION_ID].race = (pDropdown + 66) + 4 * al;
					IS_IN_GAME_LOOP = 1;
					result = randomizePlayerSlot();
					IS_IN_GAME_LOOP = 0;
				}
				else {
					playerTable[LOCAL_NATION_ID].race = 6;
					IS_IN_GAME_LOOP = 1;
					result = randomizePlayerSlot();
					IS_IN_GAME_LOOP = 0;
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
		int* v8; // eax
		int v9; // ecx
		char v10; // [esp+8h] [ebp-4h]

		stopMusic();
		if (IS_IN_REPLAY)
		{
			if (!scenarioChk)
			{
				a2 = 0;
				loadReplayFile((int)CurrentMapFileName, v2, 0);
			}
			if (IS_IN_REPLAY)
			{
				*(int*)playerForce = *(int*)playerForceSomethingReplay;
				*(int*)&playerForce[4] = *(int*)&playerForceSomethingReplay[4];
			}
		}
		if (!loadGameFileHandle)
			elapsedTimeFrames = 0;
		if (!multiPlayerMode)
		{
			v3 = levelCheatCreate(a2, a1);
			if (!v3 || !loadGameCreate(v3, a2) || !gameCreateSomething(a2) || !singleplayerMeleeCreate())
				return 0;
			if (IS_IN_REPLAY)
			{
				initialSeed = replaySeed;
			}
			else
			{
				initialSeed = getSystemTime(0);
				replaySeed = initialSeed;
			}
		}
		initScreenLayer();
		auto mouseOverCallback = (int(__stdcall*)(int))subTwoCheckTwo;
		loadGameFonts();
		memset(randomCounts, 0, sizeof(randomCounts));
		randomCountsTotal = 0;
		lastRandomNumber = initialSeed;
		srand(initialSeed);
		initPathArray();
		appAddExit((int)initGameLoad(a1, a2));
		if (!loadGameFileHandle)
			multiplayerChatSomething();
		randomizePlayerSlot();
		v5 = IS_IN_REPLAY;
		if (IS_IN_REPLAY)
			getPlayerStructs(playerTable);
		initStartingRacesAndTypes();
		if (!v5)
		{
			if (loadGameFileHandle)
				initMultiplayerSavePlayers();
			else
				initPlayers();
		}
		setVisionIndex();
		initAiPlayerNames();
		memset(randomCounts, 0, sizeof(randomCounts));
		randomCountsTotal = 0;
		lastRandomNumber = initialSeed;
		srand(initialSeed);
		loadRaceUI();
		hotkeyRemapping();
		if (!gameInit(v5))
			return 0;
		v6 = (char*)IS_IN_REPLAY;
		if (IS_IN_REPLAY)
			getPlayerColors(factionsColorsOrdering);
		if (!loadGameFileHandle)
			randomizePlayerColors();
		if (!v6 && !campaignIndex)
		{
			v6 = CurrentMapFileName;
			if (!authorizeMap((unsigned int)CurrentMapFileName, (int)&v10, 1, (int)mapArchiveHandle)
				&& !gameData.bVictoryCondition // 0x75
				&& !gameData.bStartingUnits // 0x79
				&& !gameData.bTournamentModeEnabled) // 0x7F
			{
				if (loadGameFileHandle)
				{
					bigPacketError(99, CurrentMapFileName, 0, 0, 1);
					return 0;
				}
				bigPacketError(97, 0, 0, 0, 1);
				return 0;
			}
		}
		CHEAT_STATE &= 0x28000200u;
		if (!loadGameCore((int)v6))
			return 0;
		if (!mapStarted)
		{
			if (!chooseTriggerTemplate(v7, (char*)&randomCounts[256], (int)v6))
			{
				bigPacketError(93, 0, 0, 0, 1);
				return 0;
			}
			if (!mapStarted)
			{
				*(int*)playerHasLeft = 0;
				lossType = 0;
				*(int*)&unkVictoryVariable = 0;
			}
		}
		initPlayerNames();
		if (!multiPlayerMode)
			tickCounterInit();
		saveLoadSuccess = (unsigned __int8)mapStarted;
		elapstedTimeModifier = mapStarted != 0 ? savedElapsedSeconds : 0;
		CMDRECV_SetReplaySpeed(optionList.speed, 0, 1u);
		if (IS_IN_REPLAY)
		{
			copyPlayerStructsToReplay(playerTable, &gameData);
			v8 = replayData;
			d__int16_6D5BF0 = 0;
			v9 = *((int*)replayData + 2);
			*(int*)replayData = 0;
			v8[1] = 1;
			v8[7] = v9;
			replayVisions = 255;
			playerVisions = 255;
			replayShowEntireMap = 0;
			nextReplayCommandFrame = -1;
			playerExploredVisions = 0xFF00;
		}
		else
		{
			allocReplayCommands();
		}
		return 1;
	};*/

	void displayEstablishingShot() { // 004D9200, hooked by X405
		// This is the original original function which loads the establisihing shots in campaigns:
		if (!*multiPlayerMode && !(*CHEAT_STATE & CheatFlags::NoGlues) && *campaignIndex) {
			for (int i = 0; i < 65; i++) {
				if (campaignMissions[i].missionIndex == *campaignIndex)
					loadInitCreditsBIN(campaignMissions[i].missionName);
			}
		}
		// This addition enables establishing shots in custom scenarios:
		else if (*campaignIndex == 0 && CurrentMapFileName) {
			SFileOpenArchive(CurrentMapFileName, 0, 0, mapArchiveHandle);
			void* handle;
			bool establishingShotExists = SFileOpenFileEx(*mapArchiveHandle, "rez\\est.txt", 0, &handle);
			if (handle)
				SFileCloseFile(handle);

			if (establishingShotExists)
				loadInitCreditsBIN("est");

			if (*mapArchiveHandle)
				SFileCloseArchive(*mapArchiveHandle);
		}
	}
	
} //namespace hooks

;

//-------- Helper function definitions. Do NOT modify! --------//

namespace {
	/*
	//For raceDropdown
	const u32 Func_addItemToListing = 0x0041B760;
	char addItemToListing(char pszString, BinDlg* pDlg, __int8 bFlags) {
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
	void createDialogDropdown(BinDlg* a1) {
		__asm {
			PUSHAD
			MOV a1, EAX
			CALL Func_createDialogDropdown
			POPAD
		}
	};

	const u32 Func_dlgSetSelected = 0x0041A600;
	void dlgSetSelected(unsigned __int8 a1, BinDlg* a2) {
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
	const u32 Func_stopMusic = 0x004A5F50;
	bool stopMusic() {
		static bool result;
		__asm {
			PUSHAD
			CALL Func_stopMusic
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_loadReplayFile = 0x004DF570;
	FILE loadReplayFile(int a1, int a2, int a3) {
		static int a1;
		static int a2;
		static int a3;
		static FILE result;
		__asm {
			PUSHAD
			MOV a1, EAX
			MOV a2, ECX
			MOV a3, EDI
			CALL Func_loadReplayFile
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_levelCheatCreate = 0x004EE5B0; 
	signed int levelCheatCreate(char a1, int a2) {
		static char a1;
		static int a2;
		static signed int result;
		__asm {
			PUSHAD
			MOV a1, EDI
			MOV a2, ESI
			CALL Func_levelCheatCreate
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_loadGameCreate = 0x004EE520;
	signed int loadGameCreate(int a1, char a2) {
		static int a1;
		static char a2;
		static signed int result;
		__asm {
			PUSHAD
			MOV a1, EAX
			MOV a2, EDI
			CALL Func_loadGameCreate
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_gameCreateSomething = 0x004EE3D0;
	signed int gameCreateSomething(char a1) {
		static char a1;
		static signed int result;
		__asm {
			PUSHAD
			MOV a1, EDI
			CALL Func_gameCreateSomething
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_singleplayerMeleeCreate = 0x004EE110;
	signed int singleplayerMeleeCreate() {
		__asm {
			PUSHAD
			CALL Func_singleplayerMeleeCreate
			POPAD
		}
	};

	const u32 Func_getSystemTime = 0x0040C7DF;
	int getSystemTime(LPSYSTEMTIME a1) {
		static LPSYSTEMTIME a1;
		static int result;
		__asm {
			PUSHAD
			CALL Func_getSystemTime
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_initScreenLayer = 0x0041A030;
	void initScreenLayer() {
		__asm {
			PUSHAD
			CALL Func_initScreenLayer
			POPAD
		}
	};

	const u32 Func_subTwoCheckTwo = 0x004EE0F0;
	int subTwoCheckTwo(void a1, int a2) {
		static void a1;
		static int a2;
		static int result;
		__asm {
			PUSHAD
			CALL Func_subTwoCheckTwo
			POPAD
		}
		return result;
	};

	const u32 Func_loadGameFonts = 0x004DAE50;
	void loadGameFonts() {
		__asm {
			PUSHAD
			CALL Func_loadGameFonts
			POPAD
		}
	};

	const u32 Func_srand = 0x0040C7B0;
	void srand(unsigned int a1) {
		static int a1;
		__asm {
			PUSHAD
			CALL Func_srand
			POPAD
		}
	};

	const u32 Func_initPathArray = 0x0042F4F0;
	dwPOINT initPathArray() {
		static dwPOINT result;
		__asm {
			PUSHAD
			CALL Func_initPathArray
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_appAddExit = 0x004F6100;
	signed int appAddExit(int a1) {
		static int a1;
		static signed int result;
		__asm {
			PUSHAD
			MOV a1, EBX
			CALL Func_appAddExit
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_multiplayerChatSomething = 0x0049B500;
	bool multiplayerChatSomething() {
		static bool result;
		__asm {
			PUSHAD
			CALL Func_multiplayerChatSomething
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_getPlayerStructs = 0x004DE9A0;
	void getPlayerStructs(void a1) {
		static void a1;
		__asm {
			PUSHAD
			MOV a1, EAX
			CALL Func_getPlayerStructs
			POPAD
		}
	};

	const u32 Func_initStartingRacesAndTypes = 0x0049CC40;
	PLAYER initStartingRacesAndTypes() {
		static PLAYER result;
		__asm {
			PUSHAD
			CALL Func_initStartingRacesAndTypes
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_initMultiplayerSavePlayers = 0x004A9250;
	char initMultiplayerSavePlayers() {
		static char result;
		__asm {
			PUSHAD
			CALL Func_initMultiplayerSavePlayers
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_initPlayers = 0x004A9D80;
	FILE initPlayers() {
		static FILE result;
		__asm {
			PUSHAD
			CALL Func_initPlayers
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_setVisionIndex = 0x004EE180;
	void setVisionIndex() {
		__asm {
			PUSHAD
			CALL Func_setVisionIndex
			POPAD
		}
	};

	const u32 Func_initAiPlayerNames = 0x004ED2B0;
	void initAiPlayerNames() {
		__asm {
			PUSHAD
			CALL Func_initAiPlayerNames
			POPAD
		}
	};

	const u32 Func_loadRaceUI = 0x004EE2D0;
	__int16 loadRaceUI() {
		static __int16 result;
		__asm {
			PUSHAD
			CALL Func_loadRaceUI
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_hotkeyRemapping = 0x004EEC30;
	bool hotkeyRemapping() {
		static bool result;
		__asm {
			PUSHAD
			CALL Func_hotkeyRemapping
			MOV EAX, result
			POPAD
		}
	};

	const u32 Func_gameInit = 0x004EEE00;
	signed int gameInit(int a1) {
		static int a1;
		static signed int result;
		__asm {
			PUSHAD
			MOV a1, ESI
			CALL Func_gameInit
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_getPlayerColors = 0x004DE950;
	int getPlayerColors(int a1) {
		static int a1;
		static int result;
		__asm {
			PUSHAD
			MOV a1, EAX
			CALL Func_getPlayerColors
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_authorizeMap = 0x004CC110;
	int authorizeMap(int a1, int a2, int a3, int a4) {
		static int a1;
		static int a2;
		static int a3;
		static int a4;
		static int result;
		__asm {
			PUSHAD
			MOV a1, ESI
			CALL Func_authorizeMap
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_bigPacketError = 0x004BB430;
	char bigPacketError(int a1, char a2, int a3, int a4, int a5) {
		static int a1;
		static char a2;
		static int a3;
		static int a4;
		static int a5;
		static char result;
		__asm {
			PUSHAD
			MOV a1, ECX
			CALL Func_bigPacketError
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_loadGameCore = 0x004EED10;
	signed int loadGameCore(int a1) {
		static int a1;
		static signed int result;
		__asm {
			PUSHAD
			MOV a1, ESI
			CALL Func_loadGameCore
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_chooseTriggerTemplate = 0x004ABAF0;
	void chooseTriggerTemplate(int a1, char a2, int a3) {
		static int a1;
		static char a2;
		static int a3;
		__asm {
			PUSHAD
			MOV a1, EDX
			MOV a2, EDI
			MOV a3, ESI
			CALL Func_chooseTriggerTemplate
			POPAD
		}
	};

	const u32 Func_initPlayerNames = 0x004B2DF0;
	int initPlayerNames() {
		const int result;
		__asm {
			PUSHAD
			CALL Func_initPlayerNames
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_tickCounterInit = 0x004A2BF0;
	int tickCounterInit() {
		static int result;
		__asm {
			PUSHAD
			CALL Func_tickCounterInit
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_CMDRECV_SetReplaySpeed = 0x004DEB90;
	int CMDRECV_SetReplaySpeed(int a1, unsigned __int8 a2, unsigned int a3) {
		static int a1;
		static unsigned int8 a2;
		static unsigned int a3;
		static int result;
		__asm {
			PUSHAD
			MOV a1, EAX
			MOVZX a2, DL
			MOV a3, ECX
			CALL Func_CMDRECV_SetReplaySpeed
			MOV EAX, result
			POPAD
		}
		return result;
	};

	const u32 Func_copyPlayerStructsToReplay = 0x004DE9D0;
	void copyPlayerStructsToReplay(void a1, void a2) {
		static void a1;
		static void a2;
		__asm {
			PUSHAD
			MOV a1, EAX
			MOV a2, EDX
			CALL Func_copyPlayerStructsToReplay
			POPAD
		}
	};

	const u32 Func_allocReplayCommands = 0x004CE280;
	signed int allocReplayCommands() {
		static signed int result;
		__asm {
			PUSHAD
			CALL Func_allocReplayCommands
			MOV EAX, RESULT
			POPAD
		}
		return result;
	};*/

	// For displayEstablishingShot

	void* loadInitCreditsBIN(char* a1) {
		int address = 0x4D8DF0;
		void* result_;
		__asm {
			mov eax, a1
			call address
			mov result_, eax
		}
		return result_;
	}

	const u32 Func_SFileOpenArchive = 0x004100B2;
	bool SFileOpenArchive(char* archiveName, u32 dwPriority, u32 dwFlags, void* const* handle) {
		static u32 result;
		__asm {
			PUSHAD
			PUSH handle
			PUSH dwFlags
			PUSH dwPriority
			PUSH archiveName
			CALL Func_SFileOpenArchive
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_SFileCloseArchive = 0x004100BE;
	bool SFileCloseArchive(void* archive) {
		static u32 result;
		__asm {
			PUSHAD
			PUSH archive
			CALL Func_SFileCloseArchive
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_SFileOpenFileEx = 0x004100C4;
	bool SFileOpenFileEx(void* hMPQ, const char* szFileName, u32 dwSearchScope, void** phFile) {
		static u32 result;
		__asm {
			PUSHAD
			PUSH phFile
			PUSH dwSearchScope
			PUSH szFileName
			PUSH hMPQ
			CALL Func_SFileOpenFileEx
			MOV result, eAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_SFileCloseFile = 0x4100b8;
	bool SFileCloseFile(void* hFile) {
		static u32 result;
		__asm {
			PUSHAD
			PUSH hFile
			CALL Func_SFileCloseFile
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

}