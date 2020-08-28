//for 4-byte flingies
#include "hooks/limits/fake_unitid_extender.h"
#include "globals.h"

u32 units_dat_sizes[] = {
	1,//flingy
	2, //subunit
	2, //infested unit
	4, //construction graphic
	1, //start direction
	1, //shield enabled
	2, //max shields
	4, //max hits
	1, //elevation
	1, //movement flags
	1, //rank
	1, //computer idle order
	1, //human idle order
	1, //return to idle order
	1, //attack unit order
	1, //attack move order
	1, //ground weapon
	1, //max ground hits
	1, //air weapon
	1, //max air hits
	1, //computer ai unit type
	4, //flags
	1, //seek range
	1, //sight range
	1, //armor upgrade
	1, //size
	1, //armor
	1, //right-click
	2, //READY
	2, //what-first
	2, //what-last
	2, //pss-first
	2, //pss-last
	2, //yes-first
	2, //yes-last
	4, //building dimensions (2+2)
	4, //addon placement (2+2)
	8, //unit bounds
	2, //portrait
	2, //mineral cost
	2, //gas cost
	2, //time cost
	2, //restriction flags
	1, //group flags
	1, //supply provided
	1, //supply required
	1, //space required
	1, //space provided
	2, //build score
	2, //destroy core
	2, //map string id
	1, //broodwar only
	2, //staredit avail
};
void fakeDataLoadPartial(char * datArray, u32 len, u8 index) {
	//later flingy link must be replaced with pointer to array start
	std::string str = scbw::readSamaseFile(datArray);
	int offset = 0;
	for (int i = 0; i < index; i++) {
		offset += len * units_dat_sizes[i];//will take no effect for first effect, i.e. flingy
	}
	std::stringstream sstr(str);
	sstr.ignore(offset);
	for (int i = 0; i < len; i++) {
		switch (units_dat_sizes[index]) {
		case 1:
		{
			u8 val = 0;
			sstr.read(reinterpret_cast<char*>(&val), sizeof(val));
			if (index == UnitsDatEntries::Flingy) {
				new_unitids.flingy_ids[i] = val;
			}
			break;
			//add 2 later
			//add 4 and dimensions case later, replace 1/2/4 to enums
			//add 8 (box) later
		}
		default:
			break;
		}
	}
}

void UnitIdExtender::init(){
	//006644F8 = byte units_dat_flingy[0xe4]
	/*
	SCBW_DATA(u8*,		Graphic,				unitsDat[0].address);	//006644F8
SCBW_DATA(u16*,		SubUnit,				unitsDat[1].address);	//006607C0
	*/
	Globals.unitid_extender = true;
	
	fakeDataLoadPartial("arr\\units.dat", UNIT_TYPE_COUNT, UnitsDatEntries::Flingy);//0 is index of Flingy entry
	//update unit speed wrapper is not required
	/*
	Line 125772: 00454317 | .  0FB680 F84466 > MOVZX EAX, BYTE PTR DS : [EAX + 6644F8]
		Line 148264 : 00463A71 | .  0FB68F F84466 > MOVZX ECX, BYTE PTR DS : [EDI + 6644F8]
		Line 154775 : 004683C6   .  0FB691 F84466 > MOVZX EDX, BYTE PTR DS : [ECX + 6644F8]
		Line 184214 : 0047B854 | .  0FB690 F84466 > MOVZX EDX, BYTE PTR DS : [EAX + 6644F8]
		Line 184242 : 0047B8A4 | .  0FB690 F84466 > MOVZX EDX, BYTE PTR DS : [EAX + 6644F8]
		Line 184269 : 0047B8F4   .  0FB688 F84466 > MOVZX ECX, BYTE PTR DS : [EAX + 6644F8]
		Line 210943 : 0048D756 | .  0FB688 F84466 > MOVZX ECX, BYTE PTR DS : [EAX + 6644F8]
		Line 211212 : 0048DA8A | .  0FB688 F84466 > MOVZX ECX, BYTE PTR DS : [EAX + 6644F8]
		Line 225181 : 00497148 | .  0FB690 F84466 > | MOVZX EDX, BYTE PTR DS : [EAX + 6644F8]
		Line 236925: 0049ED0B | .  0FB683 F84466 > MOVZX EAX, BYTE PTR DS : [EBX + 6644F8]
		Line 347273 : 004E9A10   .  0FB682 F84466 > MOVZX EAX, BYTE PTR DS : [EDX + 6644F8]
		*/
}

UnitIdExtender new_unitids;