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
	/*
	int initGameLoad(int a1, char a2) { // 004EF100 -- https://pastebin.com/raw/szn0Z6sp
		
	};
	*/
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
			MOV EAX, pszString
			MOV ECX, pDlg
			CALL Func_addItemToListing
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const u32 Func_createDialogDropdown = 0x0041B410;
	void createDialogDropdown(BinDlg** a1) {
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_createDialogDropdown
			POPAD
		}
	};

	const u32 Func_dlgSetSelected = 0x0041A600;
	void dlgSetSelected(unsigned __int8 a1, BinDlg** a2) {
		__asm {
			PUSHAD
			MOVZX EBX, a1
			MOV ESI, a2
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
			MOVZX DL,a1
			MOVZX CX,a2
			MOVZX BL,a3
			CALL Func_isMeleeGame
			MOV result, EAX
			POPAD
		}
		return result;
	};

	const u32 Func_randomizePlayerSlot = 0x004A9A30;
	PLAYER randomizePlayerSlot() {
		static PLAYER result;

		__asm {
			PUSHAD
			CALL Func_randomizePlayerSlot
			MOV result, EAX
			POPAD
		}
		return result;
	};

	//For initGameLoad

}