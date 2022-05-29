#include "hook_tools.h"

//V241 for VS2008

//Injects a relative CALL to [target] at the [position].
//Original function from BWAPI by Kovarex; Modified by pastelmind
void call_patch(const void* target, void* position, const u32 nops)
{
	u8* const data = new u8[5 + nops];
	data[0] = 0xE8; //Relative CALL instruction
	const u32 address = (u32)target - (u32)position - 5;  //Relative address
	*(u32*)(&data[1]) = address;
	for (u32 i = 0; i < nops; ++i)
		data[5 + i] = 0x90; //NOP instruction
	memoryPatch(position, data, 5 + nops);
	delete[] data;
}

void nops(const u32 address, u32 number)
{
	u8 nop = 0x90;
	for (s32 i = 0; i < number; i++)
	{
		memoryPatch((void*)(address + i), &nop, sizeof(u8));
	}
}

//Injects a relative JMP to [target] at the [position].
//Original function from BWAPI by Kovarex; Modified by pastelmind
void jump_patch(const void* target, void* position, u32 nops)
{
	u8* const data = new u8[5 + nops];
	data[0] = 0xE9; //Relative JMP instruction
	const s32 address = (s32)target - (s32)position - 5;  //Relative address
	*(s32*)(&data[1]) = address;
	for (u32 i = 0; i < nops; ++i)
		data[5 + i] = 0x90; //NOP instruction
	memoryPatch(position, data, 5 + nops);
	delete[] data;
}

void wrapperPatch(const u8* code, u32 len, const u32 address, u32 nop)
{
	//function address
	auto exe = wrapperMemory.exec_alloc(len);
	if (nop > 0)
	{
		nops(address + 0x5, nop);
	}
	for (s32 i = 0; i < len; i++)
	{
		exe[i] = code[i];
	}
	u32 diff = (u32)exe - (address + 0x5);
	memoryPatch(address + 0x1, diff);
	memoryPatch(address, (u8)0xe8);
}

//Inject an array of bytes, using the given length.
void memoryPatch(void* const address, const u8* data, const size_t size)
{
	DWORD oldProt = 0;
	VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProt);
	memcpy(address, data, size);
	VirtualProtect(address, size, oldProt, &oldProt);
}

ByteWrapperMemory wrapperMemory;
u8* ByteWrapperMemory::exec_alloc(u32 len)
{
	std::shared_ptr<u8[]> wrapper(new u8[len]);
	wrapper_memory.push_back(wrapper);
	auto offset = &wrapper[0];
	return offset;
}