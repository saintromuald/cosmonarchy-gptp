#include "MPQDraftPlugin.h"
#include "BW/api.h"
#include <windows.h>
#include <string.h>
#include <stdio.h>

//#define THEPLUGINID 0x62447349 // "IsDb"
//const char* plugin_name = "Iscript Debugger";
asm(".intel_syntax noprefix\n");

#include <pshpack1.h>
typedef struct {
	u8 x;
	u8 y;
	u8 w;
	u8 h;
	u32 offset;
} isdbg_grp_frame;

typedef struct {
	u16 frameCount;
	u16 w;
	u16 h;
	isdbg_grp_frame frame[1];
} isdbg_grp_head;


typedef struct {
	u8 magic[4];
	u16 type;
	u16 unk;
	u16 anims[1];
} isdbg_is_header;

typedef struct{
	/*0x00*/ u32				prev;
	/*0x04*/ u32				next;
	/*0x08*/ u16				imageID;			// officially "uwNo"
	/*0x0A*/ u8					paletteType;		// officially "ubRLE"
	/*0x0B*/ u8					direction;
	/*0x0C*/ u16				flags;
	/*0x0E*/ s8					horizontalOffset;
	/*0x0F*/ s8					verticalOffset;
	/*0x10*/ u16				iscriptHeader;
	/*0x12*/ u16				iscriptOffset;
	/*0x14*/ u16				unknown_14;		// iscriptReturn
	/*0x16*/ u8					anim;
	/*0x17*/ u8					sleep;			// iscript sleep time
	/*0x18*/ u16				frameSet;
	/*0x1A*/ u16				frameIndex;
	/*0x1C*/ s16				mapPosition[2];
	/*0x20*/ s16				screenPosition[2];
	/*0x24*/ s16				grpBounds[4];      // Bounds for GRP frame, only different from normal when part of graphic is out of bounds.
	/*0x2C*/ isdbg_grp_head*	GRPFile;
	/*0x30*/ u32				coloringData;
	/*0x34*/ u32				renderFunction;    // officially "DrawFunction"
	/*0x38*/ u32				updateFunction;    // officially "UpdateFunction"
	/*0x3C*/ u32				spriteOwner;
} isdbg_image;

#include <poppack.h>
const char* animStrs[] = {"Init","Death","GndAttkInit","AirAttkInit","Unused1","GndAttkRpt","AirAttkRpt","CastSpell","GndAttkToIdle","AirAttkToIdle","Unused2","Walking","WalkingToIdle","SpecialState1","SpecialState2","AlmostBuilt","Built","Landing","LiftOff","IsWorking","WorkingToIdle","WarpIn","Unused3","InitTurret","Disable","Burrow","UnBurrow","Enable"};
FILE * logf = NULL;
u8* iscript = NULL;
u32 idleoffset = 0;

u32 getIdleOffset(u8* isbuf);

void __fastcall getAnim();
//void __fastcall getFrame();
void __fastcall fixFrame();

void callPrintText(const char* text, u32 color);
typedef void (__stdcall *printTextFunc)();
printTextFunc printText;

#define OP_JMP     0xE9
#define OP_CALL    0xE8
#define OP_NOP     0x90

