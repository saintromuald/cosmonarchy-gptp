#include "tbl_extender.h"
#include "globals.h"

namespace hooks {

	std::vector<u32> offset_list() {
		return {
			0x004234F6 + 2,
			//004234FC  |.  66:8139 6903  CMP WORD PTR DS:[ECX],369
			//0042350A  |>  0FB781 D40600>MOVZX EAX,WORD PTR DS:[ECX+6D4]
			0x00425B7E + 2,
			//00425B84  |.  66:8139 3F03  CMP WORD PTR DS:[ECX],33F
			//00425BA4  |>  0FB781 800600>MOVZX EAX,WORD PTR DS:[ECX+680]
			0x00425C51 + 2,
			//00425C57  |.  66:813A 1305  CMP WORD PTR DS:[EDX],513
			//00425C65  |>  0FB782 280A00>MOVZX EAX,WORD PTR DS:[EDX+A28]
			0x00425CAE + 2,
			//00425CB4  |.  66:8139 1205  CMP WORD PTR DS:[ECX],512
			//00425CD5  |>  0FB781 260A00>MOVZX EAX,WORD PTR DS:[ECX+A26]
			0x00425D02 + 2,
			//00425D08  |.  66:8139 3A03  CMP WORD PTR DS:[ECX],33A
			//00425D29  |>  0FB781 760600>MOVZX EAX,WORD PTR DS:[ECX+676]

			0x00425D52 + 2,
			0x00425E3A + 2,
			0x0042607D + 2,
			0x0042614E + 2,
			0x0042620D + 2,
			0x00426247 + 2,
			0x00426283 + 2,
			0x004266AB + 2,
			0x0042688E + 2,
			0x00426BAF + 2,
			0x00427188 + 2,
			0x0042738C + 2,
			0x00427449 + 2,
			0x00427624 + 2,
			0x0042767B + 2,
			0x004276EF + 2,
			0x0042779A + 2,
			0x00427ACF + 2,
			0x00427BCD + 2,
			0x0042F3AF + 2,
			0x0042F3E8 + 2,
			0x00455C3F + 2,
			0x004575BE + 2,
			0x004575FC + 2,
			0x004576EC + 2,
			0x00457739 + 2,
			0x004577B1 + 2,
			0x00457834 + 2,
			0x00457899 + 2,
			0x0045792E + 2,
			0x0045798D + 2,
			0x004579D0 + 2,
			0x00457A25 + 2,
			0x00457BAB + 2,
			0x00457D81 + 2,
			0x0045916A + 2,
			0x004647B4 + 2,
			0x00464EAB + 2,
			0x0046F8A0 + 2,
			0x0046F987 + 2,
			0x0047456D + 2,
			0x00474606 + 2,
			0x00475538 + 2,
			0x0047557C + 2,
			0x0047B0CB + 2,
			0x0048D959 + 2,
			0x0048E509 + 2,
			0x0048E65F + 2,
			0x0048EEAB + 2,
			0x0048F760 + 2,
			0x0048F850 + 2,
			0x004927CD + 2,
			0x004928DB + 2,
			0x0049456D + 2,
			0x004A4C47 + 2,
			0x004A5131 + 2,
			0x004A5182 + 2,
			0x004C36FB + 2,
			0x004C373B + 2,
			0x004C3885 + 2,
			0x004C3AE4 + 2,
			0x004E996C + 2,
			0x004F4F70 + 2,

			0x00457C29+1,
			0x004C3720+1,
			0x004C3866+1,
		};
	}
	void injectTblExtender() {
		Globals.tbl_extender = true;
		/*auto ptr_list = offset_list();
		//DATA MUST BE INITIALIZED
		for (int i = 0; i < ptr_list.size(); i++) {
			memoryPatch(ptr_list[i], (u32)&extendedTbl);
		}
		//TODO: FIND ALL +2 offsets
		//TODO: REPLACE USAGE OF WORD IN REGARDS OF TBL TO DWORD
		*/
		//006D1238
	}
}
tblExtender extendedTbl;