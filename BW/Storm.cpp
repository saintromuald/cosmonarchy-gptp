#include "Storm.h"
#include <Windows.h>

const u32 Func_FastFileRead = 0x004D2D10;
void* FastFileRead(const char* filename, void* buffer, bool returnIfNotFound, u32 scope, u32* filesize)
{
	char dbg_filename[] = "Storm.cpp"; // __FILE__
	static u32 dbg_filename_int = (u32)(dbg_filename);
	static u32 returnIfNotFound_int;
	returnIfNotFound_int = returnIfNotFound;
	static void* result;

	__asm
	{
		PUSHAD
	
		PUSH __LINE__
		PUSH dbg_filename_int
		PUSH returnIfNotFound_int
		PUSH buffer
		PUSH filename
		MOV ECX, scope
		MOV EAX, filesize
		CALL Func_FastFileRead

		MOV result, EAX

		POPAD
	}
	return result;
}