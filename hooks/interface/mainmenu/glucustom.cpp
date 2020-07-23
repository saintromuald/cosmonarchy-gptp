#include "glucustom.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>

namespace {

	char addItemToListing(char pszString, BinDlg** pDlg, __int8 bFlags);
	void createDialogDropdown(BinDlg** a1);
	void dlgSetSelected(unsigned __int8 a1, BinDlg** a2);

}

namespace hooks {

	void raceDropdown(BinDlg** dialog, int) {
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
			*(_DWORD*)(*(_DWORD*)&dlgVar->field_42 + 4 * listing) = value;
		} while race < type;
		flags = dialog->flags;
		if (flags < 0) {
			dlgVar->lFlags = flags & 0x7FFFFFFF;
			createDialogDropdown(dlgVar);
		}
		if (dlgVar->field_46 > 3u) {
			unused2 = 14;
			dlgEvent = 11;
			unused1 = 3;
			dlgVar->pfcnInteract(dlgVar, (dlgEvent *)&dlgEvent);
			dlgSetSelected(3u, dlgVar);
		}
	}

} //namespace hooks

;

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

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
}