char nopTable[][14] = {{0},
{0x90,0}, // nop
{0x66,0x90,0}, // xchg %ax,%ax
{0x8d,0x76,0x00,0}, // leal 0(%esi),%esi
{0x8d,0x74,0x26,0x00,0}, // leal 0(%esi,1),%esi
{0x90,0x8d,0x74,0x26,0x00,0}, // nop; leal 0(%esi,1),%esi
{0x8d,0xb6,0x00,0x00,0x00,0x00,0}, // leal 0L(%esi),%esi
{0x8d,0xb4,0x26,0x00,0x00,0x00,0x00,0}, // leal 0L(%esi,1),%esi
{0x90,0x8d,0xb4,0x26,0x00,0x00,0x00,0x00,0}, // nop; leal 0L(%esi,1),%esi
{0x89,0xf6,0x8d,0xbc,0x27,0x00,0x00,0x00,0x00,0}, // movl %esi,%esi; leal 0L(%edi,1),%edi
{0x8d,0x76,0x00,0x8d,0xbc,0x27,0x00,0x00,0x00,0x00,0}, // leal 0(%esi),%esi; leal 0L(%edi,1),%edi
{0x8d,0x74,0x26,0x00,0x8d,0xbc,0x27,0x00,0x00,0x00,0x00,0}, // leal 0(%esi,1),%esi; leal 0L(%edi,1),%edi
{0x8d,0xb6,0x00,0x00,0x00,0x00,0x8d,0xbf,0x00,0x00,0x00,0x00,0}, // leal 0L(%esi),%esi; leal 0L(%edi),%edi
{0x8d,0xb6,0x00,0x00,0x00,0x00,0x8d,0xbc,0x27,0x00,0x00,0x00,0x00,0}, // leal 0L(%esi),%esi; leal 0L(%edi,1),%edi
{0x8d,0xb4,0x26,0x00,0x00,0x00,0x00,0x8d,0xbc,0x27,0x00,0x00,0x00,0x00}}; // leal 0L(%esi,1),%esi; leal 0L(%edi,1),%edi

typedef struct{
 u32 address;
 u32 length;
 u8  bytes[8];
} isdbg_data_replace;

typedef struct{
 u32 address;
 u8  opcode;
 void (*__fastcall func)();
 u8  nopBytes;
} isdbg_func_calls;


// no longer needed, but it might be for other versions so I'll keep it
isdbg_data_replace data_1161[] = {//{0x00497D32, 4, {0x8D,0x55,0xF0, 0x52, 0}}, // LEA EDX,DWORD PTR SS:[EBP-10] \ PUSH EDX
						   0};


