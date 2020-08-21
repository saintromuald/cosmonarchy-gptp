#include "tbl_extender.h"
#include "globals.h"

namespace hooks {

	std::vector<u32> offset_list() {
		return {
			// Address math \\

			0x004234F6 + 2,
			0x00425B7E + 2,
			0x00425C51 + 2,
			0x00425CAE + 2,
			0x00425D02 + 2,
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
			0x00457C29 + 1,
			0x004C3720 + 1,
			0x004C3866 + 1,

			// Pointer comparisons \\

			//004234FC  |.  66:8139 6903  CMP WORD PTR DS:[ECX],369
			//00425B84  |.  66:8139 3F03  CMP WORD PTR DS:[ECX],33F
			//00425CB4  |.  66:8139 1205  CMP WORD PTR DS:[ECX],512
			//00425D08  |.  66:8139 3A03  CMP WORD PTR DS:[ECX],33A
			//00425D58  |.  66:8139 3E03  CMP WORD PTR DS:[ECX],33E
			//00425E40  |.  66:8139 FB02  CMP WORD PTR DS:[ECX],2FB
			//00426289  |.  66:8139 1D03  CMP WORD PTR DS:[ECX],31D
			//0042744F   .  66:8139 1503  CMP WORD PTR DS:[ECX],315
			//00427ADD  |.  66:8139 0806  CMP WORD PTR DS:[ECX],608
			//00427BD3  |.  66:8139 0906  CMP WORD PTR DS:[ECX],609
			//0042F3B5   .  66:8139 3B02  CMP WORD PTR DS:[ECX],23B
			//0042F3EE   .  66:8139 3A02  CMP WORD PTR DS:[ECX],23A
			//004576F2  |.  66:8139 0803  CMP WORD PTR DS:[ECX],308
			//00457D87  |.  66:8139 2803  CMP WORD PTR DS:[ECX],328
			//004647BA   .  66:8139 1604  CMP WORD PTR DS:[ECX],416
			//00474573  |.  66:8139 6703  CMP WORD PTR DS:[ECX],367
			//0047460C  |.  66:8139 6703  CMP WORD PTR DS:[ECX],367
			//0047553E  |.  66:8139 6603  CMP WORD PTR DS:[ECX],366
			//00475582  |.  66:8139 6803  CMP WORD PTR DS:[ECX],368
			//0048E50F  |.  66:8139 5E03  CMP WORD PTR DS:[ECX],35E
			//0048E665  |.  66:8139 5E03  CMP WORD PTR DS:[ECX],35E
			//004928E1  |.  66:8139 6B03  CMP WORD PTR DS:[ECX],36B
			//004E9972   .  66:8139 6503  CMP WORD PTR DS:[ECX],365
			//004F4F76  |.  66:8139 2D03  CMP WORD PTR DS:[ECX],32D

			//00425C57  |.  66:813A 1305  CMP WORD PTR DS:[EDX],513
			//00426213  |.  66:813A 1B03  CMP WORD PTR DS:[EDX],31B
			//0042624D | .  66:813A 1C03  CMP WORD PTR DS:[EDX],31C
			//004579D6  |.  66:813A 1803  CMP WORD PTR DS:[EDX],318
			//00459170  |.  66:813A 1A03  CMP WORD PTR DS:[EDX],31A
			//004A4C50  |.  66:813A 2903  CMP WORD PTR DS:[EDX],329
			//004A5150   >  66:813A 2B03  CMP WORD PTR DS:[EDX],32B
			//004A5188   .  66:813A 2C03  CMP WORD PTR DS:[EDX],32C

			//0045783A  |.  66:813E 0903  CMP WORD PTR DS:[ESI],309
			//045789F	|.  66:813E 0A03  CMP WORD PTR DS:[ESI],30A

			//00457C2E  |.  66:8138 2703  CMP WORD PTR DS:[EAX],327

			// Pointer writing \\

			//0042350A  |>  0FB781 D40600>MOVZX EAX,WORD PTR DS:[ECX+6D4]
			//00425BA4  |>  0FB781 800600>MOVZX EAX,WORD PTR DS:[ECX+680]
			//00425CD5  |>  0FB781 260A00>MOVZX EAX,WORD PTR DS:[ECX+A26]
			//00425D29  |>  0FB781 760600>MOVZX EAX,WORD PTR DS:[ECX+676]
			//00425D79  |>  0FB781 7E0600>MOVZX EAX,WORD PTR DS:[ECX+67E]
			//00425E4E  |>  0FB781 F80500>MOVZX EAX,WORD PTR DS:[ECX+5F8]
			//0042745E   >  0FB781 2C0600>MOVZX EAX,WORD PTR DS:[ECX+62C]
			//00427AEB  |>  0FB781 120C00>MOVZX EAX,WORD PTR DS:[ECX+C12]
			//00427BE1  |>  0FB781 140C00>MOVZX EAX,WORD PTR DS:[ECX+C14]
			//0042F3BC   .  0FB781 780400>MOVZX EAX,WORD PTR DS:[ECX+478]
			//0042F3FB   >  0FB781 760400>MOVZX EAX,WORD PTR DS:[ECX+476]
			//004575E3  |>  0FB781 4A0600>MOVZX EAX,WORD PTR DS:[ECX+64A]
			//00457613  |>  0FB781 4C0600>MOVZX EAX,WORD PTR DS:[ECX+64C]
			//004577C8  |>  0FB781 2A0A00>MOVZX EAX,WORD PTR DS:[ECX+A2A]
			//00457DA0  |>  0FB781 520600>MOVZX EAX,WORD PTR DS:[ECX+652]
			//004647D9   >  0FB781 2E0800>MOVZX EAX,WORD PTR DS:[ECX+82E]
			//00474584  |>  0FB781 D00600>MOVZX EAX,WORD PTR DS:[ECX+6D0]
			//0047461D  |>  0FB781 D00600>MOVZX EAX,WORD PTR DS:[ECX+6D0]
			//0047555A  |>  0FB781 CE0600>MOVZX EAX,WORD PTR DS:[ECX+6CE]
			//0047559E  |>  0FB781 D20600>MOVZX EAX,WORD PTR DS:[ECX+6D2]
			//0048E51D  |>  0FB781 BE0600>MOVZX EAX,WORD PTR DS:[ECX+6BE]
			//0048E673  |>  0FB781 BE0600>MOVZX EAX,WORD PTR DS:[ECX+6BE]
			//004928F2  |>  0FB781 D80600>MOVZX EAX,WORD PTR DS:[ECX+6D8]
			//004E9994   >  0FB781 CC0600>MOVZX EAX,WORD PTR DS:[ECX+6CC]
			//004F4F84  |>  0FB781 5C0600>MOVZX EAX,WORD PTR DS:[ECX+65C]
			//0045917E  |>  0FB782 360600>MOVZX EAX,WORD PTR DS:[EDX+636]
			//00425C65  |>  0FB782 280A00>MOVZX EAX,WORD PTR DS:[EDX+A28]
			//0042622C  |>  0FB782 380600>MOVZX EAX,WORD PTR DS:[EDX+638]
			//0042625B  |>  0FB782 3A0600>MOVZX EAX,WORD PTR DS:[EDX+63A]
			//004A4C5E  |>  0FB782 540600>MOVZX EAX,WORD PTR DS:[EDX+654]
			//004A5157   .  0FB782 580600>MOVZX EAX,WORD PTR DS:[EDX+658]
			//004A5196   >  0FB782 5A0600>MOVZX EAX,WORD PTR DS:[EDX+65A]
			//004578AD  |>  0FB786 160600>MOVZX EAX,WORD PTR DS:[ESI+616]
			//00457848  |>  0FB78E 140600>MOVZX ECX,WORD PTR DS:[ESI+614]
			//00457700  |>  0FB7B9 120600>MOVZX EDI,WORD PTR DS:[ECX+612]
			//00457750  |>  0FB78A 2A0A00>MOVZX ECX,WORD PTR DS:[EDX+A2A]
			//004579F1  |>  0FB78A 320600>MOVZX ECX,WORD PTR DS:[EDX+632]
			//004579A4	| > 0FB799 120600>MOVZX EBX,WORD PTR DS:[ECX+612]
			//00457A3C  |>  0FB799 120600>MOVZX EBX,WORD PTR DS:[ECX+612]
			//00457C41  |>  0FB7B0 500600>MOVZX ESI,WORD PTR DS:[EAX+650]

			// Pointer writing with math \\

			//0042609E  |.  0FB74441 02   MOVZX EAX,WORD PTR DS:[ECX+EAX*2+2]
			//0042616F  |.  0FB74441 02   MOVZX EAX,WORD PTR DS:[ECX+EAX*2+2]
			//004266D1  |.  0FB74451 02   MOVZX EAX,WORD PTR DS:[ECX+EDX*2+2]
			//004268B3  |.  0FB74451 02   MOVZX EAX,WORD PTR DS:[ECX+EDX*2+2]
			//00426BC4  |.  0FB75451 02   MOVZX EDX,WORD PTR DS:[ECX+EDX*2+2]
			//004271B2  |.  0FB74441 02   MOVZX EAX,WORD PTR DS:[ECX+EAX*2+2]
			//004273A1   .  0FB74451 02   MOVZX EAX,WORD PTR DS:[ECX+EDX*2+2]
			//00427639 | .  0FB74C46 02   MOVZX ECX,WORD PTR DS:[ESI+EAX*2+2]
			//004277AF | .  0FB74442 02   MOVZX EAX,WORD PTR DS:[EDX+EAX*2+2]				
			//00427690  |.  0FB77451 02   MOVZX ESI,WORD PTR DS:[ECX+EDX*2+2]
			//00427704  |.  0FB7744A 02   MOVZX ESI,WORD PTR DS:[EDX+ECX*2+2]
			//00455C54 | .  0FB74451 02   MOVZX EAX,WORD PTR DS:[ECX+EDX*2+2]
			//00457BC0  |.  0FB77441 02   MOVZX ESI,WORD PTR DS:[ECX+EAX*2+2]
			//00464EC0  |.  0FB74451 02   MOVZX EAX,WORD PTR DS:[ECX+EDX*2+2]
			//0046F9AB  |.  0FB74441 02   MOVZX EAX,WORD PTR DS:[ECX+EAX*2+2]
			//0047B0DF  |.  0FB74451 02   MOVZX EAX,WORD PTR DS:[ECX+EDX*2+2]
			//0046F8B5 | .  0FB74451 02   MOVZX EAX,WORD PTR DS:[ECX+EDX*2+2]
			//0048D96E  |.  0FB74451 02   MOVZX EAX,WORD PTR DS:[ECX+EDX*2+2]
			//0048EEC0   .  0FB74451 02   MOVZX EAX,WORD PTR DS:[ECX+EDX*2+2]
			//0048F775  |.  0FB74441 02   MOVZX EAX,WORD PTR DS:[ECX+EAX*2+2]
			//0048F865  |.  0FB74441 02   MOVZX EAX,WORD PTR DS:[ECX+EAX*2+2]
			//004927E2  |.  0FB74446 02   MOVZX EAX,WORD PTR DS:[ESI+EAX*2+2]
			//00494582  |.  0FB74451 02   MOVZX EAX,WORD PTR DS:[ECX+EDX*2+2]
			//004C370F | .  0FB74441 02   MOVZX EAX,WORD PTR DS:[ECX+EAX*2+2]

			// Register-pointer comparisons \\

			//00426083  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//00426154  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//004266B1  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//00426894  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//00426BB5  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//0042718E  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//00427392   .  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//0042762A  |.  66:3B06       CMP AX,WORD PTR DS:[ESI]
			//00427681  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//004276F5  |.  66:3B0A       CMP CX,WORD PTR DS:[EDX]
			//004277A0 | .  66:3B02       CMP AX,WORD PTR DS:[EDX]
			//00455C45  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//004575C4  |.  66:8B39       MOV DI,WORD PTR DS:[ECX]
			//00457602  |.  66:8B39       MOV DI,WORD PTR DS:[ECX]
			//0045773F  |.  66:8B3A       MOV DI,WORD PTR DS:[EDX]
			//004577B7  |.  66:8B11       MOV DX,WORD PTR DS:[ECX]
			//00457934  |.  66:8B11       MOV DX,WORD PTR DS:[ECX]
			//00457993	| .  66:8B11      MOV DX,WORD PTR DS:[ECX]
			//00457A2B  |.  66:8B11       MOV DX,WORD PTR DS:[ECX]
			//00457BB1  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//00464EB1  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//0046F8A6  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//0046F98D  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//0047B0D1  |>  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//0048D95F  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//0048EEB1   .  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//0048F766  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//0048F856  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//004927D3  |.  66:3B0E       CMP CX,WORD PTR DS:[ESI]
			//00494573  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]
			//004C3701  |.  66:3B01       CMP AX,WORD PTR DS:[ECX]

			// Register comparisons \\
			//004575C7  |.  66:81FF 2403  CMP DI,324
			//00457605  |.  66:81FF 2503  CMP DI,325
			//00457742  |.  66:81FF 1405  CMP DI,514
			//004577BA  |.  66:81FA 1405  CMP DX,514
			//00457937	| . 66:81FA 0803  CMP DX,308
			//0045794E  |>  66:81FA 1603  CMP DX,316
			//00457996	| . 66:81FA 0803  CMP DX,308
			//00457A2E  |.  66:81FA 0803  CMP DX,308

			// Register writing \\
			//0042609B  |>  0FB7C0        MOVZX EAX,AX
			//0042616C  |>  0FB7C0        MOVZX EAX,AX
			//00427636  |>  0FB7C0        MOVZX EAX,AX
			//00427701  |>  0FB7C9        MOVZX ECX,CX
			//004277AC	| >	0FB7C0        MOVZX EAX,AX			
			//00455C51  |>  0FB7D0        MOVZX EDX,AX
			//00457BBD  |>  0FB7C0        MOVZX EAX,AX
			//00464EBD  |>  0FB7D0        MOVZX EDX,AX
			//0046F8B2  |>  0FB7D0        MOVZX EDX,AX
			//0046F9A8  |>  0FB7C0        MOVZX EAX,AX
			//0047B0DC  |>  0FB7D0        MOVZX EDX,AX
			//0048D96B  |>  0FB7D0        MOVZX EDX,AX
			//0048EEBD   >  0FB7D0        MOVZX EDX,AX
			//0048F862  |>  0FB7C0        MOVZX EAX,AX
			//004927DF  |>  0FB7C1        MOVZX EAX,CX
			//004C370C  |>  0FB7C0        MOVZX EAX,AX
			
		};
	}
	std::vector<u32> patch_convention() {
		return {
			/*
			HEADER FOR PATCH CONVENTION TYPES

			TblPatchType::CmpPatch, 1, 2, 4, 24, 8, 1, 2,

			TblPatchType::CmpPatch - patch type
			1 - number of variables
			4 - number of arguments
			4 - number of sequences
			24,8,1,2 - lengths of the sequences
			*/
			TblPatchType::CmpPatch, 1, 4, 4, 24, 8, 1, 2,
			Reg::_ECX,
			0x004234FC,0x369,0x00423501, Jmp::JA,
			0x00425B84,0x33f,0x00425B89, Jmp::JA,
			0x00425CB4,0x512,0x00425CB9, Jmp::JA,
			0x00425D08,0x33a,0x00425D0D, Jmp::JA,
			0x00425D58,0x33e,0x00425D5D, Jmp::JA,
			0x00425E40,0x2fb,0x00425E40, Jmp::JA,
			0x00426289,0x31d,0x00425E45, Jmp::JA,
			0x0042744F,0x315,0x0042744F, Jmp::JA,
			0x00427ADD,0x608,0x00427AE2, Jmp::JA,
			0x00427BD3,0x609,0x00427BD8, Jmp::JA,
			0x0042F3B5,0x23b,0x0042F3BA, Jmp::JBE,
			0x0042F3EE,0x23a,0x0042F3F3, Jmp::JA,
			0x004576F2,0x308,0x004576F7, Jmp::JA,
			0x00457D87,0x328,0x00457D8c, Jmp::JA,
			0x004647BA,0x416,0x004647BF, Jmp::JA,
			0x00474573,0x367,0x0047457B, Jmp::JA,
			0x0047460C,0x367,0x00474614, Jmp::JA,
			0x0047553E,0x366,0x00475546, Jmp::JA,
			0x00475582,0x368,0x0047558A, Jmp::JA,
			0x0048E50F,0x35e,0x0048E514, Jmp::JA,
			0x0048E665,0x35e,0x0048E66A, Jmp::JA,
			0x004928E1,0x36b,0x004928E9, Jmp::JA,
			0x004E9972,0x365,0x004E997A, Jmp::JA,
			0x004F4F76,0x32d,0x004F4F7B, Jmp::JA,
			//
			//
			Reg::_EDX,
			0x00425C57,0x513,0x00425C5C, Jmp::JA,
			0x00426213,0x31b,0x0042621F, Jmp::JA,
			0x0042624D,0x31c,0x00426252, Jmp::JA,
			0x004579D6,0x318,0x004579DB, Jmp::JA,
			0x00459170,0x31a,0x00459175, Jmp::JA,
			0x004A4C50,0x329,0x004A4C55, Jmp::JA,
			0x004A5150,0x32b,0x004A5155, Jmp::JBE,
			0x004A5188,0x32c,0x004A518D, Jmp::JA,
			Reg::_EAX,
			0x00457C2E,0x327,0x00457C38, Jmp::JA,
			Reg::_ESI,
			0x0045783A,0x309,0x0045783f, Jmp::JA,
			0x0045789F,0x30A,0x004578a4, Jmp::JA,

			//
			// GIANT PROBLEM: Ptr Comparison increase 
			//
			TblPatchType::PtrWriting, 2, 2, 8, 25, 7, 1, 1, 2, 1, 2, 1,

			Reg::_EAX,Reg::_ECX,
			0x0042350A,0x6d4,
			0x00425BA4,0x680,
			0x00425CD5,0xa26,
			0x00425D29,0x676,
			0x00425D79,0x67e,
			0x00425E4E,0x5f8,
			0x0042745E,0x62c,
			0x00427AEB,0xc12,
			0x00427BE1,0xc14,
			0x0042F3BC,0x478,
			0x0042F3FB,0x476,
			0x004575E3,0x64a,
			0x00457613,0x64c,
			0x004577C8,0xa2a,
			0x00457DA0,0x652,
			0x004647D9,0x82e,
			0x00474584,0x6d0,
			0x0047461D,0x6d0,
			0x0047555A,0x6ce,
			0x0047559E,0x6d2,
			0x0048E51D,0x6be,
			0x0048E673,0x6be,
			0x004928F2,0x6d8,
			0x004E9994,0x6cc,
			0x004F4F84,0x65c,
			Reg::_EAX,Reg::_EDX,
			0x0045917E,0x636,
			0x00425C65,0xa28,
			0x0042622C,0x638,
			0x0042625B,0x63a,
			0x004A4C5E,0x654,
			0x004A5157,0x658,
			0x004A5196,0x65a,
			Reg::_EAX,Reg::_ESI,
			0x004578AD,0x616,
			Reg::_ECX,Reg::_ESI,
			0x00457848,0x614,
			Reg::_ECX,Reg::_EDX,
			0x00457750,0xa2a,
			0x004579F1,0x632,
			Reg::_EDI,Reg::_ECX,
			0x00457700,0x612,
			Reg::_EBX,Reg::_ECX,
			0x004579A4,0x612,
			0x00457A3C,0x612,
			Reg::_ESI,Reg::_EAX,
			0x00457C41,0x650,







		TblPatchType::PtrMathWriting, 3, 1, 9, 7, 10, 1,1,1,1,1,1,1,
		Reg::_EAX,Reg::_ECX,Reg::_EAX,
		0x0042609E,
		0x0042616F,
		0x0046F9AB,
		0x0048F775,
		0x0048F865,
		0x004C370F,
		0x004271B2,
		Reg::_EAX,Reg::_ECX,Reg::_EDX,
		0x004266D1,
		0x004268B3,
		0x00464EC0,
		0x0047B0DF,
		0x0046F8B5,
		0x0048D96E,
		0x0048EEC0,
		0x00494582,
		0x004273A1,
		0x00455C54,
		Reg::_EAX,Reg::_ESI,Reg::_EAX,0x004927E2,
		Reg::_EAX, Reg::_EDX,Reg::_EAX,0x004277AF,
		Reg::_EDX,Reg::_ECX,Reg::_EDX,0x00426BC4,
		Reg::_ECX,Reg::_ESI,Reg::_EAX,0x00427639,
		Reg::_ESI,Reg::_ECX,Reg::_EDX,0x00427690,
		Reg::_ESI,Reg::_EDX,Reg::_ECX,0x00427704,
		Reg::_ESI,Reg::_ECX,Reg::_EAX,0x00457BC0,
		//
		// 1 nop
		//
			TblPatchType::RegisterPointerCmp,2,1,5,20,1,1,1,1,
			Reg::L_AX,Reg::_ECX,
			0x00426083,
			0x00426154,
			0x004266B1,
			0x00426894,
			0x00426BB5,
			0x0042718E,
			0x00427392,
			0x00427681,
			0x00455C45,
			0x00457BB1,
			0x00464EB1,
			0x0046F8A6,
			0x0046F98D,
			0x0047B0D1,
			0x0048D95F,
			0x0048EEB1,
			0x0048F766,
			0x0048F856,
			0x00494573,
			0x004C3701,
			Reg::L_CX,Reg::_EDX,
			0x004276F5,
			Reg::L_AX,Reg::_EDX,
			0x004277A0,
			Reg::L_CX,Reg::_ESI,
			0x004927D3,
			Reg::L_AX,Reg::_ESI,
			0x0042762A,
			//
			//	1 nop
			//
			TblPatchType::RegisterPointerMov,2,1,2,3,4,
			Reg::L_DI,Reg::_ECX,
			0x004575C4,
			0x00457602,
			0x0045773F,
			Reg::L_DX,Reg::_ECX,
			0x004577B7,
			0x00457934,
			0x00457993,
			0x00457A2B,
			// 1 nop
			TblPatchType::RegisterCmp,1,4,2,5,3,
			Reg::L_DX,
			0x004577BA, 0x514, 0x004577BF, Jmp::JA,
			0x00457937, 0x308, 0x0045793C, Jmp::JA,
			0x0045794E, 0x316, 0x00457953, Jmp::JA,
			0x00457996, 0x308, 0x0045799B, Jmp::JA,
			0x00457A2E, 0x308, 0x00457A33, Jmp::JA,
			Reg::L_DI,
			0x004575C7, 0x324, 0x004575CC, Jmp::JA,
			0x00457605, 0x325, 0x0045760A, Jmp::JA,
			0x00457742, 0x514, 0x00457747, Jmp::JA,	
			//SERIOUS PROBLEM - 1 extra byte
			TblPatchType::RegisterWriting,2,1,2,6,1,
			Reg::_EDX,Reg::L_AX,
			0x00455C51,
			0x00464EBD,
			0x0046F8B2,
			0x0047B0DC,
			0x0048D96B,
			0x0048EEBD,
			Reg::_ECX,Reg::L_AX,
			0x00427701,


			TblPatchType::NopPatch, 1, 1, 1, 9, //3 nops
			Reg::__NOP,
			0x0042609B,
			0x0042616C,
			0x00427636,
			0x004C370C,
			0x004277AC,
			0x0048F862,
			0x0046F9A8,
			0x00457BBD,
			0x00427701,

			TblPatchType::AddPatch,1,1,2,13,1,
			Reg::_EAX,
			0x0047B0BE,
			0x00426063,
			0x00426134,
			0x00426BA1,
			0x00427165,
			0x0042737E,
			0x00427616,
			0x0042766D,
			0x0042778C,
			0x00457B9D,
			0x00464EA1,
			0x0048D94F,
			0x00494563,
			Reg::_ECX,
			0x004276E0,
/*
			TblPatchType::JmpPatch, 1, 2, 1, 35,//for Cmp patches
			Reg::__NOP,
			0x00423501, Jmp::JA,
			0x00425B89, Jmp::JA,
			0x00425CB9, Jmp::JA,
			0x00425D0D, Jmp::JA,
			0x00425D5D, Jmp::JA,
			0x00425E40, Jmp::JA,
			0x00425E45, Jmp::JA,
			0x0042744F, Jmp::JA,
			0x00427AE2, Jmp::JA,
			0x00427BD8, Jmp::JA,
			0x0042F3BA, Jmp::JBE,
			0x0042F3F3, Jmp::JA,
			0x004576F7, Jmp::JA,
			0x00457D8c, Jmp::JA,
			0x004647BF, Jmp::JA,
			0x0047457B, Jmp::JA,
			0x00474614, Jmp::JA,
			0x00475546, Jmp::JA,
			0x0047558A, Jmp::JA,
			0x0048E514, Jmp::JA,
			0x0048E66A, Jmp::JA,
			0x004928E9, Jmp::JA,
			0x004E997A, Jmp::JA,
			0x004F4F7B, Jmp::JA,
			0x00425C5C, Jmp::JA,
			0x0042621F, Jmp::JA,
			0x00426252, Jmp::JA,
			0x004579DB, Jmp::JA,
			0x00459175, Jmp::JA,
			0x004A4C55, Jmp::JA,
			0x004A5155, Jmp::JBE,
			0x004A518D, Jmp::JA,
			0x00457C38, Jmp::JA,
			0x0045783f, Jmp::JA,
			0x004578a4, Jmp::JA,

			TblPatchType::JmpPatch, 1, 2, 1, 8,//for register cmp
			Reg::__NOP,
			0x004577BF, Jmp::JA,
			0x0045793C, Jmp::JA,
			0x00457953, Jmp::JA,
			0x0045799B, Jmp::JA,
			0x00457A33, Jmp::JA,
			0x004575CC, Jmp::JA,
			0x0045760A, Jmp::JA,
			0x00457747, Jmp::JA,*/
		};
	}
	namespace ConvReadStatus {
		enum {
			ReadHeader,
			ReadVars,
			ReadArgs,
		};
	}
	void memoryPatchVec(u32 address, std::vector<u8> &patch) {
		for (int i = 0; i < patch.size();i++) {
			memoryPatch(address+i, patch[i]);
		}
	}

//0x004234FC, 0x369, 0x00423501, Jmp::JA,
/*

004234FC  |.  66:8139 6903  CMP WORD PTR DS:[ECX],369
00423501  |.  77 07         JA SHORT 0042350A
00423503  |.  B8 7D1B5000   MOV EAX,00501B7D


static u32 wrapJumpAddressIf;
static u32 wrapJumpAddressAfter;
static u32 wrapComparison; (0x369 for example)
static char tblNew[] = "rez\\stat_txt.tblex";
*/
	/*void __declspec(naked) ecx_callset() {
		static u32 currentWrap;
		static u32 currentAddressIf;
		static u32 currentAddressAfter;
		__asm {
			PUSHAD
		}
		currentWrap = wrapComparison[currentWrapPatch];
		currentAddressIf = wrapJumpAddressIf[currentWrapPatch];
		currentAddressAfter = wrapJumpAddressAfter[currentWrapPatch];
		__asm {
			POPAD
			PUSH EAX
			MOV EAX, currentWrap
			CMP[ECX], EAX
			POP EAX
			JA SHORT currentAddressIf
			JMP currentAddressAfter
		}
	}
	void __declspec(naked) ecx_jbe_callset() {
		static u32 currentWrap;
		static u32 currentAddressIf;
		static u32 currentAddressAfter;
		__asm {
			PUSHAD
		}
		currentWrap = wrapComparison[currentWrapPatch];
		currentAddressIf = wrapJumpAddressIf[currentWrapPatch];
		currentAddressAfter = wrapJumpAddressAfter[currentWrapPatch];
		__asm {
			POPAD
			PUSH EAX
			MOV EAX, currentWrap
			CMP[ECX], EAX
			POP EAX
			JBE SHORT currentAddressIf
			JMP currentAddressAfter
		}
	}
	void __declspec(naked) eax_callset() {
		__asm {
			PUSH ESI
			MOV ESI, wrapComparison[currentWrapPatch]
			CMP[EAX], ESI
			POP ESI
			JA SHORT wrapJumpAddressIf[currentWrapPatch]
			JMP wrapJumpAddressAfter[currentWrapPatch]
		}
	}
	void __declspec(naked) edx_callset() {
		__asm {
			PUSH EAX
			MOV EAX, wrapComparison[currentWrapPatch]
			CMP [EDX], EAX
			POP EAX
			JA SHORT wrapJumpAddressIf[currentWrapPatch]
			JMP wrapJumpAddressAfter[currentWrapPatch]
		}
	}
	void __declspec(naked) edx_jbe_callset() {
		__asm {
			PUSH EAX
			MOV EAX, wrapComparison[currentWrapPatch]
			CMP[EDX], EAX
			POP EAX
			JBE SHORT wrapJumpAddressIf[currentWrapPatch]
			JMP wrapJumpAddressAfter[currentWrapPatch]
		}
	}
	void __declspec(naked) esi_callset() {
		__asm {
			PUSH EAX
			MOV EAX, wrapComparison[currentWrapPatch]
			CMP[ESI], EAX
			POP EAX
			JA SHORT wrapJumpAddressIf[currentWrapPatch]
			JMP wrapJumpAddressAfter[currentWrapPatch]
		}
	}*/
	void patchTblList() {
		int read_status = ConvReadStatus::ReadHeader;
		std::vector<u32> args;
		std::vector<u32> vars;
		int varCount = 0;
		int argCount = 0;
		int currentVarCount = 0;
		int currentArgCount = 0;
		int currentSequenceCount = 0;
		int currentHeader = -1;
		
		auto patch_list = patch_convention();
		std::vector<u8> patch;
		std::vector<u32> sequence_lengths;
		for (int i = 0; i < patch_list.size(); i++) {
			if (read_status == ConvReadStatus::ReadHeader) {
				sequence_lengths.clear();
				vars.clear();
				currentHeader = patch_list[i];
				currentVarCount = patch_list[i + 1];
				currentArgCount = patch_list[i + 2];
				currentSequenceCount = patch_list[i + 3];
				for (int j = 0; j < currentSequenceCount; j++) {
					sequence_lengths.push_back(patch_list[i + 4 + j]);
				}
				i += 3 + currentSequenceCount;
				read_status = ConvReadStatus::ReadArgs;
			}
			else if (read_status == ConvReadStatus::ReadArgs) {
				for (int a = 0; a < currentSequenceCount; a++) {
					vars.clear();
					for (int j = 0; j < currentVarCount; j++) {
						vars.push_back(patch_list[i]);
						i++;
					}
					for (int b = 0; b < sequence_lengths[a]; b++) {
						auto& elem = patch_list[i];
						/*for (int arg = 0; arg < currentArgCount; arg++) {
							i++;
						}*/
						patch.clear();
						u32 address = patch_list[i];
						if (currentHeader == TblPatchType::CmpPatch/* && currentWrapPatch < wrapPatchMax*/) {
							u8 byte_off = *reinterpret_cast<u8*>((patch_list[i + 2] + 1));
							auto address_if = byte_off + patch_list[i + 2];
							auto address_else = patch_list[i + 2] + 0x2;
							auto comparison = patch_list[i + 1];
							/*
0:  50                      push   eax
1:  b8 69 03 00 00          mov    eax,0x369
6:  39 01                   cmp    DWORD PTR [ecx],eax
8:  58                      pop    eax
9:  0f 87 fc ff ff ff       ja     b <_main+0xb>
f:  e9 fc ff ff ff          jmp    10 <_main+0x10>
							*/
							//unsafe!	
							//probably must be handled with class extender to delete the allocated memory on exit
							//u8* const byte_wrapper = new u8[20];
							// SET WRAPPER STUFF
							// MODIFY WRAPPER
							// INJECT WRAPPER
							// STORE WRAPPER DATA
							// FREE DATA IN TBLEXTENDER DESTRUCTOR


							//in whack:
							/*
								let executable = exe.exec_alloc(code.len());
								//
									pub fn exec_alloc(&mut self, size: usize) -> &'static mut [u8] {
										let mem = self.parent.state.exec_heap.allocate(size);
										unsafe { ::std::slice::from_raw_parts_mut(mem, size) }
									}
								//
								if nops>0 {
									exe.nop(nop_address, nops);
								}
								executable.copy_from_slice(&code);
								let diff = (executable.as_ptr() as u32).wrapping_sub(address as u32+5);
								exe.replace_val(address+1, diff);
								exe.replace_val(address, 0xe8 as u8);
							*/


							/*u8 byte_off = *reinterpret_cast<u8*>((patch_list[i + 2]+1));
							wrapJumpAddressIf[currentWrapPatch] = byte_off + patch_list[i + 2];
							wrapJumpAddressAfter[currentWrapPatch] = patch_list[i + 2] + 0x2;
							wrapComparison[currentWrapPatch] = patch_list[i + 1];
							if (vars[0] == Reg::_ECX && patch_list[i + 3] == Jmp::JA) {
								jmpPatch(ecx_callset,address);
							}
							if (vars[0] == Reg::_ECX && patch_list[i + 3] == Jmp::JBE) {
								jmpPatch(ecx_jbe_callset, address);
							}
/*							if (vars[0] == Reg::_EDX && patch_list[i + 3] == Jmp::JA) {
								jmpPatch(edx_callset, address);
							}
							if (vars[0] == Reg::_EDX && patch_list[i + 3] == Jmp::JBE) {
								jmpPatch(edx_jbe_callset, address);
							}
							if (vars[0] == Reg::_EAX) {
								jmpPatch(eax_callset, address);
							}
							if (vars[0] == Reg::_ESI) {
								jmpPatch(esi_callset, address);
							}
							currentWrapPatch++;*/
						}
						else if (currentHeader == TblPatchType::PtrWriting) {
							
							u16 string_id = patch_list[i + 1];
							u8 bl = patch_list[i + 1] & 0xff;
							u8 bu = (patch_list[i + 1] & 0xff00) >> 8;
							patch = { 0x8b,0x82,bl,bu,0x00,0x00,0x90 };
							if (vars[0] == Reg::_EAX) {
								if (vars[1] == Reg::_ECX) {
									patch[1] = 0x81;
								}
								else if (vars[1] == Reg::_EDX) {
									patch[1] = 0x82;
								}
								else if (vars[1] == Reg::_ESI) {
									patch[1] = 0x86;
								}
							}
							else if (vars[0] == Reg::_ECX) {
								if (vars[1] == Reg::_ESI) {
									patch[1] = 0x8e;
								}
								else if (vars[1] == Reg::_EDX) {
									patch[1] = 0x8a;
								}
							}
							else if (vars[0] == Reg::_EDI && vars[1] == Reg::_ECX) {
								patch[1] = 0xb9;
							}
							else if (vars[0] == Reg::_EBX && vars[1] == Reg::_ECX) {
								patch[1] = 0x99;
							}
							else if (vars[0] == Reg::_ESI && vars[1] == Reg::_EAX) {
								patch[1] = 0xb0;
							}
						}
						else if (currentHeader == TblPatchType::PtrMathWriting) {
							patch = { 0x8b, 0x44, 0x81, 0x04, 0x90 };
							if (vars[0] == Reg::_EAX && vars[1] == Reg::_ECX && vars[2] == Reg::_EAX) {
								//0x81
							}
							else if (vars[0] == Reg::_EAX && vars[1] == Reg::_ECX && vars[2] == Reg::_EDX) {
								patch[2] = 0x91;
							}
							else if (vars[0] == Reg::_EAX && vars[1] == Reg::_ESI && vars[2] == Reg::_EAX) {
								patch[2] = 0x86;
							}
							else if (vars[0] == Reg::_EAX && vars[1] == Reg::_EDX && vars[2] == Reg::_EAX) {
								patch[2] = 0x82;
							}
							else if (vars[0] == Reg::_EDX && vars[1] == Reg::_ECX && vars[2] == Reg::_EDX) {
								patch[1] = 0x54;
								patch[2] = 0x91;
							}
							else if (vars[0] == Reg::_ECX && vars[1] == Reg::_ESI && vars[2] == Reg::_EAX) {
								patch[1] = 0x4c;
								patch[2] = 0x86;
							}
							else if (vars[0] == Reg::_ESI && vars[1] == Reg::_ECX && vars[2] == Reg::_EDX) {
								patch[1] = 0x74;
								patch[2] = 0x91;
							}
							else if (vars[0] == Reg::_ESI && vars[1] == Reg::_EDX && vars[2] == Reg::_ECX) {
								patch[1] = 0x74;
								patch[2] = 0x8a;
								//
							}
							else if (vars[0] == Reg::_ESI && vars[1] == Reg::_ECX && vars[2] == Reg::_EAX) {
								patch[1] = 0x74;
								patch[2] = 0x81;
							}
						}
						else if (currentHeader == TblPatchType::RegisterPointerCmp) {
							patch = { 0x3b, 0x01, 0x90 };
							if (vars[0] == Reg::L_AX) {
								if (vars[1] == Reg::_ECX) {
									//0x01
								}
								else if (vars[1] == Reg::_ESI) {
									patch[1] = 0x06;
								}
								else if (vars[1] == Reg::_EDX) {
									patch[1] = 0x02;
								}
							}
							else if (vars[0] == Reg::L_CX) {
								if (vars[1] == Reg::_ECX) {
									patch[1] = 0x0a;
								}
								else if (vars[1] == Reg::_ESI) {
									patch[1] = 0x0e;
								}
							}
						}
						else if (currentHeader == TblPatchType::RegisterPointerMov) {
							patch = { 0x8b, 0x39, 0x90 };//DI, ECX
							if (vars[0] == Reg::L_DX && vars[1] == Reg::_ECX) {
								patch[1] = 0x11;
							}
						}
						else if (currentHeader == TblPatchType::RegisterCmp) {
							//require wrappers
							//reads cmp
						}
						else if (currentHeader == TblPatchType::RegisterWriting) {
							patch = { 0x89, 0xc2, 0x90 };
							if (vars[0] == Reg::_ECX) {
								patch[1] = 0xc1;
							}
						}
						else if (currentHeader == TblPatchType::NopPatch) {
							patch = { 0x90,0x90,0x90 };
						}
						else if (currentHeader == TblPatchType::AddPatch) {
							patch = { 0x83, 0xC0, 0xFF, 0x90, 0x90};
							if (vars[0] == Reg::_EAX) {

							}
							else if(vars[0]=Reg::_ECX) {
								patch[1]=0xc1;
							}
						}
						else if (currentHeader == TblPatchType::JmpPatch) {
							u8 jump = patch_list[i + 1];
							patch = { 0xeb };
							if (jump == Jmp::JBE) {
								patch = { 0x90,0x90 };
							}
						}
/*						else if (currentHeader == TblPatchType::DecPatch) {
							if (vars[0] == Reg::_EAX) {
								patch = { 0x48,0x90,0x90,0x90,0x90 };
							}
						}*/


						
						memoryPatchVec(address, patch);
							//don't patch if wrapper offset
						
						i += currentArgCount;
					}
				}
				i--;
				read_status = ConvReadStatus::ReadHeader;
			}
		}
	}
	void injectTblExtender() {
		Globals.tbl_extender = true;
		auto ptr_list = offset_list();
		//DATA MUST BE INITIALIZED
		/*for (int i = 0; i < ptr_list.size(); i++) {
			memoryPatch(ptr_list[i], (u32)&extendedTbl);
		}*/
		patchTblList();
		memoryPatch(0x004C3A6A + 1, &tblNew);
		memoryPatch(0x004C3A2E + 1, &tblNew);
		memoryPatch(0x004C3AF4 + 1, &tblNew);

		/*

		//NOT EVERYTHING IS RELATED TO TBLS, ONLY STAT_TXT STUFF MUST BE SORTED OUT
		//loadTbl must be hooked

	////Line 31543: 00415CED  |.  81C1 FFFF0000 |ADD ECX,0FFFF

	Line 56377: 00426063  |.  05 FFFF0000   ADD EAX,0FFFF
	Line 56452: 00426134  |.  05 FFFF0000   ADD EAX,0FFFF
	Line 57285: 00426BA1  |.  05 FFFF0000   ADD EAX,0FFFF
	Line 57765: 00427165  |.  05 FFFF0000   ADD EAX,0FFFF
	Line 57936: 0042737E   .  05 FFFF0000   ADD EAX,0FFFF
	Line 58142: 00427616  |.  05 FFFF0000   ADD EAX,0FFFF
	Line 58170: 0042766D  |.  05 FFFF0000   ADD EAX,0FFFF
	Line 58203: 004276E0  |.  81C1 FFFF0000 ADD ECX,0FFFF
	Line 58254: 0042778C  |.  05 FFFF0000   ADD EAX,0FFFF
	Line 130989: 00457B9D  |.  05 FFFF0000   ADD EAX,0FFFF
	Line 150062: 00464EA1  |.  05 FFFF0000   ADD EAX,0FFFF
	Line 211113: 0048D94F  |.  05 FFFF0000   ADD EAX,0FFFF
	Line 220930: 00494563  |.  05 FFFF0000   ADD EAX,0FFFF
		*/
	}
}

//tblExtender extendedTbl;