/*
isdbg_func_calls   func_113e[] = {{0x004D753D, OP_CALL, &getAnim, 0},
						   {0x004D4816, OP_CALL, &fixFrame, 2},
						   0};

isdbg_func_calls   func_1140[] = {{0x004D753D, OP_CALL, &getAnim, 0},
						   {0x004D4816, OP_CALL, &fixFrame, 2},
						   0};
						   
isdbg_func_calls   func_1150[] = {{0x004D80DD, OP_CALL, &getAnim, 0},
						   {0x004D53B6, OP_CALL, &fixFrame, 2},
						   0};

isdbg_func_calls   func_1151[] = {{0x004D80FD, OP_CALL, &getAnim, 0},
						   {0x004D53D6, OP_CALL, &fixFrame, 2},
						   0};

isdbg_func_calls   func_1152[] = {{0x004D810D, OP_CALL, &getAnim, 0},
						   {0x004D53E6, OP_CALL, &fixFrame, 2},
						   0};

isdbg_func_calls   func_1153[] = {{0x004D816D, OP_CALL, &getAnim, 0},
						   {0x004D5436, OP_CALL, &fixFrame, 2},
						   0};

isdbg_func_calls   func_1160[] = {{0x004D83BD, OP_CALL, &getAnim, 0},
						   {0x004D5696, OP_CALL, &fixFrame, 2},
						   0};

isdbg_func_calls   func_1161[] = {{0x004D84DD, OP_CALL, &getAnim, 0},
						   //{0x00497D36, OP_CALL, &getFrame, 3},
						   {0x004D57B6, OP_CALL, &fixFrame, 2},
						   0};

struct{
 u32 verHigh,verLow;
 isdbg_data_replace* data;
 isdbg_func_calls* calls;
 u32 printTextPtr;
} patchData[] = {//{0x00010001,0x00030009, data_1161, func_113e, 0x0048C120},
				 {0x0001000E,0x00000001, data_1161, func_1140, 0x0048C120},
				 {0x0001000F,0x00000001, data_1161, func_1150, 0x0048C9E0},
				 {0x0001000F,0x00010000, data_1161, func_1151, 0x0048C9D0},
				 {0x0001000F,0x00020001, data_1161, func_1152, 0x0048C9D0},
				 {0x0001000F,0x00030001, data_1161, func_1153, 0x0048CA00},
				 {0x00010010,0x00000001, data_1161, func_1160, 0x0048CC20},
				 {0x00010010,0x00010001, data_1161, func_1161, 0x0048CD30},
				 0};

u32 getIdleOffset(u8* isbuf) {
	if (iscript == isbuf)
	{
		return idleoffset;
	}
	
	iscript = isbuf;
	idleoffset = 0;	
	u32 nullfind;
	s32 null = -1;
	u32 max = *(u16*)(isbuf);
	nullfind = 0;
	for (u32 i = 0; i < max; i++) {	 
	 // find a place to add the idle loop
		if (null == -1) {
			if(isbuf[i] == 0) {
				nullfind++;
				if (nullfind >= 5 && ((i - 5) & 3) == 0) { // dword-align for isextend
					null = i-5;
				}
			}
			else {
				if(*(u32*)(isbuf + i) == 0x45504353) { // "SCPE" iscript header identifier -- not safe to write in random blocks now
					null = -2;
				}
				nullfind = 0;
			}
		}
	 
		// Found an idle loop?
		if (isbuf[i  ] == 0x05 && // WAIT
		    isbuf[i+2] == 0x07 && // GOTO
		    *(u16*)(isbuf + i + 3) == i)
		{
			idleoffset = i;
			break;
		}
	}
	
	// Create one at the previously found offset
	if(idleoffset == 0) {
		if(null >= 0) {
			isbuf[null] = 0x05; // WAIT
			isbuf[null +1] = 125;
			isbuf[null +2] = 0x07; // GOTO
			*(u16*)(isbuf + null + 3) = null;
			idleoffset = null;
		}
		else {
			fputs("Error: Could not create iscript idle loop. :(\n", logf);fflush(logf);
			callPrintText("Error: Could not create iscript idle loop. :(", 6);
		}
	}
	return idleoffset;
}

void __fastcall getAnim() {
	u32 animid;
	isdbg_image* img;
	u8* isbuf;
	isdbg_is_header* ishdr;
	u32 offset;
	u32 ebx,edx;
	char msg[128];
	__asm __volatile("" : "=a" (animid), "=b" (ebx), "=c" (img), "=d" (edx), "=D" (isbuf), "=S" (ishdr));

	if(animid >= ((ishdr->type & ~1) + 2) || ishdr->anims[animid] == 0) {
		if(animid < 28) {
			sprintf(msg, "Call to undefined animation \"%s\" on Image ID %d", animStrs[animid], img->imageID);
		}
		else {
			sprintf(msg, "Call to unknown animation #%d on Image ID %d", animid, img->imageID);
		}
		fputs(msg,logf);fputc('\n',logf);fflush(logf);
		callPrintText(msg, 6);
		offset = getIdleOffset(isbuf);
	}
	else {
		offset = ishdr->anims[animid];
	}
	
	__asm __volatile("" : : "a" (offset), "b" (ebx), "c" (img), "d" (edx), "D" (isbuf), "S" (ishdr));
}

void __fastcall fixFrame(){
	isdbg_image* img;
	isdbg_grp_head* grp;
	u32 frame;
	u32 sprite;
	u32 esi,edi;
	char msg[128];
	__asm __volatile("" : "=a" (img), "=S" (esi), "=D" (edi));

	grp = img->GRPFile;
	if (img->frameIndex >= grp->frameCount) {
		if(img->anim < 28) {
			sprintf(msg, "Invalid frame %d in animation \"%s\" on Image ID %d", img->frameIndex, animStrs[img->anim], img->imageID);fflush(logf);
		}
		else {
			sprintf(msg, "Invalid frame %d in animation #%d on Image ID %d", img->frameIndex, img->anim, img->imageID);fflush(logf);
		}
		fputs(msg,logf);fputc('\n',logf);fflush(logf);
		callPrintText(msg, 6);
		img->frameIndex = 0;
		img->frameSet = 0;
	}
	frame = img->frameIndex;
	sprite = img->spriteOwner;
 
 __asm __volatile("" : : "a" (img), "c" (frame), "d" (sprite), "S" (esi), "D" (edi));
}

void callPrintText(const char* text, u32 color){
	DWORD gtc = GetTickCount() + 7000;
	if (!text) return;
	__asm __volatile("push 0\n"
				"\tpush %1\n"
				"\tpush %2\n"
				"\tcall [DWORD PTR _printText]" : : "a" (text), "r" (gtc), "r" (color));
}
*/